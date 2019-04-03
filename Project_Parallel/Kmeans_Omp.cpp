#include <omp.h>
#include <stdlib.h>
#include "Kmeans_Omp.h"

//Calculate a diameter of clusters (OpenMP)
void clustersDiameter_Omp(Cluster* all_clusters, int numOfClusters, Point* all_points, int numOfPoints) {
#pragma omp parallel for
	for (int indexOfCluster = 0; indexOfCluster < numOfClusters; indexOfCluster++) {
		double max_distance = 0, current_distance;
		for (int i = 0; i < numOfPoints - 1; i++)
			if (all_points[i].cluster->id == indexOfCluster)
				for (int j = i + 1; j < numOfPoints; j++)
					if (all_points[j].cluster->id == indexOfCluster) {
						current_distance = distance_between_two_points(&(all_points[i].position),
							&(all_points[j].position));
						if (current_distance > max_distance)
							max_distance = current_distance;
					}
		all_clusters[indexOfCluster].diameter = max_distance;
	}
}


// Increases time for some of points
void increaseTime_Omp(Point* points, int numOfPoints, double dt, int moment) {
	double timeInterval = dt * moment;

#pragma omp parallel for
	for (int i = 0; i < numOfPoints; i++) {
		points[i].position.x += timeInterval * points[i].velocity.vx;
		points[i].position.y += timeInterval * points[i].velocity.vy;
		points[i].position.z += timeInterval * points[i].velocity.vz;
	}
}


//Calculate a center of clusters (OpenMP)
void clustersCenters_Omp(Cluster* all_clusters, int numOfClusters, Point* all_points, int numOfPoints) {
#pragma omp parallel for
	for (int indexOfCluster = 0; indexOfCluster < numOfClusters; indexOfCluster++) {
		double sum_x, sum_y, sum_z;
		sum_x = sum_y = sum_z = 0;

		if (all_clusters[indexOfCluster].numOfPoints != 0) {
			for (int pointIndex = 0; pointIndex < numOfPoints; pointIndex++) {
				if (all_points[pointIndex].cluster->id == indexOfCluster) {
					sum_x += all_points[pointIndex].position.x;
					sum_y += all_points[pointIndex].position.y;
					sum_z += all_points[pointIndex].position.z;
				}
			}
			all_clusters[indexOfCluster].center.x = sum_x / all_clusters[indexOfCluster].numOfPoints;
			all_clusters[indexOfCluster].center.y = sum_y / all_clusters[indexOfCluster].numOfPoints;
			all_clusters[indexOfCluster].center.z = sum_z / all_clusters[indexOfCluster].numOfPoints;
		}
	}
}


// Assigning all points to clusters, return true if at least one point has changed cluster (OpenMP)
Boolean assignClustersToPoints_Omp(Cluster* all_clusters, int numOfClusters, Point* all_points, int numOfPoints) {
	Boolean point_changed = FALSE;
	int* clustersPointsCounter = (int*)calloc(omp_get_max_threads() * numOfClusters, sizeof(int));

	if (clustersPointsCounter == NULL) {
		printf("\Error in allocation assignClustersToPointsOmp\n");
		exit(0);
	}
	
#pragma omp parallel for
	for (int indexOfPoint = 0; indexOfPoint < numOfPoints; indexOfPoint++) {
		if (assignClusterToPoint(all_clusters, &(all_points[indexOfPoint]), numOfClusters))
			point_changed = TRUE;

		int pointClusterId = all_points[indexOfPoint].cluster->id;
		int threadId = omp_get_thread_num();
		clustersPointsCounter[threadId * numOfClusters + pointClusterId] += 1;
	}

#pragma omp parallel for
	for (int clusterId = 0; clusterId < numOfClusters; clusterId++) {
		all_clusters[clusterId].numOfPoints = 0;
		for (int threadId = 0; threadId < omp_get_num_threads(); threadId++) {
			all_clusters[clusterId].numOfPoints += clustersPointsCounter[threadId * numOfClusters + clusterId];
		}
	}
	return point_changed;
}
