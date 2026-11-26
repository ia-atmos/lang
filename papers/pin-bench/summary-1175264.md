| program | size | c | lua54 | luajit | off | unchecked | on | on/off | on/unchecked | on/lua54 |
|---|---|---|---|---|---|---|---|---|---|---|
| binarytrees | 16 | 0.205 | 2.386 | 0.832 | 2.451 | 2.530 | 2.947 | 1.20 | 1.16 | 1.24 |
| fannkuch | 9 | 0.023 | 0.228 | 0.045 | 0.840 | 0.885 | 1.071 | 1.27 | 1.21 | 4.70 |
| fasta | 1000000 | 0.137 | 1.203 | 0.478 | 1.928 | 1.919 | 2.328 | 1.21 | 1.21 | 1.94 |
| fib | 35 | 0.016 | 0.638 | 0.082 | 1.055 | 1.054 | 1.384 | 1.31 | 1.31 | 2.17 |
| mandelbrot | 1000 | 0.067 | 0.689 | 0.074 | 2.640 | 2.652 | 3.498 | 1.32 | 1.32 | 5.08 |
| nbody | 500000 | 0.028 | 0.984 | 0.124 | 2.824 | 3.200 | 3.587 | 1.27 | 1.12 | 3.65 |
| spectralnorm | 500 | 0.013 | 0.390 | 0.017 | 0.731 | 0.711 | 0.869 | 1.19 | 1.22 | 2.23 |

Peak RSS, MB (median)
| program | c | lua54 | luajit | off | unchecked | on |
|---|---|---|---|---|---|---|
| binarytrees | 9.7 | 56.5 | 77.8 | 50.2 | 53.3 | 53.2 |
| fannkuch | 1.7 | 2.6 | 2.6 | 2.3 | 2.2 | 2.2 |
| fasta | 1.5 | 2.7 | 2.7 | 2.2 | 2.2 | 2.2 |
| fib | 1.7 | 2.6 | 2.5 | 2.3 | 2.2 | 2.2 |
| mandelbrot | 1.6 | 2.7 | 2.6 | 2.3 | 2.3 | 2.2 |
| nbody | 1.7 | 2.7 | 2.7 | 2.4 | 2.4 | 2.4 |
| spectralnorm | 2.1 | 2.7 | 2.5 | 2.3 | 2.3 | 2.2 |

Spread: max/min of the 5 runs of `on`, per program
- binarytrees: 1.035 over 5 runs
- fannkuch: 1.089 over 5 runs
- fasta: 1.076 over 5 runs
- fib: 1.098 over 5 runs
- mandelbrot: 1.054 over 5 runs
- nbody: 1.120 over 5 runs
- spectralnorm: 1.044 over 5 runs
