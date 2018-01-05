//File metrics.c

#include <stdlib.h>
#include <math.h>
#include "curves.h"
#include "metrics.h"

// double dist(Point a1, Point a2)
// {
	// return sqrt((a1.x - a2.x)*(a1.x - a2.x) + (a1.y - a2.y)*(a1.y - a2.y) + (a1.z - a2.z)*(a1.z - a2.z) + (a1.w - a2.w)*(a1.w - a2.w));
// }

double dtw(Curve *p1, Curve *p2)
{
	double **c;
	int i, j;
	double min, result;
	int m1 = p1->numOfPoints, m2 = p2->numOfPoints;
	
	c = malloc(m1 * sizeof(double*));
	for(i=0; i < m1; i++)
	{
		c[i] = malloc(m2 * sizeof(double));
	}
	
	c[0][0] = dist(p1->points[0], p2->points[0]);
	for(j=1; j < m2; j++)
	{
		c[0][j] = dist(p1->points[0], p2->points[j]) + c[0][j-1];
	}
	for(i=1; i < m1; i++)
	{
		c[i][0] = dist(p1->points[i], p2->points[0]) + c[i-1][0];
	}
	
	for(i=1; i < m1; i++)
	{
		for(j=1; j < m2; j++)
		{
			min = c[i-1][j-1];
			if(c[i-1][j] < min)
				min = c[i-1][j];
			if(c[i][j-1] < min)
				min = c[i][j-1];
			c[i][j] = dist(p1->points[i], p2->points[j]) + min;
		}
	}
	
	result = c[m1-1][m2-1];
	for(i=0; i < m1; i++)
		free(c[i]);
	free(c);
	return result;
	
}

double dfd(Curve *p1, Curve *p2)
{
	double **c;
	int i, j;
	double min, result;
	int m1 = p1->numOfPoints, m2 = p2->numOfPoints;
	double distance;
	
	c = malloc(m1 * sizeof(double*));
	for(i=0; i < m1; i++)
	{
		c[i] = malloc(m2 * sizeof(double));
	}
	
	c[0][0] = dist(p1->points[0], p2->points[0]);
	for(j=1; j < m2; j++)
	{
		distance = dist(p1->points[0], p2->points[j]);
		c[0][j] =  (MAX( distance , c[0][j-1] ));
	}
	for(i=1; i < m1; i++)
	{
		distance = dist(p1->points[i], p2->points[0]);
		c[i][0] =  (MAX( distance , c[i-1][0] ));
	}
	
	for(i=1; i < m1; i++)
	{
		for(j=1; j < m2; j++)
		{
			min = c[i-1][j-1];
			if(c[i-1][j] < min)
				min = c[i-1][j];
			if(c[i][j-1] < min)
				min = c[i][j-1];
			distance = dist(p1->points[i], p2->points[j]);
			c[i][j] = (MAX( distance , min ));
		}
	}
	
	result = c[m1-1][m2-1];
	for(i=0; i < m1; i++)
		free(c[i]);
	free(c);
	return result;
	
}

double silhouette(Curve *curves, int curvesNum, double **distances, int *centroids, int *clusters, int cluster,
 int k, int *curvesInClusters, double (*distanceFunction)(Curve*, Curve*))
{
	int i, j;
	int nearestCentroid, secondNearestCentroid;
	double nearestDistance, secondNearestDistance;
	double *tempDistance;
	double tempDistance2;
	double sumA, sumB, sum;
	double a_i, b_i, s_i;
	
	sum = 0;
	for(i=0; i<curvesNum; i++)
	{
		if(clusters[i] != cluster)
			continue;
		
		nearestDistance = INFINITY;
		secondNearestDistance = INFINITY;
		for(j=0; j<k; j++)
		{
			if(centroids[j] == i)
			{
				tempDistance = &tempDistance2;
				*tempDistance = 0;
			}
			else
			{
				if(centroids[0] < curvesNum)
				{
					if(centroids[j] > i)
						tempDistance = &(distances[centroids[j]][i]);
					else
						tempDistance = &(distances[i][centroids[j]]);
					if(*tempDistance == INFINITY)
						*tempDistance = distanceFunction(&(curves[centroids[j]]), &(curves[i]));
				}
				else
				{
					tempDistance = &tempDistance2;
					*tempDistance = distanceFunction(&(curves[centroids[j]]), &(curves[i]));
				}
			}
			
			if(*tempDistance < secondNearestDistance)
			{
				if(*tempDistance < nearestDistance)
				{
					secondNearestDistance = nearestDistance;
					secondNearestCentroid = nearestCentroid;
					nearestDistance = *tempDistance;
					nearestCentroid = j;
				}
				else
				{
					secondNearestDistance = *tempDistance;
					secondNearestCentroid = j;
				}
			}
		}
		
		if(secondNearestCentroid == cluster)
			continue;
		
		sumA = 0;
		sumB = 0;
		for(j=0; j<curvesNum; j++)
		{
			if(i==j)
				continue;
			
			if((clusters[j] != cluster) && (clusters[j] != secondNearestCentroid))
				continue;
			
			if(j > i)
				tempDistance = &(distances[j][i]);
			else
				tempDistance = &(distances[i][j]);
			if(*tempDistance == INFINITY)
				*tempDistance = distanceFunction(&(curves[j]), &(curves[i]));
			
			if(clusters[j] == cluster)
				sumA += *tempDistance;
			else
				sumB += *tempDistance;
		}
		
		if(curvesInClusters[cluster] > 1)
			a_i = sumA/(curvesInClusters[cluster] - 1);
		else
			a_i = 0;
		b_i = sumB/curvesInClusters[secondNearestCentroid];
		s_i = (b_i - a_i) / (MAX( a_i, b_i ));
		sum += s_i;
	}
	
	return sum/curvesInClusters[cluster];
	
}

