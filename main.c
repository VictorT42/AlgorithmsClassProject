//File main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "curves.h"
#include "hash.h"

double dtw(Curve*, Curve*);
double dfd(Curve*, Curve*);
int countEntries(Bucket *b);

int main(int argc, char *argv[])
{
	int i, j, m;
	FILE *dataset=NULL, *queryFile=NULL;
	Curve *curves;
	int dimension=2, curvesNum;
	int k=2, l=3;
	Vector *u;
	Vector *g_u;
	HashInfo *hashInfo;
	char hashType = 'p';
	int maxCurveLength;
	int maxGridCurveLength;
	double mU, mV, mS;
	
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
	
	if(!dataset)
	{
		printf("Invalid input file given\n");
		return 1;
	}
	
	srand(time(NULL));
	
	curves = readCurves(dataset, &dimension, &curvesNum, NULL);
	fclose(dataset);
	
	maxCurveLength = 0;
	for(i=0; i<curvesNum; i++)
		maxCurveLength = (curves[i].numOfPoints > maxCurveLength) ? curves[i].numOfPoints : maxCurveLength;
	maxGridCurveLength = maxCurveLength * dimension * k;
	
	if(hashType == 'c')
	{                                            l=1;
		hashInfo = malloc(sizeof(hashInfo));
		hashInfo->grids = malloc(k*sizeof(Point));
		for(i=0; i<k; i++)
		{
			hashInfo->grids[i].x = rand()%(int)(DELTA*10000) / (double)10000;
			hashInfo->grids[i].y = rand()%(int)(DELTA*10000) / (double)10000;
			if(dimension > 2)
			{
				hashInfo->grids[i].z = rand()%(int)(DELTA*10000) / (double)10000;
				if(dimension > 3)
					hashInfo->grids[i].w = rand()%(int)(DELTA*10000) / (double)10000;
			}
		}
		hashInfo->r = malloc(maxGridCurveLength * sizeof(int));
		for(i=0; i<maxGridCurveLength; i++)
		{
			hashInfo->r[i] = rand()%1000;
		}
		hashInfo->hashTable = createHashTable(curvesNum / 2);
		
		for(i=0; i<curvesNum; i++)
		{
			u = snapToGrid(&(curves[i]), k, dimension, hashInfo->grids);
			insertToTable(hashInfo->hashTable, u, u, i, hashInfo->r);
		}
		
	}
	
	else
	{
		g_u = malloc(sizeof(Vector));
		g_u->length = K_VEC;
		g_u->coordinates = malloc(g_u->length*sizeof(double));
		
		hashInfo = malloc(l*sizeof(hashInfo));
		for(m=0; m<l; m++)
		{
			hashInfo[m].grids = malloc(k*sizeof(Point));
			for(i=0; i<k; i++)
			{
				hashInfo[m].grids[i].x = rand()%(int)(DELTA*10000) / (double)10000;
				hashInfo[m].grids[i].y = rand()%(int)(DELTA*10000) / (double)10000;
				if(dimension > 2)
				{
					hashInfo[m].grids[i].z = rand()%(int)(DELTA*10000) / (double)10000;
					if(dimension > 3)
						hashInfo[m].grids[i].w = rand()%(int)(DELTA*10000) / (double)10000;
				}
			}
			
			hashInfo[m].r = malloc(maxGridCurveLength * sizeof(int));
			for(i=0; i<maxGridCurveLength; i++)
			{
				hashInfo[m].r[i] = rand()%1000;
			}
			
			hashInfo[m].gParameters = malloc(K_VEC*sizeof(GParameters));
			for(i=0; i<K_VEC; i++)
			{
				hashInfo[m].gParameters[i].t = rand()%W;
				hashInfo[m].gParameters[i].v = malloc(sizeof(Vector));
				hashInfo[m].gParameters[i].v->length = maxGridCurveLength;
				hashInfo[m].gParameters[i].v->coordinates = malloc(hashInfo[m].gParameters[i].v->length*sizeof(double));
				for(j=0; j<hashInfo[m].gParameters[i].v->length; j+=2)
				{
					do
					{
						mU = rand()%2000000 - 1000000;
						mU = mU/1000000;
						mV = rand()%2000000 - 1000000;
						mV = mV/1000000;
						mS = mU*mU + mV*mV;
					} while(mS >= 1 || mS == 0);
					hashInfo[m].gParameters[i].v->coordinates[j] = mU*sqrt(-2*log(mS) / mS);
					if(j+1 < hashInfo[m].gParameters[i].v->length)
						hashInfo[m].gParameters[i].v->coordinates[j+1] = mV*sqrt(-2*log(mS) / mS);
				}
			}
			
			hashInfo[m].hashTable = createHashTable(curvesNum / 2);
			
			for(i=0; i<curvesNum; i++)
			{
				u = snapToGrid(&(curves[i]), k, dimension, hashInfo[m].grids);
				g(g_u, u, hashInfo[m].gParameters);
				insertToTable(hashInfo[m].hashTable, g_u, u, i, hashInfo[m].r);
			}
		}
	}
	
	if(queryFile == NULL)
	{
		return 0;
	}
	
	
	
	int countPart,countFull=0;
	puts("Print hashtable?");
	if(getchar() == 'y'){
		for(j=0; j<l; j++)
		{
			countFull=0;
			printf("\nHashTable %d:\n", j);
			for(i=0; i<hashInfo[j].hashTable->size; i++)
			{
				countPart=countEntries(hashInfo[j].hashTable->table[i]);
				printf("%d ", countPart);
				countFull += countPart;
			}
			printf("\nAll entries are %d\n", countFull);
			puts("");
		}
	}
	
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

