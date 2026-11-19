# TODO of the paper on pin

Everything pending in the `paper-pin` session, which writes
`papers/pin.md`: explicit, unanswered, or unacknowledged. Kept current
by that session.

Items are `#N`, unique in this session and never reused (standing rule
of the author, 2026-11-18). From another session: `session#N`. The
letters A1, B1, C1... used before that date stay valid for what was
written; each such item now has its number below.

## Index

| # | Item | State |
|---|------|-------|
| #1 | Name and title | Closed: decided 2026-11-15 |
| #2 | Section 4 follows the approved rules | Closed: done, kept current |
| #3 | SNKRX bullet in Section 9 is a guess | Open |
| #4 | Sections 7 and 8 against the implementation | Closed: done 2026-11-15 |
| #5 | Section 9: reports of the ports | Open: 4 pending |
| #6 | Cost of the migration under the current rules | Open |
| #7 | Rules not declared stable | Open: waiting for `atmos-pin` |
| #8 | Experiment: CLBG in Atmos | Planned |
| #9 | Related work: read dyn-lex | Planned |
| #10 | Fill the sections with text | Waiting: approval of the outline |

## Open

- **#3 SNKRX in Section 9** (was B1). The bullet "many short-lived
  entities and pools" is a guess: item 40 of `TODO.md` names no
  friction for SNKRX. To confirm with the plan of the port, or remove.
- **#5 Reports of the ports** (was B3). Five are in (sdl-tux, doom,
  and the environments of SDL, libuv, and IUP), from `261115-pin.md`.
  Pending: pico, Flare, Lessmilk, SNKRX.
- **#6 Cost of the migration** (was B4). The figure of 230 of 830
  tests is from the first rules. Names may now hold unpinned values,
  owned places may be reassigned, and more decisions followed
  (2026-11-17), so a new count is needed for Section 9.
- **#7 Rules not declared stable** (was B5, 2026-11-18). Open with the
  author, per `atmos-pin` (session_011eECogSDhkVfyt7qme5DLh, branch
  `pin`), which will report:
    - #7a ~~a mechanism for an outer place to hold an inner value (case
      E)~~: decided 2026-11-19, `set :out` (atmos-pin#1, main `8361f88`);
    - #7b the flag that removes the checks;
    - #7c confirmation of three implementation choices (a task owns its
      `pub` and final value; an emitted event is owned by the emit;
      natives declare what they hold);
    - #7d `spawn` as the only implicit pin;
    - #7e the finalizer of native values (atmos-pin#2).

## Planned

- **#8 Experiment: CLBG in Atmos** (was C1). Re-implement the 7
  programs of Câmara (2024; github.com/alochaus/dyn-lex-benchmark):
  binary-trees, fannkuch-redux, fasta, fibonacci, mandelbrot, n-body,
  spectral-norm. Measure Atmos against Lua 5.4 and against Atmos with
  the checks disabled (a compile flag, #7b, which keeps end-of-block
  finalization). Compile once and time only the run: the earlier study
  included recompilation in every run, so its numbers are not cited.
- **#9 Related work: dyn-lex** (was C2). Read its manual
  (doc/manual.md) and runtime (src/main/kotlin/C.kt) for the
  side-by-side with the rules.
- **#10 Fill the sections with text** (was C3), after the author
  approves the outline.

## Closed

- **#1 Name and title** (was A1). Decided 2026-11-15: "Dynamic Values
  Deserve a Home: Structured Memory with Lexical Pins". The mechanism
  is "structured memory"; the construct is the "lexical pin".
- **#2 Section 4, the rules** (was A2). Approved 2026-11-15; Section 4
  follows every later decision as it is reported.
- **#4 Sections 7 and 8 against the implementation** (was B2). Done
  2026-11-15, from `261115-pin.md` and `261115-pin-migration.md`.
