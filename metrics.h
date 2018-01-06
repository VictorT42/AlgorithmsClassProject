//File metrics.h

#define MAX( A , B ) ( ( (A) > (B) ) ? (A) : (B) )
#define dist(a1, a2) sqrt((a1.x - a2.x)*(a1.x - a2.x) + (a1.y - a2.y)*(a1.y - a2.y) + (a1.z - a2.z)*(a1.z - a2.z) + (a1.w - a2.w)*(a1.w - a2.w))

double dtw(Curve*, Curve*);
double dfd(Curve*, Curve*);
// double dist(Point a1, Point a2);
double silhouette(Curve*, int, double**, int*, int*, int, int, int*, double (*distanceFunction)(Curve*, Curve*));
double cRMSD(Curve *p1, Curve *p2);

