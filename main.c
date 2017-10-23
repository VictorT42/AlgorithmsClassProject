//File main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curves.h"

double dtw(Curve*, Curve*);
Curve *readCurves(FILE*, int*, int*);

int main(int argc, char *argv[])
{
	int i, j;
	FILE *dataset;
	Curve *curves;
	int dimension, curvesNum;
	
	for(i=1; i<argc; i+=2)
	{
		if(!strcmp(argv[i], "-i"))
		{
			dataset = fopen(argv[i+1], "r");
		}
		else
		{
			printf("Invalid argument %s\n", argv[i]);
			i--;
		}
	}
	
	if(!dataset)
	{
		printf("Invalid input file given\n");
		return 1;
	}
	
	curves = readCurves(dataset, &dimension, &curvesNum);
	
	for(i=0; i<curvesNum; i++)
	{
		printf("%s %d\n", curves[i].id, curves[i].numOfPoints);
		printf("(%lf, %lf)\n", curves[i].points[0].x, curves[i].points[0].y);
		printf("(%lf, %lf)\n", curves[i].points[curves[i].numOfPoints -1].x, curves[i].points[curves[i].numOfPoints -1].y);
	}
	
	return 0;
	
}

