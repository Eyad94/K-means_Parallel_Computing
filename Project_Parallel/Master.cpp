#include "Master.h"

//Get the index of the first result that is not null or -1
int getIndexOfFirstFinished(int numOfProcesses, Output* slaveClusters) {
	int finishedSlaveIndex = NO_SLAVE_FINISHED;
	int current_Process;
	for (current_Process = 0; current_Process < numOfProcesses; current_Process++) {
		if (isOutputValid(&(slaveClusters[current_Process]))) {
			finishedSlaveIndex = current_Process;
			break;
		}
	}
	return finishedSlaveIndex;
}


//Checking the output array with results from slaves, Return the index of the first finished slave
int masterCheckResults(Boolean *is_finished, Input* input, Output *result, int numOfProcesses,
	Output *slaveOutputs) {
	int index_finished_process = NO_SLAVE_FINISHED;
	if (!(*is_finished)) {
		index_finished_process = getIndexOfFirstFinished(numOfProcesses, slaveOutputs);
		if (index_finished_process != NO_SLAVE_FINISHED) {
			*is_finished = TRUE;
			*result = slaveOutputs[index_finished_process];
		}
	}
	else
		index_finished_process = MASTER_RANK;
	return index_finished_process;
}


//Check if is a master
Boolean isMasterRank(int rank) {
	Boolean is_Master = FALSE;
	if (rank == MASTER_RANK)
		is_Master = TRUE;
	return is_Master;
}


//Master writing the result
void masterPrintResults(Cluster* all_clusters, Output *result) {
	writeOutputFile(result, all_clusters, OUTPUT_FILE);
	print(all_clusters, result->K);
}
