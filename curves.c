//File curves.c

#include <stdlib.h>
#include <math.h>
#include "curves.h"

void snapToGrid(Curve *curve, Vector *u, int k, int d, Point *grids)
{
	int i, j, g;
	int same;
	int vectorOffset=0;
	
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
					vectorOffset =- d;
			}
		}
	}
	
	u->length = vectorOffset;
	
	return;
	
}

