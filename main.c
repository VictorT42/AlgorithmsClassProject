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
	int i, j, m, n, statsCounter;
	FILE *dataset=NULL, *queryFile=NULL, *outputFile=stdout;
	Curve *curves, *queries;
	int dimension=2;
	double radius=0;
	int curvesNum, queriesNum;
	int k=2, l=3;
	Vector *u;
	Vector *g_u;
	HashInfo *hashInfo;
	char hashType = 'p';
	int maxCurveLength;
	int maxGridCurveLength;
	double mU, mV, mS;
	QueryResult **results;
	QueryResult *lResults;
	int *numOfResults;
	int numOfLResults;
	int nearest;
	int nearestOfL;
	double nearestOfLDistance;
	int trueNearest;
	double trueNearestDistance;
	double (*distanceFunction)(Curve*, Curve*) = (*dfd);
	char fileNameBuffer[1024];
	int foundGridCurve;
	int stats=0;
	int added;
	Stats *queryStats;
	clock_t start, end, tLSH;
	
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
		else if(!strcmp(argv[i], "-o"))
		{
			outputFile = fopen(argv[i+1], "w");
			if(!outputFile)
			{
				printf("Invalid output file path given, printing on screen");
				outputFile = stdout;
			}
		}
		else if(!strcmp(argv[i], "-stats"))
		{
			stats = 1;
			i--;
		}
		else if(!strcmp(argv[i], "-function"))
		{
			if(!strcmp(argv[i+1], "DTW"))
				distanceFunction = *dtw;
			else
				distanceFunction = *dfd;
		}
		else if(!strcmp(argv[i], "-hash"))
		{
			if(!strcmp(argv[i+1], "classic"))
				hashType = 'c';
			else
				hashType = 'p';
		}
		else
		{
			printf("Invalid argument %s\n", argv[i]);
			i--;
		}
	}
	
	//Read input file
	while(dataset == NULL)
	{
		printf("Please give a valid input file path: ");
		scanf("%s", fileNameBuffer);
		dataset = fopen(fileNameBuffer, "r");
	}
	curves = readCurves(dataset, &dimension, &curvesNum, NULL);
	fclose(dataset);
	
	//Read query file
	while(queryFile == NULL)
	{
		printf("Please give a valid query file path: ");
		scanf("%s", fileNameBuffer);
		queryFile = fopen(fileNameBuffer, "r");
	}
	queries = readCurves(queryFile, &dimension, &queriesNum, &radius);
	fclose(queryFile);
	queryStats = malloc(queriesNum*sizeof(Stats));
	for(i=0; i<queriesNum; i++)
	{
		queryStats[i].minDistance = INFINITY;
		queryStats[i].maxDistance = -INFINITY;
		queryStats[i].sumDistance = 0;
		queryStats[i].tLSHmin = INFINITY;
		queryStats[i].tLSHmax = -INFINITY;
		queryStats[i].tLSHsum = 0;
		start = clock();
		queryStats[i].trueNearestDistance = findTrueNearest(&(queries[i]), curves, curvesNum, &(queryStats[i].trueNearest), distanceFunction);
		end = clock();
		queryStats[i].tTrue = end-start / (double)CLOCKS_PER_SEC;
	}
	
	srand(time(NULL));
	
	//Find max curve length
	maxCurveLength = 0;
	for(i=0; i<curvesNum; i++)
		maxCurveLength = (curves[i].numOfPoints > maxCurveLength) ? curves[i].numOfPoints : maxCurveLength;
	maxGridCurveLength = maxCurveLength * dimension * k;
	
	
	for(statsCounter=0; statsCounter < ((stats) ? 100 : 1); statsCounter++)
	{
		//Put dataset curves into hash tables
		if(hashType == 'c')  //for classic hashing
		{
			l = 1;
			hashInfo = malloc(sizeof(HashInfo));
			hashInfo->gParameters = NULL;
			hashInfo->grids = malloc(k*sizeof(Point));
			for(i=0; i<k; i++)
			{
				hashInfo->grids[i].x = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
				hashInfo->grids[i].y = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
				if(dimension > 2)
				{
					hashInfo->grids[i].z = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
					if(dimension > 3)
						hashInfo->grids[i].w = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
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
		else  //for LSH
		{
			g_u = malloc(sizeof(Vector));
			g_u->length = K_VEC;
			g_u->coordinates = malloc(g_u->length*sizeof(double));
			
			hashInfo = malloc(l*sizeof(HashInfo));
			for(m=0; m<l; m++)
			{
				hashInfo[m].grids = malloc(k*sizeof(Point));
				for(i=0; i<k; i++)
				{
					hashInfo[m].grids[i].x = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
					hashInfo[m].grids[i].y = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
					if(dimension > 2)
					{
						hashInfo[m].grids[i].z = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
						if(dimension > 3)
							hashInfo[m].grids[i].w = rand()%(int)(DELTA*DELTA_ACCURACY) / (double)DELTA_ACCURACY;
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
		
		
		//Run the queries
		if(hashType == 'c')
		{
			results = malloc(sizeof(QueryResult*));
			numOfResults = malloc(sizeof(int));
		}
		else
		{
			results = malloc(l*sizeof(QueryResult*));
			numOfResults = malloc(l*sizeof(int));
		}
		for(i=0; i<queriesNum; i++)
		{
			if(stats)
			{
				trueNearestDistance = queryStats[i].trueNearestDistance;
				trueNearest = queryStats[i].trueNearest;
			}
			else
				trueNearestDistance = findTrueNearest(&(queries[i]), curves, curvesNum, &trueNearest, distanceFunction);
			
			if(hashType == 'c')
			{
				start = clock();
				*results = query(&(queries[i]), numOfResults, curves, hashInfo, radius, k, dimension, distanceFunction, &nearest);
				end = clock();
				if(*results == NULL)
				{
					printf("No curves found in bucket.\n");
					continue;
				}
				
				nearestOfL = results[nearest]->curve;
				foundGridCurve = results[nearest]->foundGridCurve;
				nearestOfLDistance = results[nearest]->distance;
				
				lResults = *results;
				numOfLResults = *numOfResults;
			}
			else
			{
				numOfLResults = 0;
				nearestOfLDistance = INFINITY;
				start = clock();
				for(j=0; j<l; j++)
				{
					results[j] = query(&(queries[i]), &(numOfResults[j]), curves, &(hashInfo[j]), radius, k, dimension, distanceFunction, &nearest);
					if(results[j] == NULL)
						continue;
					numOfLResults += numOfResults[j];
					if((results[j][nearest].distance < nearestOfLDistance))
					{
						nearestOfL = results[j][nearest].curve;
						nearestOfLDistance = results[j][nearest].distance;
						foundGridCurve = results[j][nearest].foundGridCurve;
					}
				}
				end = clock();
				lResults = malloc(numOfLResults*sizeof(QueryResult));
				
				numOfLResults = 0;
				for(j=0; j<l; j++)
				{
					for(m=0; m<numOfResults[j]; m++)
					{
						if(l>0)
						{
							// Check if the curve is already in the results of a different hashtable
							added = 0;
							for(n=0; n<numOfLResults; n++)
							{
								if(lResults[n].curve == results[j][m].curve)
								{
									added = 1;
									break;
								}
							}
							if(added)
								continue;
						}
						lResults[numOfLResults] = results[j][m];
						numOfLResults++;
					}
				}
			}
			
			if(stats)
			{
				if(nearestOfLDistance < queryStats[i].minDistance)
					queryStats[i].minDistance = nearestOfLDistance;
				if(nearestOfLDistance > queryStats[i].maxDistance)
					queryStats[i].maxDistance = nearestOfLDistance;
				queryStats[i].sumDistance += nearestOfLDistance;
				tLSH = end-start / (double) CLOCKS_PER_SEC;
				if(tLSH < queryStats[i].tLSHmin)
					queryStats[i].tLSHmin = tLSH;
				if(tLSH > queryStats[i].tLSHmax)
					queryStats[i].tLSHmax = tLSH;
				queryStats[i].tLSHsum += tLSH;
			}
			
			fprintf(outputFile, "Query: %s\n", queries[i].id);
			fprintf(outputFile, "DistanceFunction: %s\n", (distanceFunction == dtw) ? "DTW" : "DFT");
			fprintf(outputFile, "HashFunction: %s\n", (hashType == 'c') ? "Classic" : "Probabilistic");
			fprintf(outputFile, "FoundGridCurve: %s\n", foundGridCurve ? "True" : "False");
			fprintf(outputFile, "LSH Nearest neighbor: %s\n", curves[nearestOfL].id);
			fprintf(outputFile, "True Nearest neighbor: %s\n", curves[trueNearest].id);
			fprintf(outputFile, "distanceLSH: %lf\n", nearestOfLDistance);
			fprintf(outputFile, "distanceTrue: %lf\n", trueNearestDistance);
			
			if(radius > 0)
			{
				sort(lResults, numOfLResults, curves);
				fprintf(outputFile, "R-near neighbors:\n");
				for(j=0; j<numOfLResults; j++)
				{
					fprintf(outputFile, "%s\n", curves[lResults[j].curve].id);
				}
			}
		}
		
		//Cleanup between runs
		for(i=0; i<l; i++)
		{
			destroyHashTable(hashInfo[i].hashTable);
			free(hashInfo[i].grids);
			free(hashInfo[i].r);
			if(hashInfo[i].gParameters)
			{
				for(j=0; j<k; j++)
				{
					free(hashInfo[i].gParameters[j].v->coordinates);
					free(hashInfo[i].gParameters[j].v);
				}
				free(hashInfo[i].gParameters);
			}
		}
		free(hashInfo);
		free(results);
		free(numOfResults);
		
	}
	
	
	
	// int countPart,countFull=0;
	// puts("Print hashtable?");
	// if(getchar() == 'y'){
		// for(j=0; j<l; j++)
		// {
			// countFull=0;
			// printf("\nHashTable %d:\n", j);
			// for(i=0; i<hashInfo[j].hashTable->size; i++)
			// {
				// countPart=countEntries(hashInfo[j].hashTable->table[i]);
				// printf("%d ", countPart);
				// countFull += countPart;
			// }
			// printf("\nAll entries are %d\n", countFull);
			// puts("");
		// }
	// }
	
	
	//Cleanup
	for(i=0; i<curvesNum; i++)
	{
		free(curves[i].id);
		free(curves[i].points);
	}
	free(curves);
	
	return 0;
	
}

