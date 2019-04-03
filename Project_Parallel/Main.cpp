#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include "Kmeans_Omp.h"
#include "Kmeans_Cuda.h"
#include "InputAndOutput.h"
#include "Master.h"

//Calculate a center of clusters (OpenMP)
void clustersCenters(Input* input, Point* all_points, Cluster* all_clusters) {
	clustersCenters_Omp(all_clusters, input->K, all_points, input->N);
}


// Assigning all points to clusters, return true if at least one point has changed cluster (OpenMP)
Boolean assignClustersToPoints(Input* input, Point* all_points, Cluster* all_clusters) {
	Boolean pointChangedOmp = FALSE;
	pointChangedOmp = assignClustersToPoints_Omp(all_clusters, input->K, all_points, input->N);
	return pointChangedOmp;
}


//Calculate a diameter of clusters (OpenMP)
void clustersDiameter(Input* input, Point* all_points, Cluster* all_clusters) {
	clustersDiameter_Omp(all_clusters, input->K, all_points, input->N);
}


//Calculate a quality of clusters
double clusters_quality(Input* input, Point* points, Cluster* clusters) {
	double q = 0;
	int i, j;
	clustersDiameter(input, points, clusters);
	for (i = 0; i < input->K; i++)
		for (j = 0; j < input->K; j++)
			if (j != i)
				q += clusters[i].diameter / distance_between_two_points(&(clusters[i].center), &(clusters[j].center));

	q /= (input->K - 1) * (input->K);
	return q;
}


// Increases time for each point
void updateTime(Point* all_points, Point** gpu_points, int numOfPoints, double dt, int moment) {
	int cudaNumOfPoints = numOfPoints * CUDA_INC_TIME_PORTION, ompNumOfPoints = numOfPoints - cudaNumOfPoints;
	const char* cudaError = NULL;
	cudaError = updateTimeCudaStart(all_points, cudaNumOfPoints, dt, moment, gpu_points);
	if (cudaError != NULL) {
		printf(cudaError);
		exit(1);
	}
	
	increaseTime_Omp(all_points + cudaNumOfPoints, ompNumOfPoints, dt, moment);
	
	cudaError = updateTimeCudaEnd(*gpu_points, all_points, cudaNumOfPoints);
	if (cudaError != NULL) {
		printf(cudaError);
		exit(1);
	}
}


void free_all_recourses(Input* input, Point* all_points, Cluster* all_clusters, Output* outputs, Point* pointsCuda) {
	free(all_points);
	free(input);
	free(all_clusters);
	free(outputs);
	freeCuda(pointsCuda);
}


//Calculated the quality of the clusters
double kmeans_algorithm(Input* input, Point* points, Cluster** clusters) {
	Boolean changed_point = TRUE;
	int iteration;
	initClusters(points, clusters, input);

	for (iteration = 0; iteration < input->LIMIT && changed_point; iteration++) {
		changed_point = assignClustersToPoints(input, points, *clusters);
		clustersCenters(input, points, *clusters);
	}
	return clusters_quality(input, points, *clusters);
}


//Broadcast input to all Salves
void broadcastInput_ToSalves(Types_MPI *types, Input* input, Point** points, int myId) {
	MPI_Bcast(input, 1, types->Input_MPI, MASTER_RANK, MPI_COMM_WORLD);
	if (!isMasterRank(myId)) {
		(*points) = (Point*)calloc((input)->N, sizeof(Point));
		if (*points == NULL) {
			printf("\nAlloc failed points inside broadcastInput\n");
			fflush(stdout);
			exit(1);
		}
	}
	MPI_Bcast((*points), input->N, types->Point_MPI, MASTER_RANK, MPI_COMM_WORLD);
}


//Index of finished proccess
void getFinishedProcessIndex(int myId, int finishedProccess, Boolean *isFinished, Cluster** clusters, int clustersCount, Types_MPI *types) {
	MPI_Status status;
	if (finishedProccess != NO_SLAVE_FINISHED) {
		//If the result comes from one of slaves, master needs to receive it
		if (isMasterRank(myId)) {
			if (!isMasterRank(finishedProccess)) {
				MPI_Recv(*clusters, clustersCount, types->Cluster_MPI, finishedProccess, 0, MPI_COMM_WORLD, &status);
			}
		}
		else {
			//If master wants our result
			if (finishedProccess == myId) {
				MPI_Send(*clusters, clustersCount, types->Cluster_MPI, MASTER_RANK, 0, MPI_COMM_WORLD);
			}
		}
		*isFinished = TRUE;
	}
}


