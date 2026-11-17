# pico-media: declarative documents of media in Atmos, over pico

Plan for the repository `ia-atmos/pico-media`, session `pico-media`,
and for the paper `papers/media.md` of `ia-atmos/lang`, session
`paper-media`. Written in the session that maintains the compiler
(`ia-atmos/lang`), from what the author said there on 2026-11-16. The
design is his: this file records what he decided, what exists, and
what is open.

## Context

- In 2016 the author and colleagues published Céu-Media, a library of
  the synchronous language Céu for multimedia presentations: a set of
  media objects synchronized in time, as in NCL and SMIL, with the
  semantics of a synchronous language under it.
- This project takes the idea to Atmos, over the environment of pico
  (`ia-atmos/env-pico`), with a new design: **a document is a table**.
- Céu-Media is **only an inspiration** (the author, 2026-11-16): its
  examples are not tests to reproduce, and its interface is not to be
  kept. There is no original to be faithful to.
- The compiler is `ia-atmos/lang`, branch `main`. Read
  `doc/manual.md` there before writing code.

## What the author decided

His words are in the session of the compiler; this is their content.

- **The repository** is `ia-atmos/pico-media`, a fork of the original
  Céu-Media (`rodrimc/ceu-media`), over pico.
- **A document is a table that describes it.**
  - A tag in lower case names a **primitive**: `:image`, `:video`,
    and the like (text, audio).
  - A tag that starts in upper case names a **composite**: `:Srt`,
    `:List`.
- **Layout** uses the anchors and the relative positions of pico.
- **Time too**: the document describes when things happen, and not
  only where.
- **Extensions**, with ideas from slides:
  - lists (bullets);
  - a **grid**, which sets rows and columns. Each cell says the
    anchor and the position of what goes in it. The cells go in an
    array.
- **Concepts only, not necessarily to implement**: `:Srt` (the
  subtitles of Céu-Media, a composite made of timed texts), and what
  comes from Aurora.
- **Aurora** is the work of a student, Thiago Presotto
  (`ThiPresotto/Reactive-Slides`, 2026): an engine of slides in Atmos
  (the Lua implementation, over SDL), where a presentation is a Lua
  table, an array of slides, each with a template (`FrontCover`,
  `Slide_Title_OneObject`, `Slide_Title_TwoObjects`) and objects with
  a tag (`Text`, `BulletList`, an image). Its concepts enter here; its
  code does not.
- **A paper**, `papers/media.md` in `ia-atmos/lang`, by a session of
  its own: Céu-Media revisited in Atmos, with a declarative document
  as a table. The target is WebMedia again.

## Sources

