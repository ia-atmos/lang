# TODO of the paper on games

Everything pending in the `paper-games` session, which writes
`papers/games.md`: explicit, unanswered, or unacknowledged. Kept
current by that session.

Moved here on 2026-11-15, by decision of the author, from the
repository `ia-atmos/paper-src-games` (session `src-games`), at its
last commit `7ff8f88`. That repository is to be removed. The list of
this file was last updated there on 2026-11-14.

## A. Decisions waiting for the author

Each item has the recommended option.

1. **Venue of the paper.**
   Recommended: ‹Programming› first, Software: Practice and Experience as
   the alternative. Onward! only with a broader thesis. Games venues
   (IEEE Transactions on Games, FDG) not recommended: scope risk.
2. **Which CAPES rule matters for the venue.** The program rule (Qualis
   2017-2020 or 2021-2024) or the CAPES 2025-2028 cycle (per article, no
   Qualis list).
   Recommended: decide by scope first, then check the Scopus percentile of
   the chosen journal, which serves both rules.
3. **Metrics the ports record from the start.** Proposed: LOC of original
   vs port, occurrences of each mechanism, state machines and callbacks
   removed, defects of the original made impossible, memory, CPU.
   Recommended: fix the list now and send it to the port sessions.
4. **Organization of the paper.** The alternative is in
   `papers/games.md`, under "Alternative organization". The author said
   "to be defined" on 2026-11-14 and asked to be reminded: bring it up
   again.
   Recommended: the alternative (patterns as their own section, by
   pattern; case studies by question).
5. **`env-pico` in the paper.** Not answered yet.
   Recommended: leave out unless its three games add a new category.
6. **FRP in related work.** Marked "?" in the outline.
   Recommended: keep, one short paragraph (Elm, Yampa).

7. **Design of the change study with AI.** Idea of the author
   (2026-11-14): cost of adding a feature, e.g. a new weapon in Doom, in
   tokens, lines of code, and scattering.
   Recommended: same task, model, and prompt on original and port;
   several tasks per game and several runs per task; acceptance check
   per task; scattering as files, functions, and hunks touched, plus the
   "degree of scattering" metric of the aspect-oriented literature.
   Bias to state: the model knows the original code and C from training
   and does not know Atmos.

## B. Work known and not done

1. **Plan for the paper.** Not received from the compiler session
   (`ia-atmos/lang`). No port work started, by instruction.
2. **List of A1 venues, Qualis CAPES, Engenharias IV.** Not done: Sucupira
   and the mirrors are outside the allowed network of this session. Ways
   out: the author runs the prepared prompt in a normal chat, puts the
   Sucupira spreadsheet in the repository, or allows
   `sucupira.capes.gov.br` and `www.gov.br`.
3. **Rule of Engenharias IV for the 2025-2028 cycle.** Area document not
   found.
4. **Sweeney's argument.** Find the exact quote and reference (talk
   "The Next Mainstream Programming Language", POPL 2006) on trading 10%
   of performance for productivity.
5. **Originals of `env-ray`.** Confirm they are in C, as the "simple
   static" category assumes.
6. **Citations for coroutines and async/await in games.** The examples
   in the outline (Unity, Lua/LÖVE, Roblox, Defold, Godot, Unreal) were
   written from memory; verify each and find the reference.
7. **Text of the paper.** Only `papers/games.md` exists, an outline. No instruction
   yet to start writing.

## C. Reported or decided, not acknowledged

1. Venue order (item A1) and the reasons for it.
2. Qualis findings, all from unofficial lists: Software: Practice and
   Experience appears as A2 or A3, not A1; Journal of Systems and Software
   as A2; Entertainment Computing as B1.
3. The Qualis stratum is one per journal (set by the parent area), and the
   2025-2028 cycle does not use Qualis Periódicos.
4. The outline committed (`39e5b1d`, the first commit of the old
   repository; now `papers/games.md` here). SNKRX and "expressiveness" were confirmed; the author
   answered the remarks on the structure on 2026-11-14 (conclusion
   added, evaluation inside the ports section, coroutines in related
   work).
5. Prompt for a normal chat (Qualis search, old and new criteria)
   delivered; no result came back yet.

## D. Older items

1. The compiler repository is `ia-atmos/lang` (was
   `fsantanna/ceu-atmos`), and the paper is now inside it.
2. The paper lived in `ia-atmos/paper-src-games` (before that,
   `fsantanna/src-games`) until 2026-11-15, and is now in
   `ia-atmos/lang`, under `papers/`.
3. `env-ray` enters the paper as "simple static" (decided 2026-11-14);
   `env-pico` is open, see A5.
4. Decided on 2026-11-14: the section is named "Case Studies"; CPU is
   reported as whole frame and as game logic alone; coroutines and
   async/await are framed as an incomplete trend.
