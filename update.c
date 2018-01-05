//File update.c

#include <stdlib.h>
#include <math.h>
#include "curves.h"
#include "metrics.h"

Curve *mdf(Curve *p1, Curve *p2)
{
	double **c;
	int i, j;
	double min;
	int m1 = p1->numOfPoints, m2 = p2->numOfPoints;
	double distance;
	Curve *mean;
	Point *tempPoints;
	
	tempPoints = malloc((m1+m2) * sizeof(Point));
	
	c = malloc(m1 * sizeof(double*));
	for(i=0; i < m1; i++)
	{
		c[i] = malloc(m2 * sizeof(double));
	}
	
	c[0][0] = dist(p1->points[0], p2->points[0]);
	for(j=1; j < m2; j++)
	{
		distance = dist(p1->points[0], p2->points[j]);
		c[0][j] =  MAX( distance , c[0][j-1] );
	}
	for(i=1; i < m1; i++)
	{
		distance = dist(p1->points[i], p2->points[0]);
		c[i][0] =  MAX( distance , c[i-1][0] );
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
			c[i][j] = MAX( distance , min );
		}
	}
	
	mean = malloc(sizeof(Curve));
	mean->id = NULL;
	mean->numOfPoints = 0;
	i = m1-1;
	j = m2-1;
	while(i>0 && j>0)
	{
		tempPoints[mean->numOfPoints].x = (p1->points[i].x + p2->points[j].x) / 2;
		tempPoints[mean->numOfPoints].y = (p1->points[i].y + p2->points[j].y) / 2;
		tempPoints[mean->numOfPoints].z = (p1->points[i].z + p2->points[j].z) / 2;
		tempPoints[mean->numOfPoints].w = (p1->points[i].w + p2->points[j].w) / 2;
		if(c[i-1][j] < c[i-1][j-1])
		{
			if(c[i][j-1] < c[i-1][j])
				j--;
			else
				i--;
		}
		else
		{
			if(c[i][j-1] > c[i-1][j-1])
				j--;
			else
			{
				i--;
				j--;
			}
		}
		mean->numOfPoints++;
	}
	while(i>0)
	{
		tempPoints[mean->numOfPoints].x = (p1->points[i].x + p2->points[j].x) / 2;
		tempPoints[mean->numOfPoints].y = (p1->points[i].y + p2->points[j].y) / 2;
		tempPoints[mean->numOfPoints].z = (p1->points[i].z + p2->points[j].z) / 2;
		tempPoints[mean->numOfPoints].w = (p1->points[i].w + p2->points[j].w) / 2;
		i--;
		mean->numOfPoints++;
	}
	while(j>=0)
	{
		tempPoints[mean->numOfPoints].x = (p1->points[i].x + p2->points[j].x) / 2;
		tempPoints[mean->numOfPoints].y = (p1->points[i].y + p2->points[j].y) / 2;
		tempPoints[mean->numOfPoints].z = (p1->points[i].z + p2->points[j].z) / 2;
		tempPoints[mean->numOfPoints].w = (p1->points[i].w + p2->points[j].w) / 2;
		j--;
		mean->numOfPoints++;
	}
	
	mean->points = malloc(mean->numOfPoints * sizeof(Point));
	for(i=mean->numOfPoints; i>0; i--)
	{
		mean->points[mean->numOfPoints-i].x = tempPoints[i-1].x;
		mean->points[mean->numOfPoints-i].y = tempPoints[i-1].y;
		mean->points[mean->numOfPoints-i].z = tempPoints[i-1].z;
		mean->points[mean->numOfPoints-i].w = tempPoints[i-1].w;
	}
	
	for(i=0; i < m1; i++)
		free(c[i]);
	free(c);
	free(tempPoints);
	return mean;
	
}

