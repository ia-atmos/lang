// fib, naive recursion
#include <stdio.h>
#include <stdlib.h>
static long fib (long n) { return n < 2 ? 1 : fib(n-1) + fib(n-2); }
int main (int argc, char** argv) { printf("%ld\n", fib(argc > 1 ? atol(argv[1]) : 30)); return 0; }
