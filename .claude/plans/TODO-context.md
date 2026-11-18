# TODO: context (issue 21)

Session `atmos-context`. Material: `261118-context.md`. Design:
`261118-context-semantics.md`. The issue itself is `atmos-lang#39`
(grab and context).

Items are `#N` (standing rule of 2026-11-18, `TODO.md`): unique in this
session, never reused; from another session, `session#N`.

## Index

| # | Item | Kind | State |
|---|---|---|---|
| #1 | When enter and leave run: around each resume (lazy) or once per emit | decision | asked 2026-11-18, waiting |
| #2 | Filter: bracket every reaction inside, or only some events (`with :draw`) | decision | after #1 |
| #3 | `leave` receives what `enter` returned | decision | after #2 |
| #4 | Syntax | decision | after #3 |
| #5 | Errors: `emit`, `spawn`, `abort` inside `enter`/`leave` | decision | after #4 |
| #6 | SNKRX and env-ray not readable by this session | remark | open |
| #7 | The three comments of issue 21 not readable | remark | open |
| #8 | The canvas of the stars: `STARS` (author) or `BACK` (material) | remark | open, check in SNKRX |
| #9 | Implement `context` on the branch `context` | work | waits for the design |
| #10 | Tell `atmos-grab` (and `pool`) what #1 decides: all touch the resume of a trail in `rt/task.c` | work | after #1 |
| #11 | Non-drawing events (the author's question): with the lazy resume, a key costs nothing unless a task inside awakes on it | answer | reported, not acknowledged |
| #12 | `lua-atmos` has no `context`; `atmos-lang/atmos` has only a plan, once per emit | remark | reported, not acknowledged |
| #13 | No real case awaits between set and restore: `context` fixes no bug today, it saves writing and makes the stars one switch per pool | remark | reported, not acknowledged |
| #14 | The case of `env-pico` for #1 and #3 | input | reported, not acknowledged |

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

## Decided

(nothing yet)
