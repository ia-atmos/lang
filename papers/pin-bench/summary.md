| program | size | c | lua54 | luajit | off | unchecked | on | on/off | on/unchecked | on/lua54 |
|---|---|---|---|---|---|---|---|---|---|---|
| binarytrees | 16 | 0.240 | 3.169 | 1.057 | 3.268 | 3.393 | 3.214 | 0.98 | 0.95 | 1.01 |
| fannkuch | 9 | 0.025 | 0.347 | 0.051 | 0.819 | 0.778 | 0.803 | 0.98 | 1.03 | 2.31 |
| fasta | 1000000 | 0.159 | 1.429 | 0.493 | 1.798 | 1.877 | 1.970 | 1.10 | 1.05 | 1.38 |
| fib | 35 | 0.025 | 0.808 | 0.103 | 1.258 | 1.338 | 1.352 | 1.07 | 1.01 | 1.67 |
| mandelbrot | 1000 | 0.070 | 0.804 | 0.077 | 1.496 | 1.616 | 1.652 | 1.10 | 1.02 | 2.05 |
| nbody | 500000 | 0.026 | 1.012 | 0.119 | 3.116 | 3.480 | 3.621 | 1.16 | 1.04 | 3.58 |
| spectralnorm | 500 | 0.014 | 0.440 | 0.028 | 0.791 | 0.755 | 0.855 | 1.08 | 1.13 | 1.94 |

Peak RSS, MB (median)
| program | c | lua54 | luajit | off | unchecked | on |
|---|---|---|---|---|---|---|
| binarytrees | 9.6 | 56.6 | 75.3 | 50.3 | 53.3 | 53.3 |
| fannkuch | 1.6 | 2.5 | 2.5 | 2.3 | 2.3 | 2.2 |
| fasta | 1.4 | 2.7 | 2.7 | 2.1 | 2.2 | 2.2 |
| fib | 1.6 | 2.7 | 2.5 | 2.3 | 2.2 | 2.3 |
| mandelbrot | 1.6 | 2.6 | 2.6 | 2.3 | 2.3 | 2.3 |
| nbody | 1.6 | 2.7 | 2.6 | 2.4 | 2.4 | 2.2 |
| spectralnorm | 2.1 | 2.6 | 2.5 | 2.3 | 2.3 | 2.4 |

Spread: max/min of the 5 runs of `on`, per program
- binarytrees: 1.110 over 5 runs
- fannkuch: 1.179 over 5 runs
- fasta: 1.324 over 5 runs
- fib: 1.136 over 5 runs
- mandelbrot: 1.177 over 5 runs
- nbody: 1.163 over 5 runs
- spectralnorm: 1.206 over 5 runs

Best of the runs (min), seconds: less sensitive to a noisy machine
| program | c | lua54 | luajit | off | unchecked | on | on/off | on/lua54 |
|---|---|---|---|---|---|---|---|---|
| binarytrees | 0.210 | 2.959 | 1.005 | 3.020 | 2.929 | 3.131 | 1.04 | 1.06 |
| fannkuch | 0.023 | 0.249 | 0.047 | 0.743 | 0.762 | 0.756 | 1.02 | 3.04 |
| fasta | 0.154 | 1.319 | 0.476 | 1.641 | 1.644 | 1.754 | 1.07 | 1.33 |
| fib | 0.016 | 0.736 | 0.082 | 1.218 | 1.228 | 1.291 | 1.06 | 1.75 |
| mandelbrot | 0.065 | 0.690 | 0.071 | 1.416 | 1.440 | 1.564 | 1.10 | 2.27 |
| nbody | 0.025 | 0.987 | 0.116 | 2.982 | 3.260 | 3.435 | 1.15 | 3.48 |
| spectralnorm | 0.014 | 0.396 | 0.023 | 0.754 | 0.722 | 0.778 | 1.03 | 1.96 |
