//File assignment.c

#include <stdlib.h>
#include <math.h>
#include "curves.h"
#include "hash.h"

void lloyds(Curve *curves, int curvesNum, int *centroids, int *clusters, int k, double **distances, double(*distanceFunction)(Curve*, Curve*))
{
	int i, j;
	int isCentroid;
	double *tempDistance;
	double tempDistance2;
	double minDistance;
	
	for(i=0; i<curvesNum; i++)
	{
		//Check if centroid
		isCentroid = 0;
		for(j=0; j<k; j++)
		{
			if(i==centroids[j])
			{
				isCentroid = 1;
				clusters[i] = j;
				break;
			}
			
		}
		if(isCentroid)
			continue;
		
		minDistance = INFINITY;
		for(j=0; j<k; j++)
		{
			if(centroids[j] < curvesNum)
			{
				if(i>centroids[j])
					tempDistance = &(distances[i][centroids[j]]);
				else
					tempDistance = &(distances[centroids[j]][i]);
				
				if(*tempDistance == INFINITY)
					*tempDistance = distanceFunction(&(curves[i]), &(curves[centroids[j]]));
			}
			else
			{
				tempDistance = &tempDistance2;
				*tempDistance = distanceFunction(&(curves[i]), &(curves[centroids[j]]));
			}
			
			if(*tempDistance < minDistance)
			{
				minDistance = *tempDistance;
				clusters[i] = j;
			}
		}
		
	}
}

void range_search(Curve *curves, int curvesNum, int *centroids, int *clusters, int k, double **distances,
	double(*distanceFunction)(Curve*, Curve*), HashInfo *hashInfo, int l, int grid_k, int dimension)
{
	int i,j, m, n;
	double *tempDistance, *tempDistance2;
	double tempDistance3, tempDistance4;
	double r;
	int assignCounter;
	int *assigned;
	int hasMadeAssignment;
	int round;
	int **centroidBuckets;
	Vector *u;
	Vector g_u;
	Bucket *currentBucket;
	int currentPoint;
	int isCentroid;
	double minDistance;
	
	//Find the centroid buckets
	g_u.length = K_VEC;
	g_u.coordinates = malloc(g_u.length * sizeof(double));
	centroidBuckets = malloc(k*sizeof(int*));
	for(i=0; i<k; i++)
		centroidBuckets[i] = malloc(l*sizeof(int));
	for(i=0; i<k; i++)
	{
		for(j=0; j<l; j++)
		{
			u = snapToGrid(&(curves[centroids[i]]), grid_k, dimension, hashInfo[j].grids);
			g(&g_u, u, hashInfo[j].gParameters);
			centroidBuckets[i][j] = hash(&g_u, hashInfo[j].r, hashInfo[j].hashTable->size);
			free(u->coordinates);
			free(u);
		}
	}
	
	
	//Find the minimum distance between centroids
	r = INFINITY;
	for(i=0; i<k; i++)
	{
		for(j=i+1; j<k; j++)
		{
			if(centroids[0] < curvesNum)
			{
				if(centroids[i] > centroids[j])
					tempDistance = &(distances[centroids[i]][centroids[j]]);
				else
					tempDistance = &(distances[centroids[j]][centroids[i]]);
				if(*tempDistance == INFINITY)
					*tempDistance = distanceFunction(&(curves[centroids[i]]), &(curves[centroids[j]]));
			}
			else
			{
				tempDistance = &tempDistance3;
				*tempDistance = distanceFunction(&(curves[centroids[i]]), &(curves[centroids[j]]));
			}
			
			if(*tempDistance < r)
				r = *tempDistance;
		}
	}
	r = r/2;
	
	//Range search
	assigned = malloc(curvesNum*sizeof(int));
	for(i=0; i<curvesNum; i++)
	{
		assigned[i] = -1;
	}
	assignCounter = k;
	round = 0;
	while(assignCounter >= k/2)
	{
		assignCounter = 0;
		for(i=0; i<k; i++)
		{
			hasMadeAssignment = 0;
			for(j=0; j<l; j++)
			{
				currentBucket = hashInfo[j].hashTable->table[centroidBuckets[i][j]];
				while(currentBucket != NULL)
				{
					for(m=0; m<currentBucket->entries; m++)
					{
						currentPoint = currentBucket->data[m];
						
						//Skip point if it has been previously assigned
						if(assigned[currentPoint] != -1 && assigned[currentPoint] < round)
							continue;
						
						//Skip point if it is a centroid
						isCentroid = 0;
						for(n=0; n<k; n++)
						{
							if(currentPoint == centroids[n])
							{
								assigned[currentPoint] = round;
								clusters[currentPoint] = n;
								isCentroid = 1;
								break;
							}
						}
						if(isCentroid)
							continue;
						
						if(centroids[0] < curvesNum)
						{
							if(centroids[i] > currentPoint)
								tempDistance = &(distances[centroids[i]][currentPoint]);
							else
								tempDistance = &(distances[currentPoint][centroids[i]]);
							if(*tempDistance == INFINITY)
								*tempDistance = distanceFunction(&(curves[centroids[i]]), &(curves[currentPoint]));
						}
						else
						{
							tempDistance = &tempDistance3;
							*tempDistance = distanceFunction(&(curves[centroids[i]]), &(curves[currentPoint]));
						}
						
						if(*tempDistance <= r)
						{
							if(assigned[currentPoint] == round && centroids[clusters[currentPoint]] != i)
							{
								if(centroids[0] < curvesNum)
								{
									if(currentPoint > centroids[clusters[currentPoint]])
										tempDistance2 = &(distances[currentPoint][centroids[clusters[currentPoint]]]);
									else
										tempDistance2 = &(distances[centroids[clusters[currentPoint]]][currentPoint]);
								}
								else
								{
									tempDistance2 = &tempDistance4;
									*tempDistance2 = distanceFunction(&curves[currentPoint], &curves[centroids[clusters[currentPoint]]]);
								}
								
								if(*tempDistance < *tempDistance2)
								{
									clusters[currentPoint] = i;
									hasMadeAssignment = 1;
								}
							}
							else
							{
								assigned[currentPoint] = round;
								clusters[currentPoint] = i;
								hasMadeAssignment = 1;
							}
						}
					}
					currentBucket = currentBucket->overflow;
				}
			}
			if(hasMadeAssignment)
				assignCounter++;
		}
		
		round++;
		r *= 2;
	}
	
	for(i=0; i<curvesNum; i++)
	{
		if(assigned[i] != -1)
		{
			continue;
		}
		
		minDistance = INFINITY;
		for(j=0; j<k; j++)
		{
			if(centroids[0] < curvesNum)
			{
				if(i>centroids[j])
					tempDistance = &(distances[i][centroids[j]]);
				else
					tempDistance = &(distances[centroids[j]][i]);
				
				if(*tempDistance == INFINITY)
					*tempDistance = distanceFunction(&(curves[i]), &(curves[centroids[j]]));
			}
			else
			{
				tempDistance = &tempDistance3;
				*tempDistance = distanceFunction(&(curves[i]), &(curves[centroids[j]]));
			}
			
			if(*tempDistance < minDistance)
			{
				minDistance = *tempDistance;
				clusters[i] = j;
			}
		}
	}
	
	//Cleanup
	for(i=0; i<k; i++)
		free(centroidBuckets[i]);
	free(centroidBuckets);
	free(assigned);
	free(g_u.coordinates);
}

