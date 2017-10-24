//File curves.h

#define DELTA 0.5

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

