//File main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "curves.h"

double dtw(Curve*, Curve*);
double dfd(Curve*, Curve*);
Curve *readCurves(FILE*, int*, int*);
void snapToGrid(Curve *curve, Vector *u, int k, int d, Point *grids);
int hash(Vector*, int);

int main(int argc, char *argv[])
{
	int i, j;
	FILE *dataset=NULL, *queryFile=NULL;
	Curve *curves;
	int dimension, curvesNum;
	int k=2, l=3;
	Vector *u;
	
	//Get command line arguments
	for(i=1; i<argc; i+=2)
	{
		if(!strcmp(argv[i], "-d"))
			dataset = fopen(argv[i+1], "r");
		else if(!strcmp(argv[i], "-q"))
			queryFile = fopen(argv[i+1], "r");
		else if(!strcmp(argv[i], "-k"))
			k = atoi(argv[i+1]);
		else if(!strcmp(argv[i], "-L"))
			l = atoi(argv[i+1]);
		else
		{
			printf("Invalid argument %s\n", argv[i]);
			i--;
		}
	}
	hash(NULL,0);
	if(!dataset)
	{
		printf("Invalid input file given\n");
		return 1;
	}
	
	curves = readCurves(dataset, &dimension, &curvesNum);
	fclose(dataset);
	
	
	
	//TESTS
	// printf("DTW: %lf\n", dtw(&(curves[0]), &(curves[1])));
	// for(j=0; j<curves[1].numOfPoints -1; j++)
	// {
		// printf("(%lf, %lf), ", curves[1].points[j].x, curves[1].points[j].y);
	// }
	// printf("(%lf, %lf)\n", curves[1].points[j].x, curves[1].points[j].y);
	// for(j=0; j<curves[6].numOfPoints -1; j++)
	// {
		// printf("(%lf, %lf), ", curves[6].points[j].x, curves[6].points[j].y);
	// }
	// printf("(%lf, %lf)\n", curves[6].points[j].x, curves[6].points[j].y);
	
	// Curve test;
	// test.numOfPoints = 3;
	// test.points = malloc(3*sizeof(Point));
	// test.points[0].x = 5.2;
	// test.points[0].y = 6.7;
	// test.points[1].x = 3.4;
	// test.points[1].y = 4.1;
	// test.points[2].x = 1.4;
	// test.points[2].y = 4.9;
	// Point grid2;
	// grid2.x = 0.2;
	// grid2.y = 0.2;
	
	
	// srand(time(NULL));
	// Point grids[k];
	// for(i=0;i<k;i++)
	// {
		// grids[i].x=rand()%(int)(DELTA*10000) / (float)10000;
		// grids[i].y=rand()%(int)(DELTA*10000) / (float)10000;
	// }
	// u=malloc(sizeof(Vector));
	// snapToGrid(&(curves[0]), u, k, dimension, grids);
	
	// snapToGrid(&test, u, 1, 2, &grid2);
	
	// for(i=0;i<u->length; i++)
		// printf("%lf ", u->coordinates[i]);
	
	puts("Print curves?");
	if(getchar() == 'y'){
	for(i=0; i<curvesNum; i++)
	{
		printf("%s\t%d\t", curves[i].id, curves[i].numOfPoints);
		for(j=0; j<curves[i].numOfPoints -1; j++)
		{
			printf("(%lf, %lf), ", curves[i].points[j].x, curves[i].points[j].y);
		}
		printf("(%lf, %lf)\n", curves[i].points[j].x, curves[i].points[j].y);
	}}
	
	if(k && l && queryFile)
		puts("filler");
	/////////
	
	//Cleanup
	for(i=0; i<curvesNum; i++)
	{
		free(curves[i].id);
		free(curves[i].points);
	}
	free(curves);
	
	return 0;
	
}

