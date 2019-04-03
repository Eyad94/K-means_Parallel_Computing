#include "Point.h"
#include <math.h>


//distance between two points
double distance_between_two_points(Position* point1, Position* point2) {
	double x = point2->x - point1->x;
	double y = point2->y - point1->y;
	double z = point2->z - point1->z;
	return sqrt(x*x + y*y + z*z);
}