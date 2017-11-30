//File update.h

#include "curves.h"

Curve *mdf(Curve *p1, Curve *p2);
void meanFrechet(Curve *curves, int curvesNum, int *clusters, int *centroids, int k);

