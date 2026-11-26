# Pages as Programs: Structured Synchronous Concurrency over the DOM

The title is a placeholder. Alternatives:

- A Page is a Sequence in Time: Atmos in the Browser
- No Callbacks, No Handles: Structured Synchronous Programming of Web
  Pages
- Four Functions and Five Events: a Synchronous Language over the DOM

> Status: outline, first version, 2026-11-26. Nothing in it is decided.
> The venue, the kind of paper, and its scope (the DOM only, or games
> too; a paper of its own, or a section of another) wait for the author
> (`paper-web#1` to `#3`, in `.claude/plans/TODO-paper-web.md`).
> **[claim]** marks what the paper argues, **[have]** what exists and
> was read, **[plan]** what must still be built or measured, **[open]**
> what waits for a decision. The facts about other systems are in
> `papers/web-related.md`, each with its source; until a fact is there,
> it is from memory and is not to be written here as true.

The material is the plan `.claude/plans/261126-paper-web.md`, written by
the session `env-web`. The environment is `ia-atmos/env-web` (read at
`main` 4b006c3).

## 1. Introduction

- Context: a web page is a reactive system
    - it waits for the user, the network and the clock, and answers
    - its logic is often a sequence in time: "after a click, wait 300ms
      with no key, then ask the server, unless another key arrives"
- Problem: the platform gives callbacks
    - the sequence is cut into handlers, and the state that links them
      goes to variables that outlive each step
    - what is in course must be cancelled by hand: a timer, a request, a
      listener, an element that was put on the page
- State of the art, by what each does with the sequence (Section 7)
    - state and view: Elm, React, Solid, Svelte. The sequence becomes
      a state; cancellation is a cleanup function of an effect
    - streams: RxJS, Flapjax. The sequence becomes a combination of
      operators; cancellation is `switchMap`, `takeUntil`
    - the platform: `async`/`await`. The sequence is back, but
      cancellation is a signal passed by hand
    - synchronous: HipHop.js. The sequence and the preemption are both
      in the language; the nearest relative
- Proposal **[claim]**: structured synchronous concurrency written
  directly over the DOM
    - the page is the state: no model to keep in step with it
    - the program is the sequence: `await`, `watching`, `par`
    - what a task put on the page leaves with the task: `defer`
- Contributions
    - **[have]** a binding of a synchronous language to the DOM with
      four functions and events as data, with no callbacks and no
      handles
    - **[have]** an environment with no loop of its own: the browser
      drives the program, one reaction per event or frame
    - **[plan]** the seven tasks of 7GUIs and a set of cases with
      preemption, against public versions in the other families
    - **[plan]** the same programs over a desktop toolkit (IUP): how
      much of a program is the environment
    - **[plan]** the costs: the size of the page and its time to start

## 2. A running example

One interaction with time and preemption, small enough to show whole in
each family. Candidates **[open]** (`paper-web#5`):

- a type-ahead search: a debounce, a request, and a new key that
  cancels both;
- the tree of threads of `exs/threads.atm` **[have]**: a task per
  element, a pool, `defer` removes the element;
- the counter with a clock and a stop of `exs/counter.atm` **[have]**:
  the smallest program with a `watching` over two activities.

Shown first with callbacks in plain JavaScript: the handlers, the
flags, the timers to clear. Then where the difficulty is: state spread
over handlers, cancellation by hand, the lifetime of what was created.

## 3. Atmos, in short

- The synchronous hypothesis: a reaction to one event runs to the end
  before the next; no data races, a deterministic order.
- The mechanisms the paper uses: `await`, `loop on`, `watching`, `par`,
  tasks and pools (`tasks`, `spawn @pool`), `pin`, `defer`.
- Events are data: tags with fields, filtered in the `await`
  (`:click [id="inc"]`).
- Lineage: Esterel, Céu.

## 4. The environment

### 4.1 The interface **[have]**

| Function | What it does |
|---|---|
| `web.set(id, name, v)` | sets a property: `"text"`, `"class.<c>"`, `"style.<p>"`, or a property of the DOM |
| `web.get(id, name)` | reads a property, with the same names |
| `web.new(tag, t)` | creates an element and gives its id; named fields are properties, positional ones are children |
| `web.del(id)` | removes an element, with its children |

| Event | Fields |
|---|---|
| `:click` | `id` |
| `:input`, `:change` | `id`, `value` |
| `:toggle` | `id`, `open` |
| `:key.dn`, `:key.up` | `id`, `key`, `code` |

The time arrives at each frame, as in every environment of Atmos.

### 4.2 The decisions, and why

- An element is named by its id, a string, as in the HTML: no handle,
  so nothing to keep alive nor to release. (How the API was reached,
  from handles as in IUP to ids: `env-web` items `#3` to `#10`.)
- One listener of each kind on the document, in the capture phase:
  nothing is registered, an element created later emits too. (Measured
  in Chromium: `toggle`, `focus`, `blur` do not bubble.)
- One `set` for text, classes, styles and properties, with prefixes.
- A page that exists (HTML written by hand, the program only reacts)
  or a page that the program creates: both.

### 4.3 The browser drives the program **[have]**

- An environment with no `step`: the main chunk runs at the load, then
  the page calls in with the time of each frame and with each event.
- No Asyncify for the DOM (the games need it, for their blocking loop).
- One reaction per event: the synchronous hypothesis over the event
  loop of the browser.

### 4.4 What is open **[plan]**

Preventing the default of an event, selectors, `:focus`, the clock in a
hidden tab, the end of the program. Asked to `env-web`.

## 5. Patterns

Each one: the case, the code in Atmos, and the same in each family
(Section 7 has the systems). The list is a first one **[open]**.

