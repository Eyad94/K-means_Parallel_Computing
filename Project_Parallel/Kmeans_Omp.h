#pragma once

#include "Cluster.h"


//Calculate a diameter of clusters (OpenMP)
void clustersDiameter_Omp(Cluster* all_clusters, int numOfClusters, Point* all_points, int numOfPoints);


// Increases time for some of points
void increaseTime_Omp(Point* points, int numOfPoints, double dt, int moment);


//Calculate a center of clusters (OpenMP)
void clustersCenters_Omp(Cluster* all_clusters, int numOfClusters, Point* all_points, int numOfPoints);


// Assigning all points to clusters, return true if at least one point has changed cluster (OpenMP)
Boolean assignClustersToPoints_Omp(Cluster* all_clusters, int numOfClusters, Point* all_points, int numOfPoints);