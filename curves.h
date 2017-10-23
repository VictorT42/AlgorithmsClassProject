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
	char *id;
	Point *points;
	int numOfPoints;
} Curve;

