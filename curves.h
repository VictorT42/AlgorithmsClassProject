//File curves.h

#ifndef CURVES
#define CURVES

#include <stdio.h>

#define DELTA 0.0001
#define DELTA_ACCURACY 100000

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

typedef struct Stats
{
	double minDistance;
	double maxDistance;
	double sumDistance;
	double tLSHmin;
	double tLSHmax;
	double tLSHsum;
	double tTrue;
	int trueNearest;
	double trueNearestDistance;
} Stats;

Vector *snapToGrid(Curve *curve, int k, int d, Point *grids);
Curve *readCurves(FILE*, int*, int*, double*);
double findTrueNearest(Curve *queryCurve, Curve *curves, int curvesNum, int *nearest, double(*distanceFunction)(Curve*, Curve*));
void sort(int*, int , Curve*);

#endif

