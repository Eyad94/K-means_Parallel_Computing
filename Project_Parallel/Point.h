#pragma once

struct Position {
	double x;
	double y;
	double z;
};

struct Velocity {
	double vx;
	double vy;
	double vz;
};

struct Cluster;

struct Point {
	Position position;
	Velocity velocity;
	Cluster* cluster;
};


//distance between two points
double distance_between_two_points(Position* point1, Position* point2);
