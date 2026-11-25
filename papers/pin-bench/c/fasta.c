// fasta (CLBG), plain C, simple version (one character at a time into a line)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IM 139968
#define IA 3877
#define IC 29573
static long last = 42;
static double rnd (double max) { last = (last*IA + IC) % IM; return max*last/IM; }
static const char* ALU =
 "GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGG"
 "GAGGCCGAGGCGGGCGGATCACCTGAGGTCAGGAGTTCGAGA"
 "CCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAAT"
 "ACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCA"
 "GCTACTCGGGAGGCTGAGGCAGGAGAATCGCTTGAACCCGGG"
 "AGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCC"
 "AGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAA";
typedef struct { char c; double p; } P;
static P IUB[] = {{'a',0.27},{'c',0.12},{'g',0.12},{'t',0.27},{'B',0.02},{'D',0.02},{'H',0.02},{'K',0.02},{'M',0.02},{'N',0.02},{'R',0.02},{'S',0.02},{'V',0.02},{'W',0.02},{'Y',0.02}};
static P HS[] = {{'a',0.3029549426680},{'c',0.1979883004921},{'g',0.1975473066391},{'t',0.3015094502008}};
static void repeat (const char* s, int n) {
    int len = strlen(s), k = 0; char line[61];
    while (n > 0) { int m = n < 60 ? n : 60; for (int i=0;i<m;i++){ line[i]=s[k]; k=(k+1)%len; } line[m]='\n'; fwrite(line,1,m+1,stdout); n -= m; }
}
static void random_ (P* t, int nt, int n) {
    double acc = 0; for (int i=0;i<nt;i++){ acc += t[i].p; t[i].p = acc; }
    char line[61];
    while (n > 0) { int m = n < 60 ? n : 60;
        for (int i=0;i<m;i++){ double r = rnd(1.0); int j=0; while (t[j].p < r) j++; line[i]=t[j].c; }
        line[m]='\n'; fwrite(line,1,m+1,stdout); n -= m; }
}
int main (int argc, char** argv) {
    int n = argc > 1 ? atoi(argv[1]) : 1000;
    printf(">ONE Homo sapiens alu\n"); repeat(ALU, 2*n);
    printf(">TWO IUB ambiguity codes\n"); random_(IUB, 15, 3*n);
    printf(">THREE Homo sapiens frequency\n"); random_(HS, 4, 5*n);
    return 0;
}
