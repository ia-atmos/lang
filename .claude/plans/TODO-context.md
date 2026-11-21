# TODO: context (issue 21)

Session `atmos-context`. Material: `261118-context.md`. Design:
`261118-context-semantics.md`. The issue itself is `atmos-lang#39`
(grab and context).

Items are `#N` (standing rule of 2026-11-18, `TODO.md`): unique in this
session, never reused; from another session, `session#N`.

## Index

| # | Item | Kind | State |
|---|---|---|---|
| #1 | When enter and leave run: around each resume (lazy) or once per emit | decision | by the session, to be confirmed (2026-11-20): around each resume, lazy; on the branch `context` |
| #2 | Filter: bracket every reaction inside, or only some events (`with :draw`) | decision | by the session, to be confirmed: no filter |
| #3 | `leave` receives what `enter` returned | decision | by the session, to be confirmed: yes |
| #4 | Syntax | decision | by the session, to be confirmed: `context (enter, leave) {...}`, two functions (`\{...}`, `\(old) {...}`); `context` becomes a keyword |
| #5 | Errors: `emit`, `spawn`, `abort` inside `enter`/`leave` | decision | by the session, to be confirmed: `emit` and `spawn` there are errors (`abort` not checked); an error in `enter`/`leave` is raised at the await of the trail resumed, or in the resumer |
| #6 | SNKRX and env-ray not readable by this session | remark | open: cloning `ia-atmos/snxkx` and `ia-atmos/raylib` refused again (2026-11-20) |
| #7 | The three comments of issue 21 not readable | remark | open |
| #8 | The canvas of the stars: `STARS` (author) or `BACK` (material) | remark | settled (checked by `atmos-lang` in `src/shared.atm`): both exist; the stars draw on `STARS` (line 71), the background on `BACK` (line 22): the author is right |
| #9 | Implement `context` on the branch `context` | work | done (2026-11-20), branch `context` 21d00d0 (main merged in again 2026-11-21, no conflict, suite with ASAN 1023; earlier main 61979ce merged in, the conflict in `KEYS` with `args` resolved by hand), suite with ASAN passes (1007, rt, env); measured by `atmos-lang` (2026-11-20): 2152 `.atm` of 15 clones (snxkx, raylib included) give identical C and diagnostics (no port uses `context` as a name); sdl-pingus suite 1m51 vs 1m53 on main; 300,000 emits 0.21-0.24 s vs 0.19-0.29 s: the hooks cost nothing measurable; SNKRX not timed (migrating). Waits for the author's approval to merge. Before the merge: bring in main again (fix of `atm_emit_to` coming); `pool` (695afa1) and `grab` (4aed336) touch the same functions: the second and third to merge rebase by hand |
| #10 | Tell `atmos-grab` (and `pool`) what the prototype does at the resume of a trail in `rt/task.c` | work | done 2026-11-20; `atmos-grab#14`: branch `grab` (4aed336) does not touch `trail_resume`/`emit_top`, wraps `emit_trail` (body kept), and also touches `trail_off` and `ATrailX`: conflicts are textual only, whoever merges second rebases. `atmos-blocks` (branch `blocks` 77b2ff3): `trail_wake` resumes the parent of a block through `trail_resume` outside an emit: the switch applies and goes back eagerly (correct, not lazy) |
| #11 | Non-drawing events (the author's question): with the lazy resume, a key costs nothing unless a task inside awakes on it | answer | reported, not acknowledged |
| #12 | `lua-atmos` has no `context`; `atmos-lang/atmos` has only a plan, once per emit | remark | reported, not acknowledged |
| #13 | No real case awaits between set and restore: `context` fixes no bug today, it saves writing and makes the stars one switch per pool | remark | reported, not acknowledged |
| #14 | The case of `env-pico` for #1 and #3 | input | reported, not acknowledged |
| #16 | Optimize: aborting a pool inside a context switches once per task (context-03 at the end); the walk up the tree on each resume could be a pointer cached at spawn | work | known, not done |
| #17 | Rewrite the real cases with `context` on branches `exp-context` of `sdl-snxkx` (the stars) and `env-pico` (the current layer), building from the branch `context`; they report lines before and after, tests, cost and misfits | work | asked by `atmos-lang` (2026-11-21, `#112a`, the author's approval relayed, not checked here); this session keeps `context` near `main`. `env-pico` reported (2026-11-21, on 44cde44): 72 tests unchanged, pictures identical, switches per frame 4.0 vs 4.02 (one task per layer) and 22.0 vs 4.02 (pool of 10 into one layer), frame time unchanged (33.59 vs 33.58 ms); no real pico program needs it (synthetic case). Misfits: (a) a body that sets the layer and awaits gets the context's layer back: now one paragraph in the manual (branch `context`); (b) a task cannot set a layer for others with `context` (by design); (c) see #18. `sdl-snxkx`: waiting |
| #18 | The lambdas of `context` are long for the common case ("set this layer") and cannot be wrapped in a helper, since `context` is a statement (env-pico, #17c) | remark | open, nothing proposed yet: a library can return the pair, which works today (checked on `context` 46703cd): `val c = pico.ctx.layer(l)` returning `[enter, leave]`, then `context (c@0, c@1) {...}` |
| #15 | `pin` merged into `main` (8361f88, `set :out place = value`) and declared STABLE (`atmos-pin#9`, `TODO-pin.md` "Stable"): design #3 on top of it as fixed; a change to a rule of pin goes to `atmos-pin` first | work | when #3 comes |

