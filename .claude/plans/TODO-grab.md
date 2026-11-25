# TODO: grab (issue 22)

Session `atmos-grab`. Material: `261118-grab.md` (from atmos-lang,
where this is `atmos-lang#39`), `261118-grab-cases.md` (cases before
and after, prior art).

Items are `#N` (standing rule of 2026-11-18, `TODO.md`): unique here,
never reused nor changed. The questions asked as "questão 1..8" before
the rule are `#1`..`#8`.

## Paused (2026-11-23)

The author pauses new steps until next week. #26 is merged (2026-11-23).
#23 and #24 are closed (the ports rewrote on 2026-11-22). Next decision:
#21 (1), a default `stage`.

Coordination (2026-11-24, atmos-lang#119): status and questions for the
author go to `atmos` (session_011eECohhJbc4GMpAj3yyeEo); measurements on
the ports and merges into `main` stay with `atmos-lang`. New items put to
the author keep this file's own `#N` sequence (local, seen from outside as
`atmos-grab#N`; corrected by `atmos` on 2026-11-25, the author's rule #71).

## Index

| # | Item | State |
|---|------|-------|
| #1 | Authorization by `grabbable`, found dynamically | by the session, to be confirmed (branch `grab`) |
| #2 | What is captured: patterns in both | by the session, to be confirmed |
| #3 | Who receives: the `grab` block and its descendants | by the session, to be confirmed |
| #4 | Nesting: a stack per `grabbable` | by the session, to be confirmed |
| #5 | Time: timers outside a grab pause; no zero tick on release | decided by the author 2026-11-22: kept; the pattern chooses (`<!:draw>` freezes, a predicate lets the clock pass) |
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
| #18 | Ports measured: the keyword `grab` breaks one parameter in env-sdl (`sdl.atm:1110`), copied in Pingus, used by sdl-tux | decided by the author 2026-11-22: rename the parameter. Already done by the ports: env-sdl `sdl.display` now takes `grabbed` (sdl.atm:1280), and Pingus no longer copies `sdl.atm` (it uses `../env-sdl`); its `OPTIONS.grab` is a field, still valid. env-sdl passes the Atmos stage with the `grab` compiler (stops only at the missing SDL headers here). Nothing to change |
| #17 | The blocks register before they start: an emit the block makes in its first run is already captured | by the session, to be confirmed |
| #26 | Cleanup: internal names follow mute, stage and solo (E_MUTE, E_SOLO, a_mute, a_trail_solo, A_STAGE, muted...), branch `grab` eb028ad; `261118-grab-cases.md` rewritten to the new syntax | done; merged into main by `atmos-lang` (no port calls the old names) |
| #25 | Performance regression of f416c03 in programs with no solo (doom variant +10%): fixed by a43cc01 (merged 7c85a14), the traversal compiled twice with the capture as a constant. doom on 7c85a14: main 259.8 M (259.4 before), variant 714.4 M (708.9 before), +0.8% left | closed by `atmos-lang` |
| #24 | Uses of `toggle` in the ports (`atmos-lang`, 2026-11-22): 27 lines in 5 repos (env-sdl 2, env-pico 3, sdl-pingus 3, env-ray 16, sdl-snxkx 2) plus 10 emits that drive a `toggle on :T` and flip their boolean. Forms: 10 `toggle t(false)`, 7 `toggle t(false) with :draw` (`mute <!:draw> t(true)`), 10 `toggle t(true)`, 4 `toggle on :T` (2 with `with :draw`). Tags as `:Shop.on [false]` read backwards under `mute`: flip the boolean or rename the tag (`:Shop.mute [true]`) | closed: the 7 ports rewrote on 2026-11-22 (ISSUES.md #115: doom 9182b14, env-sdl, env-pico 1e68b0c6, ray-lessmilk c9284df, sdl-snxkx 2b41ab0, sdl-pingus 4aed063, env-ray 4c713f2), checked by `atmos-lang` |
| #23 | Collisions of the names of #22 (`atmos-lang`, 2197 files of the ports, each word alone): `mute` 2 files, one local `pin mute` in lessmilk princess_quest/menu.atm:49 (2 lines); `solo` none; `stage` 4 files, one local `val stage` in doom fi.atm:301 (4 lines). No field named so anywhere | closed: the locals renamed by the ports on 2026-11-22 (ISSUES.md #115) |
| #22 | Syntax, by the author 2026-11-22: no `with`; one pattern right after the keyword names the events affected (none = all): `mute [<P>] t(b)`, `mute [P] on :T {B}`, `stage [P] {B}`, `solo [P] {B}`; bare pattern allowed in the block forms (as `await`), `<>` required in the task form (a bare call would read as a spawn). `mute` silences with `true` (inverse of `toggle`) | decided (form); `toggle` replaced at once (the author, 2026-11-22). Implemented on `grab` 644a014: 1046 pass with ASAN, test-rt and test-env pass; names `mute`/`solo`/`stage` wait on the measurement of collisions in the ports; every port that uses `toggle` must be rewritten (boolean inverted, `with P` becomes `<!P>`) |
| #21 | sdl-snxkx `exp-grab` (1167084, the shop behind Options): fits; the click at 42 buys nothing; 41/41, the 40 old outputs identical. Did not fit: (1) tests that spawn screens without `main.atm` need their own `grabbable` (asks a default one, or a grab with none to be a no-op); (2) a `grabbable` is a block, so a task pinned above it cannot escape into it (pin moved inside, lent with `set :out`); printers must stay outside; (3) the input must come back before the 0.25 s tween ends: Options split in two tasks; (4) the shop's tutorial stays `toggle` (it freezes the shop's time); (5) five patterns repeated: a name for a set of patterns | reported; (1) weighs against #1 (the error); (4) and the clock of #20 b both ask #5 again |
| #20 | env-iup `exp-grab` (local 3f0eadf, 7GUIs circles, +9 −6): fits; 15/15 plain and ASAN; a click on Undo during the dialog is denied with grab, acts without. Did not fit: (a) the look of a disabled dialog stays IUP's; (b) "all but the canvas and the time" is not obvious; (c) `grabbable` must be placed ahead around the reactions; (d) the patterns name what is denied, which reads backwards | reported; (b) answered: `grab <until ((it != :draw) && (type(it) != :number))>` lets the clock pass (checked on grab-05); time passing a grab by default is the author's (#5) |
| #19 | The real cases rewritten on branches of the ports (`atmos-lang` #112a, 2026-11-21): sdl-snxkx `exp-grab` (the shop), env-iup local `exp-grab` (7GUIs), built from `grab` | asked by `atmos-lang`; `grab` kept near `main` (c66c735) |

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

- Merge (2026-11-22): the author approved the merge of `grab` into `main` (f416c03), and with it #1–#4 and #6–#8 as implemented. 1048 pass with ASAN, test-rt and test-env pass. The ports must now rewrite their uses of `toggle` (#24) and two locals (#23).
- #22 (2026-11-22): the form of `mute`/`solo`/`stage`, see the index.
- #18 (2026-11-22): rename the parameter, keep the keyword `grab`; found already renamed in env-sdl (`grabbed`).
- #5 (2026-11-22): kept as is. The pattern of the grab decides whether the time stops; the manual shows the predicate that lets it pass (`tst/cases/grab-08`, branch `grab`).

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
- #18 `atmos-lang` measured `grab` 4aed336 against `main` 513767c
  (2152 files of 15 clones): 8 files stop compiling, all from one
  name, the parameter `grab` of `display` in env-sdl `sdl.atm:1110`
  (`func (full, w, h, grab, cursor)`, used as `$grab`), copied in
  Pingus `engine/sdl.atm` and reached by sdl-tux `game/main.atm`.
  Every other file gives identical C. One rename of a parameter in
  env-sdl and in the copy of Pingus, or another keyword (#8). Not
  asked of the ports: the keyword and the merge are the author's.
- #17 A block registers before it starts, so that a grab in the first
  run of a grabbable finds it.
- #15 `atmos-lang` (2026-11-19): `main` merged `pin`, with `set :out
  place = value` (a place outside refers to a pinned value until the
  block of the `set` ends). It gives a lexical way to hand a
  `grabbable` to an inner block. It does not change the recommendation
  of #1: `Options` is spawned by a task that does not hold the
  `grabbable`, so the dynamic lookup is still what SNKRX needs.
