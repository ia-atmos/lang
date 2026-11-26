# TODO: context (issue 21)

Session `atmos-context`. Material: `261118-context.md`. Design:
`261118-context-semantics.md`. The issue itself is `atmos-lang#39`
(grab and context).

Since 2026-11-24 (`261124-atmos.md`, the split of `atmos-lang#119`, "Ok" of the author): coordination, status and questions for the author go to the session `atmos`; measurements on the ports and merges into `main` stay with `atmos-lang`. Item numbers stay local to this session (rule #71, confirmed by the author 2026-11-24 in `atmos`): `#N` here, `atmos-context#N` from outside. The pause of 2026-11-23 ended on 2026-11-25 (relayed by `atmos` at the author's request); nothing approved is pending here (#16 waits for a port to feel the cost).

Items are `#N` (standing rule of 2026-11-18, `TODO.md`): unique in this
session, never reused; from another session, `session#N`.

## Index

| # | Item | Kind | State |
|---|---|---|---|
| #1 | When enter and leave run: around each resume (lazy) or once per emit | decision | decided by the author (2026-11-22): B, around each resume, lazy (as on the branch `context`) |
| #2 | Filter: bracket every reaction inside, or only some events (`with :draw`) | decision | decided by the author (2026-11-22): no filter; a filter stays as #19 |
| #3 | `leave` receives what `enter` returned (decided 2026-11-22) | decision | decided by the author (2026-11-22): yes |
| #4 | Syntax | decision | decided by the author (2026-11-22): D, `context (enter, leave) {...}` or `context X {...}` with X giving `[enter, leave]`; `context` is a keyword; done on the branch `context` 14d9304 |
| #5 | Errors inside `enter`/`leave` | decision | decided by the author (2026-11-22): `emit`, `spawn`, `abort`, `toggle` there (also from a function they call) are errors, checked at runtime (one test per operation); an error in `enter`/`leave` is raised at the await of the trail resumed, or in the resumer; done on the branch `context` 052e823; `toggle` renamed `mute` on main since (the check now says `mute`, context-18) |
| #6 | SNKRX and env-ray not readable by this session | remark | settled (2026-11-22): public, read through the git proxy (`ia-atmos/env-ray` cloned; the earlier refusals were of this session's permission classifier, not of GitHub). env-ray: 5 sites; 2 split the pair across tasks (`classics/platformer.atm` 172/190: `begin_mode2d` in one task, `end_mode2d` in another; `games/retro_maze_3d/main.atm` 455/493: `begin_texture_mode`/`end_texture_mode` in two tasks): real cases of `context`; 3 keep it in one task (`wave_collector`, `raylife`, `repair`). SNKRX done by `sdl-snxkx` (#17) |
| #7 | The three comments of issue 21 | remark | settled (2026-11-22): read from the public page. (1) the implicit form in a `func T ()` (not taken: wrap the body in `context`); (2) an expansion into `par :any` with loops of enter/leave around `await(true)`, i.e. once per reaction (semantics A; the author took B, #1); (3) `context { :init :enter :body :leave }` (`:init` is code before the `context`; `old` is what `enter` returns, #3). The body's example sets a fixed color in leave (`\(_) {...}` works). None contradicts #1 to #5 |
| #8 | The canvas of the stars: `STARS` (author) or `BACK` (material) | remark | settled (checked by `atmos-lang` in `src/shared.atm`): both exist; the stars draw on `STARS` (line 71), the background on `BACK` (line 22): the author is right |
| #9 | Implement `context` on the branch `context` | work | done and merged into `main` (b71fbe4, 2026-11-22, the author's "merge"); suite with ASAN 1030, rt, env. Ports measured by `atmos-lang` on 46703cd, before `context X` and the checks of `abort`/`toggle` |
| #10 | Tell `atmos-grab` (and `pool`) what the prototype does at the resume of a trail in `rt/task.c` | work | done 2026-11-20; `atmos-grab#14`: branch `grab` (4aed336) does not touch `trail_resume`/`emit_top`, wraps `emit_trail` (body kept), and also touches `trail_off` and `ATrailX`: conflicts are textual only, whoever merges second rebases. `atmos-blocks` (branch `blocks` 77b2ff3): `trail_wake` resumes the parent of a block through `trail_resume` outside an emit: the switch applies and goes back eagerly (correct, not lazy) |
| #11 | Non-drawing events (the author's question): with the lazy resume, a key costs nothing unless a task inside awakes on it | answer | acknowledged by the author (2026-11-22), closed |
| #12 | `lua-atmos` has no `context`; `atmos-lang/atmos` has only a plan, once per emit | remark | acknowledged by the author (2026-11-22), closed |
| #13 | No real case awaits between set and restore: `context` fixes no bug today, it saves writing and makes the stars one switch per pool | remark | acknowledged by the author (2026-11-22), closed |
| #14 | The case of `env-pico` for #1 and #3 | input | acknowledged by the author (2026-11-22), closed |
| #16 | Optimize: aborting a pool inside a context switches once per task (context-03 at the end); the walk up the tree on each resume could be a pointer cached at spawn | work | known, not done; seen by env-pico (2026-11-22, `tst/ctx-layer.atm`): 64 switches over 20 frames = 40 (one pair per frame) + 2 (start) + 22 (`:quit` aborts a pool of 10 inside the context: 11 pairs); reproduced on main |
| #17 | Rewrite the real cases with `context` on branches `exp-context` of `sdl-snxkx` (the stars) and `env-pico` (the current layer), building from the branch `context`; they report lines before and after, tests, cost and misfits | work | asked by `atmos-lang` (2026-11-21, `#112a`, the author's approval relayed, not checked here); this session keeps `context` near `main`. `env-pico` reported (2026-11-21, on 44cde44): 72 tests unchanged, pictures identical, switches per frame 4.0 vs 4.02 (one task per layer) and 22.0 vs 4.02 (pool of 10 into one layer), frame time unchanged (33.59 vs 33.58 ms); no real pico program needs it (synthetic case). Misfits: (a) a body that sets the layer and awaits gets the context's layer back: now one paragraph in the manual (branch `context`); (b) a task cannot set a layer for others with `context` (by design); (c) see #18. `sdl-snxkx` reported (2026-11-21, branch `exp-context` 88da713, on `context` 46703cd, not to merge): the stars (pool around a `context` on `STARS`) and the background (`BACK`); 40 of 40 tests identical; canvas switches per frame 24.72 -> 4.86 (stress, 1500 frames); stress logic 2.973 s -> 2.855 s (within the spread; a switch only sets an index); lines `src/shared.atm` 115 -> 117, `eng/eng.atm` 199 -> 213 (new `eng.on(c)`, `eng.target_get()`). Misfits: (1) `enter` must return what `leave` needs, so the engine needed a getter of the current canvas (#3); (2) two expressions take two lines per use, see #18; (3) only these two sites switch canvases from tasks; 2026-11-22, the author ("Sim"): asked `sdl-snxkx` to bring the rewrite into its main and `env-pico` to make `pico.ctx.layer` official; waiting for both; env-pico done (main 5d96e3f2, on lang 666dcd6): `pico.ctx.layer(l)` official, documented in doc/api.md, test tst/ctx-layer.atm, suite 73/73; sdl-snxkx done (2026-11-22, main of `ia-atmos/sdl-snkrx`, on lang 666dcd6): `context eng.on(BACK)` and `context eng.on(STARS)` around the pool, 41 tests identical, 24.72 -> 4.89 canvas switches per frame, nothing that did not fit. Closed |
| #18 | The lambdas of `context` are long for the common case and cannot be wrapped in a helper (env-pico #17c, sdl-snxkx #17(2)) | remark | settled by #4 (2026-11-22): `context eng.on(X) {...}` |
| #19 | A filter of events for `context` | decision | decided by the author (2026-11-22, "Sim"): `context <P> (enter, leave) {...}` / `context <P> X {...}`; applied only while an emit that matches P is traversed, never outside an emit; done on the branch `context` 1d48c2a (suite with ASAN 1076, rt, env; cases context-19 to 22, ast context-3, context-err-2); merged into `main` 152cba6 (2026-11-22, the author's "Sim"); the three details of the runtime confirmed by the author (2026-11-22, "sim"): one match per emit per context, the zero tick of `mute` is the event `0`, an error in the pattern counts as no match |
| #20 | Rewrite the two env-ray sites that split a begin/end pair across tasks with `context`, on a branch `exp-context` | work | reported by `env-ray` (2026-11-22, branch `exp-context` d4f30e3 pushed to `ia-atmos/env-ray` for reading, on lang b71fbe4 before `toggle` -> `mute`, not to merge): binding `ray.ctx.mode2d`, `ray.ctx.texture_mode` (+11 in ray.atm); platformer 37 -> 47 lines (Play), retro_maze_3d same length; tests identical; cost: noise in whole frames, retro_maze_3d logic +6% (~10 us/frame); enters per frame 1 -> ~3.5 (platformer), ~2 (retro_maze_3d). Misfits: (1) the block is inline: Play needed `spawn { context ... { ... ; emit :taken } }` + `await :taken`; (2) a pool drawing in the mode moves inside; (3) enter/leave run on every event that wakes a task inside (:frame, keys, watching), outside BeginDrawing: harmless here, 2-3.5x the calls, and a mode with side effects could not be limited to :draw: demand for #19; (4) a clear right after begin became the first task inside (no hook after enter); 2026-11-22, the author ("ok"): asked env-ray to redo it with `context <:draw>` on current main, re-measure (1 enter per frame expected), and if tests stay identical bring it into its main; waiting; env-ray (2026-11-22, local branch ctx2 fa9e30d, lang d5323ce) stopped at the measure: platformer exactly 1 enter per frame (734/734, 900/900), tests identical; retro_maze_3d fails at frame 300: `await(s)` gives nil inside `context <:draw>` (bug of #19: the match of the filter cleared the payload being delivered, also of plain events); fixed on the branch `context` 67be195 (cases context-23, 24, suite 1078), merged into `main` by `atmos-lang` as 656e7eb under the standing rule (`TODO.md`: a defect fix that changes no rule goes to main through atmos-lang after the suite and the measurement): ports' C unchanged, sdl-pingus passes; env-ray to measure again; misfits 1, 2, 4 unchanged; DONE (2026-11-22): adopted on env-ray main (ba6770e platformer, ba8a8b6 retro_maze_3d, on lang 656e7eb) with `context <:draw>`: suite 57/57, 5 runs identical, exactly 1 enter per frame (was 2583, 3659, 9854, 3006 per run); retro_maze_3d logic +6% (~9 us/frame), whole frames unchanged; misfits 1, 2, 4 remain (not of the language: an inline block, pools declared inside, no hook after enter). Closed |
| #21 | The block form of `context` (the author's preference, 2026-11-26 in `env-ray`, checked): reopens #4 | decision | refined with the author (2026-11-26): no `:init`, no `:body`; `enter {...}` and `leave {...}` as statements inside the block, like `defer`, seeing the block's variables; the context applies from where `enter` executes; `leave` gets `it` = what `enter` returned (or `\(x) {...}`); `context X {...}` kept; `(enter, leave)` removed; enter/leave once per emit stays as is (repeats only after emit/spawn/abort/mute outward). The author said "sim, confira com atmos-pin": asked `atmos-pin` whether closures capturing the block's own locals, held by the runtime, fit the stable rules of pin; waiting; atmos-pin (2026-11-26): allowed by the stable rules, on condition that leave runs with the defers, before the pins are finalized. Done on the branch `context` de24422: `context [<P>] {... enter F ... leave F ...}` and `context [<P>] X {...}`; `(enter, leave)` removed; leave runs with the defers (case context-26); suite with ASAN 1110, rt, env; manual updated. Waits for the author's approval to merge (through atmos-lang) |
| #22 | env-ray#48: `platformer` Play needs `spawn { context ... { ...; await :all coins; emit :taken; await false } }` + `await :taken` to learn that a phase inside the context ended | decision | answered through `atmos` (2026-11-26): not a matter of context (the block form does not change it); (a) `spawn context` is sugar only, (b) a pool outside living inside would undo #1, (c) awaiting a phase of another task is a general feature; recommendation: no change now, open as a general item if more ports ask; waiting for the author |
| #23 | A context as a value, loaded on demand (the author, 2026-11-26) | decision | the author: (1) the value could be `[enter=f, leave=g]`; (2) the current form stays, as an expansion to it (`context`, `enter`, `leave`); (3) it could be bound to a spawn, a pool or an await. My reading, to confirm: identity by the value (uses of the same value share one context, no switch between them), which would solve #22 (pool outside, each task bound). The author (2026-11-26): no grouping without a block ("a indentação engana", his own objection); the value is `:Context [enter=f, leave=g]`; the block form uses `:enter`/`:leave`; bound to an `await` it holds for what the await starts (B), not for the code after it; the binding needs an explicit separator (`in`, at), preferably no new keyword. Decided by the author (2026-11-26): the binding is `in X, cmd` (his: "in ctx, ..."), cmd = `spawn`, `await` of a task, `tasks()`, `do {...}`; inside a list it needs parentheses; ("A") `in X, do {...}` replaces `context X {...}` (6 uses in the ports to convert), and `context` only defines: `context { :enter {...} :leave {...} ... }`. The filter goes as `in <P> X, cmd` and `context <P> {...}` (the author, "ok"). Done on the branch `context` 7a9e637 (with #21 and main): the value is checked (`invalid context : expected :Context`), `leave` is optional, a value nested in itself applies once, a list needs no parentheses (`in` takes its own comma); cases context-28 to 33, suite with ASAN 1118, rt, env; manual rewritten. To do: `atmos-lang` measures (the 6 uses of `context X {...}` in the ports and the libraries that return the pair must change with the merge); the author approves; `atmos-lang` measured 7a9e637: suite passes, only the 4 port files with `context X {...}` (6 uses) stop compiling, 4 library functions to convert; the author approved the merge ("sim", 2026-11-26); asked `atmos-lang` to merge and send the ports their lines; merged into `main` by `atmos-lang` as d0fe0ac (2026-11-26): suite with ASAN, 48 configurations, sdl-pingus pass; env-ray, sdl-snxkx and env-pico got their lines. Closed (#21 and #22 with it: #22 is solved by a pool bound outside) |
| #24 | env-ray#51: platformer repeats `in <:draw> world,` three times (tiles, pool, player) | decision | answered through `atmos` (2026-11-26): the accepted cost of #23, recommend keeping (one block hides the pool again; an unscoped block was rejected by the author); possible refinement: the value carries its filter (`on=`, a simple value or a predicate function, since a pattern is not a value). Decided by the author (2026-11-26, "Nao. Mantemos como esta"): no `on`, the repetition stays. Closed |
| #15 | `pin` merged into `main` (8361f88, `set :out place = value`) and declared STABLE (`atmos-pin#9`, `TODO-pin.md` "Stable"): design #3 on top of it as fixed; a change to a rule of pin goes to `atmos-pin` first | work | done: #3 needed no change of pin; closed (2026-11-22) |

## Waiting for the author (one at a time, in this order)

(nothing)

- **#21** merge of the branch `context` (de24422), the block form.

(#1 to #5 decided; the branch `context` merged into `main` b71fbe4 on 2026-11-22, approved by the author: "merge")

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

## Settled by the session (2026-11-20), all decided by the author (2026-11-22)

Under the rule "Work with no input" (`TODO.md`, 2026-11-20, `#95`),
#1 to #5 are implemented with the recommendations on the branch
`context`, one test file per decision (`tst/cases/context-*.atm`,
`tst/ast/context-*.atm`): another answer changes one place.

| # | Taken | Tests |
|---|---|---|
| #1 | around each resume, lazy (decided 2026-11-22) | context-01 to 05 |
| #2 | no filter (decided 2026-11-22) | context-06 |
| #3 | `leave` receives what `enter` returned | context-07 |
| #4 | D (decided 2026-11-22): `context (enter, leave) {...}` or `context X {...}` | ast context-1, context-2, context-err-1; cases context-13 to 16 |
| #5 | `emit`, `spawn`, `abort`, `toggle` in `enter`/`leave` are errors (decided 2026-11-22) | context-08 to 10, 17, 18 |

Measured (cases context-11, context-12): 50 stars, 100 canvas switches per
frame today, 2 with a context around the pool, 0 for `:update` and `:key`;
the layer of env-pico does not leak to another task when the task that set
it awaits.

## Decided

- #1 (2026-11-22, the author, "B"): `enter` and `leave` run around each
  resume of code that lives in the block, lazily; see
  `261118-context-semantics.md`, sections 3 to 6 and 8.
- #2 (2026-11-22, the author, "sem filtro, mas deixar como TODO"): no
  filter of events; a filter is #19, known, not done.
- #3 (2026-11-22, the author, "sim"): `leave` receives what `enter`
  returned.
- #4 (2026-11-22, the author, "D"): `context (enter, leave) {...}` or
  `context X {...}`, X giving the pair `[enter, leave]`; `enter` and
  `leave` that are not functions are an error.
- #5 (2026-11-22, the author, "ok"): `emit`, `spawn`, `abort` and
  `toggle` inside `enter`/`leave`, also indirectly, are errors at runtime;
  the reason: they run other code in the middle of a switch (a nested
  switch, or the context freed while its `enter` runs).
