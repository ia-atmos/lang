# TODO of the paper on media

Everything pending in the `paper-media` session, which writes
`papers/media.md`: explicit, unanswered, or unacknowledged. Kept
current by that session. The plan is `261116-pico-media.md`.

## Index of items

Items are `#N` (standing rule of the author, 2026-11-18): unique in this
session, in order, never reused nor changed. From another session:
`paper-media#N`. The identifiers used before (`A1`..`A5`, `B1`..`B5`,
`C1`) are `#1`..`#11` in the same order.

States: **open** (waits for the author), **work** (known, not done),
**wait** (depends on another session), **done**, **closed**.

| # | Item | State |
|---|------|-------|
| 1 | The framing of the paper: the approach, or a revisit of Céu-Media | open (asked 2026-11-16; recommended: the approach) |
| 2 | The authors: those of 2016 and Presotto | open |
| 3 | The scope of the evaluation | open |
| 4 | A document that changes while it plays (an editor) | open (recommended: future work) |
| 5 | The vocabulary of Section 4 | wait: `pico-media` (does not exist). Decided by the author on 2026-11-26: `pico-slides` is no longer a dependency of the paper |
| 6 | The session of the project: its identifier, then the questions | wait: `pico-media` does not exist. `pico-slides` (session_011eECogEdSaHMwZpk7NGmgk) sent its vocabulary on 2026-11-25; kept only as a possible cited example or case study of the layout half |
| 7 | The published version of the paper of 2016 | work |
| 8 | Related work checked against sources | work |
| 9 | Lines of the samples of Céu-Media, for the comparison | work |
| 10 | The sketch of Section 4 is not compiled | wait: `pico-media` |
| 11 | Fill the sections with text | wait: `#1`, `#5` |
| 12 | What the player does when the document is replaced (`set doc`): replay, or keep what did not change | wait: `pico-media`; related to `#4` |
| 13 | Redo Aurora first, over Atmos and pico, with no concern for Céu-Media (asked by the author, 2026-11-19) | closed 2026-11-26: done in another project (`ia-atmos/pico-slides`), outside the paper. It became a tool of slides, with no time nor media; the paper does not depend on it |

State of the session: paused by the author since 2026-11-19 ("por
enquanto nao vamos fazer nada aqui"). The paper has no project that
implements it until `pico-media` exists.

## A. Decisions waiting for the author

Asked one at a time, in the order of their numbers. Each has the recommended option.

- `#1` **The framing of the paper.** Either (a) the contribution is the
   approach, "a document is a table, played by tasks", with Céu-Media as
   background and as the source of the examples; or (b) a revisit of
   Céu-Media, "ten years later", whose contribution is what changed.
   Recommended: (a); it makes the paper stand on its own, and the title
   follows ("Documents as Values: Declarative Multimedia in a Structured
   Synchronous Language"). Alternatives are in `papers/media.md`.
- `#2` **The authors.** Whether the authors of 2016 (Santos, Lima,
   Rodriguez) and Thiago Presotto (Aurora) are invited.
   Recommended: to ask Presotto if Aurora is used beyond a citation;
   the others are the author's call.
- `#3` **The scope of the evaluation.** Recommended: the examples of 2016
   and one presentation of slides, in lines and in what each version
   makes explicit, with traces under scripted time; no comparison with
   an NCL or SMIL player.
- `#4` **A document that changes while it plays** (an editor). Recommended:
   out of scope, as future work, to keep apart from `snapshots`.
- `#5` **The vocabulary** (Section 4): not for the paper to decide. It
   follows the milestone M1 of `pico-media`. The paper asks the author
   only if the project leaves a point open that the paper needs.

## B. Pending in the outline

- `#6` **Session `pico-media`.** Does not exist yet. When it does, the
   compiler session (`ceu-atmos`) gives its identifier; then ask it for
   the vocabulary, the player, and what plays with no device.
- `#7` **The final version of the paper of 2016.** Read only the blind
   version (`fsantanna/ceu-media`, `paper.pdf`). Its title differs from
   the published one; the lip synchronization of the plan is a sample of
   the library (`lip-sync.ceu`), and the blind paper has only the case
   of four muted videos and an audio (its Listing 8). To check against
   the published version.
- `#8` **Related work from sources.** All of Section 8 is from memory or
   from the paper of 2016: NCL, SMIL 3.0, CSS grid, WebVTT, slide tools,
   Elm and React, the WebMedia 2017 paper, and the thesis of 2018. To
   check and to add references.
- `#9` **Numbers of Céu-Media for the comparison**: lines of each sample of
   `rodrimc/ceu-media` that matches an example. Not counted.
- `#10` **The sketch of Section 4** is the one of the plan, not compiled.

## C. Next steps, not started

- `#11` Fill the sections with text, after the author approves the outline
   and the project agrees the vocabulary.
