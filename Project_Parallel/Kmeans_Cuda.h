#pragma once
#include "InputAndOutput.h"
#include "Cluster.h"
#include "Point.h"
#include <math.h>


#define CUDA_INC_TIME_PORTION 0.9   //The portion of points for Cuda

//Current GPUs, a thread block may contain up to 1024 threads.
#define BLOCK_SIZE 1024   //Maximum block size for Cuda

//Maximum of a grid of thread blocks
#define MAX_BLOCKS pow(2, 31) - 1 


//Setting the Cuda device(0)
const char* initCuda();


//Calling device reset
const char* stopCuda();


const char*  freeCuda(Point* gpu_points);


//Increases time for some of points
const char* updateTimeCudaStart(Point* points, int numOfPoints, double dt, int moment, Point** gpu_points);


//Waits for the kernel to finish, and returns any errors encountered during the launch.
const char* updateTimeCudaEnd(Point* dev_points, Point* pointsArr, int numOfPoints);


//CUDA C Programming Guide
// http://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html
