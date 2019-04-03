#pragma once

#include "InputAndOutput.h"

struct Cluster {
	int numOfPoints;
	Position center;
	double diameter;
	int id;
};


// Assigning K clusters centers to first K point
void initClusters(Point* all_points, Cluster** all_clusters, Input* input);


//Assigning the point to the suitable cluster, Return True if the point's cluster has changed
Boolean assignClusterToPoint(Cluster* all_clusters, Point* point, int clustersCount);