# Atmos against Lua 5.4, per operation (paper-pin#17)

Measurements behind the diagnosis of why compiled Atmos loses to the
Lua 5.4 interpreter on the CLBG programs of `../`, made by `atmos-lang`
on 2026-11-26. Machine and versions as `../env.txt` (gcc 13.3, Lua
5.4.6 of Ubuntu, 4 cores); Atmos built with `ATMOS_PIN=off` and `-O2`.

- `gen.py`: one program per operation, a loop around it, in Atmos
  (inside a function, and in the main body) and in Lua;
- `instr.sh`: instructions per iteration, by callgrind (exact);
- `time.py`: CPU time of 50 million iterations, best of 15, the
  variants alternating.

**The times are noisy**: the same binary varies about 10% between
sessions of measurement on this machine, even with the best of 15.
Compare variants only inside one table. The instruction counts are
exact.

## Instructions per iteration (main d0fe0ac, before the fixes)

`loop` is the empty loop; subtract it from the others.

| op | Atmos, in a function | Atmos, in the main body | Lua 5.4 |
|---|---|---|---|
| loop | 42 | 55 | 32 |
| add | 72 | 90 | 75 |
| mul3 | 196 | 202 | 197 |
| cmp | 126 | 139 | 97 |
| field | 248 | 253 | 126 |
| setf | 247 | 251 | 89 |
| vget | 183 | 188 | 115 |
| vset | 180 | 188 | 77 |
| call | 277 | 334 | 254 |
| new | 1271 | 1280 | 1318 |
| global | 80 | 98 | 99 |

Whole programs, at small sizes (instructions, Atmos / Lua):

| program | size | Atmos | Lua 5.4 | ratio |
|---|---|---|---|---|
| fib | 27 | 361153522 | 170107518 | 2.12 |
| nbody | 20000 | 1380429622 | 614774955 | 2.25 |
| fannkuch | 8 | 727971065 | 307158709 | 2.37 |
| mandelbrot | 200 | 546931382 | 416830299 | 1.31 |
| spectralnorm | 100 | 399693508 | 270545440 | 1.48 |
| binarytrees | 12 | 1491191339 | 1160128389 | 1.29 |
| fasta | 100000 | 2119666034 | 1628307629 | 1.3 |

In `add`, Atmos runs fewer instructions than Lua and took twice the
time: the loss was not the amount of code, but the trips of each value
through memory (see the answer to paper-pin#17).

## CPU time of 50 million iterations, in seconds

`main` is 3e453a5 (the compiler of d0fe0ac plus `watching` with two
values); `perf2` is the branch merged as e687bd2: a value is stored
field by field (no load of 16 bytes after two smaller stores, which
cannot be forwarded and stalls), and the result of an operator that
never gives an object goes straight to the variable.

| op | main | perf2 | lua54 |
|---|---|---|---|
| loop | 0.095 | 0.117 | 0.151 |
| add | 0.471 | 0.142 | 0.216 |
| mul3 | 0.760 | 0.607 | 0.551 |
| cmp | 0.583 | 0.318 | 0.266 |
| field | 0.791 | 0.627 | 0.348 |
| setf | 0.693 | 0.667 | 0.252 |
| vget | 0.612 | 0.452 | 0.311 |
| vset | 0.472 | 0.461 | 0.236 |
| call | 0.869 | 0.745 | 0.717 |
| new | 3.733 | 3.679 | 4.163 |
| global | 0.484 | 0.166 | 0.272 |

## The seven programs, CPU time in seconds, same run

Sizes as `../run.sh`.

| program | main | perf2 | lua54 |
|---|---|---|---|
| binarytrees | 2.999 | 2.852 | 3.147 |
| fannkuch | 0.581 | 0.545 | 0.181 |
| fasta | 1.517 | 1.426 | 1.023 |
| fib | 0.973 | 0.844 | 0.558 |
| mandelbrot | 1.181 | 0.868 | 0.503 |
| nbody | 2.184 | 2.182 | 0.888 |
| spectralnorm | 0.617 | 0.533 | 0.329 |
