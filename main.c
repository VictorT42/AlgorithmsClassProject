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

#define END_FACTOR 0.01
#define MAX_LOOPS 20

int main(int argc, char *argv[])
{
	int i, j;
	int dimension, curvesNum;
	FILE *inputFile=NULL, *outputFile=stdout;
	Curve *curves;
	int *clusters, *previousClusters=NULL, *centroids, *curvesInClusters;
	int k_of_means_fame = 5;
	double **distances;
	double (*distanceFunction)(Curve*, Curve*) = (*trDFD);
	int assignments;
	double s_i, s_total;
	char data = 'r', clustering = 'k';
	HashInfo *hashInfo;
	Vector *u, *g_u;
	double mU, mV, mS;
	int m, l=1, k=5;
	int maxCurveLength, maxGridCurveLength;
	struct timespec start = {0,0}, end = {0,0};
	
	//Get command line arguments
	for(i=1; i<argc; i+=2)
	{
		if(!strcmp(argv[i], "-i"))
			inputFile = fopen(argv[i+1], "r");
		else if(!strcmp(argv[i], "-o"))
			outputFile = fopen(argv[i+1], "w");
		else if(!strcmp(argv[i], "-k"))
			k_of_means_fame = atoi(argv[i+1]);
		else if(!strcmp(argv[i], "-t"))
			data = (!strcmp(argv[i+1], "proteins")) ? 'p' : 'r';
		else if(!strcmp(argv[i], "-c"))
			clustering = (!strcmp(argv[i+1], "LSH")) ? 'l' : 'k';
		else if(!strcmp(argv[i], "-d"))
			if(!strcmp(argv[i+1], "Frechet"))
				distanceFunction = trDFD;
			else if(!strcmp(argv[i+1], "DTW"))
				distanceFunction = trDTW;
			else
				distanceFunction = cRMSD;
		else
		{
			printf("Invalid argument %s\n", argv[i]);
			i--;
		}
	}
	
	if(inputFile == NULL)
	{
		printf("Invalid or no input file given.\n");
		return 0;
	}
	if(outputFile == NULL)
		outputFile = stdout;
	
	srand(time(NULL));
	
	if(data == 'p')
	{
		if(distanceFunction == dtw)
			distanceFunction = cRMSD;
		
		//Read curves
		curves = readCurves(inputFile, &dimension, &curvesNum, NULL);
		
		//K-Means
		distances = malloc(curvesNum*sizeof(double*));
		for(i=0; i<curvesNum; i++)
		{
			distances[i] = malloc(i*sizeof(double));
			for(j=0; j<i; j++)
				distances[i][j] = INFINITY;
		}
		curvesInClusters = malloc(k_of_means_fame*sizeof(int));
		
		//Initialization
		clusters = malloc(curvesNum*sizeof(int));
		previousClusters = malloc(curvesNum*sizeof(int));
		centroids = k_means_pp(k_of_means_fame, curvesNum, curves, distances, distanceFunction);
		
		//Assignment
		lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
		
		for(j=0; j<MAX_LOOPS; j++)
		{
			memcpy(previousClusters, clusters, curvesNum*sizeof(int));
			pam(curves, curvesNum, clusters, centroids, k_of_means_fame, distances, distanceFunction);
			lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
			assignments = 0;
			for(i=0; i<curvesNum; i++)
				if(clusters[i] != previousClusters[i])
					assignments++;
			
			if(assignments < (curvesNum*END_FACTOR))
				break;
		}
		
		//Find size of clusters
		for(i=0; i<k_of_means_fame; i++)
			curvesInClusters[i] = 0;
		for(i=0; i<curvesNum; i++)
			curvesInClusters[clusters[i]]++;
		
		//Print results
		fprintf(outputFile, "k: %d\n", k_of_means_fame);
		
		s_total=0;
		for(i=0; i<k_of_means_fame; i++)
		{
			s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
			s_total += s_i*curvesInClusters[i];
		}
		fprintf(outputFile, "s: %lf\n", s_total/curvesNum);
		
		for(i=0; i<k_of_means_fame; i++)
		{
			for(j=0; j<curvesNum; j++)
			{
				if(clusters[j] == i)
					fprintf(outputFile, "%d\t", j+1);
			}
			fprintf(outputFile, "\n");
		}
	}
	else
	{
		if(distanceFunction == cRMSD)
			distanceFunction = dfd;
		
		//Read curves
		curves = readRoads(inputFile, &dimension, &curvesNum, NULL);
		
		if(clustering == 'k')
		{
			clock_gettime(CLOCK_MONOTONIC, &start);
			
			//K-Means
			distances = malloc(curvesNum*sizeof(double*));
			for(i=0; i<curvesNum; i++)
			{
				distances[i] = malloc(i*sizeof(double));
				for(j=0; j<i; j++)
					distances[i][j] = INFINITY;
			}
			curvesInClusters = malloc(k_of_means_fame*sizeof(int));
			
			//Initialization
			clusters = malloc(curvesNum*sizeof(int));
			previousClusters = malloc(curvesNum*sizeof(int));
			centroids = k_means_pp(k_of_means_fame, curvesNum, curves, distances, distanceFunction);
			
			//Assignment
			lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
			
			for(j=0; j<MAX_LOOPS; j++)
			{
				memcpy(previousClusters, clusters, curvesNum*sizeof(int));
				pam(curves, curvesNum, clusters, centroids, k_of_means_fame, distances, distanceFunction);
				lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
				assignments = 0;
				for(i=0; i<curvesNum; i++)
					if(clusters[i] != previousClusters[i])
						assignments++;
				
				if(assignments < (curvesNum*END_FACTOR))
					break;
			}
			
			//Find size of clusters
			for(i=0; i<k_of_means_fame; i++)
				curvesInClusters[i] = 0;
			for(i=0; i<curvesNum; i++)
				curvesInClusters[clusters[i]]++;
			
			clock_gettime(CLOCK_MONOTONIC, &end);
			
			//Print results
			fprintf(outputFile, "k: %d\n", k_of_means_fame);
			
			s_total=0;
			for(i=0; i<k_of_means_fame; i++)
			{
				s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
				s_total += s_i*curvesInClusters[i];
			}
			fprintf(outputFile, "s: %lf\n", s_total/curvesNum);
			
			fprintf(outputFile, "clustering time: %lf\n", (end.tv_sec-start.tv_sec)*1000+(end.tv_nsec - start.tv_nsec) / (double) 1000000);
			
			for(i=0; i<k_of_means_fame; i++)
			{
				for(j=0; j<curvesNum; j++)
				{
					if(clusters[j] == i)
						fprintf(outputFile, "%d\t", j+1);
				}
				fprintf(outputFile, "\n");
			}
		}
		else
		{
			clock_gettime(CLOCK_MONOTONIC, &start);
			
			distances = malloc(curvesNum*sizeof(double*));
			for(i=0; i<curvesNum; i++)
			{
				distances[i] = malloc(i*sizeof(double));
				for(j=0; j<i; j++)
					distances[i][j] = INFINITY;
			}
			centroids = malloc(k_of_means_fame*sizeof(int));
			curvesInClusters = malloc(k_of_means_fame*sizeof(int));
			for(i=0; i<k_of_means_fame; i++)
				curvesInClusters[i] = 0;
			
			//Find max curve length
			maxCurveLength = 0;
			for(i=0; i<curvesNum; i++)
				maxCurveLength = (curves[i].numOfPoints > maxCurveLength) ? curves[i].numOfPoints : maxCurveLength;
			maxGridCurveLength = maxCurveLength * dimension * k;
			
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
				
				hashInfo[m].hashTable = createHashTable(k_of_means_fame);
				
				clusters = malloc(curvesNum*sizeof(int));
				for(i=0; i<curvesNum; i++)
				{
					u = snapToGrid(&(curves[i]), k, dimension, hashInfo[m].grids);
					g(g_u, u, hashInfo[m].gParameters);
					clusters[i] = insertToTable(hashInfo[m].hashTable, g_u, u, i, hashInfo[m].r);
					centroids[clusters[i]] = i;
					curvesInClusters[clusters[i]]++;
				}
			}
			
			free(g_u->coordinates);
			free(g_u);
			
			clock_gettime(CLOCK_MONOTONIC, &end);
			
			//Print results
			fprintf(outputFile, "k: %d\n", k_of_means_fame);
			
			fprintf(outputFile, "clustering time: %lf\n", (end.tv_sec-start.tv_sec)*1000+(end.tv_nsec - start.tv_nsec) / (double) 1000000);
			
			//Run an update to find centroids for the LSH clusters
			pam(curves, curvesNum, clusters, centroids, k_of_means_fame, distances, distanceFunction);
			
			s_total=0;
			for(i=0; i<k_of_means_fame; i++)
			{
				s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
				s_total += s_i*curvesInClusters[i];
			}
			
			fprintf(outputFile, "s: %lf\n", s_total/curvesNum);
			
			for(i=0; i<k_of_means_fame; i++)
			{
				for(j=0; j<curvesNum; j++)
				{
					if(clusters[j] == i)
						fprintf(outputFile, "%d\t", j+1);
				}
				fprintf(outputFile, "\n");
			}
		}
	}
	
	//Full cleanup
	fclose(inputFile);
	fclose(outputFile);
	
	if(clustering == 'l')
	{
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
	}
	free(curvesInClusters);
	free(centroids);
	free(clusters);	
	free(previousClusters);
	for(i=0; i<curvesNum; i++)
	{
		free(curves[i].id);
		free(curves[i].points);
		free(distances[i]);
	}
	free(curves);
	free(distances);
	
	return 0;
	
}

