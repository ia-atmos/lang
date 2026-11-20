# TODO of the paper on snapshots

Everything pending in the `paper-snapshots` session (formerly
`snapshots`), which writes `papers/snapshots.md`: explicit, unanswered,
or unacknowledged. Kept current by that session.

The idea is `atmos-lang` item 64 of `TODO.md`, related to item 45.

Items are numbered `#N` since 2026-11-18 (standing rule of the author,
`atmos-lang#71`): unique in this session, in order, never reused. Labels
used before that date (A1, B2, ...) stay valid for what was written then.

> **Paused** by the author on 2026-11-20: "por enquanto não vamos fazer
> nada aqui". No work in this session, also under "work with no input",
> until he resumes it.

## Index

| # | Title | State |
|---|---|---|
| #1 | Title and name of the architecture | Open, left open by the author on 2026-11-16 |
| #2 | Freechains: commands as posts, finality, what the user sees | Open, in part |
| #3 | Case studies | Open |
| #4 | The eight rules of Section 4 | Open |
| #5 | What this paper adds to Croquet | Open |
| #6 | Related work still from memory | Mostly done 2026-11-20; CRDTs, Time Warp, Elm time travel not read |
| #7 | The example of Section 3, not compiled | Done 2026-11-20: `papers/snapshots/` |
| #8 | Fill the sections with text | Drafts of Sections 3 and 9, by the session, to be confirmed |

## Open items

Work of 2026-11-20 under the rule "work with no input" (`atmos-lang#95`):
nothing below was decided for the author; each screen is ready for him.

- **#1 Title and name.** Left open by the author on 2026-11-16, not to
  ask again until the outline is otherwise settled. Proposed: "Snapshots
  and Commands: Structured Reactive Programs over a Permissionless
  Consensus"; four alternatives are in `papers/snapshots.md`.

- **#2 Freechains.** Approved on 2026-11-16: the consensus reorders
  commands, and the resolution is automatic (back to a snapshot, apply
  again in the new order, commands that no longer apply rejected by all
  peers). Bayou (1995, read on 2026-11-20) did the same with an undo
  log and a primary.
    - #2a Is each command a post, and the order of the consensus the
      order of the journal?
        - Option A (recommended): yes, one post per command. Simple, and
          the journal is the chain.
        - Option B: a post holds a batch of commands (a session, a
          turn). Fewer posts, coarser reordering.
        - Option C: Freechains only carries closed snapshots and
          journals. No reordering, but no live sharing.
    - #2b When is a command final, and what does the user see before?
        - Option A (recommended): Bayou's split. A command is tentative
          until the consensus settles the posts before it, then stable.
          The program shows the tentative state, and marks what may
          still change.
        - Option B: show only the stable state. Simple, but every action
          of the user waits for the consensus.
        - Option C: show the tentative state with no mark. Simple, but a
          reordering surprises the user.

- **#3 Case studies.** No port follows the architecture yet. Candidates,
  with what each would show (the facts on the original games are from
  memory, to confirm with each session):
    - `doom`: the motivation, measured (967 states kept in C because a
      save may happen at any tic); with checkpoints, things as tasks
      instead of rows. Also the case of `atmos-lang#45`, a snapshot of
      the tasks by the runtime, the way this paper does not take.
    - `sdl-tux` (SuperTux): a platformer with checkpoints in its levels;
      the most direct case of Section 7.
    - `sdl-snxkx` (SNKRX): runs of arenas; a save between arenas is a
      natural checkpoint.
    - `sdl-pingus`: levels; the state that persists is which levels are
      done; small snapshot, few commands.
    - `sdl-flare`: an RPG whose original saves at any point; the port
      has no save yet, and the tasks of a map are already spawned from
      its file; a save at the entry of a map is a checkpoint.
    - `pico-1oom` (not started): a game of turns whose save is the whole
      state, reproducible from a seed; the case without checkpoints,
      where every turn is a command.
    - `pico-slides`: a presentation as one table; the slide is the
      state and next/back are commands; the smallest case that is not a
      game.
    - an editor written for the paper, grown from
      `papers/snapshots/board.atm`: the case among peers (Section 8).
    - Recommended: `doom` (motivation), `sdl-tux` (checkpoints),
      `pico-1oom` (turns), and the editor (peers). Two to four, by the
      length the venue allows.

