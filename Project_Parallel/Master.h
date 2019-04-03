#pragma once
#include "InputAndOutput.h"

#define NO_SLAVE_FINISHED -1
#define MASTER_RANK 0

//Checking the output array with results from slaves, Return the index of the first finished slave
int masterCheckResults(Boolean *is_finished, Input* input, Output *result, int numOfProcesses,
	Output *slaveOutputs);


//Check if is a master
Boolean isMasterRank(int rank);


//Master writing the result
void masterPrintResults(Cluster* all_clusters, Output *result);