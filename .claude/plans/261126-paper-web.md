# paper-web: the material of the session (2026-11-26)

For the session `paper-web`, created by `atmos` on the author's request
in `env-web` (2026-11-26, checked in its transcript). The session writes
`papers/web.md` and keeps `.claude/plans/TODO-paper-web.md`.

The plan below was written by `env-web`
(session_011eECoffhJTecG6BZSq3GQi) and is copied unchanged from
`ia-atmos/env-web`, `.claude/plans/261126-paper-web.md` (`main` 1537fb0).
Nothing in it is decided. The facts about other systems come from
memory and must be checked against the sources. The environment and
its examples live in `ia-atmos/env-web` (public; read it by cloning),
where only `env-web` pushes.

---

# paper-web: a plan for a paper on Atmos in the browser (2026-11-26)

For the session `paper-web`, to be created. Written by the session
`env-web` (session_011eECoffhJTecG6BZSq3GQi), which keeps the
environment and answers questions about it. Nothing here is decided: it
is a starting point for that session and for the author.

## 1. The request

**Autor** (2026-11-26, in `env-web`, after approving the API as `#10a`):
> Depois criar uma sessão paper-web. Tem muita coisa boa aí... Mas vamos
> precisar comparar com elm,hiphop e outros. Mande um plano pra lá

## 2. What exists (the material of the paper)

All in `ia-atmos/env-web`, `main` 2150f5e or later:

| What | Where |
|---|---|
| The environment: 4 functions (`set`, `get`, `new`, `del`), 5 kinds of events, elements named by their ids | `web.atm` (the header is the reference), `README.md` |
| How the browser drives the program: an environment with no `step`; the page calls in with the time of each frame and with each event; no Asyncify for the DOM | `web.atm`, `.claude/plans/261125-dom.md` |
| Examples: a counter over an existing page; a tree of threads that the program creates (a task per thread, `defer` removes its element) | `exs/counter.atm`, `exs/threads.atm` |
| Tests in Chromium (Playwright), 26 checks | `tst/` |
| How the API was reached: handles as env-iup, then ids; `text` folded into `set` | `.claude/plans/TODO.md` (items `#3` to `#10`), `261126-web-api.md` |
| A first study of similar systems (mostly from memory: to be checked against the sources) and a measurement of which events bubble | `.claude/plans/261126-web-study.md` |
| Games in the browser: Rocks of env-sdl and of env-pico run unchanged through Emscripten (Asyncify for the blocking loop) | `261125-rocks-emscripten.md`, `261126-pico.md` |
| The old plan of the DOM (Lua over Wasmoon, Mar/2026) | `atmos-lang/web` `.claude/plans/dom.md` |
| The same seven tasks in a desktop toolkit, with a study against C with callbacks | `ia-atmos/env-iup` `exs/7guis/`, `doc/study.md` |

Sizes measured: the counter is 18 lines of Atmos (43 for the threads),
with no blank lines nor comments; its page is 237 KB of WebAssembly and
64 KB of JS (`-O2`, the whole runtime).

## 3. A possible thesis

A page is a reactive system, and its logic is a sequence in time.
Structured synchronous concurrency (tasks, `await`, `watching`, `par`,
pools, `defer`) written directly over the DOM gives, with an interface
of four functions and events as data:
- no callbacks and no handles, so no lifetimes of closures to manage;
- cancellation by structure: a `watching` that ends aborts what is
  inside it, and a `defer` removes what a task put on the page;
- the same language and the same idioms as the desktop (env-iup), the
  games (env-sdl, env-pico) and the boards (env-esp32).

To be tested against the alternatives, not assumed.

## 4. The comparison the author asked for

For each system: how an element is named, how an event arrives, how the
page changes, how something in course is cancelled, how time is
written, and what the runtime costs (size, start).

| System | Family | Why it matters |
|---|---|---|
| Elm | functional, model-update-view, a virtual DOM | events as data, as ours; the opposite on the page: a view rebuilt from the state, and no sequence in time (a multi-step interaction is a state machine in the model) |
| HipHop.js (and Hop) | Esterel in JavaScript | the nearest relative: synchronous reactions, preemption (`abort`, `every`), DOM events as signals; the comparison of preemption and of how values reach the page is the core |
| Esterel, Céu | the lineage | where `watching`/`par` come from; Céu's own work on reactive GUIs and games |
| React (hooks), Solid, Svelte | components, signals | the mainstream: state plus effects, cleanup functions in place of `defer` |
| RxJS, Flapjax, FRP in general | streams over time | time as a first-class value: `switchMap` against `watching` |
| JavaScript `async`/`await` with `AbortController` | the platform itself | sequences in time without structured cancellation |
| XState (statecharts) | explicit state machines | the hierarchical alternative to a sequence |
| htmx, hyperscript | attributes in the HTML | ids and selectors as names; little or no program |
| Arrowlets, Links, Eliom (if they hold) | research on web control flow | related work to check |

The facts of this table come from memory: each one must be checked in
the papers and the documentation before it is written.

## 5. A common ground for the evaluation

- **7GUIs** (Counter, Temperature Converter, Flight Booker, Timer, CRUD,
  Circle Drawer, Cells): it has public versions in Elm, React, Svelte
  and others, and Atmos already has the seven over IUP
  (`env-iup/exs/7guis/`). Port them to `env-web`: the diff between the
  IUP and the web versions measures how much of a program is the
  environment.
- A case with real preemption and time, where the families differ most:
  a type-ahead search with a debounce and cancellation, a drag and drop,
  a wizard of several steps with "back", an animation that a click
  interrupts.
- TodoMVC, for the size of its public set of implementations.

Measures: lines and tokens; the number of callbacks, state variables and
flags; what must be written to cancel; the size of the page and its time
to start; where a bug of lifetime can exist and where it cannot.

## 6. What the paper needs from env-web (requests to that session)

- The 7GUIs on the web (needs at least: a canvas or SVG for the Circle
  Drawer, a modal dialog, lists and selections).
- What the API has open: preventing the default of an event, selectors,
  `:focus`, the clock in a hidden tab, the end of the program.
- Whether pico (the canvas) and the DOM live on one page.

## 7. Risks

- Size: the whole runtime in each page (237 KB), against a few KB of a
  hand-written page; Elm and Svelte are small here.
- One program per page, single-threaded, no server side and no routing:
  the scope must be said.
- The DOM by ids is imperative: the declarative systems will call it a
  step back; the answer must be the control flow, shown by the cases.
- HipHop.js is close: the difference must be precise (the language is
  its own, not embedded; memory by lexical structure; one model across
  desktop, games and boards), or the paper is a replication.

## 8. Questions for the author (none decided)

- The venue and the kind of paper (a language paper, an experience
  report, a workshop).
- Whether games in the browser (pico) are part of it or only the DOM.
- Whether it stands alone or is a section of the paper on games or of a
  paper on environments.
