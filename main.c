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

void readConf(FILE *confFile, int *k_of_means_fame, int *k, int *l)
{
	int valueGiven;
	
	valueGiven = fscanf(confFile, "number_of_clusters: %d\n", k_of_means_fame);
	if(!valueGiven)
		*k_of_means_fame = 5;
	valueGiven = fscanf(confFile, "number_of_grid_curves: %d\n", k);
	if(!valueGiven)
		*k = 2;
	valueGiven = fscanf(confFile, "number_of_hash_tables: %d", l);
	if(!valueGiven)
		*l = 3;
}

int main(int argc, char *argv[])
{
	int i, j;
	int d, curvesNum;
	FILE *inputFile=NULL, *outputFile=stdout;
	Curve *curves;
	int *clusters, *previousClusters, *centroids, *curvesInClusters;
	int k_of_means_fame = 5;
	double **distances;
	double (*distanceFunction)(Curve*, Curve*) = (*cRMSD);
	int assignments;
	double s_i, s_total;
	
	inputFile = fopen("bio_small_input.dat", "r");
	
	//Get command line arguments
	for(i=1; i<argc; i+=2)
	{
		if(!strcmp(argv[i], "-i"))
			inputFile = fopen(argv[i+1], "r");
		else if(!strcmp(argv[i], "-o"))
			outputFile = fopen(argv[i+1], "w");
		else if(!strcmp(argv[i], "-k"))
			k_of_means_fame = atoi(argv[i+1]);
		else if(!strcmp(argv[i], "-d"))
			distanceFunction = (!strcmp(argv[i+1], "Frechet")) ? trDFD : cRMSD;
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
		outputFile = fopen("crmsd.dat", "w");
	
	srand(time(NULL));
	
	
	//Read curves
	curves = readCurves(inputFile, &d, &curvesNum, NULL);
	
	
	
	
	
	
	//test
	trDFD(&(curves[0]), &(curves[1]));
	
	
	
	
	
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
	
	free(centroids);
	free(clusters);
	free(previousClusters);
	
	
	
	
	
	
	fclose(inputFile);
	
	return 0;
	
}

/*
int main2(int argc, char *argv[])
{
	int i, j, m;
	FILE *inputFile=NULL, *confFile=NULL, *outputFile=NULL;
	double (*distanceFunction)(Curve*, Curve*) = (*dfd);
	int dimension = 2;
	int curvesNum;
	Curve *curves;
	int maxCurveLength;
	int maxGridCurveLength;
	int k, l, k_of_means_fame;
	char hashType = 'p';
	HashInfo *hashInfo;
	Vector *u, *g_u;
	double mU, mV, mS;
	int *centroids;
	int *clusters, *previousClusters;
	double **distances;
	int assignments;
	int complete = 0;
	int *curvesInClusters;
	Curve *curCurve;
	double s_i;
	double s_total;
	int start, end;
	
	//Get command line arguments
	for(i=1; i<argc; i+=2)
	{
		if(!strcmp(argv[i], "-i"))
			inputFile = fopen(argv[i+1], "r");
		else if(!strcmp(argv[i], "-c"))
			confFile = fopen(argv[i+1], "r");
		else if(!strcmp(argv[i], "-o"))
			outputFile = fopen(argv[i+1], "w");
		else if(!strcmp(argv[i], "-d"))
			distanceFunction = (!strcmp(argv[i+1], "Frechet")) ? dfd : dtw;
		else if(!strcmp(argv[i], "-complete"))
			complete = 1;
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
	if(confFile == NULL)
	{
		printf("Invalid or no configuration file given.\n");
		return 0;
	}
	if(outputFile == NULL)
		outputFile = stdout;
	
	srand(time(NULL));
	
	//Read configuration file
	readConf(confFile, &k_of_means_fame, &k, &l);
	fclose(confFile);
	
	//Read input file
	curves = readCurves(inputFile, &dimension, &curvesNum, NULL);
	fclose(inputFile);
	
	//Find max curve length
	maxCurveLength = 0;
	for(i=0; i<curvesNum; i++)
		maxCurveLength = (curves[i].numOfPoints > maxCurveLength) ? curves[i].numOfPoints : maxCurveLength;
	maxGridCurveLength = maxCurveLength * dimension * k;
	
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
	
	//K-Means
	distances = malloc(curvesNum*sizeof(double*));
	for(i=0; i<curvesNum; i++)
	{
		distances[i] = malloc(i*sizeof(double));
		for(j=0; j<i; j++)
			distances[i][j] = INFINITY;
	}
	curvesInClusters = malloc(k_of_means_fame*sizeof(int));
	curves = realloc(curves, (curvesNum+k_of_means_fame)*sizeof(Curve)); //To keep the mean Frechet curves
	
	
	//I1A1U1
	//Initialization
	if(distanceFunction == dfd)
	{
		start = time(NULL);
		
		clusters = malloc(curvesNum*sizeof(int));
		previousClusters = malloc(curvesNum*sizeof(int));
		centroids = k_means_pp(k_of_means_fame, curvesNum, curves, distances);
		
		//Assignment
		lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
		
		for(j=0; j<MAX_LOOPS; j++)
		{
			memcpy(previousClusters, clusters, curvesNum*sizeof(int));
			meanFrechet(curves, curvesNum, clusters, centroids, k_of_means_fame);
			lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
			assignments = 0;
			for(i=0; i<curvesNum; i++)
				if(clusters[i] != previousClusters[i])
					assignments++;
			
			if(assignments < (curvesNum*END_FACTOR))
				break;
		}
		
		end = time(NULL);
		
		//Find size of clusters
		for(i=0; i<k_of_means_fame; i++)
			curvesInClusters[i] = 0;
		for(i=0; i<curvesNum; i++)
			curvesInClusters[clusters[i]]++;
		
		//Print results
		fprintf(outputFile, "Algorithm: I1A1U1\n");
		fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: [", i, curvesInClusters[i]);
			curCurve = &(curves[centroids[i]]);
			switch(dimension)
			{
				case 2:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y);
					break;
				case 3:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z);
					break;
				case 4:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z, curCurve->points[j].w);
					break;
			}
			fprintf(outputFile, "\b]}\n");
		}
		
		fprintf(outputFile, "clustering_time: %d\n", end-start);
		
		s_total=0;
		fprintf(outputFile, "Silhouette: [");
		for(i=0; i<k_of_means_fame; i++)
		{
			s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
			fprintf(outputFile, "%lf,", s_i);
			s_total += s_i*curvesInClusters[i];
		}
		fprintf(outputFile, "%lf]\n", s_total/curvesNum);
		
		if(complete)
		{
			for(i=0; i<k_of_means_fame; i++)
			{
				fprintf(outputFile, "CLUSTER-%d {", i);
				for(j=0; j<curvesNum; j++)
				{
					if(clusters[j] == i)
						fprintf(outputFile, "%s, ", curves[j].id);
				}
				fprintf(outputFile, "\b\b}\n");
			}
		}
		
		free(centroids);
		free(clusters);
		free(previousClusters);
		
		for(i=curvesNum; i<curvesNum+k_of_means_fame; i++)
			free(curves[i].points);
	}
	
	//I1A1U2
	//Initialization
	
	start = time(NULL);
	
	clusters = malloc(curvesNum*sizeof(int));
	previousClusters = malloc(curvesNum*sizeof(int));
	centroids = k_means_pp(k_of_means_fame, curvesNum, curves, distances);
	
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
	end = time(NULL);
	
	//Find size of clusters
	for(i=0; i<k_of_means_fame; i++)
		curvesInClusters[i] = 0;
	for(i=0; i<curvesNum; i++)
		curvesInClusters[clusters[i]]++;
	
	//Print results
	fprintf(outputFile, "Algorithm: I1A1U2\n");
	fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
	for(i=0; i<k_of_means_fame; i++)
		fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: %s}\n", i, curvesInClusters[i], curves[centroids[i]].id);
		
	fprintf(outputFile, "clustering_time: %d\n", end-start);
	
	s_total=0;
	fprintf(outputFile, "Silhouette: [");
	for(i=0; i<k_of_means_fame; i++)
	{
		s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
		fprintf(outputFile, "%lf,", s_i);
		s_total += s_i*curvesInClusters[i];
	}
	fprintf(outputFile, "%lf]\n", s_total/curvesNum);
	
	if(complete)
	{
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {", i);
			for(j=0; j<curvesNum; j++)
			{
				if(clusters[j] == i)
					fprintf(outputFile, "%s, ", curves[j].id);
			}
			fprintf(outputFile, "\b\b}\n");
		}
	}
	
	free(centroids);
	free(clusters);
	free(previousClusters);
	
	//I1A2U1
	//Initialization
	if(distanceFunction == dfd)
	{
		start = time(NULL);
		
		clusters = malloc(curvesNum*sizeof(int));
		previousClusters = malloc(curvesNum*sizeof(int));
		centroids = k_means_pp(k_of_means_fame, curvesNum, curves, distances);
		
		//Assignment
		range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
					distanceFunction, hashInfo, l, k, dimension);
		assignments = curvesNum;
		for(j=0; j<MAX_LOOPS; j++)
		{
			memcpy(previousClusters, clusters, curvesNum*sizeof(int));
			meanFrechet(curves, curvesNum, clusters, centroids, k_of_means_fame);
			range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
						distanceFunction, hashInfo, l, k, dimension);
			assignments = 0;
			for(i=0; i<curvesNum; i++)
				if(clusters[i] != previousClusters[i])
					assignments++;
			
			if(assignments < (curvesNum*END_FACTOR))
				break;
		}
		end = time(NULL);
		
		//Find size of clusters
		for(i=0; i<k_of_means_fame; i++)
			curvesInClusters[i] = 0;
		for(i=0; i<curvesNum; i++)
			curvesInClusters[clusters[i]]++;
		
		//Print results
		fprintf(outputFile, "Algorithm: I1A2U1\n");
		fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: [", i, curvesInClusters[i]);
			curCurve = &(curves[centroids[i]]);
			switch(dimension)
			{
				case 2:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y);
					break;
				case 3:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z);
					break;
				case 4:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z, curCurve->points[j].w);
					break;
			}
			fprintf(outputFile, "\b]}\n");
		}
		
		fprintf(outputFile, "clustering_time: %d\n", end-start);
		
		s_total=0;
		fprintf(outputFile, "Silhouette: [");
		for(i=0; i<k_of_means_fame; i++)
		{
			s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
			fprintf(outputFile, "%lf,", s_i);
			s_total += s_i*curvesInClusters[i];
		}
		fprintf(outputFile, "%lf]\n", s_total/curvesNum);
		
		if(complete)
		{
			for(i=0; i<k_of_means_fame; i++)
			{
				fprintf(outputFile, "CLUSTER-%d {", i);
				for(j=0; j<curvesNum; j++)
				{
					if(clusters[j] == i)
						fprintf(outputFile, "%s, ", curves[j].id);
				}
				fprintf(outputFile, "\b\b}\n");
			}
		}
		
		free(centroids);
		free(clusters);
		free(previousClusters);
		
		for(i=curvesNum; i<curvesNum+k_of_means_fame; i++)
			free(curves[i].points);
	}
	
	//I1A2U2
	//Initialization
	
	start = time(NULL);
	
	clusters = malloc(curvesNum*sizeof(int));
	previousClusters = malloc(curvesNum*sizeof(int));
	centroids = k_means_pp(k_of_means_fame, curvesNum, curves, distances);
	
	//Assignment
	range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
				distanceFunction, hashInfo, l, k, dimension);
	
	for(j=0; j<MAX_LOOPS; j++)
	{
		memcpy(previousClusters, clusters, curvesNum*sizeof(int));
		pam(curves, curvesNum, clusters, centroids, k_of_means_fame, distances, distanceFunction);
		range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
					distanceFunction, hashInfo, l, k, dimension);
		assignments = 0;
		for(i=0; i<curvesNum; i++)
			if(clusters[i] != previousClusters[i])
				assignments++;
		
		if(assignments < (curvesNum*END_FACTOR))
			break;
	}
	end = time(NULL);
	
	//Find size of clusters
	for(i=0; i<k_of_means_fame; i++)
		curvesInClusters[i] = 0;
	for(i=0; i<curvesNum; i++)
		curvesInClusters[clusters[i]]++;
	
	//Print results
	fprintf(outputFile, "Algorithm: I1A2U2\n");
	fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
	for(i=0; i<k_of_means_fame; i++)
		fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: %s}\n", i, curvesInClusters[i], curves[centroids[i]].id);
		
	fprintf(outputFile, "clustering_time: %d\n", end-start);
	
	s_total=0;
	fprintf(outputFile, "Silhouette: [");
	for(i=0; i<k_of_means_fame; i++)
	{
		s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
		fprintf(outputFile, "%lf,", s_i);
		s_total += s_i*curvesInClusters[i];
	}
	fprintf(outputFile, "%lf]\n", s_total/curvesNum);
	
	if(complete)
	{
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {", i);
			for(j=0; j<curvesNum; j++)
			{
				if(clusters[j] == i)
					fprintf(outputFile, "%s, ", curves[j].id);
			}
			fprintf(outputFile, "\b\b}\n");
		}
	}
	
	free(centroids);
	free(clusters);
	free(previousClusters);
	
	//I2A1U1
	//Initialization
	if(distanceFunction == dfd)
	{
		start = time(NULL);
		
		clusters = malloc(curvesNum*sizeof(int));
		previousClusters = malloc(curvesNum*sizeof(int));
		centroids = random_selection(k_of_means_fame, curvesNum, curves, distances);
		
		//Assignment
		lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
		
		for(j=0; j<MAX_LOOPS; j++)
		{
			memcpy(previousClusters, clusters, curvesNum*sizeof(int));
			meanFrechet(curves, curvesNum, clusters, centroids, k_of_means_fame);
			lloyds(curves, curvesNum, centroids, clusters, k_of_means_fame, distances, distanceFunction);
			assignments = 0;
			for(i=0; i<curvesNum; i++)
				if(clusters[i] != previousClusters[i])
					assignments++;
			
			if(assignments < (curvesNum*END_FACTOR))
				break;
		}
		end = time(NULL);
		
		//Find size of clusters
		for(i=0; i<k_of_means_fame; i++)
			curvesInClusters[i] = 0;
		for(i=0; i<curvesNum; i++)
			curvesInClusters[clusters[i]]++;
		
		//Print results
		fprintf(outputFile, "Algorithm: I2A1U1\n");
		fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: [", i, curvesInClusters[i]);
			curCurve = &(curves[centroids[i]]);
			switch(dimension)
			{
				case 2:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y);
					break;
				case 3:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z);
					break;
				case 4:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z, curCurve->points[j].w);
					break;
			}
			fprintf(outputFile, "\b]}\n");
		}
		
		fprintf(outputFile, "clustering_time: %d\n", end-start);
		
		s_total=0;
		fprintf(outputFile, "Silhouette: [");
		for(i=0; i<k_of_means_fame; i++)
		{
			s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
			fprintf(outputFile, "%lf,", s_i);
			s_total += s_i*curvesInClusters[i];
		}
		fprintf(outputFile, "%lf]\n", s_total/curvesNum);
		
		if(complete)
		{
			for(i=0; i<k_of_means_fame; i++)
			{
				fprintf(outputFile, "CLUSTER-%d {", i);
				for(j=0; j<curvesNum; j++)
				{
					if(clusters[j] == i)
						fprintf(outputFile, "%s, ", curves[j].id);
				}
				fprintf(outputFile, "\b\b}\n");
			}
		}
		
		free(centroids);
		free(clusters);
		free(previousClusters);
		
		for(i=curvesNum; i<curvesNum+k_of_means_fame; i++)
			free(curves[i].points);
	}
	
	//I2A1U2
	//Initialization
	start = time(NULL);
	
	clusters = malloc(curvesNum*sizeof(int));
	previousClusters = malloc(curvesNum*sizeof(int));
	centroids = random_selection(k_of_means_fame, curvesNum, curves, distances);
	
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
	end = time(NULL);
	
	//Find size of clusters
	for(i=0; i<k_of_means_fame; i++)
		curvesInClusters[i] = 0;
	for(i=0; i<curvesNum; i++)
		curvesInClusters[clusters[i]]++;
	
	//Print results
	fprintf(outputFile, "Algorithm: I2A1U2\n");
	fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
	for(i=0; i<k_of_means_fame; i++)
		fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: %s}\n", i, curvesInClusters[i], curves[centroids[i]].id);
		
	fprintf(outputFile, "clustering_time: %d\n", end-start);
	
	s_total=0;
	fprintf(outputFile, "Silhouette: [");
	for(i=0; i<k_of_means_fame; i++)
	{
		s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
		fprintf(outputFile, "%lf,", s_i);
		s_total += s_i*curvesInClusters[i];
	}
	fprintf(outputFile, "%lf]\n", s_total/curvesNum);
	
	if(complete)
	{
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {", i);
			for(j=0; j<curvesNum; j++)
			{
				if(clusters[j] == i)
					fprintf(outputFile, "%s, ", curves[j].id);
			}
			fprintf(outputFile, "\b\b}\n");
		}
	}
	
	free(centroids);
	free(clusters);
	free(previousClusters);
	
	//I2A2U1
	//Initialization
	if(distanceFunction == dfd)
	{
		start = time(NULL);
		
		clusters = malloc(curvesNum*sizeof(int));
		previousClusters = malloc(curvesNum*sizeof(int));
		centroids = random_selection(k_of_means_fame, curvesNum, curves, distances);
		
		//Assignment
		range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
					distanceFunction, hashInfo, l, k, dimension);
		assignments = curvesNum;
		for(j=0; j<MAX_LOOPS; j++)
		{
			memcpy(previousClusters, clusters, curvesNum*sizeof(int));
			meanFrechet(curves, curvesNum, clusters, centroids, k_of_means_fame);
			range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
						distanceFunction, hashInfo, l, k, dimension);
			assignments = 0;
			for(i=0; i<curvesNum; i++)
				if(clusters[i] != previousClusters[i])
					assignments++;
			
			if(assignments < (curvesNum*END_FACTOR))
				break;
		}
		end = time(NULL);
		
		//Find size of clusters
		for(i=0; i<k_of_means_fame; i++)
			curvesInClusters[i] = 0;
		for(i=0; i<curvesNum; i++)
			curvesInClusters[clusters[i]]++;
		
		//Print results
		fprintf(outputFile, "Algorithm: I2A2U1\n");
		fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: [", i, curvesInClusters[i]);
			curCurve = &(curves[centroids[i]]);
			switch(dimension)
			{
				case 2:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y);
					break;
				case 3:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z);
					break;
				case 4:
					for(j=0; j<curCurve->numOfPoints; j++)
						fprintf(outputFile, "(%lf,%lf,%lf,%lf),", curCurve->points[j].x, curCurve->points[j].y,
						curCurve->points[j].z, curCurve->points[j].w);
					break;
			}
			fprintf(outputFile, "\b]}\n");
		}
		
		fprintf(outputFile, "clustering_time: %d\n", end-start);
		
		s_total=0;
		fprintf(outputFile, "Silhouette: [");
		for(i=0; i<k_of_means_fame; i++)
		{
			s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
			fprintf(outputFile, "%lf,", s_i);
			s_total += s_i*curvesInClusters[i];
		}
		fprintf(outputFile, "%lf]\n", s_total/curvesNum);
		
		if(complete)
		{
			for(i=0; i<k_of_means_fame; i++)
			{
				fprintf(outputFile, "CLUSTER-%d {", i);
				for(j=0; j<curvesNum; j++)
				{
					if(clusters[j] == i)
						fprintf(outputFile, "%s, ", curves[j].id);
				}
				fprintf(outputFile, "\b\b}\n");
			}
		}
		
		free(centroids);
		free(clusters);
		free(previousClusters);
		
		for(i=curvesNum; i<curvesNum+k_of_means_fame; i++)
			free(curves[i].points);
	}
	
	//I2A2U2
	//Initialization
	start = time(NULL);
	
	clusters = malloc(curvesNum*sizeof(int));
	previousClusters = malloc(curvesNum*sizeof(int));
	centroids = random_selection(k_of_means_fame, curvesNum, curves, distances);
	
	//Assignment
	range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
				distanceFunction, hashInfo, l, k, dimension);
	
	for(j=0; j<MAX_LOOPS; j++)
	{
		memcpy(previousClusters, clusters, curvesNum*sizeof(int));
		pam(curves, curvesNum, clusters, centroids, k_of_means_fame, distances, distanceFunction);
		range_search(curves, curvesNum, centroids, clusters, k_of_means_fame, distances,
					distanceFunction, hashInfo, l, k, dimension);
		assignments = 0;
		for(i=0; i<curvesNum; i++)
			if(clusters[i] != previousClusters[i])
				assignments++;
		
		if(assignments < (curvesNum*END_FACTOR))
			break;
	}
	end = time(NULL);
	
	//Find size of clusters
	for(i=0; i<k_of_means_fame; i++)
		curvesInClusters[i] = 0;
	for(i=0; i<curvesNum; i++)
		curvesInClusters[clusters[i]]++;
	
	//Print results
	fprintf(outputFile, "Algorithm: I2A2U2\n");
	fprintf(outputFile, "Metric: %s\n", (distanceFunction == dfd) ? "Frechet" : "DTW");
	for(i=0; i<k_of_means_fame; i++)
		fprintf(outputFile, "CLUSTER-%d {size: %d, centroid: %s}\n", i, curvesInClusters[i], curves[centroids[i]].id);
		
	fprintf(outputFile, "clustering_time: %d\n", end-start);
	
	s_total=0;
	fprintf(outputFile, "Silhouette: [");
	for(i=0; i<k_of_means_fame; i++)
	{
		s_i = silhouette(curves, curvesNum, distances, centroids, clusters, i, k_of_means_fame, curvesInClusters, distanceFunction);
		fprintf(outputFile, "%lf,", s_i);
		s_total += s_i*curvesInClusters[i];
	}
	fprintf(outputFile, "%lf]\n", s_total/curvesNum);
	
	if(complete)
	{
		for(i=0; i<k_of_means_fame; i++)
		{
			fprintf(outputFile, "CLUSTER-%d {", i);
			for(j=0; j<curvesNum; j++)
			{
				if(clusters[j] == i)
					fprintf(outputFile, "%s, ", curves[j].id);
			}
			fprintf(outputFile, "\b\b}\n");
		}
	}
	
	free(centroids);
	free(clusters);
	free(previousClusters);
	
	//Full cleanup
	fclose(outputFile);
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
	for(i=0; i<curvesNum; i++)
	{
		free(curves[i].id);
		free(curves[i].points);
		free(distances[i]);
	}
	free(curves);
	free(distances);
	free(curvesInClusters);
	
	return 0;
}
*/
