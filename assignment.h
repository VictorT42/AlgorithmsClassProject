//File assignment.h

void lloyds(Curve *curves, int curvesNum, int *centroids, int *clusters, int k, double **distances, double(*distanceFunction)(Curve*, Curve*));
void range_search(Curve *curves, int curvesNum, int *centroids, int *clusters, int k, double **distances,
	double(*distanceFunction)(Curve*, Curve*), HashInfo *hashInfo, int l, int grid_k, int dimension);

