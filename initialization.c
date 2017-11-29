//File initialization.c

#include <stdlib.h>
#include <math.h>
#include "curves.h"
#include "metrics.h"

#define P_ACCURACY 1000000
#define CENTROID -1

int *random_selection(int k, int numOfCurves, Curve *curves, double **distances)
{
	int i, j;
	int *centroids;
	int same;
	
	centroids = malloc(k * sizeof(int));
	
	for(i=0; i<k; i++)
	{
		same = 1;
		while(same == 1)
		{
			centroids[i] = rand() % numOfCurves;
			same = 0;
			for(j=0; j<i; j++)
			{
				if(centroids[i] == centroids[j])
				{
					same = 1;
					break;
				}
			}
		}
	}
	
	return centroids;
	
}

int *k_means_pp(int k, int numOfCurves, Curve *curves, double **distances)
{
	int i, j, l;
	int *centroids;
	double *D, *P;
	double x;
	int r;
	int same;
	double *tempDistance;
	
	centroids = malloc(k * sizeof(int));
	D = malloc(numOfCurves * sizeof(double));
	P = malloc(numOfCurves * sizeof(double));
	
	for(i=0; i<numOfCurves; i++)
		D[i] = INFINITY;
	
	centroids[0] = rand() % numOfCurves;
	for(i=1; i<k; i++)
	{
		for(j=0; j<numOfCurves; j++)
		{
			same = 0;
			for(l=0; l<i; l++)
			{
				if(j == centroids[l])
				{
					same = 1;
					break;
				}
			}
			if(same)
			{
				P[j] = CENTROID;
				continue;
			}
			
			if(centroids[i-1] > j)
				tempDistance = &(distances[centroids[i-1]][j]);
			else
				tempDistance = &(distances[j][centroids[i-1]]);
			*tempDistance = dfd(&(curves[j]), &(curves[centroids[i-1]]));
			if(*tempDistance < D[j])
				D[j] = *tempDistance;
			
			P[j] = D[j] * D[j];
			l = j-1;
			while(l >= 0 && P[l] == CENTROID)
				l--;
			if(l != -1)
				P[j] += P[l];
		}
		
		for(j=numOfCurves-1; j>0; j--)  //Find the last non-centroid point 
			if(P[j] != CENTROID)
				break;
			
		x = rand() % (int) (P_ACCURACY * P[j]);
		x = x / (double) (P_ACCURACY);
		
		for(r=0; r<numOfCurves; r++)
		{
			if(x <= P[r])
				break;
		}
		centroids[i] = r;
	}
	
	free(D);
	free(P);
	
	return centroids;
	
}