- **#4 The eight rules of Section 4.** A reading of the discussion, not
  his words. The example (`#7`) follows all eight and checks R4 and R6.
  Recommended: to review them one at a time. Options per rule: keep,
  merge, drop, or reword.

- **#5 What this paper adds to Croquet.** Croquet (current docs, read on
  2026-11-17) has model and view, a reflector that orders events, time
  as ticks, shared random numbers, snapshots plus the events after them,
  and persisted data apart from snapshots.
    - #5a the architecture inside a synchronous language: the model as
      plain data, tasks as the view, a restart as the abortion of a
      block, with the rules of pin.
    - #5b checkpoints for action games.
    - #5c order by a permissionless consensus, with automatic
      reordering.
    - Recommended: all three, with #5a as the main one, since #5b and
      #5c depend on it.

- **#6 Related work from sources.** Read on 2026-11-20, with the passage
  in Section 13: event sourcing and CQRS (Fowler), Elm and Redux (their
  guides), Raft (Section 7 of the paper), Bayou (SOSP 1995), rr (ATC
  2017). Not read: CRDTs (Shapiro et al. 2011, the report would not
  download), Time Warp (Jefferson 1985), time travel in Elm or Redux in
  a primary source, the 2003 Croquet paper.

- **#7 The example.** Done on 2026-11-20: `papers/snapshots/board.atm`
  compiles and runs on `main`, also with the address sanitizer;
  `papers/snapshots/test.sh` compares it with `expected.txt`. It checks
  resume, any instant, R6, and R4 (it finds a note that keeps state in a
  local). The language did not get in the way. One finding, now in
  Section 9: the restart test only holds between gestures, not at any
  point.

- **#8 Text.** Drafts of Sections 3 and 9 in `papers/snapshots-draft.md`,
  by the session, to be confirmed: the outline is not approved yet, and
  the other sections wait for `#1` to `#5`.

## Done before 2026-11-18 (labels of that time)

- A5, the saved game of Doom between this paper and the one on games:
  decided on 2026-11-16. That paper keeps one paragraph, as a limit of
  structured concurrency, pointing here; this one tells the whole
  story, as its motivation. `paper-games` agreed with the author too.
- Answers from other sessions, all received:
    - `atmos-lang` (formerly `ceu-atmos`), 2026-11-15: no rule beyond
      the form of the other outlines; venue, length, and title are the
      author's; nothing unmeasured or unimplemented is claimed.
    - `doom`, 2026-11-15: in Sections 7, 11, and 12. The numbers of the
      state table are from its plan, not recounted.
    - `atmos-pin` (formerly "Pin rules discussion"), 2026-11-15, and the
      changes of 2026-11-17: in Section 5, cited as "by the rules of
      pin".
    - `paper-pin`, 2026-11-15: cite the rules by name ("store", "end",
      "result", "pin"); do not restate them nor the comparison with GC,
      RC, Rust, and regions; it may cite this paper in its Section 9.
    - `paper-games`, 2026-11-15: no overlap.
    - `flare`, 2026-11-16: in Section 11. "A save at the entry of a map"
      is a reading of this session, not a plan of the port.
    - `pico-1oom`: not asked, the port is not started; its plan is
      `261114-pico-1oom.md`.
- Related work read from sources: "Prevalent Systems" (Johnson and
  Wuestefeld 2014, PDF from the author), Bettner and Terrano (2001, in
  full: nothing on saved games), StarCraft (interview of Wyatt and
  community sources only), Croquet and Akka (their documentation).
