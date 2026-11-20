# TODO: grab (issue 22)

Session `atmos-grab`. Material: `261118-grab.md` (from atmos-lang,
where this is `atmos-lang#39`), `261118-grab-cases.md` (cases before
and after, prior art).

Items are `#N` (standing rule of 2026-11-18, `TODO.md`): unique here,
never reused nor changed. The questions asked as "questão 1..8" before
the rule are `#1`..`#8`.

## Index

| # | Item | State |
|---|------|-------|
| #1 | Authorization by `grabbable`, found dynamically | by the session, to be confirmed (branch `grab`) |
| #2 | What is captured: patterns in both | by the session, to be confirmed |
| #3 | Who receives: the `grab` block and its descendants | by the session, to be confirmed |
| #4 | Nesting: a stack per `grabbable` | by the session, to be confirmed |
| #5 | Time: timers outside a grab pause; no zero tick on release | by the session, to be confirmed |
| #6 | `grab` as an expression | by the session, to be confirmed |
| #7 | Emits that do not traverse the `grabbable`: not captured | by the session, to be confirmed |
| #8 | Keyword: `grabbable` (and `grab`) | by the session, to be confirmed |
| #9 | The comment of the issue, read here | reported |
| #10 | 7GUIs weak, env-ray neutral, SNKRX the case | revised 2026-11-19: 7GUIs is a strong case too |
| #11 | SNKRX: the shop answers the mouse behind the options | reported |
| #12 | The summary pasted from another chat | reported |
| #13 | Implement on the branch `grab` | done, 4aed336; ports asked to `atmos-lang`; merge is the author's |
| #14 | Tell `atmos-context` about resuming | sent 2026-11-20: grab does not touch the resume |
| #15 | `set :out` as an alternative to a dynamic `grabbable` | assessed: not comparable without a new design (#16) |
| #16 | A grabbable as a value (`grab @g P {...}`), the lexical alternative to #1 | noted, not proposed |
| #17 | The blocks register before they start: an emit the block makes in its first run is already captured | by the session, to be confirmed |

## Decisions waiting for the author (asked one at a time)

- #1 ASKED. Authorization: keep the `grabbable` of the comment of the
  issue, found dynamically (the nearest in the tree of tasks, so that
  `Options` deep in the arena finds the one in `main.atm`), with a
  runtime error when there is none. Recommended: yes.
- #2 What is captured: an await pattern in both (`grabbable P`, `grab
  Q`; an event is captured if it matches both), so `<!:draw>` replaces
  `toggle ... with`. Recommended: yes.
- #3 Who receives: the `grab` block and all its descendants, including
  pools declared inside it; a pool outside it (but inside the
  `grabbable`) does not receive. Recommended: yes (as GTK).
- #4 Nesting: a stack per `grabbable`, the most recent grab wins; its
  end restores the previous one. Recommended: stack (the issue's "yes").
- #5 Time: a grab that captures the clock stops the timers outside it;
  on release they get the zero tick as `toggle t(true)`.
- #6 `grab` as an expression (the value of its block), as `do`.
- #7 Emits that do not traverse the `grabbable` (`emit @t` to a task
  below it): not captured? To check against the order of awaking.
- #8 Keyword: `grabbable` (comment) vs `grabbing` (the error text of the
  comment says `grabbing`).

## Decided

(none by the author yet)

## By the session, to be confirmed (2026-11-20, rule "Work with no input")

Implemented on the branch `grab` (ia-atmos/lang, 4aed336, `main`
merged in, `tst/run.sh` 1006 pass and `make test-rt test-env` pass,
with `ATMOS_ASAN=1`). Each decision has its test, so that another
answer changes one place:

| # | Choice | Test |
|---|--------|------|
| #1 | nearest `grabbable` in the tree of tasks; else `invalid grab : expected enclosing grabbable` | `tst/cases/grab-01`, `grab-err-01` |
| #2 | await patterns in both; captured if both match | `grab-02` |
| #3 | the grab block, its tasks and its pools; a pool outside it does not | `grab-03` |
| #4 | a stack: the most recent grab whose patterns match wins | `grab-04` |
| #5 | the timers outside pause (they get no clock); no zero tick on release | `grab-05` |
| #6 | `grab` evaluates to its block | `grab-06` |
| #7 | an emit that does not traverse the grabbable is not captured | `grab-07` |
| #8 | keywords `grabbable` and `grab`; still valid as table keys and fields | `tst/ast/grab-01` |

The real cases, as they are today and with `grab`, side by side:
`tst/cases/grab-case-snkrx-0` / `-1` (the shop answers the mouse behind
the options, #11) and `grab-case-7guis-0` / `-1` (undo still alive
while adjusting). The manual has a section "Grab" on the branch.

Runtime (`rt/task.c`): the emit traversal only. `emit_trail` wraps the
old body; a capture (`ACap`, keyed by the time of the emit) follows the
path from the grabbable down to the grab, a copy taken when the
grabbable is reached; the trails on the path do not awake. `trail_off`
removes a grab from its grabbable.

## Known work, not done

- #13 After the decisions: implement it here, on the branch `grab`
  (rule of 2026-11-18, `TODO.md` "Standing rules", checked in the
  transcript of `atmos-lang`): tests with small programs and their
  expected output; before merging, bring `main` in, run `tst/run.sh`,
  `make test-rt test-env` with `ATMOS_ASAN=1`; ask `atmos-lang`
  (session_011eECogKVQ7ZzmWCoMST2wK) to measure the ports if existing
  programs may change (`toggle` stays, so probably not); merge only
  after the author approves the design. `atmos-context` and `pool`
  also touch `rt/task.c`: coordinate.
- #14 Tell `atmos-context` (issue 21) what is decided about resuming
  (zero tick on release, #5).

## Reported, not acknowledged

- #9 The comment of the issue (could not be read by atmos-lang) was
  read here: it introduces `grabbable`. See `261118-grab-cases.md` §0.
- #10 The 7GUIs case is weak (IUP already does the modality) and
  env-ray's is neutral (the pause is already outside the game); SNKRX
  is the case.
  Revised (2026-11-19, from a summary the author pasted): 7GUIs is a
  strong case, the need said by the waiting branch with no cooperation
  of its siblings; it also asks for a filter (#2). See the cases, §4.
- #11 SNKRX today: the shop's buttons and cards still answer the mouse
  behind the options modal (the `eng.slow` freezes time, not input).
- #12 The author pasted the summary of another chat (2026-11-18): it
  agrees (stack, SNKRX the clearest case) but misses the `grabbable` of
  his comment and the mouse leaking in the shop; told him. #1 asked
  again.
- #16 A lexical alternative to #1 would hand the grabbable as a value
  (`pin g = grabbable P {...}` and `grab @g Q {...}`, the value passed
  down with `set :out` or as an argument). In SNKRX it would have to
  travel from `main.atm` through the screens to `Options`. Not
  proposed: noted for the author, against #1.
- #17 A block registers before it starts, so that a grab in the first
  run of a grabbable finds it.
- #15 `atmos-lang` (2026-11-19): `main` merged `pin`, with `set :out
  place = value` (a place outside refers to a pinned value until the
  block of the `set` ends). It gives a lexical way to hand a
  `grabbable` to an inner block. It does not change the recommendation
  of #1: `Options` is spawned by a task that does not hold the
  `grabbable`, so the dynamic lookup is still what SNKRX needs.
