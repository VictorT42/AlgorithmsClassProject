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

Curve *readCurves(FILE *input, int *d, int *curvesNum)
{
	char buffer[1024];
	int dimension;
	int i, j;
	Curve *curves;
	
	*curvesNum = countLines(input);
	
	fscanf(input, "%s %d", buffer, &dimension);
	if(!strcmp(buffer, "@dimension"))
	{
		*d = dimension;
		(*curvesNum)--;
	}
	else
	{
		dimension = 2;
		*d = 2;
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
			if(dimension > 2)
			{
				fscanf(input, ", %lf", &(curves[i].points[j].z));
				if(dimension > 3)
					fscanf(input, ", %lf", &(curves[i].points[j].w));
			}
			fscanf(input, "), ");
		}
	}
	
	return curves;
}

