# TODO of the paper on pin

Everything pending in the `paper-pin` session, which writes
`papers/pin.md`: explicit, unanswered, or unacknowledged. Kept current
by that session.

Items are `#N`, unique in this session and never reused (standing rule
of the author, 2026-11-18). From another session: `session#N`. The
letters A1, B1, C1... used before that date stay valid for what was
written; each such item now has its number below.

Since 2026-11-24 (`atmos`, atmos-lang#119, handoff
`.claude/plans/261124-atmos.md`): coordination, status and questions for
the author go to `atmos`; measurements on the ports and merges into `main`
stay with `atmos-lang`. Item numbers stay local to this session (rule
#71, corrected by `atmos` on 2026-11-25): the next is #18, seen from
outside as paper-pin#18. Resumed on
2026-11-25 (author: "desbloqueado, vá em frente").

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
| #8 | Experiment: CLBG in Atmos | Done 2026-11-25 (`papers/pin-bench/`) |
| #9 | Related work: read dyn-lex | Done for dyn-lex and the benchmark repo |
| #10 | Fill the sections with text | Done 2026-11-25 (author: "vá em frente"), in the draft |
| #11 | Draft of all sections (`papers/pin-draft.md`) | By the session, to be confirmed |
| #17 | Why compiled Atmos loses to the Lua 5.4 VM | Done: answered, two fixes on main `306f7e9`, fourth run on `ce8e8da`, 2026-11-26 (1.0x to 2.3x) |
| #16 | Atmos slower than Lua on the CLBG: diagnosis sent to atmos-lang | Done: three remedies on main `9ea77e1`; rerun 2026-11-26 |
| #15 | Rerun the CLBG after atmos-pin#32 reaches main | Done 2026-11-26: +2% to +15% (was +19% to +33%) |
| #12 | Câmara's scripts compile once: the stated flaw is not in them | Remark, to be confirmed |

## Open

- **#17 Why a compiled Atmos loses to the Lua 5.4 interpreter** (asked to
  `atmos-lang`, 2026-11-26, at the author's word). After the rerun on
  `9ea77e1`, Atmos off is 1.0 to 3.0 times Lua 5.4. Asked: the central
  reason, whether and how far it can improve, and which kernels could reach
  1.0. Answered 2026-11-26: compiling removes only the dispatch; values
  are 16-byte tagged cells that go through memory at each operation
  (store-forwarding stalls, a temporary per assignment), table access is
  a chain of calls, and counting costs per copy. Two fixes measured by
  hand (store by fields; compute into the destination): `x = x + 1.5`
  0.47 -> 0.21 s (Lua 0.22), mandelbrot 1.30 -> 1.00 (Lua 0.63).
  Estimated ceilings: days of work for mandelbrot, spectral-norm, fasta
  near 1.0 to 1.2; inline table paths for n-body and fannkuch near 1.5;
  direct calls for fib near 1.0; numeric locals in C locals (weeks) to
  beat Lua. `atmos-lang` implements the two fixes and tells the commit.
  To do: rerun, and update Section 9.

- **#16 Atmos slower than Lua 5.4 on the CLBG** (sent to `atmos-lang`,
  2026-11-26, at the author's request). Independent of pin: with
  `ATMOS_PIN=off`, mandelbrot runs 963 M instructions against 417 M on
  Lua 5.4 (callgrind, N=200). Causes: operands copied to temporaries and
  cleared, arithmetic out of line with a status test, locals in the heap
  frame of the task, the numeric loop as an iterator. Asked for a
  measurement of a game against its Lua version, to cite beside the
  kernels. To do: rerun the benchmark if the generator changes; decide
  with the author whether Section 9 keeps the Lua column with this
  explanation (the session's recommendation).

- **#15 Rerun the CLBG after atmos-pin#32** (to do, no decision needed).
  atmos-pin reduced the cost of the checks (branch `pin` `c63c084`: the end
  of a block unnames only its variables; no test for a value that is never
  an object). Its numbers against `off`: binary-trees +8%, fannkuch +5%,
  fasta +4%, fib +10%, mandelbrot +3%, n-body +16%, spectral-norm +1%
  (were +16% to +31%). When it reaches `main`, rerun `papers/pin-bench`
  and update Section 9 and the remark on elision in Section 12.

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
  spectral-norm. Variants (author, 2026-11-23): C, Lua 5.4, LuaJIT
  (context only), and Atmos with `ATMOS_PIN=off`, `unchecked` and the
  default; time and peak RSS. Compile once and time only the run: the earlier study
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
- **#14 Cycle collection removed** (atmos-pin#18, branch `pin` at
  `3cd5169`, author's choice "não checar"). The old rule is restored in the
  outline and the draft: a cyclic structure must be pinned; an unpinned
  cycle leaks under counting. The trial deletion and its cost are removed
  from Sections 4, 8 and 9. On `main` since 2026-11-21 (`f999c52`); #19,
  #23 and #29 since 2026-11-22 (`0b5b8ad`).
- **#12 Remark on Câmara (2024).** The scripts in
  `alochaus/dyn-lex-benchmark` run `ceu ./x.ceu` once and then time only
  `./x.exe`, ten runs each. The flaw the author reported (recompilation in
  every timed run) is not in these scripts. The outline still says not to
  cite the numbers; whether the flaw lies elsewhere is for the author.
- **#8 blocked.** No Lua 5.4 interpreter here (only the library). Building
  Lua 5.4.6 from lua.org in the scratchpad was denied by the permission
  classifier, so neither Lua nor the C references of the benchmark were
  built. Needed: a Lua 5.4 interpreter in the environment. Checked
  2026-11-23: `lua5.4` and `luajit` are in Ubuntu universe, which the
  sources list, but the package lists were never fetched; gcc and make are
  here. The author proposed waiting for next week's credits.
- **#6 not done.** The ports are in other repositories, not in this
  session; the count from their history needs their sessions.
- **#5 partly.** 12 ports pass their suites on the final rules
  (atmos-lang#81, merge `64005fd`); sdl-snkrx is migrating and reports to
  atmos-pin. Per-port numbers are not in `ISSUES.md`.

## Closed

- **#13 Reference counting and the semantics.** Decided by the author on
  2026-11-21 (atmos-pin#29): deterministic reference counting is part of
  the semantics; a value no longer referred to is finalized at once. The
  block wording of rule 4 was withdrawn; the outline and the draft follow
  the manual (branch `pin` at `443736c`). Also there: the warning for an
  unpinned recursive closure (atmos-pin#19) and `set :out pub` (#23).

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
