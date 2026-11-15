# TODO of the paper on snapshots

Everything pending in the `snapshots` session, which writes
`papers/snapshots.md`: explicit, unanswered, or unacknowledged. Kept
current by that session.

The idea is item 64 of `TODO.md`, related to item 45.

## A. Decisions waiting for the author

Each item has the recommended option.

1. **Name and title.** Proposed: the architecture is "snapshots and
   commands"; the title is "State is Data, Tasks are Disposable:
   Snapshots and Commands for Structured Reactive Programs". Three
   alternatives are in `papers/snapshots.md`.
2. **Freechains.** Asked on 2026-11-14, not answered: is each command a
   post in a chain, with the order of the consensus as the order of the
   journal, or does Freechains only distribute closed snapshots and
   journals?
   Recommended: the first, and to apply again when the order changes.
   Until then it stays in Future Work.
3. **Case studies.** No port follows the architecture.
   Recommended: Doom with checkpoints after the faithful port, one game
   by turns or with checkpoints among the ports, and one small editor
   written for the paper.
4. **The eight rules of Section 4.** They are a reading of the
   discussion, not his words.
   Recommended: to review them one at a time.

## B. Pending in the outline

1. **Answers from other sessions.** Asked on 2026-11-15:
    - `ceu-atmos` (the parent): rules for the papers, which sessions
      write papers, which sessions hold evidence.
    - ~~`doom`~~: answered on 2026-11-15, in Sections 7, 10 and 11.
      The numbers of the state table are from its plan, not recounted.
    - ~~`pin`~~: answered on 2026-11-15, in Section 5. The rules are
      approved but not in the compiler: the paper says "by the rules
      of pin". Exact anchors of the rules of store, end of block, and
      result: to ask the `paper-pin` session
      (session_011eECofALi31oSimzbLxYZp) when the text is written.
    - The session of `papers/games.md`: not reached (`src-games` is
      inactive). To ask the parent which session writes it now.
2. **Related work from sources.** Prevayler, record and replay, and
   Freechains were checked on the web on 2026-11-14. Event sourcing,
   Elm and Redux, Raft, and lockstep are from memory: to check and to
   add references.
3. **The example of Section 3.** A sketch, not compiled.

## C. Next steps, not started

1. Fill the sections with text, after the author approves the outline.
