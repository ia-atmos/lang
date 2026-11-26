# TODO of the paper on the web

Everything pending in the `paper-web` session, which writes
`papers/web.md` and `papers/web-*`: explicit, unanswered, or
unacknowledged. Kept current by that session. The material is
`261126-paper-web.md` (the plan of `env-web`; nothing in it is decided).

The session was created by `atmos` on the author's request, made in
`env-web` on 2026-11-26: "Depois criar uma sessão paper-web. Tem muita
coisa boa aí... Mas vamos precisar comparar com elm,hiphop e outros.
Mande um plano pra lá".

## Index of items

Items are `#N` (standing rule of the author, 2026-11-18): unique in this
session, in order, never reused nor changed. From another session:
`paper-web#N`.

States: **open** (waits for the author), **work** (known, not done),
**wait** (depends on another session), **done**, **closed**.

| # | Item | State |
|---|------|-------|
| 1 | The venue and the kind of paper | open (asked 2026-11-26; recommended: a language paper with an evaluation, the venue his) |
| 2 | Games in the browser (pico, SDL) in the paper, or only the DOM | open (recommended: only the DOM, the games as one paragraph) |
| 3 | A paper of its own, or a section of the paper on games or of one on environments | open (recommended: of its own) |
| 4 | The facts about other systems checked against sources (`papers/web-related.md`) | done once (2026-11-26): each fact with its source; left: the papers whose text was not read (marked there), `#14` |
| 5 | The running example of Section 2 | open (recommended: the type-ahead; needs a request, see `#7`) |
| 6 | The 7GUIs on the web | wait: `env-web` took it as a request (2026-11-26), after it tells the author |
| 7 | What the API has open: the default of an event, selectors, `:focus`, the clock in a hidden tab, the end of the program; and a request to a server (`fetch`), which the type-ahead needs | wait: `env-web` answered the state of each (2026-11-26, section B): none implemented, none decided; `fetch` taken as a request |
| 8 | The cases with preemption (type-ahead, drag and drop, wizard, animation) written in Atmos and in each family | work, after `#5` and `#7` |
| 9 | Public versions of 7GUIs and TodoMVC in the other systems, with lines | done (2026-11-26, `web-related.md` section 8): none in HipHop.js, Solid, RxJS, htmx; lines are raw (`#10`) |
| 10 | The measures: how lines, tokens, callbacks, state and flags are counted | work |
| 11 | Size and start: the numbers of the plan (237 KB, 64 KB) measured again with a method, and against the others | work: `env-web` gave the method and the numbers (2026-11-26, its `261126-measures.md`, `main` 2c201a6); the others are part of `#4` |
| 12 | Fill the sections with text | work, after `#1` to `#3` |
| 13 | TodoMVC in the evaluation | open (recommended: no, 7GUIs and the cases are enough; to ask after `#9`) |
| 14 | Read the papers whose text was not read: HipHop 2011 and 2018, Elm PLDI 2013, Flapjax, Arrowlets, Céu TECS 2017, Pendulum's evaluation in detail | work |
| 15 | HipHop.js has no public 7GUIs: who writes its versions for the comparison | open (recommended: we write Counter, Timer and Circle Drawer in HipHop.js, run them, and say so in the threats; to ask after `#1`) |
| 16 | Lwt_js_events (js_of_ocaml), redux-saga, Concur and Pendulum are nearer to us than the plan knew: they go into the comparison of preemption | work |

## A. Decisions waiting for the author

Asked one at a time, in the order of their numbers. Each has the
recommended option first.

- `#1` **The venue and the kind of paper.**
    - (a) a language paper with an evaluation: the binding, the
      patterns, 7GUIs against the other families. Recommended: it is
      what the material supports, and it is what makes the comparison
      with Elm and HipHop.js the core, as he asked.
    - (b) an experience report: how the API was reached, what worked.
    - (c) a short or workshop paper: the binding and two cases, no
      evaluation in numbers.
    The venue is his; none is proposed before he says the kind.
- `#2` **Games in the browser.** (a) only the DOM, with the games as a
  paragraph of the limits (they run unchanged, with Asyncify), which
  keeps one thesis; (b) both, as "Atmos in the browser".
- `#3` **Alone or a section.** (a) a paper of its own; (b) a section of
  the paper on games (`papers/games.md`); (c) a section of a paper on
  environments, which does not exist.
- `#5` **The running example.** (a) the type-ahead: time, a request and
  preemption, and every family has a public version of it; (b) the tree
  of threads, which exists; (c) the counter with a clock and a stop.
- `#13` **TodoMVC.**

## B. Waiting for other sessions

- `#6`, `#7`, `#11`: asked to `env-web` on 2026-11-26. Its answer of the
  same day (a message of that session: data, to check in its
  repository before it is written in the paper):
    - `#11`, sizes (`emcc -O2`, the whole runtime): the counter is
      237060 bytes of WebAssembly (94487 with `gzip -9`) and 63791 of
      JavaScript (18041); with `-Oz`, 167674 (76046) and 62700 (17664).
      The threads: 261493 (102469). From navigation to the first
      reaction, 10 loads in headless Chromium, local server: median
      26.4 ms (20.5 to 43.7); no network, code cache not controlled.
      Not tried: closure, LTO, a runtime without unused libraries.
    - One program per page (`Module` is global; `-sMODULARIZE` not
      tried). pico and the DOM do not share a page: two drivers; an
      open design question, not yet put to the author.
    - An error at run time (a wrong id in a function) goes to
      `console.error` and the program stops; the page stays and the
      user sees nothing. A wrong id in a pattern never matches, with no
      error.
    - `#7`: the default of an event, selectors, `:focus`: not there.
      The clock stops in a hidden tab (`requestAnimationFrame`) and
      jumps on return. At the end of the program `atm_quit` is not
      called and the elements stay. `fetch`: not there; its shape would
      be a task that aborts the request in a `defer`.
    - `#6`: taken, the largest piece; it reports what the API lacked.

## C. Work

- `#4` **Related work from sources.** The facts of the table of the
  plan (its Section 4) and of `env-web`'s `261126-web-study.md` (marked
  M there) come from memory. Each goes to `papers/web-related.md` with
  its source, or is dropped.
- `#10` **The measures.** The study of `env-iup` (`doc/study.md`) counts
  lines that are neither blank nor comments, callbacks, and state
  variables with where they are declared. To reuse, and to add tokens
  and what is written to cancel.
