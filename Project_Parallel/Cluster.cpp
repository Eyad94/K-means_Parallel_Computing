#include "Cluster.h"
#include <stdlib.h>

// Assigning K clusters centers to first K point
void initClusters(Point* all_points, Cluster** all_clusters, Input* input) {
	int numOfClusters, i;
	numOfClusters = input->K;
	if (*all_clusters == NULL)
		*all_clusters = (Cluster*)malloc(sizeof(Cluster) * numOfClusters);
	if (*all_clusters == NULL) {
		printf("\nError in allocation clusters ==> initClusters\n");
		exit(1);
	}

	for (i = 0; i < numOfClusters; i++) {
		(*all_clusters)[i].center.x = all_points[i].position.x;
		(*all_clusters)[i].center.y = all_points[i].position.y;
		(*all_clusters)[i].center.z = all_points[i].position.z;
		(*all_clusters)[i].id = i;
		(*all_clusters)[i].numOfPoints = 0;
	}
	for (i = 0; i < input->N; i++)
		all_points[i].cluster = NULL;
}


//Assigning the point to the suitable cluster, Return True if the point's cluster has changed
Boolean assignClusterToPoint(Cluster* all_clusters, Point* point, int clustersCount) {
	Cluster* new_cluster = NULL;
	double current_distance, min_distance = -1;
	int indexOfCluster;

	for (indexOfCluster = 0; indexOfCluster < clustersCount; indexOfCluster++) {
		current_distance = distance_between_two_points(&(point->position),
			&(all_clusters[indexOfCluster].center));

		if (min_distance == -1 || current_distance < min_distance) {
			min_distance = current_distance;
			new_cluster = &(all_clusters[indexOfCluster]);
		}
	}
	// Fist assignment ::: Or the point has changed cluster
	if (point->cluster == NULL || point->cluster->id != new_cluster->id) {
		point->cluster = new_cluster;
		return TRUE;
	}
	return FALSE;
}