## Waiting for the author (one at a time, in this order)

- **#1 When enter and leave run** (asked 2026-11-18): around each
  resume of a task inside, lazy (recommended), or once around the block
  per emit. See `261118-context-semantics.md`, sections 3 to 6.
- #2 Filter: bracket every reaction inside, or only those to some
  events (`context ... with :draw`)? Recommended: no filter (the lazy
  switch already costs nothing when nothing inside awakes).
- #3 Can `leave` receive what `enter` returned (the pico idiom
  `val old = pico.set.layer(l)` ... `pico.set.layer(old)`)?
  Recommended: yes.
- #4 Syntax: the issue's `=> {}` / `<= {}`, the `atmos-lang/atmos`
  plan's `:init :enter :body :leave`, or `context (enter, leave) {...}`
  with functions as expressions.
- #5 Errors: `emit`, `spawn`, `abort` inside `enter`/`leave`.

## Known, not done

- #6 SNKRX (`ia-atmos/sdl-snxkx`) and env-ray (`ia-atmos/env-ray`):
  access refused to this session (add_repo denied). Their cases come
  from `261118-context.md`, not from their code.
- #7 The three comments of issue 21: not readable (only search reaches
  GitHub outside `ia-atmos/lang`).
- #8 The author's note (from another session) names the canvas of the
  stars `STARS`, not `BACK`: the examples here follow
  `261118-context.md`.
- #9 Rule of 2026-11-18 (checked in the transcript of `atmos-lang`, and
  `TODO.md`, "Standing rules", 58eafba): this session implements
  `context` itself, in the compiler and runtime, on the branch
  `context`; plans and TODO still go to `main`. Before merging: latest
  `main` into the branch, full suite (`tst/run.sh`, `make test-rt
  test-env`, `ATMOS_ASAN=1`), ask `atmos-lang` to measure the ports if
  existing programs may change, and the author's approval of the
  design. Nothing implemented yet.
- #10 Nothing told to `atmos-grab` yet; `pool` also touches
  `rt/task.c`: coordinate.

## Reported, not acknowledged

- #11 2026-11-18: answered the author's question "does every reaction
  include events that are not drawing?": with the lazy resume, a key
  costs nothing unless a task inside awakes on it; if one does, that
  is #2 (filter).
- #12 2026-11-18: `lua-atmos` has no `context` (no branch);
  `atmos-lang/atmos` has only a plan (`260628-context.md`), once per
  emit.
- #13 2026-11-18: no real case awaits between set and restore;
  `context` fixes no bug in the ports today, it saves writing and makes
  the stars of SNKRX one switch per pool.
- #14 2026-11-18, `env-pico` (sent, it says, at the author's request;
  the author's choice it reports is not checked here, and is no
  decision of this session): pico dropped its helper
  `pico.with.layer(l, \{...})` (wrong when the body awaits) to wait for
  `context`, for the current layer and the pencil. It supports #1 =
  around each resume, and #3 = yes, in the form
  `=> { pico.set.layer(panel) }` / `<= (old) { pico.set.layer(old) }`
  (setters return the previous state). New argument: a context in the
  block that owns a pinned layer would keep the current layer from
  outliving its owner, so the binding could drop the bookkeeping of a
  current layer whose block ended. Its counts match section 2 of
  `261118-context-semantics.md`.

## Settled by the session, to be confirmed (2026-11-20)

Under the rule "Work with no input" (`TODO.md`, 2026-11-20, `#95`),
#1 to #5 are implemented with the recommendations on the branch
`context`, one test file per decision (`tst/cases/context-*.atm`,
`tst/ast/context-*.atm`): another answer changes one place.

| # | Taken | Tests |
|---|---|---|
| #1 | around each resume, lazy | context-01 to 05 |
| #2 | no filter | context-06 |
| #3 | `leave` receives what `enter` returned | context-07 |
| #4 | `context (enter, leave) {...}` | ast context-1, context-err-1 |
| #5 | `emit`, `spawn` in `enter`/`leave` are errors; errors raised in the trail resumed or the resumer | context-08 to 10 |

Measured (cases context-11, context-12): 50 stars, 100 canvas switches per
frame today, 2 with a context around the pool, 0 for `:update` and `:key`;
the layer of env-pico does not leak to another task when the task that set
it awaits.

## Decided

(nothing yet)
