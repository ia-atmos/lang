# TODO of the paper on pin

Everything pending in the `paper-pin` session, which writes
`papers/pin.md`: explicit, unanswered, or unacknowledged. Kept current
by that session.

## A. Decisions waiting for the author

1. ~~Name and title.~~ Decided by the author on 2026-11-15: "Dynamic
   Values Deserve a Home: Structured Memory with Lexical Pins". The
   mechanism is "structured memory"; the construct is the "lexical pin".
2. ~~Section 4, the rules.~~ Done on 2026-11-15: the author approved
   the rules in "Pin rules discussion", and Section 4 follows them.

## B. Pending in the outline

1. **SNKRX in Section 9.** The bullet "many short-lived entities and
   pools" is a guess: item 40 of `TODO.md` names no friction for SNKRX.
   To confirm with the plan of the port, or to remove.

2. ~~Sections 7 and 8 against the implementation.~~ Done on
   2026-11-15, from `261115-pin.md` and `261115-pin-migration.md`.
3. **Section 9, the reports of the ports.** Five are in (sdl-tux,
   doom, and the environments of SDL, libuv, and IUP), from
   `261115-pin.md`. Pending: pico, Flare, Lessmilk, SNKRX. The bullet
   of SNKRX in the case studies is still a guess (item B1).

4. **The cost of the migration under the current rules.** The figure
   of 230 of 830 tests is from the first rules. Names may now hold unpinned
   values and owned places may be reassigned (2026-11-17), so a new
   count is needed for Section 9.

## C. Next steps, not started

1. **Experiment: CLBG in Atmos.** Re-implement the 7 programs of Câmara
   (2024; github.com/alochaus/dyn-lex-benchmark): binary-trees,
   fannkuch-redux, fasta, fibonacci, mandelbrot, n-body, spectral-norm.
   Measure Atmos against Lua 5.4 and against Atmos with the checks
   disabled (a compile flag under discussion, which keeps end-of-block
   finalization). Compile once and time only the run: the earlier study
   included recompilation in every run, so its numbers are not cited.
2. **Related work: dyn-lex.** Read its manual (doc/manual.md) and
   runtime (src/main/kotlin/C.kt) for the side-by-side with the rules.
3. Fill the sections with text, after the author approves the outline.
