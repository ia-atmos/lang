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
| #3 | SNKRX bullet in Section 9 is a guess | Closed: 2026-11-19 |
| #4 | Sections 7 and 8 against the implementation | Closed: done 2026-11-15 |
| #5 | Section 9: reports of the ports | Partly: 12 pass (atmos-lang#81); SNKRX migrating |
| #6 | Cost of the migration under the current rules | Open |
| #7 | Rules not declared stable | Closed: stable 2026-11-19, but #7b |
| #8 | Experiment: CLBG in Atmos | Blocked: no Lua here; the flag now exists (`ATMOS_PIN`) |
| #9 | Related work: read dyn-lex | Done for dyn-lex and the benchmark repo |
| #10 | Fill the sections with text | Waiting: approval of the outline |
| #11 | Draft of Sections 3, 4, 8, 9 (`papers/pin-draft.md`) | By the session, to be confirmed |
| #12 | Câmara's scripts compile once: the stated flaw is not in them | Remark, to be confirmed |

## Open

- **#5 Reports of the ports** (was B3). Five are in (sdl-tux, doom,
  and the environments of SDL, libuv, and IUP), from `261115-pin.md`.
  Pending: pico, Flare, Lessmilk, SNKRX.
- **#6 Cost of the migration** (was B4). The figure of 230 of 830
  tests is from the first rules. Names may now hold unpinned values,
  owned places may be reassigned, and more decisions followed
  (2026-11-17), so a new count is needed for Section 9.
- **#7b The flag that removes the checks** (part of #7). Exists on `main`
  since 2026-11-21: `ATMOS_PIN=unchecked` and `ATMOS_PIN=off`
  (atmos-pin#3). First use: Doom, 2.8% (Section 9). Closed.

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

## By the session, to be confirmed (rule "Work with no input", 2026-11-20)

- **#11 Draft text.** `papers/pin-draft.md`: Sections 3, 4, 8, and the
  measured costs of 9, written on the stable rules while the outline (#10)
  waits. Reversible: it is a separate file; the outline is unchanged.
- **#13 Reference counting is orthogonal to the pin.** Confirmed by the
  author, 2026-11-21: counting is used for simplicity and could be replaced
  by a tracing collector, which would also solve the cycles. The outline
  and the draft now present the reclaimer as pluggable (Sections 0, 1, 4,
  5, 8, 10, 12). The wording of three rules in block terms (a dropped
  unpinned value, a retired value, a replaced native: "at the latest at
  the end of the block", counting being the earlier case) is by the
  session, to be confirmed; sent to `atmos-pin` for the manual.
- **#12 Remark on Câmara (2024).** The scripts in
  `alochaus/dyn-lex-benchmark` run `ceu ./x.ceu` once and then time only
  `./x.exe`, ten runs each. The flaw the author reported (recompilation in
  every timed run) is not in these scripts. The outline still says not to
  cite the numbers; whether the flaw lies elsewhere is for the author.
- **#8 blocked.** No Lua 5.4 interpreter here (only the library). Building
  Lua 5.4.6 from lua.org in the scratchpad was denied by the permission
  classifier, so neither Lua nor the C references of the benchmark were
  built. Needed: a Lua 5.4 interpreter in the environment.
- **#6 not done.** The ports are in other repositories, not in this
  session; the count from their history needs their sessions.
- **#5 partly.** 12 ports pass their suites on the final rules
  (atmos-lang#81, merge `64005fd`); sdl-snkrx is migrating and reports to
  atmos-pin. Per-port numbers are not in `ISSUES.md`.

## Closed

- **#3 SNKRX in Section 9** (was B1). The guessed bullet is replaced,
  2026-11-19, by its real cases from `atmos-pin`: a map body as a task in
  a world made by the caller, and a registry of springs, both by
  `set :out`.
- **#7 Rules not declared stable** (was B5). Declared stable by the author
  on 2026-11-19 (`atmos-pin`, main `8361f88`):
    - #7a decided: `set :out` (atmos-pin#1);
    - #7c confirmed: a task owns its `pub` and final value; an emitted
      event is owned by the emit; natives declare what they hold;
    - #7d confirmed: the `spawn` statement is the only implicit pin;
    - #7e the finalizer of native values (atmos-pin#2): not listed as open
      when the rules were declared stable;
    - #7b still open, above.

- **#1 Name and title** (was A1). Decided 2026-11-15: "Dynamic Values
  Deserve a Home: Structured Memory with Lexical Pins". The mechanism
  is "structured memory"; the construct is the "lexical pin".
- **#2 Section 4, the rules** (was A2). Approved 2026-11-15; Section 4
  follows every later decision as it is reported.
- **#4 Sections 7 and 8 against the implementation** (was B2). Done
  2026-11-15, from `261115-pin.md` and `261115-pin-migration.md`.
