#include <stdio.h>
#include <stdlib.h>
#include "Point.h"
#include "Cluster.h"
#include "InputAndOutput.h"
#define INVALID_OUTPUT_K 0

// Read input file
void readInputFile(Input **input, Point **points, char *fileName) {
	double x, y, z, vx, vy, vz;
	FILE *fileInput;
	int i, numOfPoints;
	fopen_s(&fileInput, fileName, "r");
	if (fileInput != NULL) {
		*input = (Input*)malloc(sizeof(Input));
		if (input != NULL) {
			fscanf_s(fileInput, "%d %d %lf %lf %d %lf",
				&((*input)->N), &((*input)->K), &((*input)->T),
				&((*input)->dT), &((*input)->LIMIT), &((*input)->QM));

			numOfPoints = (*input)->N;
			(*points) = (Point*)calloc(numOfPoints, sizeof(Point));
			if ((*points) != NULL) {
				for (i = 0; i < numOfPoints; i++) {
					fscanf_s(fileInput, "%lf %lf %lf %lf %lf %lf", &x, &y, &z, &vx, &vy, &vz);
					(*points)[i].position.x = x;
					(*points)[i].position.y = y;
					(*points)[i].position.z = z;
					(*points)[i].velocity.vx = vx;
					(*points)[i].velocity.vy = vy;
					(*points)[i].velocity.vz = vz;
				}
			}
			fclose(fileInput);
		}
	}
}


// Write input file
void writeOutputFile(Output *output, Cluster* clusters, char* fileName) {
	FILE *file;
	int i;

	fopen_s(&file, fileName, "w+");
	if (file != NULL) {
		fprintf_s(file, "First occurrence at t = %lf with q = %lf\n", (output->t), (output->q));

		fprintf_s(file, "Centers of the clusters:\n");
		for (i = 0; i < output->K; i++)
			fprintf_s(file, "%lf %lf %lf\n", clusters[i].center.x, clusters[i].center.y, clusters[i].center.z);
		fclose(file);
	}
	else {
		printf("\n ERROR in Output File !!!! \n");
		fflush(stdout);
		exit(1);
	}
}


//MPI_Type for Position 
void createPositionType(MPI_Datatype *Position_MPI) {
	Position point;
	MPI_Datatype type[3] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };
	int blocklen[3] = { 1, 1, 1 };
	MPI_Aint d[3];
	d[0] = (char *)&point.x - (char *)&point;
	d[1] = (char *)&point.y - (char *)&point;
	d[2] = (char *)&point.z - (char *)&point;
	MPI_Type_create_struct(3, blocklen, d, type, Position_MPI);
	MPI_Type_commit(Position_MPI);
}


//MPI_Type for Cluster 
void createClusterType(MPI_Datatype *Cluster_MPI, MPI_Datatype *Position_MPI) {
	Cluster cluster;
	MPI_Datatype type[4] = { MPI_INT, *Position_MPI, MPI_DOUBLE, MPI_INT };
	int blocklen[4] = { 1, 1, 1, 1 };
	MPI_Aint d[4];
	d[0] = (char *)&cluster.numOfPoints - (char *)&cluster;
	d[1] = (char *)&cluster.center - (char *)&cluster;
	d[2] = (char *)&cluster.diameter - (char *)&cluster;
	d[3] = (char *)&cluster.id - (char *)&cluster;
	MPI_Type_create_struct(4, blocklen, d, type, Cluster_MPI);
	MPI_Type_commit(Cluster_MPI);
}


// MPI_Type for Input 
void createInputParamsType(MPI_Datatype *Input_MPI) {
	Input input;
	MPI_Datatype type[6] = { MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };
	int blocklen[6] = { 1, 1, 1, 1, 1, 1 };
	MPI_Aint d[6];
	d[0] = (char *)&input.N - (char *)&input;
	d[1] = (char *)&input.K - (char *)&input;
	d[2] = (char *)&input.LIMIT - (char *)&input;
	d[3] = (char *)&input.QM - (char *)&input;
	d[4] = (char *)&input.T - (char *)&input;
	d[5] = (char *)&input.dT - (char *)&input;
	MPI_Type_create_struct(6, blocklen, d, type, Input_MPI);
	MPI_Type_commit(Input_MPI);
}


