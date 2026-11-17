# Documents as Values: Declarative Multimedia in a Structured Synchronous Language

Proposed name of the approach: **documents as tables** (a document is a
table; tasks play it).

The title is a placeholder; the target is WebMedia (the author,
2026-11-16). Alternative titles:

- Céu-Media Revisited: Declarative Multimedia Documents as Tables in Atmos
- Ten Years Later: from a Synchronous Media Library to Synchronous Media
  Documents
- A Document is a Table: Declarative Layout and Time for Multimedia,
  Played by Structured Tasks
- Declarative as NCL, Extensible as a Program: Multimedia Documents in
  Atmos

> Status: outline, first version, 2026-11-16. **Nothing of it is
> implemented yet.** The project that will build it,
> `ia-atmos/pico-media` (session `pico-media`), does not exist yet. The
> design is the author's, recorded in
> `.claude/plans/261116-pico-media.md`; the vocabulary there is "a first
> shape, to discuss", not decided. In this outline, **[claim]** marks
> what the paper argues, **[plan]** what the project must still build or
> measure, and **[open]** what waits for a decision. No sketch here was
> compiled. Pending decisions are in
> `.claude/plans/TODO-paper-media.md`.

## Abstract

*(Written as for the finished paper; every result in it is a [plan].)*

Multimedia authoring sits between two extremes.
Libraries such as GStreamer and FFmpeg give full control, at the level of
samples and pipelines; languages such as NCL and SMIL give a declarative
document, but its semantics of time is loose, and anything outside its
vocabulary needs scripts in another language.
In 2016, Céu-Media showed that a synchronous language gives a
presentation a precise logical time, with media objects played by
concurrent trails.
Its presentations, however, were programs: the document existed only as
the code that played it.
We revisit the idea in Atmos, a structured synchronous language with
tables, and make the document itself a value of the language.
A document is a table: a tag in lower case names a primitive
(`:image`, `:video`, `:text`, `:audio`), a tag that starts in upper case
names a composite (`:Seq`, `:Par`, `:List`, `:Grid`, `:Srt`).
Layout uses the anchors and relative positions of the pico environment;
time is given by durations, by the end of a medium, or by events; lists
and grids come from slides.
Each part is played by a task, and a composite is a task that spawns its
parts: structured concurrency gives each part a lifetime, abortion, and
a deterministic order, and a lexical pin gives the document one owner.
The document is thus declarative as in NCL and SMIL, can be saved,
loaded, checked, and generated as data, and is extensible as a program,
since a new composite is a task.
We present the vocabulary, its semantics in terms of tasks, the examples
of 2016 written again as documents, and an evaluation in size and in
what each version makes explicit.

## 1. Introduction

- Problem: authoring presentations of synchronized media
    - images, text, audio, video; where and when each appears
    - interaction: a key, a click, the next slide
- Two extremes
    - low-level libraries (GStreamer, FFmpeg, LibVLC): pipelines, samples,
      callbacks; for specialists
    - declarative languages (NCL, SMIL; HTML with CSS and timed media):
      a document; but time is loose and extension is outside the language
- Céu-Media (2016): the middle, from the side of the program
    - a synchronous language gives logical time a precise meaning
    - but the presentation is a program, not a document
- Thesis **[claim]**: in a language with tables and structured tasks, the
  document can be a value, and playing it can be a function of it
    - declarative where the document is data
    - precise where tasks play it
    - extensible where a new tag is a new task
- Contributions
    - **[claim]** the approach: a document is a table, played by tasks
    - **[plan]** a vocabulary of primitives and composites, for layout and
      for time, with lists and grids from slides
    - **[plan]** its semantics, given as the tasks that play each tag
    - **[plan]** an implementation over pico, in `ia-atmos/pico-media`
    - **[plan]** the examples of 2016 written again, and a comparison

## 2. Background

- Céu-Media in one page (WebMedia 2016)
    - Scene, Media (a tagged data type), Player (an organism)
    - four layers: LibPlay over GStreamer (0), the library (1), programs
      (2), new abstractions such as a slideshow (3)
    - the scene clock advances only when the program says: logical time
      drives the timestamps of the samples
    - its claims against NCL and SMIL: inflexibility, ambiguity, and
      logical and physical dyssynchrony
    - its own seed of this paper: the slideshow could already be given as
      a Lua table (`background` and `sequence` lists of `tag`, `uri`,
      `rect`), parsed by Céu code
