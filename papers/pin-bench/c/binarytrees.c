// binary-trees (CLBG), plain C with malloc/free, no pool
#include <stdio.h>
#include <stdlib.h>
typedef struct N { struct N *l, *r; } N;
static N* make (int d) {
    N* n = malloc(sizeof(N));
    if (d > 0) { n->l = make(d-1); n->r = make(d-1); } else { n->l = n->r = NULL; }
    return n;
}
static long check (N* n) { return n->l ? 1 + check(n->l) + check(n->r) : 1; }
static void del (N* n) { if (n->l) { del(n->l); del(n->r); } free(n); }
int main (int argc, char** argv) {
    int N_ = argc > 1 ? atoi(argv[1]) : 10, mind = 4, maxd = N_ > mind+2 ? N_ : mind+2;
    { N* t = make(maxd+1); printf("stretch tree of depth %d\t check: %ld\n", maxd+1, check(t)); del(t); }
    N* ll = make(maxd);
    for (int d = mind; d <= maxd; d += 2) {
        long it = 1L << (maxd - d + mind), c = 0;
        for (long i = 0; i < it; i++) { N* t = make(d); c += check(t); del(t); }
        printf("%ld\t trees of depth %d\t check: %ld\n", it, d, c);
    }
    printf("long lived tree of depth %d\t check: %ld\n", maxd, check(ll));
    del(ll);
    return 0;
}
