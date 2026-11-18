# TODO of the paper on snapshots

Everything pending in the `paper-snapshots` session (formerly
`snapshots`), which writes `papers/snapshots.md`: explicit, unanswered,
or unacknowledged. Kept current by that session.

The idea is `atmos-lang` item 64 of `TODO.md`, related to item 45.

Items are numbered `#N` since 2026-11-18 (standing rule of the author,
`atmos-lang#71`): unique in this session, in order, never reused. Labels
used before that date (A1, B2, ...) stay valid for what was written then.

## Index

| # | Title | State |
|---|---|---|
| #1 | Title and name of the architecture | Open, left open by the author on 2026-11-16 |
| #2 | Freechains: commands as posts, finality, what the user sees | Open, in part |
| #3 | Case studies | Open |
| #4 | The eight rules of Section 4 | Open |
| #5 | What this paper adds to Croquet | Open |
| #6 | Related work still from memory | Open, work of this session |
| #7 | The example of Section 3, not compiled | Open, work of this session |
| #8 | Fill the sections with text | Not started, after the outline is approved |

## Open items

- **#1 Title and name.** The author asked on 2026-11-16 that the title
  has one of: decentralized, blockchain, permissionless, consensus.
  Proposed: "Snapshots and Commands: Structured Reactive Programs over
  a Permissionless Consensus"; four alternatives are in
  `papers/snapshots.md`. He left it open: not to ask again until the
  outline is otherwise settled.
- **#2 Freechains.** The consensus reorders commands, and the
  resolution of branches is automatic: back to a snapshot, apply again
  in the new order, commands that no longer apply rejected by all peers
  (approved on 2026-11-16). Still to confirm:
    - #2a each command is a post, and the order of the consensus is the
      order of the journal;
    - #2b when a command is final, and what the user sees before that.
- **#3 Case studies.** No port follows the architecture.
  Recommended: Doom with checkpoints after the faithful port, 1oom as a
  game by turns, and one small editor written for the paper (which also
  serves Section 8, among peers).
- **#4 The eight rules of Section 4.** A reading of the discussion, not
  his words. Recommended: to review them one at a time.
- **#5 What this paper adds to Croquet.** Croquet (read on 2026-11-17)
  has model and view, a reflector that orders events, time as ticks,
  shared random numbers, snapshots plus the events after them, and
  persisted data apart from snapshots. Proposed contributions left:
    - #5a the architecture inside a synchronous language, the model as
      plain data and tasks as the view, restart as the abortion of a
      block, with the rules of pin;
    - #5b checkpoints for action games;
    - #5c order by a permissionless consensus, with automatic
      reordering.
- **#6 Related work from memory.** Event sourcing, Elm and Redux, Raft,
  Bayou, Time Warp, and CRDTs: to check and to add references. The 2003
  Croquet paper is not read (web.archive.org resets the connection).
- **#7 The example of Section 3.** A sketch, not compiled.
- **#8 Text.** Fill the sections after the author approves the outline.

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