void meanFrechet(Curve *curves, int curvesNum, int *clusters, int *centroids, int k)
{
	int i, j;
	int *clusterSize;
	Curve ***treeLevel;
	Curve **tempCurves;
	Curve *copyCurve;
	int newSize;
	
	clusterSize = malloc(k*sizeof(int));
	for(i=0; i<k; i++)
		clusterSize[i] = 0;
	
	for(i=0; i<curvesNum; i++)
		clusterSize[clusters[i]]++;
	
	treeLevel = malloc(k* sizeof(Curve**));
	tempCurves = malloc(k*sizeof(Curve*));
	for(i=0; i<k; i++)
	{
		treeLevel[i] = malloc(((clusterSize[i] / 2) + 1)*sizeof(Curve*));
		tempCurves[i] = NULL;
		clusterSize[i] = 0;
	}
	
	for(i=0; i<curvesNum; i++)
	{
		if(tempCurves[clusters[i]] == NULL)
		{
			tempCurves[clusters[i]] = &(curves[i]);
		}
		else
		{
			treeLevel[clusters[i]][clusterSize[clusters[i]]] = mdf(&(curves[i]), tempCurves[clusters[i]]);
			clusterSize[clusters[i]]++;
			tempCurves[clusters[i]] = NULL;
		}
	}
	for(i=0; i<k; i++)
	{
		if(tempCurves[i] != NULL)
		{
			copyCurve = malloc(sizeof(Curve));
			copyCurve->id = NULL;
			copyCurve->numOfPoints = tempCurves[i]->numOfPoints;
			copyCurve->points = malloc((copyCurve->numOfPoints)*sizeof(Point));
			for(j=0; j<copyCurve->numOfPoints; j++)
			{
				copyCurve->points[j].x = tempCurves[i]->points[j].x;
				copyCurve->points[j].y = tempCurves[i]->points[j].y;
				copyCurve->points[j].z = tempCurves[i]->points[j].z;
				copyCurve->points[j].w = tempCurves[i]->points[j].w;
			}
			treeLevel[i][clusterSize[i]] = copyCurve;
			clusterSize[i]++;
		}
	}
	
	for(i=0; i<k; i++)
	{
		if(clusterSize[i] == 0)  //If cluster is empty
			continue;
		
		while(clusterSize[i] > 1)
		{
			copyCurve = treeLevel[i][0];
			treeLevel[i][0] = mdf(treeLevel[i][0], treeLevel[i][1]);
			free(copyCurve->points);
			free(copyCurve);
			free(treeLevel[i][1]->points);
			free(treeLevel[i][1]);
			
			newSize = 1;
			for(j=2; j<clusterSize[i]-1; j=j+2)
			{
				treeLevel[i][newSize] = mdf(treeLevel[i][j], treeLevel[i][j+1]);
				free(treeLevel[i][j]->points);
				free(treeLevel[i][j+1]->points);
				free(treeLevel[i][j]);
				free(treeLevel[i][j+1]);
				newSize++;
			}
			if(j == clusterSize[i]-1)
			{
				treeLevel[i][newSize] = treeLevel[i][j];
				newSize++;
			}
			clusterSize[i] = newSize;
		}
		
		if(centroids[i] >= curvesNum)
			free(curves[curvesNum+i].points);
		else
			centroids[i] = curvesNum + i;
		
		curves[curvesNum+i].numOfPoints = treeLevel[i][0]->numOfPoints;
		curves[curvesNum+i].points = treeLevel[i][0]->points;
		free(treeLevel[i][0]);
		
	}
	
	//Cleanup
	for(i=0; i<k; i++)
		free(treeLevel[i]);
	free(treeLevel);
	free(clusterSize);
	free(tempCurves);
	
}


void pam(Curve *curves, int curvesNum, int *clusters, int *centroids, int k, double **distances, double (*distanceFunction)(Curve*, Curve*))
{
	int i,j,m;
	double objective;
	double *tempDistance;
	double minObjective;
	int bestCentroid;
	
	for(i=0; i<k; i++)
	{
		minObjective = INFINITY;
		for(j=0; j<curvesNum; j++)
		{
			if(clusters[j] != i)
				continue;
			
			objective = 0;
			for(m=0; m<curvesNum; m++)
			{
				if(clusters[m] != i)
					continue;
				if(m == j)
					continue;
				
				if(m > j)
					tempDistance = &(distances[m][j]);
				else
					tempDistance = &(distances[j][m]);
				if(*tempDistance == INFINITY)
					*tempDistance = distanceFunction(&(curves[m]), &(curves[j]));
				objective += *tempDistance;
			}
			if(objective < minObjective)
			{
				minObjective = objective;
				bestCentroid = j;
			}
		}
		centroids[i] = bestCentroid;
	}
}

