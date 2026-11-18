# TODO of the paper on snapshots

Everything pending in the `paper-snapshots` session (formerly
`snapshots`), which writes
`papers/snapshots.md`: explicit, unanswered, or unacknowledged. Kept
current by that session.

The idea is item 64 of `TODO.md`, related to item 45.

## A. Decisions waiting for the author

Each item has the recommended option.

1. **Name and title.** On 2026-11-16 the author asked that the title
   has one of: decentralized, blockchain, permissionless, consensus.
   Proposed: "Snapshots and Commands: Structured Reactive Programs over
   a Permissionless Consensus". Four alternatives are in
   `papers/snapshots.md`.
   Left open by the author on 2026-11-16: not to ask again until the
   outline is otherwise settled.
2. **Freechains.** Partly answered on 2026-11-16: the consensus reorders
   commands, and the resolution of branches is to be automatic. Section
   8 was written from that, as a proposal. To confirm with him:
    - each command is a post, and the order of the consensus is the
      order of the journal;
    - ~~the resolution is "back to a snapshot, apply again in the new
      order", with commands that no longer apply rejected by all
      peers~~: approved on 2026-11-16;
    - when a command is final, and what the user sees before that.
3. **Case studies.** No port follows the architecture.
   Recommended: Doom with checkpoints after the faithful port, one game
   by turns or with checkpoints among the ports, and one small editor
   written for the paper.
4. **The eight rules of Section 4.** They are a reading of the
   discussion, not his words.
   Recommended: to review them one at a time.

5. ~~The saved game of Doom, between this paper and the one on
   games.~~ Decided on 2026-11-16: that paper keeps it as a limit of
   structured concurrency, in one paragraph that points here; this one
   tells the whole story, as its motivation. `paper-games` was told.

## B. Pending in the outline

1. **Answers from other sessions.** Asked on 2026-11-15:
    - ~~`ceu-atmos` (the parent)~~: answered on 2026-11-15. No rule
      beyond the form of the other outlines; venue, length, and title
      are the author's; nothing unmeasured or unimplemented is claimed.
      Overlap to avoid: with `pin`, state tied to blocks; with `games`,
      the saved game of Doom.
    - ~~`doom`~~: answered on 2026-11-15, in Sections 7, 11 and 12.
      The numbers of the state table are from its plan, not recounted.
    - ~~`pin`~~: answered on 2026-11-15, in Section 5. The rules are
      approved but not in the compiler: the paper says "by the rules
      of pin". Exact anchors of the rules of store, end of block, and
      result: to ask the `paper-pin` session
      (session_011eECofALi31oSimzbLxYZp) when the text is written.
      The rules changed on 2026-11-17 (told by the compiler session):
      a place that owns its value may be assigned a fresh one, and the
      old one is finalized when its last reference goes. Section 5 now
      says that a load is `set S = X.load(...)`. Implemented in `main`;
      see the manual, "Pin".
    - ~~`paper-games`~~ (session_011eECogKyRYMVJJfbmpHYv9): answered
      on 2026-11-15. Its outline claims nothing on the saved game of
      Doom, on determinism, or on replays. It expects determinism only
      as a property of the language, and the test modes only as the
      method of its case studies. The split of item A5 is not agreed:
      it is the author's, and is also in `TODO-paper-games.md`.
    - ~~`paper-pin`~~ (session_011eECofALi31oSimzbLxYZp): answered
      on 2026-11-15. Cite the rules by name ("store", "end", "result",
      "pin"), not only by section. No overlap. Its request: not to
      restate the rules, nor the comparison with GC, RC, Rust, and
      regions. It may cite this paper as a case study in its Section 9.
    - ~~`flare`~~ (session_011eECoegYsZr7nSPxcugGtt): answered on
      2026-11-16, in Section 11. No save in the port yet. "A save at
      the entry of a map" is a reading of this session, not a plan of
      the port.
    - `pico-1oom` (session_011eECoiEGgQ6ZBFHUCoxt77): not asked, the
      port is not started. Its plan is `261114-pico-1oom.md`.
2. **Related work from sources.** "Prevalent Systems" (2014) was read
   on 2026-11-17 from a PDF the author uploaded; its examples are in
   Section 13. Its claim on Age of Empires and StarCraft has no
   citation. Primary sources searched on 2026-11-17, in Section 13:
   Bettner and Terrano (2001) for Age of Empires; for StarCraft no
   paper by Blizzard, only an interview of Wyatt (2015) and community
   sources. Bettner and Terrano read in full on 2026-11-17, after the
   author opened the network: it says nothing on saved games.
   Croquet (current docs) and Akka Persistence (docs) read on
   2026-11-17. Croquet is the closest work found: to discuss with the
   author what this paper adds to it. The 2003 Croquet paper was not
   read: web.archive.org resets the connection.
   Earlier note: Prevayler, record and replay, and
   Freechains were checked on the web on 2026-11-14. Event sourcing,
   Elm and Redux, Raft, lockstep, Bayou, Time Warp, and CRDTs are from
   memory: to check and to
   add references.
3. **The example of Section 3.** A sketch, not compiled.

## C. Next steps, not started

1. Fill the sections with text, after the author approves the outline.
