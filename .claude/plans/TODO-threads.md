# TODO: session `atmos-threads`

The session that discusses threads in Atmos with the author, and
prototypes them on the branch `threads` of this repository (cut from
`origin/main` at `13d753f`). Material: `261122-threads.md`. This session
does not merge into `main`: when the author approves a merge,
`atmos-lang` measures the ports and merges.

Last update: 2026-11-22.

## Index of the items (`atmos-threads#N`)

| # | Title | State |
|---|---|---|
| #1 | What threads are for (heavy computation, blocking C calls, several cores, parity with the reference) | asked, 2026-11-22 |

## Facts (2026-11-22)

- No support: `thread` is a reserved word, parsed as a prefix with no
  grammar (`src/lexer.c`, `src/parser.c`).
- The reference: `val v = thread { ... }` inside a task; the block runs
  elsewhere, cannot await (`0516`), its value is the value of the
  expression (`0517`, `0518`); an abort of the task abandons it (`0519`).
- The runtime is single threaded: non-atomic reference counts, global
  library tables, global scheduler and trace, values owned by blocks.

## Shapes on the table

| Shape | What it is | Cost to the runtime |
|---|---|---|
| A. Expression `thread { ... }` | as the reference; the task awaits, the result arrives as an event of the loop | a second interpreter state, or copies in and out |
| B. Library call `atmos.thread(f, ...)` | gives a task to await; same mechanics as A, no grammar | same as A |
| C. Environments only | a C call runs in a pool (libuv, FreeRTOS) and emits its result; no Atmos code off the main thread | none in the runtime |
