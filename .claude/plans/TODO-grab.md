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
| #1 | Authorization by `grabbable`, found dynamically | asked, waiting |
| #2 | What is captured: patterns in both | waiting |
| #3 | Who receives: the `grab` block and its descendants | waiting |
| #4 | Nesting: a stack per `grabbable` | waiting |
| #5 | Time: timers outside a grab, zero tick on release | waiting |
| #6 | `grab` as an expression | waiting |
| #7 | Emits that do not traverse the `grabbable` | waiting |
| #8 | Keyword: `grabbable` or `grabbing` | waiting |
| #9 | The comment of the issue, read here | reported |
| #10 | 7GUIs weak, env-ray neutral, SNKRX the case | revised 2026-11-19: 7GUIs is a strong case too |
| #11 | SNKRX: the shop answers the mouse behind the options | reported |
| #12 | The summary pasted from another chat | reported |
| #13 | Implement on the branch `grab` | after the decisions |
| #14 | Tell `atmos-context` about resuming | after #5 |
| #15 | `set :out` (branch `pin`, merged) as an alternative to a dynamic `grabbable` | noted, bears on #1 |

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

(none yet)

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
- #15 `atmos-lang` (2026-11-19): `main` merged `pin`, with `set :out
  place = value` (a place outside refers to a pinned value until the
  block of the `set` ends). It gives a lexical way to hand a
  `grabbable` to an inner block. It does not change the recommendation
  of #1: `Options` is spawned by a task that does not hold the
  `grabbable`, so the dynamic lookup is still what SNKRX needs.
