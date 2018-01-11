//File input.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "curves.h"

#define BUFFER_SIZE 8

int countLines(FILE *input)
{
	char c;
	int lines=0;
	
	do
	{
		c = fgetc(input);
		if(c == '\n')
			lines++;
	} while(c != EOF);
	
	fseek(input, 0, SEEK_SET);
	return lines;
}


Curve *readCurves(FILE *input, int *d, int *curvesNum, double *r)
{
	int n;
	int i, j;
	Curve *curves;
	
	*d = 3;
	
	fscanf(input, "%d\n", curvesNum);
	fscanf(input, "%d\n", &n);
	
	curves = malloc(*curvesNum * sizeof(Curve));
	for(i=0; i<*curvesNum; i++)
	{
		curves[i].id = NULL;
		curves[i].numOfPoints = n;
		curves[i].points = malloc(n *sizeof(Point));
		for(j=0; j<n; j++)
		{
			fscanf(input, "%lf %lf %lf\n", &(curves[i].points[j].x), &(curves[i].points[j].y), &(curves[i].points[j].z));
			curves[i].points[j].w = 0;
		}
	}
	
	return curves;
	
}

/*
Curve *readCurves(FILE *input, int *d, int *curvesNum, double *r)
{
	char buffer[1024];
	double parameter;
	int i, j;
	Curve *curves;
	
	*curvesNum = countLines(input);
	
	fscanf(input, "%s %lf", buffer, &parameter);
	if(!strcmp(buffer, "@dimension"))
	{
		*d = (int)parameter;
		(*curvesNum)--;
	}
	else if(!strcmp(buffer, "R:"))
	{
		*r = parameter;
		(*curvesNum)--;
	}
	else
	{
		fseek(input, 0, SEEK_SET);
	}
	
	curves = malloc(*curvesNum * sizeof(Curve));
	
	for(i=0; i<(*curvesNum); i++)
	{
		curves[i].id = malloc(BUFFER_SIZE);
		fscanf(input, "%s\t%d\t", curves[i].id, &(curves[i].numOfPoints));
		curves[i].points = malloc(curves[i].numOfPoints * sizeof(Point));
		for(j=0; j < curves[i].numOfPoints; j++)
		{
			fscanf(input, "(%lf, %lf", &(curves[i].points[j].x), &(curves[i].points[j].y));
			if(*d > 2)
				fscanf(input, ", %lf", &(curves[i].points[j].z));
			else
				curves[i].points[j].z = 0;
			if(*d > 3)
				fscanf(input, ", %lf", &(curves[i].points[j].w));
			else
				curves[i].points[j].w = 0;
			fscanf(input, "), ");
		}
	}
	
	return curves;
}
)

*/