| Pattern | Mechanism | Case |
|---|---|---|
| A sequence of steps | `await` in sequence | a wizard with "back"; the two clicks of the Flight Booker |
| A timeout, a debounce | `watching` a time, `loop` that restarts | the type-ahead |
| Cancel what is in course | `watching` an event | the stop of the counter; an animation that a click interrupts |
| Activities side by side | `par`, `spawn` | the clock beside the button |
| A lifetime tied to an element | a task with `defer` | a thread of the tree; a row of the CRUD |
| A set that changes | a pool (`tasks`) | the threads; the cells |
| A modal step | `await` inside the flow | the dialog of the Circle Drawer |
| A press, a move, a release | `watching` the release | a drag and drop |

## 6. Evaluation **[plan]**

- Subjects
    - 7GUIs (Counter, Temperature Converter, Flight Booker, Timer, CRUD,
      Circle Drawer, Cells) on the web. **[have]** the seven over IUP
      (`ia-atmos/env-iup`, `exs/7guis/`, 581 lines; its study against C
      with callbacks is `doc/study.md`)
    - the cases with preemption of Section 5
    - TodoMVC **[open]**
- Against: public versions in Elm, React, Svelte, HipHop.js and others,
  where they exist (the list, with sources, in `papers/web-related.md`)
- Questions
    - Q1: how does the code change? Lines and tokens; callbacks, state
      variables, flags
    - Q2: what must be written to cancel? Per case and per family
    - Q3: where can a bug of lifetime exist, and where can it not?
    - Q4: how much of a program is the environment? The diff between
      the IUP and the web version of each task
    - Q5: what does it cost? The size of the page and its time to start
      **[have]** (`env-web`, `261126-measures.md`): the counter is 18
      lines; its page is 237 KB of WebAssembly and 64 KB of JavaScript
      (`-O2`, the whole runtime), 94 KB and 18 KB with gzip; 168 KB of
      WebAssembly with `-Oz`; a median of 26 ms from navigation to the
      first reaction (headless Chromium, local, no network)
- Measures (a first definition, `paper-web#10`; the first three as in
  the study of `env-iup`, `doc/study.md`)
    - lines: those that are neither blank nor only a comment; the HTML
      and the CSS of a page counted apart
    - reactions: the callbacks a version registers (handlers, effects,
      subscriptions); in Atmos, the `await` and `loop on` over events
    - state: the variables that live between two reactions, and where
      they are declared (global, a component, a block)
    - to cancel: what a version writes so that something in course
      stops (a flag, a `clearTimeout`, a cleanup, an `abort`, a `kill`),
      and how far it is from what it cancels; in Atmos, the `watching`
      that encloses it
    - tokens: to define
- A warning from the thesis of 7GUIs (Kiss 2014): "the toolkit in
  general played the most crucial role", more than the paradigm. Only
  the Timer and the dialog of the Circle Drawer are about concurrency
  and steps; the cases with preemption and with elements created at
  run time carry the claim, and 7GUIs shows that nothing is lost in
  the rest.
- Threats: the programs are small; the Atmos versions are written by
  who proposes the approach; lines are a weak measure

## 7. Related work

By family; the table of the plan (Section 4 of it), each fact checked
against its source in `papers/web-related.md`.

- Synchronous languages on the web: HipHop.js and Pendulum (checked;
  no other published one was found). The difference must be precise: a language of
  its own or an embedding; a static program or tasks created at run
  time; how values reach the page; one model for desktop, games and
  boards.
- The lineage: Esterel, Céu (its GUIs and games).
- State and view: Elm; React, Solid, Svelte.
- Streams and FRP: Flapjax, RxJS, FRP in general.
- The platform: `async`/`await`, `AbortController`; structured
  concurrency in other languages and in libraries of JavaScript.
- Statecharts: XState.
- Attributes in the HTML: htmx, hyperscript.
- Sequences of events in a library: `Lwt_js_events` of js_of_ocaml
  (`Lwt.pick`, the nearest to our `par/or`, with a weaker
  cancellation), redux-saga (`race`, `takeLatest`, `finally`), Concur,
  CSP in the browser.
- Research on control flow for the web: Arrowlets, Links, Ur/Web,
  continuations.
- Other languages over WebAssembly and the DOM: the cost of handles
  and of closures (Go `syscall/js`, Rust `web-sys`).

The nearest ones, by what the paper claims (facts and quotes in
`papers/web-related.md`, sections 1, 7 and 9):

| System | The page bound in the language | Activities created at run time | Preemption | Undoing what was started |
|---|---|---|---|---|
| HipHop.js | no: listeners and reads of signals, by hand or by Hop.js | from outside, between reactions; compiles again | `abort`, `every` | `kill` of an `async` |
| Pendulum | yes: events and properties are signals | no: static elements | `trap`, `exit` | none |
| Arrowlets | events as arrows | arrows are values | `or` cancels the loser; `cancel` from outside | cancellers of listeners only |
| Lwt_js_events | events as promises | threads | `Lwt.pick` "tries to cancel", by an exception | handlers of the exception |
| redux-saga | no: actions of Redux | `fork` | `race`, `takeLatest`, `cancel` | `finally` |
| Atmos | yes: `await` of events, `web.set` | `spawn` in a pool | `watching`, `par` | `defer` |

## 8. Limits and discussion

- Size: the whole runtime in each page.
- One program per page, one thread, no server side, no routing.
- The DOM by ids is imperative: no view derived from a state. The
  answer is the control flow, shown by the cases; where a derived view
  is better (a list that is filtered and sorted), to say so.
- Ids are strings: a wrong id in a function is an error at run time,
  which stops the program and shows only in the console; a wrong id in
  the pattern of an `await` never matches, with no error.
- Games in the browser (pico and SDL through Emscripten, with
  Asyncify) **[open]** (`paper-web#2`).

## 9. Conclusion
