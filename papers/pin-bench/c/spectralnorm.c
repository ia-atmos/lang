// spectral-norm (CLBG), plain C
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
static double A (int i, int j) { return 1.0 / ((i+j)*(i+j+1)/2 + i + 1); }
static void Av (int n, double* v, double* o) { for (int i=0;i<n;i++){ double s=0; for (int j=0;j<n;j++) s+=A(i,j)*v[j]; o[i]=s; } }
static void Atv (int n, double* v, double* o) { for (int i=0;i<n;i++){ double s=0; for (int j=0;j<n;j++) s+=A(j,i)*v[j]; o[i]=s; } }
static void AtAv (int n, double* v, double* o, double* t) { Av(n,v,t); Atv(n,t,o); }
int main (int argc, char** argv) {
    int n = argc > 1 ? atoi(argv[1]) : 100;
    double *u = malloc(n*sizeof(double)), *v = malloc(n*sizeof(double)), *t = malloc(n*sizeof(double));
    for (int i=0;i<n;i++) u[i]=1;
    for (int i=0;i<10;i++) { AtAv(n,u,v,t); AtAv(n,v,u,t); }
    double vBv=0, vv=0;
    for (int i=0;i<n;i++) { vBv+=u[i]*v[i]; vv+=v[i]*v[i]; }
    printf("%.9f\n", sqrt(vBv/vv));
    return 0;
}