- Atmos in one page
    - tasks, `await`, `emit`, `par`, `watching`, pools; `defer`
    - synchronous and deterministic: tasks yield only at an `await`, wake
      in a fixed order
    - time is an event; clock literals (`3s`) are numbers
    - tables and tags: `:image [src=...]` is `[tag=:image, src=...]`;
      hierarchical tags (`:media.image ?? :media`)
    - data: `X.dump` and `X.load`, with no code and no cycles
    - `pin`: a block owns the values it pins (refer to `papers/pin.md`)
- pico in one page
    - layers, images, text with fonts, sounds, video frames
    - Pos, Rect, Dim with a mode (`'!'` pixels, `'%'` percentage, `'#'`
      tiles) and an anchor (`'C'`, `'NW'`, ..., or `[x=,y=]`)
    - scripted time and input for tests (`PICO_ATM_FRAMES`, `_KEYS`,
      `_SHOT`)

## 3. What Changes in Atmos

The four points of the thesis, each against Céu-Media.

- The document is a value **[claim]**
    - Céu-Media: a `Media` per object, and the structure in the code
      (`par/or`, `watching`, `loop`)
    - Atmos: the whole structure is one table; the code is a player
    - consequences: a document can be printed, saved, loaded, compared,
      checked before it plays, and written by a tool or a template
- Tasks play it **[claim]**
    - a primitive is a task that draws it while it lasts
    - a composite is a task that spawns its parts and gives them time and
      place
    - the player is a function of the table, as in `papers/snapshots.md`,
      applied to media (cited, not restated)
- Structured concurrency gives time and abortion **[claim]**
    - what ends a part: its `dur`, the end of its medium, or an event
    - a part is a task in a pool of its composite: when the composite
      ends, its parts are aborted, and their resources released
    - the order of a reaction is fixed: two parts that start together
      start in the same frame (the "LEDs" of 2016)
- `pin` gives the document one owner **[claim]**
    - `pin doc = ...` owns every table inside it; tasks refer to parts,
      they do not own them
    - the document may be replaced as a whole (`set doc = <new>`): the
      old one is finalized when its last reference goes, at the latest
      at the end of its block (rules of pin as of 2026-11-17)
    - **[open]** what the player does then: abort and replay, or keep
      the parts that did not change (close to `snapshots`; see
      Section 9)
    - resources (layers, fonts, sounds, videos) are pinned by who loads
      them, and freed when that block ends
    - by the rules of pin, cited by name, not restated
- What does not change: the synchronous hypothesis, and the argument of
  2016 on logical time

## 4. The Vocabulary

**[open]** All of it waits for the agreement of the vocabulary in the
project (its milestone M1). What follows is the shape the author gave,
and the first sketch of the plan.

- The convention of tags
    - lower case: a primitive (`:image`, `:video`, `:text`, `:audio`)
    - upper case: a composite (`:Seq`, `:Par`, `:List`, `:Grid`, `:Srt`)
    - **[open]** a convention only, or a hierarchy (`:media.image`) that
      `??` can match
- Primitives: fields
    - the source (`src`), a position or a rectangle in pico's terms, a
      layer or a `z`, an alpha, a volume
    - time: `dur`, or the end of the medium, or none (until aborted)
- Composites of time **[open]**
    - "one after the other" and "together": names, and what ends each
      (the first part, all parts, a given one)
    - the `sequence` and `parallel` of the slideshow of 2016 as the
      precedent
- Layout
    - the anchors and relative positions of pico: a part is placed in
      the rectangle that its composite gives it
    - `'%'` positions make a document independent of the resolution
- Lists (bullets), from slides
- The grid
    - rows and columns; the cells in an array
    - each cell says the anchor and the position of what goes in it
    - **[open]** spans; sizes of rows and columns (equal, percentages)
- Concepts, not necessarily implemented
    - `:Srt`: a composite of timed texts, from a SubRip file (the SRT
      organism of 2016)
    - slides, from Aurora (`ThiPresotto/Reactive-Slides`): a presentation
      is an array of slides, each with a template and tagged objects
      (`Text`, `BulletList`, an image); here a template would be a
      function that builds a table **[open]**
- Is a document only data, or also code? **[open]**
    - only data: it can be dumped, loaded, checked, edited by a tool
    - with functions: actions at the end of a part, links
- A sketch (from the plan, not compiled, not agreed):

```
pin doc = :Seq [
    :image [ src="cover.png", pos=['%', x=50, y=50, anchor='C'], dur=3s ],
    :Par [
        :video [ src="talk.mp4", pos=['%', x=0, y=0, anchor='NW'] ],
        :Srt   [ src="talk.srt", pos=['%', x=50, y=95, anchor='S'] ],
    ],
    :Grid [ rows=2, cols=2,
        [ :image [src="a.png"], anchor='C' ],
        [ :List  ["one", "two", "three"], anchor='NW' ],
    ],
]
spawn Play(doc)
```