// MPI_Type for Velocity 
void createVelocityType(MPI_Datatype *Velocity_MPI) {
	Velocity velocity;
	MPI_Datatype type[3] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };
	int blocklen[3] = { 1, 1, 1 };
	MPI_Aint d[3];
	d[0] = (char *)&velocity.vx - (char *)&velocity;
	d[1] = (char *)&velocity.vy - (char *)&velocity;
	d[2] = (char *)&velocity.vz - (char *)&velocity;
	MPI_Type_create_struct(3, blocklen, d, type, Velocity_MPI);
	MPI_Type_commit(Velocity_MPI);
}


// MPI_Type for Point
void createPointType(MPI_Datatype *Point_MPI, MPI_Datatype *Velocity_MPI, MPI_Datatype *Position_MPI) {
	Point point;
	MPI_Datatype type[3] = { *Position_MPI, MPI_INT, *Velocity_MPI };
	int blocklen[3] = { 1, 1, 1 };
	MPI_Aint d[3];
	d[0] = (char *)&(point.position) - (char *)&point;
	d[1] = (char *)&(point.cluster) - (char *)&point;
	d[2] = (char *)&(point.velocity) - (char *)&point;
	MPI_Type_create_struct(3, blocklen, d, type, Point_MPI);
	MPI_Type_commit(Point_MPI);
}


// MPI_Type for Output
void createOutputType(MPI_Datatype *Output_MPI) {
	Output output;
	MPI_Datatype type[3] = { MPI_INT, MPI_DOUBLE, MPI_DOUBLE };
	int blocklen[3] = { 1, 1, 1 };
	MPI_Aint d[3];
	d[0] = (char *)&(output.K) - (char *)&output;
	d[1] = (char *)&(output.t) - (char *)&output;
	d[2] = (char *)&(output.q) - (char *)&output;
	MPI_Type_create_struct(3, blocklen, d, type, Output_MPI);
	MPI_Type_commit(Output_MPI);
}


void createMpiTypes(Types_MPI *Types_MPI) {
	createInputParamsType(&(Types_MPI->Input_MPI));
	createPositionType(&Types_MPI->Position_MPI);
	createVelocityType(&(Types_MPI->Velocity_MPI));
	createClusterType(&(Types_MPI->Cluster_MPI), &(Types_MPI->Position_MPI));
	createPointType(&(Types_MPI->Point_MPI), &(Types_MPI->Velocity_MPI), &(Types_MPI->Position_MPI));
	createOutputType(&(Types_MPI->Output_MPI));
}


//Print results
void print(Input* input, Point* points) {
	printf("N = %d K = %d T =  %lf \ndT = %lf LIMIT = %d QM = %lf\n",
		(input->N), (input->K), (input->T),
		(input->dT), (input->LIMIT), (input->QM));

	printf("\nPoints:\n");
	for (int i = 0; i < input->N; i++) {
		printf("%lf %lf %lf\t%lf %lf %lf\n",
			points[i].position.x, points[i].position.y, points[i].position.z,
			points[i].velocity.vx, points[i].velocity.vy, points[i].velocity.vz);
	}
	fflush(stdout);
}


//Print results
void print(Cluster* clusters, int numOfClusters) {
	printf("\nCenters of the clusters:\n");
	for (int i = 0; i < numOfClusters; i++) {
		printf("Id Of Cluster: %d\t(%lf, %lf , %lf)\nNum of points = %d  Diam = %lf\n\n",
			clusters[i].id, clusters[i].center.x, clusters[i].center.y, clusters[i].center.z,
			clusters[i].numOfPoints, clusters[i].diameter);
	}
	fflush(stdout);
}


//Check if the output struct has valid
Boolean isOutputValid(Output *output) {
	Boolean isOutputValid = (Boolean)FALSE;
	if (output->K != INVALID_OUTPUT_K) {
		isOutputValid = (Boolean)TRUE;
	}
	return isOutputValid;
}
