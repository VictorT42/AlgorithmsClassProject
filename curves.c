//File curves.c

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "curves.h"
#include "hash.h"

Vector *snapToGrid(Curve *curve, int k, int d, Point *grids)
{
	int i, j, g;
	int same;
	int vectorOffset=0;
	Vector *u;
	
	u=malloc(sizeof(Vector));
	
	u->length = curve->numOfPoints * d * k;
	u->coordinates = malloc(u->length * sizeof(double));
	
	for(i=0; i<k; i++)
	{
		for(j=0; j< curve->numOfPoints; j++)
		{
			u->coordinates[vectorOffset++] = round((curve->points[j].x - grids[i].x) / DELTA) * DELTA + grids[i].x;
			u->coordinates[vectorOffset++] = round((curve->points[j].y - grids[i].y) / DELTA) * DELTA + grids[i].y;
			if(d > 2)
				u->coordinates[vectorOffset++] = round((curve->points[j].z - grids[i].z) / DELTA) * DELTA + grids[i].z;
			if(d > 3)
				u->coordinates[vectorOffset++] = round((curve->points[j].w - grids[i].w) / DELTA) * DELTA + grids[i].w;
			
			if(j > 0)
			{
				same = 1;
				for(g=0; g<d; g++)
				{
					if(u->coordinates[vectorOffset-1-g] != u->coordinates[vectorOffset-1-g-d])
						same = 0;
				}
				if(same)
					vectorOffset -= d;
			}
		}
	}
	
	u->length = vectorOffset;
	
	return u;
	
}

double findTrueNearest(Curve *queryCurve, Curve *curves, int curvesNum, int *nearest, double(*distanceFunction)(Curve*, Curve*))
{
	int i;
	double distance, minDistance;
	
	*nearest = 0;
	minDistance = (*distanceFunction)(queryCurve, &(curves[0]));
	for(i=1; i<curvesNum; i++)
	{
		distance = (*distanceFunction)(queryCurve, &(curves[i]));
		if(distance < minDistance)
		{
			minDistance = distance;
			*nearest = i;
			if(minDistance == 0)
				break;
		}
	}
	
	return minDistance;
	
}

void sort(QueryResult *results, int numOfResults, Curve *curves)
{
	int i,j;
	QueryResult temp;
	
	for(i=1; i<numOfResults; i++)
	{
		for(j=0; j>0; j--)
		{
			if(strcmp(curves[results[j].curve].id, curves[results[j-1].curve].id) < 0)
			{
				temp = results[j];
				results[j] = results[j-1];
				results[j-1] = temp;
			}
		}
	}
}

