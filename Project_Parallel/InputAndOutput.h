#pragma once

#include "Point.h"
#include <mpi.h>

typedef enum Boolean { FALSE, TRUE } Boolean;
#define INPUT_FILE "input.txt"
#define OUTPUT_FILE "Output.txt"


struct Types_MPI {
	MPI_Datatype Point_MPI;
	MPI_Datatype Position_MPI;
	MPI_Datatype Velocity_MPI;
	MPI_Datatype Cluster_MPI;
	MPI_Datatype Input_MPI;
	MPI_Datatype Output_MPI;
};


struct Input {
	int N;       // Number of points
	int K;       // Number of clusters to find
	double dT;   // defines moments t = n*dT, n = { 0, 1, 2, … , T/dT} for which calculate the clusters and the quality
	int LIMIT;	 // The maximum number of iterations for K - MEAN algorithm
	double QM;   // Quality measure to stop
	double T;	 // Defines the end of time interval[0, T]
};


struct Output {
	int K;       // Number of clusters
	double t;    // Defines the time the algo has stopped at
	double q;	 // The quality of the clusters
};


// Read input file
void readInputFile(Input **input, Point **points, char *fileName);


// Write output file
void writeOutputFile(Output *output, Cluster* clusters, char* fileName);


// Create MPI_Types
void createPointType(MPI_Datatype *Point_MPI, MPI_Datatype *Velocity_MPI, MPI_Datatype *Position_MPI);
void createPositionType(MPI_Datatype *Position_MPI);
void createVelocityType(MPI_Datatype *Velocity_MPI);
void createClusterType(MPI_Datatype *Cluster_MPI, MPI_Datatype *Position_MPI);
void createInputParamsType(MPI_Datatype *Input_MPI);
void createOutputType(MPI_Datatype *Output_MPI);
void createMpiTypes(Types_MPI *Types_MPI);


//print results
void print(Input* input, Point* points);
void print(Cluster* clusters, int numOfClusters);


// check if the output struct has valid
Boolean isOutputValid(Output *output);