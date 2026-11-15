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
8. **Border with the paper on snapshots** (`papers/snapshots.md`, session
   `snapshots`, which asked on 2026-11-15). It proposes: the saved game
   of Doom (the simulation kept in C because a save must cover it) is
   one paragraph here, as a limit of structured concurrency, and the
   motivation there; each paper cites the other. It also asked where
   this paper claims determinism, test modes with scripted input, or
   replays. Answered: no claim today; nothing agreed.
   Recommended: accept. Here, determinism only as a property of the
   language (section on Atmos), and the test modes only in the method
   of the case studies; no claim on save, resume, or replay. Tell that
   session the names of the sections when A4 is decided.

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
