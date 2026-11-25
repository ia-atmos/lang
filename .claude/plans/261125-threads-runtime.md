# atmos-threads: the global state a thread meets

Done under the rule "Work with no input" (`atmos-lang#95`), by the
session, to be confirmed. Facts only; it decides nothing of
`atmos-threads#1` (what threads are for). Read on `main` at `0dbdabd`
(2026-11-25), files in `rt/`.

## Inventory

| Group | Where | What | Shared by Atmos code running in two threads? |
|---|---|---|---|
| Value stack | `rt/val.c` | `a_stk`, `a_top`, `a_pay`, `a_pay_n`, `a_lin`, `a_depth`, the error buffer `ERR`, the trace `TRACE_CALLS` | yes, every call |
| Reference counts | `rt/atmos.h` | `a_inc` / `a_dec`, plain `rc++` / `--rc` | yes, every value touched |
| Interned strings | `rt/str.c` | one weak intern table `TAB` (open addressing) | yes, every string made |
| Library tables | `rt/std.c`, `rt/l*.c` | `atm_g_print`, `atm_g_math`, ..., `atm_g_io`, `atm_g_os`, `atm_g_string`, `atm_g_vector` | yes, read on each call of the library |
| Environments | `rt/std.c` | `atm_envs`, `atm_nenvs` | only the main loop needs them |
| Scheduler | `rt/task.c` | `a_cur`, `a_trl`, `ROOT`, `MAIN`, `TIME`, the clock (`CLK_MIN`, `CLK_LATE`), `EXIT`, the stack of resumes `RSTK`, `solo` / `context` (`NSOLO`, `CTX`, `NCTX`, ...), the event being emitted (`EVT_TIME`, `EVT_VAL`) | yes, if the block may spawn, await or emit |
| Pin | `rt/task.c` | the registry of `set :out` (`OUTS`, `OUTH`), adoption (`ADOPT_*`), transfers (`XFER_*`), drops (`DROPS`), returns (`RETS`, `RETS_K`, `RETS_O`), `SPAWN_PIN` / `SPAWN_DEPTH` | yes, every pin, return or store of a pinned value |

About 70 globals in all, most in `rt/task.c`. None is per thread
(no `_Thread_local`), and nothing takes a lock.

## What it means for the shapes of `TODO-threads.md`

| Shape | Cost found |
|---|---|
| A / B with nothing shared (copy in, copy out) | every group above but the environments must become per thread: a state struct passed or a `_Thread_local` on each global; the intern table must be per thread or locked, since a string made in a thread may be compared with one of the main thread after the copy back. Values cross by a deep copy at the start and at the end. Large: it touches most of `rt/` and the generated C that names `a_stk` / `a_top` |
| A / B with shared values | also atomic counts (a cost on every value, in every program, unless an option of the build) and locks on the intern table and on pin's registries. Larger, and it breaks the determinism of the order of finalization |
| C (environments only) | none in the runtime: the C code in the pool must not touch Atmos values; it hands plain C data to the loop, which makes the values and emits |

A middle way, if `#1` wants Atmos code in the thread: the block runs a
restricted subset (no spawn, await, emit, pin; calls only to pure
functions of the library) on its own value stack and intern table,
with the result copied back. That needs a thread-local stack and
intern table, not the scheduler or pin.
