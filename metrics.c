//File metrics.c

#include <stdlib.h>
#include <math.h>
#include "curves.h"
#include "metrics.h"

double dist(Point a1, Point a2)
{
	return sqrt((a1.x - a2.x)*(a1.x - a2.x) + (a1.y - a2.y)*(a1.y - a2.y) + (a1.z - a2.z)*(a1.z - a2.z) + (a1.w - a2.w)*(a1.w - a2.w));
}

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
	
	result = c[m1-1][m2-1];
	for(i=0; i < m1; i++)
		free(c[i]);
	free(c);
	return result;
	
}

