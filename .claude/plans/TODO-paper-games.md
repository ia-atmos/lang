# TODO of the paper on games

Everything pending in the `paper-games` session, which writes
`papers/games.md`: explicit, unanswered, or unacknowledged. Kept
current by that session.

Moved here on 2026-11-15, by decision of the author, from the
repository `ia-atmos/paper-src-games` (session `src-games`), at its
last commit `7ff8f88`. That repository is to be removed. The list of
this file was last updated there on 2026-11-14.

**Paused by the author on 2026-11-20** ("por enquanto nao vamos fazer
nada aqui"), said in this session: it overrides "Work with no input"
(`atmos-lang#95`) here. No work on the paper until he says so. Answers
of the ports are only recorded below. The reminder of 2026-11-24 (#1,
#2) stays scheduled, since he asked for it.

## Index of items (`#N`)

Rule of the author, 2026-11-18 (`.claude/plans/TODO.md`, "Standing
rules"): everything put to him has a number, unique in this session,
in order, never reused nor changed. From another session:
`paper-games#N`. The identifiers of the sections below (`A4`, `B6`)
stay valid for what was written with them; an item gets a number the
next time it is put to him.

| # | Title | State |
|---|-------|-------|
| #1 | Organization of the paper (was A4) | Open. Reminder on 2026-11-24 |
| #2 | Title and venue of the Pingus paper (E, references) | Checked on the web on 2026-11-20 (`papers/games-related.md`, 2.1): SBGames 2018, single author, pp. 591-600, DOI 10.1109/SBGAMES.2018.00036. By the session, to be confirmed by him |
| #3 | Work with no input (`atmos-lang#95`, 2026-11-20): data of the ports, references checked, drafts, changes of the language | Done on 2026-11-20, by the session, to be reviewed. See G |
| #4 | Common method for the CPU of the game logic alone (part of A3) | Set on 2026-11-20, by the session, to be confirmed. See G |

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
   "to be defined" on 2026-11-14 and asked to be reminded. Reminded on
   2026-11-16 and 2026-11-17; on 2026-11-17 he said he still does not
   know and asked to be reminded in 7 days: a scheduled task fires in
   this session on 2026-11-24, 12:00 UTC, for this item and for the
   title and venue of the Pingus paper.
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

1. **Plan for the paper.** None exists. The old session waited for one
   from the compiler session and never got it. No port was read yet.
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
8. **The rules of `pin` changed on 2026-11-17**, twice (reported by the
   compiler session; the second version approved by the author). Now:
   a pinned value never leaves its block (a block gives away a `val`);
   `pin val` and `pin var`; parameter modifiers (`func f (var x, pin
   y)`); declarations of prototypes are names; a pool held by `val`
   works. The ports are migrating again. Never cite the migration
   numbers reported before (about 1770 lines, about 240 rewrites), nor
   any until the ports report on these rules. Any `pin` code in the
   paper follows `.claude/plans/261117-pin-migration-2.md` and the
   manual, "Pin".

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

## E. Handoff of the old session (2026-11-15)

Notes sent by the session `src-games` when the paper moved. They are
the notes of a peer: everything marked unverified was written from
memory or from search snippets, and is to be checked before use.

### What the author said, where the files lose it

- 2026-11-13: "A ideia do artigo é explorar os mecanismos de
  concorrência estruturada para jogos. Faremos uma discussão qualitativa
  e mostrar os resultados quantitativos dos projetos que estamos
  trabalhando."
- 2026-11-13: he thought of Software: Practice and Experience as an
  alternative venue, and asked for venues A1 in Qualis CAPES,
  Engenharias IV. So A1 in that area matters to him.
- 2026-11-14, on the C++ baselines: "O desfavorecimento em C++ talvez
  fique abaixo de 10-20% o que ja seria um resultado positivamente
  inesperado", and on the two CPU numbers: "Sim, importante separar as
  duas. Ali podemos usar o argumento do Sweeney sobre pagar mais 10% por
  melhor expressividade".
- 2026-11-14, on coroutines: "O interessante é colocar como uma
  tendência, mas incompleta, ainda mais sabendo que linguagens síncronas
  existem desde 1980".
- 2026-11-14, on `env-ray`: his doubt was that it compares to Lessmilk
  and not to the complex games in C++. The table of two axes answered
  it ("Perfeito").
- 2026-11-14, on Lessmilk: "foco em aprendizado e exploração". He seems
  to value a narrative per port, which the alternative organization
  loses: that trade-off is why A4 is open.
- He writes from a phone: "snxkx" is SNKRX, "exclusividade" was
  "expressividade", "sessões" was "seções". He answers numbered lists
  by number.

### Reasons behind the recommendations of section A

- A1, ‹Programming› first: the contribution is a language evaluated
  through games, not games as such. Games venues tend to ask for AI,
  design, or player studies. ‹Programming› accepts case studies and
  rewrites as evidence, and has several rounds a year. SP&E fits, but
  its review is slow and it is less seen by language people.
  Onward! and SLE need a stronger thesis than "we ported N games".
  Short venues, if wanted first: REBLS (SPLASH), GAS (ICSE). Scopes and
  deadlines unverified.
- A3, metrics fixed early: to reconstruct them after the ports are done
  is expensive.
- "Case Studies" and not "Experiments": the latter invites reviewers to
  demand the rigor of controlled experiments.
- Two CPU numbers: rendering dominates the frame, and the frame alone
  hides the cost of the logic. Baselines on both sides: Lua and JS
  originals favor Atmos, C and C++ do not, and only one side looks like
  cherry-picking.
- A4, the alternative: in the first organization each port presents the
  same patterns again.
- A7, the change study, beyond what A7 says: tokens of input and output
  separately; input tokens depend on the size of the code base, so the
  cost of reading must be separated from the cost of the change; a
  control with no AI, the scattering of the diff of a reference
  implementation; the bias works against the port, so a favorable
  result is stronger and an unfavorable one is inconclusive.
  "Scattering" is the term of the aspect-oriented literature, with
  "tangling". References, unverified: "degree of scattering" (Eaddy and
  others, about 2007-2008); "concern diffusion" metrics (Sant'Anna,
  Garcia and others, about 2003; another Sant'Anna, not the author);
  Kiczales and others, ECOOP 1997.

### Venues and Qualis, all unverified (unofficial lists, snippets)

- Software: Practice and Experience: A2 in a Computing list of 2017, A3
  in a page labeled 2017-2020.
- Journal of Systems and Software: A2 (2017). Entertainment Computing:
  B1 (2017).
- IEEE Transactions on Games, Science of Computer Programming,
  ‹Programming›: nothing found. Scimago snippet for IEEE Transactions
  on Games: SJR 0.551, Q2, 2025.
- Sources:
    - https://medium.com/@aserg.ufmg/qualis-capes-ranking-of-software-engineering-journals-2013-2016-a76fbb5205a4
    - https://sites.google.com/view/ivanmachado/se-confsjournals
    - https://www.scribd.com/document/714943384/cc-qualis-periodicos-2017-ranking
    - https://www.gov.br/capes/pt-br/centrais-de-conteudo/engenharias_iv.pdf
    - https://ppee.unb.br/wp-content/uploads/2023/07/Esclarecimento_Qualis_2017-2020.pdf
    - https://www.ufmg.br/periodicos/mudanca-no-qualis-periodicos/
    - https://revistapesquisa.fapesp.br/qualis-periodicos-sera-substituido-por-classificacao-com-foco-nos-artigos/
    - https://www.scielo.br/j/jbchs/a/T8tbRGdhqFRQ3PKZ9qV6G5w/?lang=en
- The prompt given to the author for a normal chat (C5) asks, for ten
  candidate journals: the stratum in Qualis 2017-2020 and 2021-2024;
  the procedure of Engenharias IV for 2025-2028; the Scopus percentile
  and the quartile in JCR or Scimago; and up to five other journals.
  The candidates: Software: Practice and Experience; ‹Programming›;
  Journal of Systems and Software; Science of Computer Programming;
  Journal of Computer Languages; IEEE Transactions on Games;
  Entertainment Computing; IEEE Transactions on Software Engineering;
  Empirical Software Engineering; ACM TOPLAS.

### References, all from memory, to verify (B4, B6)

- Sweeney, "The Next Mainstream Programming Language: A Game
  Developer's Perspective", invited talk, POPL 2006. Recalled: a slide
  on giving up 10% of performance for 10% more productivity. The author
  says "expressiveness"; the original, as recalled, says productivity.
- The Pingus paper of the author: recalled as SBGames 2018, "Structured
  Synchronous Reactive Programming for Game Development: Case Study: On
  Rewriting Pingus from C++ to Céu". To confirm with him.
- Sure: Unity coroutines; `await` on signals in GDScript 4 (`yield` in
  3); latent actions of Blueprints (`Delay`); Roblox `task.wait` and
  `task.spawn`.
- Fairly sure: Unity `Awaitable` and UniTask; LÖVE has no coroutine API
  of its own, only libraries over those of Lua (none named: check what
  SNKRX itself uses).
- Less sure: Defold; Kotlin coroutines in KorGE; the state scripts of
  Naughty Dog (GDC talk on Uncharted 2).
- FRP: Elm (early versions) and Yampa. Synchronous languages: Esterel,
  Lustre, Signal. No citations collected.

## F. Settled

- **Border with the paper on snapshots** (`papers/snapshots.md`, session
  `snapshots`; was A8). The author decided the saved game of Doom there
  on 2026-11-16, and answered "Ok" here, on the same day, to the report
  of it and to the recommendation on the rest. This paper: the saved
  game of Doom in one paragraph, as a limit of structured concurrency,
  pointing to that paper; determinism only as a property of the
  language (section on Atmos); the test modes with scripted input only
  in the method of the case studies; no claim on save, resume, or
  replay. To do: tell that session the names of the sections when A4 is
  decided.

## G. Work with no input (#3, 2026-11-20)

The author, in `atmos-lang`, on 2026-11-20 (read in its transcript):
"I want you to review all sessions and propose them changes that need no
input. The sessions may also work suggest by themselves. No input form
me. Go ahead". The rule is in `.claude/plans/TODO.md`, "Work with no
input". Everything below was done by the session, to be reviewed by
him; nothing outward, and the outline `papers/games.md` keeps both
organizations (#1 is still his).

### Changes of the language since 2026-11-15 that bear on the paper

Read from the log of `doc/manual.md`. The outline has no code yet; the
code in `papers/games-drafts.md` is copied from the ports as they were
on 2026-11-20.

- `pin`: the rules are stable (`pin val`, `pin var`, modifiers on
  parameters, a pinned value never leaves its block, `set :out` for a
  place outside that refers to a value until the block ends). Any
  explanation of memory in the paper follows the manual, "Pin", and
  points to the paper on `pin` for the rules.
- No method calls: `o::f()`, `func o::f`, and the implicit `self` left
  the language. A sample with `::` no longer compiles.
- Calls with a single atom and no parentheses (`f x`), and the bare
  `await` of a name (`await x`).
- `X.in` instead of `X.xin`; the native tag `:atmos` instead of
  `:value`; `await :all` over an empty pool is satisfied at once; only
  the last spawn of a block is the compile error.
- `X.loadx`: data with names and includes. It bears on the ports that
  keep content as data (SNKRX, Flare).
- Numeric options: `ATMOS_REAL=float` (reals with 32 bits) and
  `ATMOS_INT=32`. They bear on a threat to validity: Pingus and
  SuperTux kept exact `float` math in C because reals were doubles. To
  check with each port whether its formulas went back to Atmos before
  the paper says what stayed in C.
- Environments may be written in Atmos (`atmos.env`), with a generic
  `main`; Rocks and the SDL library left for `ia-atmos/env-sdl`. The
  README no longer has `lib/sdl.atm`.

### What was written (2026-11-20)

- `papers/games-data.md`: the measures that the documents of the ports
  report, one table per claim, each cell with its source. Nothing was
  measured again. Ends with "Gaps" and "Inconsistencies".
- `papers/games-drafts.md`: drafts of the subjects, the method, and
  the threats to validity, independent of #1. Ends with the open
  points for the author.
- `papers/games-related.md`: the references checked on the web, with
  the URL read. Ends with "Could not verify". It settles B4 (Sweeney)
  and most of B6 (coroutines in engines).

### Findings to carry to the text

- Sweeney's slide says "Will gladly sacrifice 10% of our performance
  for 10% higher productivity": productivity, not expressiveness.
- The Pingus paper names four patterns (finite state machines,
  continuation passing, dispatching hierarchies, lifespan
  hierarchies); the port documents seven cases, signaling included.
- Not to claim: that a Unity coroutine runs no cleanup when stopped,
  nor that an `await` of Godot cannot be cancelled (the documents are
  silent); that the state scripts of Naughty Dog lack parallelism,
  abortion, or finalization (they have tracks, abort on a change of
  state, and exit handlers).
- No publication on Atmos was found.
- B5 is settled: the originals of `env-ray` are in C
  (`papers/games-data.md`, Subjects).

### To ask the ports (through `atmos-lang`), not done

- SNKRX: the size of the port, 4755 lines (`doc/bench.md`) or 8647
  (`doc/case-study.md`); the 946 MB of the original, unexplained.
- `env-ray`: 38 or 35 control variables left.
- Doom: 44 against 33 ms with no rendering (its TODO) against "no
  difference" (`doc/bench.md`).
- Lessmilk, Flare, SuperTux: memory and CPU of the port against the
  original; Flare and SuperTux: the size of the port.
- All: the CPU of the game logic alone, measured the same way.
- Pingus, SuperTux: whether the formulas in `float` went back to Atmos
  with `ATMOS_REAL=float`.
- All but Flare: a statement in the README of how the port was
  written (AI agents under the direction of the author).

### The CPU of the game logic alone (#4, by the session, to be confirmed)

Asked by `env-ray` on 2026-11-20, which proposed it; told to `env-ray`
and to `atmos-lang` for the other ports.

- The same run as the whole-frame measure (same scripted input, same
  frames, no cap), with the rasterizer and the presentation skipped in
  the original and in the port alike: a frame costs its logic plus the
  draw calls with no pixels. Doom already does it ("no rendering").
  In-process timers (SNKRX) only exist in the port.
- CPU time of the process, median of 3 runs, same machine; ms per
  frame and the ratio of port to original, next to the whole-frame
  number of the same run. Each `doc/bench.md` says what is skipped.

### Answers of the ports

- `env-ray` (2026-11-20): the control variables left are 38 of 134 in
  the faithful ports (branch `faithful`, `e0b6a01`) and 35 on `main`
  (`9f5d069`), with 14 games relaxed: both were right. Its README,
  "State", now says how the port was written. Logic alone: not
  measured yet (`env-ray#45`); it has the whole run (1.02 times the C
  on average) and one stress (space_invaders, 1.4 against 0.75
  microseconds per enemy). `papers/games-data.md` still has the state
  of `0d8b477`: to refresh when the ports settle.
- `doom` (2026-11-20, reported by `atmos-lang`, its item
  `atmos-lang#96`; not read in `doom/doc/bench.md` yet): with no
  rendering, on demo1, 263.4 M instructions for the main build against
  230.2 M of the C; 711.7 M for the variant with the door and the
  zombieman as tasks (2.7 times), about 40% of it the traversal of a
  broadcast. It bears on the claim of cost: tasks in the simulation
  have a price that grows with the broadcast. To read at the source
  before it enters `papers/games-data.md`.
- `sdl-snkrx` (2026-11-20, relayed by `atmos-lang`, read at `07f5905`):
  a correction. The old benchmark ran the port with no audio. Now:
  1.07 to 1.22 times the ticks of the original (not 1.8 to 3.0), CPU
  at 83% to 91% (not 56% to 71%), peak memory 546 against 946 MB, both
  of decoded sounds (not a gain of the language). Logic alone: 0.089
  ms per tick (level 1), 2.77 (stress), port only, in process, which
  is not the method of #4. One count of size: 9455 lines and 91671
  tokens against 10381 and 134827. Its README says how it was
  written. A note of correction is at the top of
  `papers/games-data.md` and `papers/games-drafts.md`, and under the
  line of SNKRX in the outline, which expected "even more in memory
  and CPU": that expectation of the author does not hold as measured,
  to tell him.
- To do: rewrite the cells and sentences of SNKRX from the new
  documents, when the ports have answered and run on the new `main`
  (one refresh of the two files, not one per answer).
- `env-ray` (2026-11-20, not read at the source yet): the logic alone,
  by the method of #4, in its `doc/bench.md`, "The logic alone". The
  switch `RL_NODRAW=1` writes no pixel and copies no screen, in the
  original and in the port; the draw calls stay. The value is the CPU
  of the run of 1200 frames minus that of a run of 1 frame. Over the
  23 games the logic of the ports is 2.14 times that of the originals
  in sum (3069 against 1432 microseconds for one frame of each game),
  while the whole frames are 1.02 times. Per game from 0.66 to 3.97,
  and one outlier, koala_seasons, at 24.24 (24 against 589
  microseconds: hundreds of fields in tables, and each float operation
  rounded by hand). Values under some 20 microseconds carry the noise.
  It is the first measure of the two numbers side by side, and it
  bears on the expectation of the outline (a loss below 10% to 20%
  against a static original): that holds for the whole frame, and not
  for the logic alone, which is about twice. The absolute cost is
  small (microseconds in a frame of 16 ms): the argument of Sweeney
  applies to the frame.
