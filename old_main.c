//File main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "curves.h"
#include "hash.h"
#include "metrics.h"
#include "initialization.h"
#include "assignment.h"
#include "update.h"

#define VERBOSE 0

int main(int argc, char *argv[])
{
	int i, j, m, statsCounter;
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
	int *rCurves = NULL;
	int numOfRCurves = 0;
	QueryResult tempNearest;
	QueryResult nearest;
	int trueNearest;
	double trueNearestDistance;
	double (*distanceFunction)(Curve*, Curve*) = (*dfd);
	char fileNameBuffer[1024];
	int stats=0;
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
	
	srand(time(NULL));
	
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
	
	//Find max curve length
	maxCurveLength = 0;
	for(i=0; i<curvesNum; i++)
		maxCurveLength = (curves[i].numOfPoints > maxCurveLength) ? curves[i].numOfPoints : maxCurveLength;
	maxGridCurveLength = maxCurveLength * dimension * k;
	
	if(stats && VERBOSE)
	{
		printf("Progress: 0%%[");
		for(i=0;i<100;i++)putchar(' ');
		printf("]100%%");
		for(i=0;i<105;i++)putchar('\b');
		fflush(stdout);
	}
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
				
				hashInfo[m].hashTable = createHashTable(curvesNum / 8);
				
				for(i=0; i<curvesNum; i++)
				{
					u = snapToGrid(&(curves[i]), k, dimension, hashInfo[m].grids);
					g(g_u, u, hashInfo[m].gParameters);
					insertToTable(hashInfo[m].hashTable, g_u, u, i, hashInfo[m].r);
				}
			}
			
			free(g_u->coordinates);
			free(g_u);
		}
		
		
		
		
	
	
	
	
	
	
		//TEST mdf
		// Curve *mean = mdf(&(curves[0]), &(curves[1]));
		// printf("%d points:\n", mean->numOfPoints);
		// for(i=0; i<mean->numOfPoints; i++)
		// {
			// printf("(%lf, %lf),",mean->points[i].x,mean->points[i].y);
		// }
		// return 0;
		
		//TEST mfd
		
		
		// Curve *mean = mdf(&c1, &c2);
		// printf("%d points:\n", mean->numOfPoints);
		// for(i=0; i<mean->numOfPoints; i++)
		// {
			// printf("(%lf, %lf),",mean->points[i].x,mean->points[i].y);
		// }
		// return 0;
		
		
		// double a=dfd(&c1, &c2);
		// printf("dfd gives %lf\n", a);
		// return 0;
		
		
		
		//TEST CLUSTERING
		int k_of_means_fame=5;
		
		curves = realloc(curves, (curvesNum+k_of_means_fame)*sizeof(Curve));
		
		
		double **distances = malloc(curvesNum*sizeof(double*));
		for(i=0; i<curvesNum; i++)
		{
			distances[i] = malloc(i*sizeof(double));
			for(j=0; j<i; j++)
			{
				distances[i][j] = INFINITY;
			}
		}
		
		int *clusters = malloc(curvesNum*sizeof(int));
		int *centroids;
		// centroids = malloc(k_of_means_fame*sizeof(int));
		centroids = k_means_pp(k_of_means_fame, curvesNum, curves, distances);
		
		
		
		// for(i=0;i<k_of_means_fame;i++)
			// centroids[i]=i;
		
		for(i=0; i<k_of_means_fame; i++)
			printf("%d\n", centroids[i]);
		
		
		for(j=0; j<3; j++)
		{
			lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
			
			// range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
			// distanceFunction, hashInfo, l, k, dimension);
			
			int sum[k_of_means_fame];
			for(i=0; i<k_of_means_fame; i++)
				sum[i]=0;
			
			for(i=0;i<curvesNum;i++)
			{
				sum[clusters[i]]++;
			}
			for(i=0;i<k_of_means_fame;i++)
				printf("cluster %d: %d members\n", centroids[i], sum[i]);
			
			pam(curves, curvesNum, clusters, centroids, k_of_means_fame, distances, distanceFunction);
			
			// meanFrechet(curves, curvesNum, clusters, centroids, k_of_means_fame);
		}
		lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
		
		puts("Silhoiettte");
		
		int *curvesInClusters = malloc(k*sizeof(int));
		for(i=0; i<k_of_means_fame; i++)
			curvesInClusters[i] = 0;
		
		for(i=0; i<curvesNum; i++)
			curvesInClusters[clusters[i]]++;
		
		for(i=0; i<k_of_means_fame; i++)
			printf("cluster %d has %d curves\n",i,curvesInClusters[i]);
		
		double sum=0;
		double s_i;
		for(i=0; i<k_of_means_fame; i++)
		{
			s_i=silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
			printf("s(%d) = %1.15lf\n", i, s_i);
			if(s_i == 0)
				printf("Is zero.\n");
			sum += s_i*curvesInClusters[i];
		}
		printf("Total silhouette is %lf/%d=%lf\n", sum,curvesNum,sum/(double)curvesNum);
		
		return 0;
	
	
	
		
		
		
		
		//Run the queries
		nearest.curve = -1;
		nearest.distance = INFINITY;
		nearest.foundGridCurve = 0;
		
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
				query(&(queries[i]), &nearest, &rCurves, &numOfRCurves, curves, hashInfo, radius, k, dimension, distanceFunction);
				end = clock();
			}
			else
			{
				start = clock();
				for(j=0; j<l; j++)
				{
					query(&(queries[i]), &tempNearest, &rCurves, &numOfRCurves, curves, &(hashInfo[j]), radius, k, dimension, distanceFunction);
					
					if(tempNearest.curve == -1)
						continue;
					if(tempNearest.distance < nearest.distance)
					{
						nearest.curve = tempNearest.curve;
						nearest.distance = tempNearest.distance;
						nearest.foundGridCurve = tempNearest.foundGridCurve;
					}
				}
				end = clock();
			}
			
			if(stats)
			{
				if(nearest.curve != -1)
				{
					if(nearest.distance < queryStats[i].minDistance)
						queryStats[i].minDistance = nearest.distance;
					if(nearest.distance > queryStats[i].maxDistance)
						queryStats[i].maxDistance = nearest.distance;
					queryStats[i].sumDistance += nearest.distance;
					tLSH = end-start / (double) CLOCKS_PER_SEC;
					if(tLSH < queryStats[i].tLSHmin)
						queryStats[i].tLSHmin = tLSH;
					if(tLSH > queryStats[i].tLSHmax)
						queryStats[i].tLSHmax = tLSH;
					queryStats[i].tLSHsum += tLSH;
				}
			}
			//Print query results if not calculating statistics
			else
			{
				fprintf(outputFile, "Query: %s\n", queries[i].id);
				fprintf(outputFile, "DistanceFunction: %s\n", (distanceFunction == dtw) ? "DTW" : "DFT");
				fprintf(outputFile, "HashFunction: %s\n", (hashType == 'c') ? "Classic" : "Probabilistic");
				fprintf(outputFile, "FoundGridCurve: %s\n", nearest.foundGridCurve ? "True" : "False");
				fprintf(outputFile, "LSH Nearest neighbor: %s\n", (nearest.curve != -1) ? curves[nearest.curve].id : "None found");
				fprintf(outputFile, "True Nearest neighbor: %s\n", curves[trueNearest].id);
				fprintf(outputFile, "distanceLSH: %lf\n", nearest.distance);
				fprintf(outputFile, "distanceTrue: %lf\n", trueNearestDistance);
			
				if(radius > 0)
				{
					sort(rCurves, numOfRCurves, curves);
					fprintf(outputFile, "R-near neighbors:\n");
					for(j=0; j<numOfRCurves; j++)
					{
						fprintf(outputFile, "%s\n", curves[rCurves[j]].id);
					}
				}
			}
			
			if(stats && VERBOSE)
			{
				putchar('|');
				fflush(stdout);
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
				for(j=0; j<K_VEC; j++)
				{
					free(hashInfo[i].gParameters[j].v->coordinates);
					free(hashInfo[i].gParameters[j].v);
				}
				free(hashInfo[i].gParameters);
			}
		}
		free(hashInfo);
		free(rCurves);
		rCurves = NULL;
		
	}
	
	//Print stats
	if(stats)
	{
		if(VERBOSE)
			printf("\n");
		for(i=0; i<queriesNum; i++)
		{
			fprintf(outputFile, "Query: %s\n", queries[i].id);
			fprintf(outputFile, "DistanceFunction: %s\n", (distanceFunction == dtw) ? "DTW" : "DFT");
			fprintf(outputFile, "HashFunction: %s\n", (hashType == 'c') ? "Classic" : "Probabilistic");
			fprintf(outputFile, "|minDistanceLSH – distanceTrue|: %lf\n", queryStats[i].minDistance - queryStats[i].trueNearestDistance);
			fprintf(outputFile, "|maxDistanceLSH – distanceTrue|: %lf\n", queryStats[i].maxDistance - queryStats[i].trueNearestDistance);
			fprintf(outputFile, "|avgDistanceLSH – distanceTrue|: %lf\n", (queryStats[i].sumDistance / 100) - queryStats[i].trueNearestDistance);
			fprintf(outputFile, "tLSHmin: %lf\n", queryStats[i].tLSHmin);
			fprintf(outputFile, "tLSHmax: %lf\n", queryStats[i].tLSHmax);
			fprintf(outputFile, "tLSHavg: %lf\n", queryStats[i].tLSHsum / 100);
			fprintf(outputFile, "tTrue: %lf\n", queryStats[i].tTrue);
		}
	}
	
	//Full cleanup
	fclose(outputFile);
	free(queryStats);
	for(i=0; i<curvesNum; i++)
	{
		free(curves[i].id);
		free(curves[i].points);
	}
	free(curves);
	for(i=0; i<queriesNum; i++)
	{
		free(queries[i].id);
		free(queries[i].points);
	}
	free(queries);
	
	return 0;
	
}

