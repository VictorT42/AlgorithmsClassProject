//File update.c

#include <stdlib.h>
#include "curves.h"
#include "metrics.h"

Curve *mdf(Curve *p1, Curve *p2)
{
	double **c;
	int i, j;
	double min;
	int m1 = p1->numOfPoints, m2 = p2->numOfPoints;
	int distance;
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
			if(c[i][j-1] < c[i-1][j-1])
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
	
	mean->points = malloc(mean->numOfPoints);
	for(i=mean->numOfPoints; i>=0; i--)
	{
		mean->points[mean->numOfPoints-i].x = tempPoints[i].x;  //TODO: Check if need to add -1
		mean->points[mean->numOfPoints-i].y = tempPoints[i].y;
		mean->points[mean->numOfPoints-i].z = tempPoints[i].z;
		mean->points[mean->numOfPoints-i].w = tempPoints[i].w;
	}
	
	for(i=0; i < m1; i++)
		free(c[i]);
	free(c);
	free(tempPoints);
	return mean;
	
}