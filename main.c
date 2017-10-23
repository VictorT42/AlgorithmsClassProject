//File main.c

#include <stdio.h>
#include <stdlib.h>
#include "curves.h"

double dtw(Curve*, Curve*);

int main(int argc, char *argv[])
{
	Curve p1,p2;
	
	p1.numOfPoints = 3;
	p2.numOfPoints = 4;
	p1.points = malloc(3*sizeof(Point));
	p2.points = malloc(4*sizeof(Point));
	
	p1.points[0].x=4;
	p1.points[0].y=3;
	p1.points[0].z=0;
	p1.points[0].w=0;
	
	p1.points[1].x=5;
	p1.points[1].y=1;
	p1.points[1].z=0;
	p1.points[1].w=0;
	
	p1.points[2].x=4;
	p1.points[2].y=2;
	p1.points[2].z=0;
	p1.points[2].w=0;
	
	p2.points[0].x=4;
	p2.points[0].y=2;
	p2.points[0].z=0;
	p2.points[0].w=0;
	
	p2.points[1].x=3;
	p2.points[1].y=1;
	p2.points[1].z=0;
	p2.points[1].w=0;
	
	p2.points[2].x=7;
	p2.points[2].y=2;
	p2.points[2].z=0;
	p2.points[2].w=0;
	
	p2.points[3].x=1;
	p2.points[3].y=5;
	p2.points[3].z=0;
	p2.points[3].w=0;
	
	printf("%f\n", dtw(&p1, &p2));
	
	
	
	return 0;
}

