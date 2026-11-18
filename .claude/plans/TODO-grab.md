# TODO: grab (issue 22)

Session `atmos-grab`. Material: `261118-grab.md` (from atmos-lang),
`261118-grab-cases.md` (cases before and after, prior art).

## Decisions waiting for the author (asked one at a time)

1. ASKED. Authorization: keep the `grabbable` of the comment of the
   issue, found dynamically (the nearest in the tree of tasks, so that
   `Options` deep in the arena finds the one in `main.atm`), with a
   runtime error when there is none. Recommended: yes.
2. What is captured: an await pattern in both (`grabbable P`, `grab Q`;
   an event is captured if it matches both), so `<!:draw>` replaces
   `toggle ... with`. Recommended: yes.
3. Who receives: the `grab` block and all its descendants, including
   pools declared inside it; a pool outside it (but inside the
   `grabbable`) does not receive. Recommended: yes (as GTK).
4. Nesting: a stack per `grabbable`, the most recent grab wins; its end
   restores the previous one. Recommended: stack (the issue's "yes").
5. Time: a grab that captures the clock stops the timers outside it;
   on release they get the zero tick as `toggle t(true)`.
6. `grab` as an expression (the value of its block), as `do`.
7. Emits that do not traverse the `grabbable` (`emit @t` to a task
   below it): not captured? To check against the order of awaking.
8. Keyword: `grabbable` (comment) vs `grabbing` (the error text of the
   comment says `grabbing`).

## Decided

(none yet)

## Known work, not done

- After the decisions: implement it here, on the branch `grab` (rule
  of 2026-11-18, `TODO.md` "Standing rules", checked in the transcript
  of `atmos-lang`): tests with small programs and their expected
  output; before merging, bring `main` in, run `tst/run.sh`, `make
  test-rt test-env` with `ATMOS_ASAN=1`; ask `atmos-lang`
  (session_011eECogKVQ7ZzmWCoMST2wK) to measure the ports if existing
  programs may change (`toggle` stays, so probably not); merge only
  after the author approves the design. `atmos-context` and `pool`
  also touch `rt/task.c`: coordinate.
- Tell `atmos-context` (issue 21) what is decided about resuming
  (zero tick on release).

## Reported, not acknowledged

- The comment of the issue (could not be read by ceu-atmos) was read
  here: it introduces `grabbable`. See `261118-grab-cases.md` §0.
- The 7GUIs case is weak (IUP already does the modality) and env-ray's
  is neutral (the pause is already outside the game); SNKRX is the case.
- SNKRX today: the shop's buttons and cards still answer the mouse
  behind the options modal (the `eng.slow` freezes time, not input).
- The author pasted the summary of another chat (2026-11-18): it agrees
  (stack, SNKRX the clearest case) but misses the `grabbable` of his
  comment and the mouse leaking in the shop; told him. Q1 asked again.
