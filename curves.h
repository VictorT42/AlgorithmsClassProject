//File curves.h

typedef struct Point
{
	double x;
	double y;
	double z;
	double w;
} Point;

typedef struct Curve
{
	Point *points;
	int numOfPoints;
} Curve;

