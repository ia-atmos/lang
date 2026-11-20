# pool: what is on the branch `pool` (2026-11-20)

Done under the rule "Work with no input" (`atmos-lang#95`). Nothing is
merged into `main`: each part waits for the author. The branch is
`origin/pool` (`695afa1`).

## 1. `find` and `filter` (`atmos-lang#1a`)

```
val t, k = find(xs, f)      ;; first member with f(v, k) truthy, and its key; or nil
val vs   = filter(xs, f)    ;; a new vector with all of them
val t = await :tick until find(pingus, \(t) { inside(zone, t.pub) })
```

- `xs` is a pool or a table; anything else is an error (`invalid
  argument to 'find' : expected table or tasks`).
- They are globals of the runtime (`rt/std.c`), next to `next`. A
  program that defines its own `find` shadows it.
- Over a pool they follow the loop over a pool: a task that the
  predicate aborts is not visited (`find-03`).
- `filter` over a table with keys gives a vector of the values: the
  keys are not kept.
- Cases: `find-01..03`, `filter-01`, `find-err-01..03`. Manual: "Basic
  Functions", and the idiom in "Await".

## 2. The exception of `emit @t` (`atmos-lang#60`)

On the branch, an exception that the target of `emit @t (e)` raises
goes to who holds `t`, as in a broadcast, and not to who emits.

| Program (`emit-to-exc-01`) | `main` | branch |
|---|---|---|
| `emit @t :X`, `t` throws | the emitter catches | the holder catches |
| `emit @:global :X`, `t` throws | the holder catches | the holder catches |

To know before deciding:

- When who emits also holds the target, nothing changes: the exception
  is raised at the emit (`emit-to-exc-03`).
- When the holder is in the middle of a reaction of its own (its `emit`
  led to this one), it receives the exception when it resumes. Who
  emitted goes on after its `emit @t` (`emit-to-exc-02`: on `main` the
  emitter dies at the emit; on the branch it finishes its loop over the
  pool, and then the holder catches). In a broadcast the exception
  leaves at once; here it is late by the rest of the reaction of the
  emitter.
- No existing case changed, the golden suite of the reference included
  (pass=1005 fail=0): the reference does not test who receives it.
- It reuses what `atm_emit_to` already did for an environment.

## 3. The grid (`atmos-pool#2`)

`lib/grid.atm`, in Atmos, no change in the language: `grid_new(cell)`,
`grid_clear(g)`, `grid_add(g, x, y, v)`, `grid_find(g, x1, y1, x2, y2,
f)`. It is filled again in each tick.

`lib/grid-bench.atm`: W zones of 32x32 against P points that move in
every tick; a `find` over all members per zone, against the grid
(cells of 64). Microseconds per tick, `-O2`, one run on the machine of
the session (not a port: the sizes are those of the cases).

| W zones | P members | like | scan | grid | gain |
|---|---|---|---|---|---|
| 20 | 20 | a small level of Pingus | 66 | 51 | 1.3x |
| 20 | 100 | a full level of Pingus | 302 | 130 | 2.3x |
| 10 | 60 | SNKRX, units against enemies | 87 | 68 | 1.3x |
| 50 | 200 | | 1115 | 209 | 5.3x |
| 100 | 1000 | | 11388 | 964 | 11.8x |

Reading: at the sizes the ports have, the scan costs 0.1 to 0.3 ms per
tick (a frame is 16 ms) and the grid saves little; it pays from a few
hundred members on. The cost W x P is not a problem of the ports today.
Not measured: the ports themselves (the grid is not applied to any).

## 4. The behaviours of section 3 (`atmos-pool#1`)

| Behaviour | Manual | Case |
|---|---|---|
| The order of awaking | "Structured Deterministic Concurrency" | `wake-order-01` (new, on the branch) |
| A loop over a pool with spawns and aborts in its body | "Loop" | `pool-iter-01` |
| `await(n, p)` | "Await" | `await-cnt-01`, `await-cnt-02`, `await-cnt-err-01..03` |

## 5. Validation

- `tst/run.sh`: pass=1005 fail=0 (ASAN), before `main` (`93a4fd7`, only
  `doc/manual.md` changed) was merged in.
- `ATMOS_ASAN=1 make test-rt test-env`: pass.
- The measurement on the ports was asked of `atmos-lang` on
  2026-11-20; not back yet.