//Handling proccesses left wihtout a time
void handle_LeftProccesses(int myId, int numOfProcesses, Boolean isFinished, Input* input, Output *result, int finishedProccess, Types_MPI *types) {
	int lastProcessesLeft = (int)(input->T / input->dT) % numOfProcesses;
	
	if (!isFinished && myId >= lastProcessesLeft) {
		//Waiting for all others to finish:
		fflush(stdout);
		MPI_Gather(result, 1, types->Output_MPI, NULL, 1, types->Output_MPI, MASTER_RANK, MPI_COMM_WORLD);
		MPI_Bcast(&finishedProccess, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);
	}
}


//Checking if the current result  q < QM
Boolean isFound_result(double q, Output *result, Input* input, double currTime) {
	Boolean isFinished = FALSE;
	if (q < input->QM) {
		isFinished = TRUE;
		(*result).t = currTime;
		(*result).K = input->K;
		(*result).q = q;
	}
	return isFinished;
}


//Checking if someone has finished, Returning the id of the finished proccess
int exchange_proccessesResults(Boolean *isFinished, Output *result, Cluster** clusters, Output *slaveOutputs,
		Input* input, int myId, int numOfProcesses, Types_MPI types) {
	int finishedProccess;
	MPI_Gather(result, 1, types.Output_MPI, slaveOutputs, 1, types.Output_MPI, MASTER_RANK, MPI_COMM_WORLD);

	if (isMasterRank(myId))
		finishedProccess = masterCheckResults(isFinished, input, result, numOfProcesses, slaveOutputs);

	//Master sends to all the index of the finished process
	MPI_Bcast(&finishedProccess, 1, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

	//Checking if we are finished and if we need to send the clusters
	getFinishedProcessIndex(myId, finishedProccess, isFinished, clusters, input->K, &types);
	return finishedProccess;
}


//Master print results
void print_results(int myId, Output* result, Cluster* all_clusters, Input* input, Point* points, Point* gpu_points, Output* slaveOutputs, double q, double currTime) {
	if (isMasterRank(myId)) {
		//If no one reached the q
		if (!(isOutputValid(result))) {
			(*result).t = currTime;
			(*result).K = input->K;
			(*result).q = q;
		}
		masterPrintResults(all_clusters, result);
	}
	free_all_recourses(input, points, all_clusters, slaveOutputs, gpu_points);
	stopCuda();
}


//K-means parallel
void kmeans_parallel(int myId, int numOfProcesses) {
	Point* points = NULL, *gpu_points = NULL;;
	Cluster* clusters = NULL;
	Output result = { 0, 0, 0 }, *slaveOutputs = NULL;
	int n, finishedProccess;
	Boolean isFinished = FALSE;
	double q, currTime = -1;
	Types_MPI types;

	initCuda();

	Input* input = (Input*)malloc(sizeof(Input));
	if (input == NULL) {
		printf("\nError in allocation input, parallel k-means");
		exit(1);
	}
	if (isMasterRank(myId)) {
		slaveOutputs = (Output*)calloc(numOfProcesses, sizeof(Output));
		if (slaveOutputs == NULL) {
			printf("\nError in allocation slaveOutputs, parallel k-means");
			exit(1);
		}
		readInputFile(&input, &points, INPUT_FILE);
		if (points == NULL) {
			printf("\nError in file master\n");
			fflush(stdout);
			exit(1);
		}
	}
	createMpiTypes(&types);
	broadcastInput_ToSalves(&types, input, &points, myId);
	updateTime(points, &gpu_points, input->N, input->dT, myId);
	
	for (n = myId; n < (input->T / input->dT) && !isFinished; n += numOfProcesses) {
		q = kmeans_algorithm(input, points, &clusters);
		currTime = n * input->dT;
		isFinished = isFound_result(q, &result, input, currTime);
		finishedProccess = exchange_proccessesResults(&isFinished, &result, &clusters, slaveOutputs, input,
			myId, numOfProcesses, types);

		if (!isFinished)
			updateTime(points, &gpu_points, input->N, input->dT, numOfProcesses);
	}
	handle_LeftProccesses(myId, numOfProcesses, isFinished, input, &result, finishedProccess, &types);
	print_results(myId, &result, clusters, input, points, gpu_points, slaveOutputs, q, currTime);
}



int main(int argc, char *argv[]) {
	int numOfProcesses, myId; 
	clock_t start, end;
	double cpu_time_used;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);
	MPI_Comm_size(MPI_COMM_WORLD, &numOfProcesses);

	printf("\n id = %d Started, Number of procces = %d", myId, numOfProcesses);
	fflush(stdout);

	start = clock();
	kmeans_parallel(myId, numOfProcesses);
	end = clock();
	
	cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("\nProcess id = %d finished in %lf\n", myId, cpu_time_used);
	fflush(stdout);
	MPI_Finalize();
	return 0;
}