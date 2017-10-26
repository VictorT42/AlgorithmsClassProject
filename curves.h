//File curves.h

#include <stdio.h>

#define DELTA 0.005

typedef struct Point
{
	double x;
	double y;
	double z;
	double w;
} Point;

typedef struct Curve
{
	char *id;
	Point *points;
	int numOfPoints;
} Curve;

typedef struct Vector
{
	double *coordinates;
	int length;
} Vector;

Vector *snapToGrid(Curve *curve, int k, int d, Point *grids);
Curve *readCurves(FILE*, int*, int*, double*);

