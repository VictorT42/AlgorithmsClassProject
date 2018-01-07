//File initialization.h

int *random_selection(int k, int numOfCurves, Curve *curves, double**, double (*distanceFunction)(Curve*, Curve*));
int *k_means_pp(int k, int numOfCurves, Curve *curves, double **, double (*distanceFunction)(Curve*, Curve*));

