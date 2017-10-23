//File euclid.c

#include <math.h>
#include "curves.h"

double dist(Point a1, Point a2)
{
	return sqrt((a1.x - a2.x)*(a1.x - a2.x) + (a1.y - a2.y)*(a1.y - a2.y) + (a1.z - a2.z)*(a1.z - a2.z) + (a1.w - a2.w)*(a1.w - a2.w));
}