| What | Where | Notes |
|------|-------|-------|
| The paper | "CÉU-MEDIA: Local Inter-Media Synchronization using CÉU", WebMedia 2016 (Teresina), pp. 143-150, Santos, Lima, Sant'Anna, Rodriguez | The authors and pages are from citation records. A PDF of the version for blind review, titled "CÉU-MEDIA: A Multimedia Library for the Synchronous Language CÉU", is in `fsantanna/ceu-media`, `paper.pdf` |
| Later work | A workshop paper of WebMedia 2017 (Santos, Sant'Anna, Rodriguez), and the thesis of Rodrigo Santos (PUC-Rio, 2018), with Céu-Media and the middleware Mars | For related work |
| The original library | `rodrimc/ceu-media`: 136 commits, the last in 2018-07 | `include/media/`: `media.ceu` (a scene, a player, properties, events), `raw.ceu`, `srt.ceu`, `slideshow.ceu`. 22 samples in `src/`. Over LibPlay (TeleMidia), which is over GStreamer |
| The author's fork | `fsantanna/ceu-media`, stopped in 2017-08 | Older than the original; it has the PDF |
| Aurora | `ThiPresotto/Reactive-Slides` | `aurora` (101 lines, the validation of a presentation), `slides.atm` (526), `README.md` with the templates and the objects, three presentations |
| The environment | `ia-atmos/env-pico`: `pico.atm`, `doc/api.md`, `doc/guide.md` | Rect, Pos, and Dim with a mode (`'!'` pixels, `'%'` percentage, `'#'` tiles) and an anchor (`'C'`, `'NW'`, ..., or `[x=,y=]`); layers; images; text with fonts; sounds; video (`pico.output.draw.video`, `pico.get.video`) |

A precedent inside Céu-Media itself: its slideshow already took a
table (in Lua) that described the document, with `parallel` and
`sequence` lists of media, each with a `tag` (`'image'`, `'audio'`),
a `uri`, a `rect`, a `z`, an `alpha`.

## What the paper of 2016 claimed

For the design and for the new paper to answer:

- Libraries of media (GStreamer, FFmpeg) are too low level; NCL and
  SMIL are high level, and inflexible (any extension is outside the
  language), ambiguous, and with problems of synchronization.
- A synchronous language gives a precise meaning to time: reactions
  are instantaneous and deterministic, and the passage of time is an
  ordinary event that the program controls.
- The logical time of the program matches the time of the
  presentation.
- Its examples: blinking LEDs against NCL and SMIL, the subtitles
  (SRT), a slideshow, lip synchronization.

## A first shape, to discuss

Not decided. One way to read what the author said, for the first
conversation in the session of the project:

```
pin doc = :Seq [                            ;; a composite: in sequence
    :image [ src="cover.png", pos=['%', x=50, y=50, anchor='C'], dur=3s ],
    :Par [                                  ;; a composite: together
        :video [ src="talk.mp4", pos=['%', x=0, y=0, anchor='NW'] ],
        :Srt   [ src="talk.srt", pos=['%', x=50, y=95, anchor='S'] ],
    ],
    :Grid [ rows=2, cols=2,
        [ :image [src="a.png"], anchor='C' ],           ;; the cells, an array
        [ :List  ["one", "two", "three"], anchor='NW' ],
        ...
    ],
]
spawn Play(doc)
```

- A primitive is played by a task that draws it while it lasts; a
  composite is a task that spawns its parts and gives them time and
  place. The document is data; the tasks are a function of it.
- What ends a part: its own duration (`dur`), the end of its media (a
  video), or an event (a key, the next slide).

## Questions that the project must answer early

They are the author's, in the session, one at a time.

- **The composites of time.** He named `:Srt` and `:List`. A document
  with time needs at least "one after the other" and "together"
  (the `sequence` and `parallel` of the slideshow of Céu-Media): their
  names and what ends each (the first part, all parts, a given one).
- **Time in a part**: a duration, a start relative to the composite,
  or events only; and how a part reacts to the user (a link, a key).
- **Is the document only data, or also code?** A table can carry a
  function (an action when a part ends); a document that is only data
  can be saved, loaded (`X.load`), checked, and edited by a tool.
- **The grid**: whether a cell may span rows or columns, and how the
  sizes of rows and columns are given (equal parts, percentages).
- **Slides**: whether a slide is a composite of this vocabulary (a
  `:Slide`, with templates as functions that build tables), which is
  where Aurora enters.
- **What of the fork is kept.** The repository is a fork of Céu-Media
  and the code is new: where the old code goes (a directory, or only
  the history), and what the readme says of it.
- **Tests**: with no original to compare, the tests are traces and
  screenshots of documents under scripted time and input
  (`PICO_ATM_FRAMES`, `_KEYS`, `_SHOT`). Whether video and sound play
  with no screen and no audio device must be checked first.

## Rules of the author, for all sessions

- Commit and push straight to the default branch, in fine-grained
  commits: one small commit per change, pushed as you go. No other
  branch and no pull request, unless he asks.
- Keep `.claude/plans/TODO.md` in the repository: decisions that wait
  for him (each with a recommended option), work known and not done,
  what was reported and he did not acknowledge.
- A decision that belongs to him is asked, one at a time, with a
  recommended option.
- Talk to him in Portuguese, short, leading with the outcome.
  Documents and code in English.
- The events of an environment have no prefix (`:key.dn`), and
  programs are built with the dedicated compiler (`atmos-pico`).
- What is found in the compiler goes to its session (`ceu-atmos`,
  session_011eECogKVQ7ZzmWCoMST2wK) by message, with a small program
  that reproduces it. What pico lacks goes to its session
  (`pico-sdl`, session_011eECoikzsmBiPNXNF7ffpf).

## What the other projects taught

Read `.claude/plans/261115-pin-migration.md` in `ia-atmos/lang`: the
rules of `pin` are new since 2026-11-15, and they shape this design.

- **A document is one pinned table**: `pin doc = [...]` owns every
  table inside it (the pin is deep). The tasks that play it refer to
  its parts; they do not own them.
- A part that a task needs to change (the current frame of a video,
  the position of an animation) is state of the task, or a field of
  the document that the task changes in place. A variable does not
  receive an unpinned table.
- A task that publishes creates what it publishes in its `pub`.
- A task spawned in the body of a loop or of an `if` is aborted when
  that block ends: the parts of a composite live in a pool of the
  composite.
- The time passes in pieces: what runs once per frame awaits the
  frame event of pico, and not `:clock`.
- A resource (a layer, a font, a sound, a video) is a native value,
  pinned by who loads it, and a child holds its parent
  (`a_nat_holds`).

## Milestones

To be agreed with the author; a proposal.

- [ ] M0: repository, readme (what it is, its relation to Céu-Media,
      made with AI), `.claude/plans/TODO.md`, the setup that fetches
      the compiler and `env-pico` at fixed commits. The check of what
      plays with no screen: an image, a text, a sound, a video.
- [ ] M1: the vocabulary, written as a document (`doc/design.md`) and
      agreed with the author before code: the primitives, the
      composites of time and of space, the fields of each.
- [ ] M2: the player of primitives: image and text, with anchors and
      relative positions, and a duration.
- [ ] M3: the composites of time. The slideshow as the first example.
- [ ] M4: sound and video; what ends with its media.
- [ ] M5: lists and the grid.
- [ ] M6: concepts, if the author wants them built: subtitles
      (`:Srt`), slides and templates.
- [ ] M7: examples for the paper: the ones of 2016 written again
      (blinking, subtitles, slideshow, lip synchronization), as
      inspiration, and the comparison in lines and in what each
      version makes explicit.

## The paper

Session `paper-media`, in `ia-atmos/lang`: `papers/media.md` and
`.claude/plans/TODO-paper-media.md`. An outline first (a title with
alternatives, a status note, an abstract, sections as bullets), as
`papers/pin.md`, `papers/games.md`, and `papers/snapshots.md`.

- The claim to start from: Céu-Media showed that a synchronous
  language gives multimedia presentations a precise time; ten years
  later, in Atmos, the document itself is a value of the language (a
  table), played by structured tasks, so a presentation is declarative
  as in NCL and SMIL and extensible as a program.
- It depends on the project: nothing is implemented yet, and the
  outline must say what is a claim and what is a plan.
- Overlap to avoid with the other papers: `games` (structured
  concurrency in games), `pin` (memory), `snapshots` (state as data
  and commands). The closest is `snapshots`: a document that is data
  from which tasks are spawned is the same idea, applied to media.

## Risks

- **Video and sound in the container**: they may not play with no
  device. The first milestone checks it.
- **A design with no original**: there is nothing to compare with, so
  the vocabulary must be agreed before code, and the examples of 2016
  are the measure of what it must express.
- **The fork** carries a history and a license of others: say in the
  readme what is theirs and what is new.