## 5. Semantics: the Player

**[plan]** Given as the tasks that play each tag; written once the
vocabulary is agreed.

- One task per tag; `Play` dispatches on the tag
- A primitive: load the resource (pinned), draw it every frame while it
  lasts, end by `dur`, by the end of the medium, or by abortion
- A composite of time: a pool of parts; spawn in order or together; end
  by its rule
- A composite of space: give each part its rectangle; the part places
  itself by its anchor
- Time passes in frames: what runs once per frame awaits the frame event
  of pico
- Interaction: a part that awaits a key or a click (the next slide)
- Extension: a new tag is a new task, registered with the player
    - the claim of 2016 against NCL and SMIL, now at the level of the
      document **[claim]**
- What the player guarantees, and what it cannot
    - logical time is exact; physical time depends on pico and on the
      decoders (2016 had the same caveat for native calls)
    - **[plan]** whether pico can bind the clock of video and audio to
      logical time, as the scene clock of Céu-Media did

## 6. Examples

**[plan]** The examples of 2016, written again as documents. Inspiration
only: no fidelity to the original code or interface (the author,
2026-11-16).

- Blinking (the "LEDs"): two images, 2 s and 4 s, until a key
    - against Céu, NCL, and SMIL, as in 2016
- Side-by-side videos for 15 s, restarted when both end
- Subtitles: `:Srt` over a video
- The slideshow: three images in a loop, music, subtitles, a key to quit
- Lip synchronization: muted videos and one audio started in the same
  reaction (the library has `lip-sync.ceu`; the blind version of the
  paper has the case of four muted videos and an audio)
- A presentation of slides, from Aurora's concepts: cover, title and one
  object, title and two objects
- A grid: a page of mixed media
- For each: the document, the lines of the player it needs, and a
  screenshot under scripted time

## 7. Evaluation

**[plan]** Nothing measured.

- Expressiveness
    - each example of 2016: lines in Céu-Media (from `rodrimc/ceu-media`),
      in NCL and SMIL (from the paper), and as a document
    - what each version makes explicit: time, end, abortion, layout
- Documents as data
    - dump and load every example; check a document before it plays
    - a template (slides) as a function that builds a table
- Precision of time
    - traces of start and end of parts under scripted time
    - two parts started together appear in the same frame
- Extension
    - the cost of a new tag (a composite written as a task): lines, and
      what in the player changes
- Limits, to state
    - video and sound with no screen and no device (to check first in
      the project)
    - no comparison with an NCL or SMIL player on the same media

## 8. Related Work

- Multimedia document languages
    - NCL (Ginga), SMIL 3.0: containers of time, links, regions
    - their semantics: the ambiguity cited in 2016
    - templates for them: TAL, XTemplate (cited in 2016)
- The Web
    - HTML with CSS: layout by boxes, flex and grid; the grid of CSS as
      the closest to `:Grid`
    - timed media: the media elements, WebVTT and text tracks,
      animations; time is not part of the layout
- Slide tools as documents
    - Beamer, reveal.js, Marp, Slidev: text that is compiled to slides
    - Aurora (Presotto, 2026): slides as a Lua table played in Atmos
- Synchronous languages and media
    - Céu-Media (WebMedia 2016) and its workshop paper (WebMedia 2017);
      Mars and the thesis of Rodrigo Santos (PUC-Rio, 2018)
    - Esterel; ChucK, Faust, Pure Data (cited in 2016, for a different
      audience)
- Declarative UI as data
    - the Elm architecture, React: the view as a function of state
    - the difference: here the document also says *when*, and its parts
      are tasks with a lifetime
- **[plan]** to check each against its sources and add references; the
  list is from memory and from the paper of 2016

## 9. Overlap with the Other Papers

- `papers/games.md`: structured concurrency in games, evaluated on ports
    - not here: games, frame budgets, ports; here only media documents
- `papers/pin.md`: memory
    - here `pin` is used, cited by its rules, not restated
- `papers/snapshots.md`: state as data, tasks spawned from it, commands
    - the closest: "tasks as a function of a table" is its idea
    - here the table is a document authored by hand, read-only while it
      plays, and the point is time and layout, not persistence nor
      consensus
    - **[open]** whether a document that changes while it plays (an
      editor) is in scope; if so, it is a case of `snapshots`, and cited
- Céu-Media (2016) itself: this paper must say what is new against it,
  and not rely on its claims without restating them in a sentence

## 10. Conclusion

- From a library of media for a synchronous language to documents that
  are values of it
- Future work
    - an editor of documents, live
    - an NCL or SMIL player written as a set of tags (a future work
      already named in 2016)
    - the Web as a target of the same documents
