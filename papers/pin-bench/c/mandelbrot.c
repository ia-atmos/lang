// mandelbrot (CLBG), plain C; prints a checksum of the bytes instead of the image
#include <stdio.h>
#include <stdlib.h>
int main (int argc, char** argv) {
    int n = argc > 1 ? atoi(argv[1]) : 200;
    long sum = 0; int bits = 0, nb = 0;
    double delta = 2.0 / n;
    for (int y = 0; y < n; y++) {
        double ci = y*delta - 1.0;
        for (int x = 0; x < n; x++) {
            double cr = x*delta - 1.5, zr = 0, zi = 0, zr2 = 0, zi2 = 0;
            int bit = 1;
            for (int i = 0; i < 50; i++) {
                zi = 2.0*zr*zi + ci; zr = zr2 - zi2 + cr;
                zi2 = zi*zi; zr2 = zr*zr;
                if (zi2 + zr2 > 4.0) { bit = 0; break; }
            }
            bits = bits*2 + bit; nb++;
            if (nb == 8) { sum += bits; bits = 0; nb = 0; }
        }
        if (nb > 0) { for (; nb < 8; nb++) bits = bits*2; sum += bits; bits = 0; nb = 0; }
    }
    printf("P4 %d %d checksum %ld\n", n, n, sum);
    return 0;
}
