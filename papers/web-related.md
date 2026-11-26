# Related work for the paper on the web: facts and their sources

For `papers/web.md`, item `paper-web#4`. Each fact about another system
is here with the source it was read from on 2026-11-26, or is marked
**unverified**. What is not here is from memory and does not go into
the paper. A quote is the text of the page as fetched.

All sections were checked once. A line count is of physical lines
(blank ones and comments included), taken with `wc -l` on a clone made
on 2026-11-26; it is not yet the measure of the paper (`paper-web#10`).

## 1. Synchronous languages on the web

### HipHop.js

Sources: Berry and Serrano, PLDI 2020 (the PDF was read); the
repository `manuel-serrano/hiphop` (`README.md`, `doc/`, branch
`master`). Not read: the papers of 2011 and of 2018 (only their
metadata).

| Point | Fact | Source |
|---|---|---|
| What it is | An extension of the syntax of JavaScript (`hiphop <stmt>`, files `*.hh.js`), "inspired from Esterel". It runs on unmodified engines, in the server and in the browser; Hop.js is not required. | PLDI 2020; `README.md` |
| How it is compiled | To an "augmented boolean circuit" (a net list); the reaction is "a linear-time software simulator of the circuit". The documentation of today adds an option that compiles acyclic net lists to plain JavaScript. | PLDI 2020, section 5; `doc/dev.md` |
| Statements | `module`, `run`, `signal`, `emit`, `sustain`, `if`, `yield` (the pause), `halt`, `loop`, `fork {} par {}`, `await` (with `count`, `immediate`), `abort {} when`, `weakabort {} when`, `every`, `do {} every`, `suspend`, traps as labels with `break`, and `async`. | `doc/syntax/hiphop.bnf`, `doc/lang/flow.md` |
| Events in | A listener of JavaScript calls the machine: `mach.react({sig: value})`. In the login page of the paper: `<input onkeyup=~{M.react({name: this.value})}/>`. A shortcut `mach.bindEvent(event, obj)` exists. | PLDI 2020, section 2.4; `doc/api.md` |
| Values out | A listener on an output signal (`mach.addEventListener("O", ...)`), called at the end of the reaction; or the signal read as a property of the machine (`M.O.nowval`) inside reactive expressions of Hop.js (`~{...}`, `<react>`). The binding to the DOM is of Hop.js or by hand, not of HipHop. | same |
| Asynchronous JavaScript | `async [sig] { ... }` starts a body of JavaScript and blocks its own thread until `this.notify(v)`. When an enclosing `abort` or `every` preempts it, its clause `kill { ... }` runs (the timer of the paper: `kill { clearInterval(this.intv) }`). | PLDI 2020, section 2.2; `doc/lang/async.md` |
| Static or dynamic | Programs are values of JavaScript and are staged (`${...}` splices statements and branches) before the machine is built. With `{dynamic: true}`, `mach.appendChild` and `mach.removeChild` change a labelled `fork` or sequence between two reactions. The paper: "dynamic at source-code level: it allows the user to partially reconfigure the program between two synchronous reactions", not discussed further there. | `doc/staging.md`, `doc/api.md`; PLDI 2020, section 6 |
| Semantics | The constructive semantics of Esterel: signals are instantaneous, a cycle that is not constructive is detected at run time, in the reaction, with an error. | PLDI 2020, sections 5.1 and 5.2 |
| Examples and numbers | A login panel (two versions, with a timer of the session), a pillbox of prescriptions (399 nets, about 86 KB), an interactive music system (up to 10,000 nets, about 2.1 MB, a reaction never above 15 ms). 192 to 216 bytes per net. No type-ahead and no 7GUIs. | PLDI 2020 |

What follows for our comparison (a reading of ours, to be shown with
code): the element reaches the program through a listener written in
JavaScript or in the page, so the callback still exists at the border;
what a statement started is undone by a `kill` clause, which is the
counterpart of `defer`; a set of activities that grows is a `fork`
changed from outside, between reactions, where we have a pool of tasks;
and its signals are instantaneous with a check of causality, where our
events are a queue of data with no cycles to check.

### Others

| System | Fact | Source |
|---|---|---|
| Pendulum | "An immersion of a subset of Esterel into a Web client context": an extension of OCaml (`let%sync`), compiled to sequential code and then to JavaScript by js_of_ocaml. Events of the DOM are input signals, and it emits into properties of elements; evaluated with TodoMVC. | El Sibaïe and Chailloux, REBLS 2016 (the PDF was read) |
| ReactiveML | The reference is confirmed; a toplevel in the browser was **not found** (its pages gave 404). | Crossref |
| SugarCubesJS | A library of JavaScript after the reactive model of Boussinot, "first targeted to ease the build of WebApps". No paper was found. | github.com/LordManta/SugarCubesJS |
| Céu in the browser | Only a tutorial of a third party that compiles Céu with Emscripten on a server; no binding to the DOM, no publication. | github.com/VicentiuM/Ceu-Emscripten |

Besides HipHop.js, Pendulum is the only published language in the
style of Esterel that was found over the DOM.

## 2. The lineage (Esterel, Céu) and Elm

### Esterel and Céu

| Fact | Source |
|---|---|
| The kernel of Esterel has `do stat watching S`, `stat1 \|\| stat2` and `trap`; it rests on the "synchrony hypothesis: the outputs of a system are conceptually synchronous with its inputs". | Berry and Gonthier (read in the report INRIA RR-842, 1988) |
| `watching` is the form before Esterel v5: "we now suggest to replace the 'do...watching' statement by 'abort...when'". | Berry, The Esterel v5 Language Primer, 2000 |
| Céu, 2013: `par/or`, `par/and`, `finalize`, internal events with a stack, checks of concurrency at compile time. No `watching` and no organisms there. | Sant'Anna et al., SenSys 2013 |
| Céu, 2015: the name "Structured Synchronous Reactive Programming", organisms, pools with a lexical scope, and `watching`; its evaluation mentions a game over SDL. | Sant'Anna et al., Modularity 2015 |
| Céu, 2017 (TECS) and Céu-Media (WebMedia 2016): references confirmed, text not read. | Crossref |
| Pingus in Céu: 9,186 lines of C++ into 3,697 of Céu, a qualitative study of four behaviours. | Sant'Anna, SBGames 2018 (the PDF was read) |

### Elm

| Point | Fact | Source |
|---|---|---|
| Architecture | Model ("the state of your application"), view ("a way to turn your state into HTML"), update ("a way to update your state based on messages"). The view runs again at each change and a virtual DOM is diffed. | guide.elm-lang.org/architecture; elm/virtual-dom |
| Events | Attributes that carry a message or a decoder: `onClick : msg -> Attribute msg`, `on : String -> Json.Decoder msg -> Attribute msg`. | elm/html 1.0.0, `Html/Events.elm` |
| Elements | No reference to an element is held; where one is needed, it is named by its id, a string (`Browser.Dom.focus : String -> Task Error ()`). The first clause is our reading of the API. | elm/browser 1.0.2, `Browser/Dom.elm` |
| The default of an event | Data given back by the decoder: `preventDefaultOn : String -> Json.Decoder (msg, Bool) -> Attribute msg`, and `stopPropagationOn`, `custom`. | `Html/Events.elm` |
| Time | A subscription computed from the model: `Time.every : Float -> (Posix -> msg) -> Sub msg`; a delay is `Process.sleep`. | elm/time 1.0.0; elm/core 1.0.5 |
| Sequence | `Task.andThen` puts effects in sequence. No task waits for an event of the page (in the modules read), so an interaction in steps is a state of the model with cases in `update`: tasks sequence effects, not events of the user. Our reading, not a sentence of the documentation. | elm/core 1.0.5, `Task.elm`, `Process.elm` |
| Cancellation | Explicit, by a name or an identifier: `Http.cancel : String -> Cmd msg` with the `tracker` of the request; `Process.kill : Id -> Task x ()`. Not tied to a scope. | elm/http 2.0.0; elm/core `Process.elm` |
| Size | No official number of a minimal page. TodoMVC: 122,297 bytes, 24,123 minified, 9,148 with gzip. The RealWorld application: 29 KB. | guide.elm-lang.org/optimization/asset_size.html; "Small Assets without the Headache", 2018-08-21 |
| FRP | Elm left signals in 0.17: "So is Elm about FRP anymore? No." | "A Farewell to FRP", elm-lang.org/news, 2016-05-10 |

## 3. Components and signals: React, Solid, Svelte

| System | Fact | Source |
|---|---|---|
| React | An effect returns a cleanup: "After every commit with changed dependencies, React will first run the cleanup function (if you provided it) with the old values, and then run your setup function with the new values. After your component is removed from the DOM, React will run your cleanup function." | react.dev/reference/react/useEffect |
| React | The cleanup shown clears a timer or removes a listener. For a request, the documentation shows a flag `ignore` set in the cleanup: the stale answer is discarded, the request is not cancelled. It says: "There's quite a bit of boilerplate code involved when writing `fetch` calls in a way that doesn't suffer from bugs like race conditions." | same page |
| React | A handler is a function passed as a prop (`onClick={handleClick}`). | react.dev/learn/responding-to-events |
| React | The current documentation says "render tree", not "virtual DOM". The words "virtual DOM" and "reconciliation" are in the legacy documentation. | legacy.reactjs.org/docs/faq-internals.html |
| Solid | "There is no virtual DOM and no component re-render"; "a component function runs once, and the reactive expressions inside it keep running as their inputs change". | docs.solidjs.com |
| Solid | The version matters. In Solid 2 (npm `latest` is 2.0.4) `createEffect` takes two functions and "The effect function can return a cleanup". The form with one function and `onCleanup` is Solid 1. | docs.solidjs.com/concepts/reactivity; the `solid-docs` repository for `onCleanup` (its page gave 404) |
| Svelte 5 | Runes (`$state`, `$derived`, `$effect`, ...) are "symbols that you use in `.svelte` and `.svelte.js` / `.svelte.ts` files to control the Svelte compiler". | svelte.dev/docs/svelte/what-are-runes |
| Svelte 5 | "An effect can return a teardown function which will run immediately before the effect re-runs", and when the effect is destroyed. | svelte.dev/docs/svelte/$effect |
| Svelte | No virtual DOM: not said in the documentation; the source is a post. | R. Harris, "Virtual DOM is pure overhead", svelte.dev/blog, 2018-12-27 |

A reading of ours, not a statement of those pages: none of the three
gives a sequence of awaits with a cancellation by scope. An interaction
in steps becomes a variable of phase and an effect whose cleanup clears
the timer; the cancellation is tied to the effect that runs again or to
the component that leaves, not to a lexical block. Not checked: the
asynchronous features of each (React `use` and Suspense, the async
memos of Solid 2, `await` in Svelte).

Sizes: no official number was found for any of the three. The numbers
of Bundlephobia are of whole packages and are not to be used; the
paper measures its own minimal pages.

## 4. Streams and FRP

| System | Fact | Source |
|---|---|---|
| RxJS | `switchMap`: "Projects each source value to an Observable which is merged in the output Observable, emitting values only from the most recently projected Observable." The implementation unsubscribes the previous inner subscription. | ReactiveX/rxjs, branch `7.x`, `src/internal/operators/switchMap.ts` (the pages of rxjs.dev could not be fetched) |
| RxJS | `takeUntil`: "Emits the values emitted by the source Observable until a `notifier` Observable emits a value." `debounceTime`: "Emits a notification from the source Observable only after a particular time span has passed without another source emission." | same repository |
| RxJS | The function passed to an Observable returns its teardown (`unsubscribe`). | rxjs.dev guide "Observable", from the repository |
| RxJS | A type-ahead as the canonical example of the documentation: **not found** in the guides read. The documented example of `switchMap` is `fromEvent(document, 'click').pipe(switchMap(() => interval(1000)))`. | same |
| Flapjax | The reference is confirmed (below). What the paper says was not read: **unverified**. | Crossref |
| FRP | Elliott and Hudak 1997; Courtney and Elliott 2001 (Fruit: "defines GUIs compositionally as signal transformers"). | Crossref; the PDF of the second |
| Surveys | Bainomugisha et al. 2013; Salvaneschi et al. 2014 and 2017. | Crossref |

The numbers of Adobe, often repeated. The slide of Sean Parent says:
"1/3 of the code in Adobe's desktop applications is devoted to event
handling logic" and "1/2 of the bugs reported during a product cycle
exist in this code". Maier, Rompf and Odersky (2010) quote it as "an
Adobe presentation from 2008"; the title slide is dated July 25, 2007.
To write "of the bugs reported during a product cycle", not "half of
all bugs". Edwards, "Coherent reaction" (2009): the reference is
confirmed, its text was not read, and it is not a source for the words
"callback hell".

## 5. The platform: `async`/`await` and cancellation

| Fact | Source |
|---|---|
| An API that supports aborting accepts an `AbortSignal` ("The API which wishes to support aborting can accept an `AbortSignal` object"). The signal is passed to each call: in the options of `fetch`, and in `addEventListener` ("The listener will be removed when the `abort()` method of the `AbortController` which owns the `AbortSignal` is called"). "Cooperative" is our word. | DOM Standard (dom.spec.whatwg.org); MDN, `AbortSignal`, `addEventListener` |
| `AbortSignal.timeout` and `AbortSignal.any` exist. | MDN, `AbortSignal` |
| `Promise.race`: "Settling the returned promise does not cancel the losing operations or unsubscribe the handlers attached to their promises." | MDN, `Promise.race` |
| Explicit resource management (`using`, `Symbol.dispose`, `DisposableStack`) is a finished proposal of TC39, expected in the edition of 2027. `using` is not yet in every browser. | tc39/proposals, `finished-proposals.md`; MDN, `using` |
| There is no active proposal of cancellation in TC39: "Cancellation API" is inactive, "Subsumed by AbortController"; "Cancelable Promises" was withdrawn. | tc39/proposals, `inactive-proposals.md` |
| Observable: Stage 1 in TC39; the active work is in WICG, with `element.when('mousemove').takeUntil(document.when('mouseup'))`; in Chrome since 135, not in Firefox nor Safari. | tc39/proposals; github.com/WICG/observable; MDN browser-compat-data |

Structured concurrency elsewhere:

| System | Fact | Source |
|---|---|---|
| libdill | "Structured concurrency means that lifetimes of concurrent functions are cleanly nested." | sustrik.github.io/libdill/structured-concurrency.html; Sústrik, "Structured Concurrency", 2016-02-07 |
| Trio | "a parent task cannot start any child tasks unless it first creates a place for the children to live: a nursery"; the block of the nursery ends only when its tasks ended. | N. J. Smith, "Notes on structured concurrency, or: Go statement considered harmful", 2018-04-25; trio.readthedocs.io |
| Kotlin | "If the parent coroutine fails or gets canceled, all its child coroutines are recursively canceled too." | kotlinlang.org/docs/coroutines-basics.html |
| Swift | SE-0304, in Swift 5.5; cancellation "is fully cooperative and synchronous". | swift-evolution, proposal 0304 |
| Effection (JavaScript) | Structured concurrency with generators: "No operation runs longer than its parent" and "Every operation exits fully"; a `finally` runs when an operation is halted. | frontside.com/effection |
| Effect (TypeScript) | "Effect follows a structured concurrency model, where child fibers' lifetimes are tied to their parent"; an interruption runs the finalizers. | effect.website/docs/concurrency/fibers |
| Concur (Haskell, PureScript) | A UI framework where a widget ends with a value: sequence is the monad, `orr` shows widgets side by side and ends with the first that ends, cancelling the others. No paper; its author is not confirmed from a primary page. Near to us: a GUI as a sequence with a parallel-or. | github.com/ajnsit/concur; purescript-concur-core, `Types.purs` |

## 6. Statecharts

| System | Fact | Source |
|---|---|---|
| XState | Compound and parallel states. "Delayed transition timers are canceled when the state is exited." Invoked actors "are created and started when the state is entered, and stopped when the state is exited". | stately.ai/docs (`parent-states`, `parallel-states`, `delayed-transitions`, `invoke`) |
| XState | A promise of a state that is left: "the result of the promise is discarded"; the work stops only if the function passes on the `signal` it receives. | stately.ai/docs/promise-actors |
| Statecharts, SCXML | References below. | Crossref; w3.org/TR/scxml |

## 7. Attributes in the HTML; research on control flow for the web

### Attributes

| System | Fact | Source |
|---|---|---|
| htmx | Behaviour in attributes: `hx-get`, `hx-trigger`, `hx-target` (which "takes a CSS selector"), `hx-swap`; "on the server side you typically respond with HTML, not JSON". "htmx is small (~16k min.gz'd), dependency-free". | htmx.org; htmx.org/docs |
| htmx | Time and cancellation are modifiers: `delay:<time>` (starts again if the event comes again), `throttle:<time>`, `changed`; `hx-sync` with `drop`, `abort`, `replace`, `queue`. What is cancelled is a request, not code. | htmx.org/docs; htmx.org/attributes/hx-sync |
| _hyperscript | A language in attributes, "inspired by HyperTalk", "no async or await" (38 KB). `wait 2s`, `wait for <event>`, `wait for load or 1s`: a sequence in time, written in line. | hyperscript.org; hyperscript.org/commands/wait |
| _hyperscript | Events that come while a handler runs are queued (`queue last` is the default; `none`, `all`, `first`; `every` runs in parallel). No way to abort a running handler from outside was found: `halt` leaves the current one, and `repeat until event` looks at the event only at the end of a turn. No paper was found. | hyperscript.org/features/on; hyperscript.org/commands/halt |
| Alpine.js | "a collection of 15 attributes, 6 properties, and 2 methods" (`x-data`, `x-on`). | alpinejs.dev |

### Sequences of events in a library

| System | Fact | Source |
|---|---|---|
| Lwt_js_events (js_of_ocaml) | The nearest in style to our `await` and `par/or`: `let%lwt _ = Lwt_js_events.click elt in ...` waits in sequence, and `Lwt.pick [...]` "behaves as the first thread in the list to terminate, and cancels the others". The tutorial of Ocsigen draws on a canvas with `mousedowns`, and inside it a `pick` of `mousemoves` and `mouseup`. | `lwt_js_events.mli` (ocsigen/js_of_ocaml); ocsigen.org/tuto/latest/application.html |
| Lwt | The cancellation is weaker than an abortion: `Lwt.pick` "tries to cancel all other promises that are still pending"; it arrives as the exception `Lwt.Canceled`; "By default, cancelling the loop will not cancel the potential currently running handler"; and "It is recommended to avoid Lwt.cancel". | `lwt.mli` (ocsigen/lwt); `lwt_js_events.mli` |
| redux-saga | Generators with `take` (waits for an action), `race` ("the middleware automatically cancels all the losing Effects"), `takeLatest` ("automatically cancels any previous saga task started previously if it's still running"), and `cancel`, which "will use Generator.prototype.return to make the Generator jump directly to the finally block". "Cancellation propagates downward." `fork` is attached, `spawn` detached. | redux-saga.js.org/docs/advanced/TaskCancellation; redux-saga.js.org/docs/api |
| core.async, js-csp | CSP in the browser; Nolen: "global event loops defy local reasoning". | swannodette.github.io, 2013-07-12 and 2013-07-31; github.com/js-csp/js-csp |

### Papers

References confirmed in Crossref; the text of none was read, so what
each contributes is **unverified** and from memory: Arrowlets (Khoo et
al. 2009), Links (Cooper et al. 2006), Ur/Web (Chlipala 2015),
continuations for the web (Queinnec 2000 and 2003; Krishnamurthi et al.
2007), Stopify (Baxter et al. 2018), Eliom (Radanne et al. 2016), Lwt
(Vouillon 2008), js_of_ocaml (Vouillon and Balat 2014).

## 8. Task sets and sizes

### 7GUIs

"7GUIs was been created as a spin-off of the master's thesis Comparison
of Object-Oriented and Functional Programming for GUI Development by
Eugen Kiss at the Human-Computer Interaction group of the Leibniz
Universität Hannover in 2014" (7guis.github.io/7guis/more).

| Task | The challenge, as the site says |
|---|---|
| Counter | "Understanding the basic ideas of a language/toolkit." |
| Temperature Converter | "bidirectional data flow, user-provided text input." |
| Flight Booker | "Constraints." |
| Timer | "concurrency, competing user/signal interactions, responsiveness." |
| CRUD | "separating the domain and presentation logic, managing mutation, building a non-trivial layout." |
| Circle Drawer | "undo/redo, custom drawing, dialog control." |
| Cells | "change propagation, widget customization, implementing a more authentic/involved GUI application." |

Two of the seven are about what we claim: the Timer ("a timer process
that updates the elapsed time runs concurrently to the user's
interactions") and the dialog of the Circle Drawer ("keeping the
relevant context between several successive GUI interaction steps").
The others are about data flow and constraints, where a derived view
does well: the paper must say so. Kiss himself notes that 7GUIs lacks
"tricky asynchrony".

Public versions, lines per task (Counter, Temperature, Flight, Timer,
CRUD, Circles, Cells):

| System | Repository | Lines |
|---|---|---|
| Elm | github.com/joakin/elm-7guis | 21, 158, 305, 116, 242, 367, 1043 |
| Elm | github.com/dwayne/elm-7guis | 54, 270, 442, 230, 601, 914, 1609 |
| React and MobX (by Kiss) | github.com/eugenkiss/7guis-React-TypeScript-MobX | 18, 120, 120, 105, 115, about 443, 546 |
| React | github.com/andrewgreenh/7guis | 18, 76, 102, 87, 273, 254, 731 |
| React and XState (machine + page) | github.com/abhimanyuPathania/seven-guis | 28+62, 74+80, 106+212, 73+85, 188+175, 284+101, 200+122 |
| Svelte (official examples, six tasks, no Cells) | sveltejs/svelte, branch `svelte-4`, `documentation/examples/20-7guis` | 7, 24, 58, 38, 90, 129 |
| Plain JavaScript | github.com/bradwoods/7guis-html-css-js | 11, 41, 185, 96, 227, 311, 341 (and 86 to 151 of HTML each) |
| Plain JavaScript | github.com/marshallworks/7GUIs | 9, 16, 54, 34, 119, 136, 216 |
| ClojureScript, Reagent | github.com/polymeris/7guis-clojurescript | 14, 53, 72, 51, 110, 139, 268 |
| Atmos over IUP (ours, not the web) | `ia-atmos/env-iup`, `exs/7guis/` | 20, 36, 91, 51, 110, 169, 209 |

**Not found** as a public repository: 7GUIs in HipHop.js, Solid, RxJS
alone, htmx or _hyperscript. For HipHop.js there is one task in a
paper: the Timer, in SAC 2018 (section 9). The others would be written
by us.
No peer-reviewed paper that uses 7GUIs as its evaluation was found (a
weak negative: DBLP was not reachable).

### TodoMVC

The specification (tastejs/todomvc, `app-spec.md`): a new item on
Enter, a toggle of all, edit on a double click (Enter and blur save,
Escape discards), a counter, "clear completed", persistence in
`localStorage`, and routes `#/`, `#/active`, `#/completed`.

| Example | Lines of source |
|---|---|
| Elm | 434 |
| React | 312 |
| Svelte | 193 |
| jQuery | 196, and 68 of HTML |
| JavaScript (ES5, ES6) | 933, 840 |
| Reagent | 305 |
| js_of_ocaml | 477 |
| Solid, HipHop.js | not there |

Used in papers: Pendulum (REBLS 2016), and O'Connor and Wickström,
"Quickstrom", PLDI 2022, which found bugs in almost half of the
implementations.

### Sizes of other languages over WebAssembly

| System | Number | Source |
|---|---|---|
| Go | "smallest possible size being around ~2MB", about 500 kB with gzip | go.dev/wiki/WebAssembly |
| TinyGo | "The 'Hello world' example is 575 bytes"; "~10kB isn't unusual" | same |
| Rust and Yew | the `minimal` example: 158 KB, 116 KB with `wasm-opt -Os`, 99 KB with wasm-pack | yew.rs/docs/advanced-topics/optimizations |
| Flutter web | "the inherent file size of CanvasKit (around 1.5 MB)" | flutter.dev/blog |
| Lua | Wasmoon 393 kB (130 kB with gzip); Fengari 214 kB (69 kB) | github.com/ceifa/wasmoon, its own table |
| Blazor, Leptos, Kotlin/Wasm | no official number found | |
| Atmos (ours) | 237 KB and 64 KB of JavaScript (94 KB and 18 KB with gzip); 168 KB with `-Oz` | `ia-atmos/env-web`, `261126-measures.md` |

In this company the size is in the range of Rust with Yew and of Lua,
far below Go and Flutter, and above the JavaScript libraries (htmx
about 16 kB, Elm's TodoMVC 9 kB with gzip).

## 9. The papers, read (second pass, 2026-11-26)

The text of each was obtained and read, except where said. This
section corrects the ones above where they differ.

### HipHop, from 2011 to 2020

| Paper | Fact |
|---|---|
| PLASTIC 2011 (Scheme, over Hop) | The work is cut "into two parts: a reactive HipHop program and the Hop linking part". A listener written by hand feeds the machine (`(hiphop-input-and-react! M ...)`), and "When the machine is called to react is left to the user." Examples: requests in parallel with an abort button and a timeout, and an audio player. A limit it states, against Orc: "HipHop does not offer the flexibility of the Orc f > x > g operator that dynamically creates parallel executions". No evaluation. |
| SAC 2018 (three pages) | **Its one example is the Timer of 7GUIs** ("We study the timer application of the 7 GUIs project"), then with a button to suspend. The claim: the JavaScript version "has been modified in a non-local way by adding isSusp, patching existing functions, and adding another function", and in Hiphop.js "the BasicTimer code can be directly reused without modification". The binding is by hand in both directions: `stm.input("reset"); stm.react();` and `stm.addEventListener("elapsed", evt => { $("meter").value = evt.signalValue; ... })`. No evaluation. |
| Vidal's thesis, "Programmation web réactive", Université Côte d'Azur, 2018 (read in part) | Section 5.3.2: branches of a parallel are added and removed between two instants (`m.getElementById("parallelFoo").appendChild(lp)`); the branch starts at the next instant, and the change "modifie l'AST du programme ... et recompile l'ensemble du programme". Limits it states (chapter 9): no local variables ("les variables ne sont pas implémentées"), and closures in their place escape the checks. On Pendulum: "le travail connexe le plus proche de Hiphop.js". |
| PLDI 2020 | The page of the login: `<input onkeyup=~{M.react({name: this.value})}/>`, `<button ... onclick=~{M.react({login: true})}>`, `<react>~{M.time.nowval}</react>`. The stated goal is the opposite of ours: "the boundary between traditional HTML code and reactive HipHop.js code is kept clear. Both can be developed separately". Its related work does not mention Céu, Pendulum nor ReactiveML. On the data-flow family (Flapjax, Elm, React): "These constructs are not really relevant to the control-based orchestration issues we want to address." On promises: "none of the examples considered in their study required preemption as used here. We think that preemptive operations are absolutely fundamental". On `async`: "These additions do help writing basic temporal behaviors, but in a limited way." |

So the precise difference to claim (to be shown with code): in every
HipHop the program is a machine apart from the page, and the two are
joined by listeners and by reads of signals; in ours the program names
the elements and awaits their events, with nothing to join. And a new
concurrent activity is, there, a change of the program from outside
that compiles it again; here, a `spawn` in a pool.

### Pendulum (REBLS 2016)

- Elements are parameters of the program; their events are input
  signals and their properties output signals: `present
  window##onmousemove (emit sp##.textContent ...)`. The reaction is
  implicit: each event that is referred triggers a step.
- No abort and no finalizer: preemption is `trap`/`exit`.
- Static: "the element construct that is currently constrained to
  static element but that could be extended to a collection"; of
  HipHop, "It even allows the programmer to dynamically add statements,
  which is powerful but could possibly leads to runtime errors".
- TodoMVC is a benchmark of speed only (add, select and remove 100
  items; 82 ms against 153 ms of Elm and 424 ms of React of that time,
  values read from a bar chart).

Of the three (HipHop.js, Pendulum, ours), Pendulum binds the DOM as we
do, in the language, and is static; HipHop.js is dynamic from outside
and binds by hand.

### Elm (PLDI 2013), Flapjax (OOPSLA 2009), Arrowlets (DLS 2009)

| Paper | Fact |
|---|---|
| Elm 2013 | The Elm of signals, left in 2016. No signals of signals ("We avoid these issues by ruling out signals of signals"), so no dynamic graph. It cites no synchronous language. "We do not provide direct performance evaluations of Elm". |
| Flapjax | Behaviors ("like a variable—it always has a value—except that changes to its value propagate automatically") and event streams; `$E(elt, "click")`, `$B(input)`, `insertValueB`, `insertDomB`. On callbacks: "Ajax applications are agglomerations of callbacks with largely implicit control flows", and "even local reasoning about a program's data depends on global reasoning about the program's control flow". Its examples include a drag and drop (`mergeE(moveEE,dropEE).switchE()`) and a search with `calmE(1000)`. The evaluation is qualitative. |
| Arrowlets | The drag and drop as a state machine of arrows: `next` (sequence), `or` ("executes whichever of f and g is triggered first, and cancels the other"), `repeat`, `product`. Cancellation from outside by the "progress arrow" that `run()` gives (`p.cancel()`); each `EventA` registers a canceller that removes its listener. No finalizer beyond that. |

Arrowlets is, in 2009, a sequence with a parallel-or that cancels the
loser: the same shape as ours, as combinators and with no finalizers.

### Céu (TECS 2017, the author's preprint)

Its contributions: external events in a queue, internal events with a
stack, static checks of concurrency, `finalize` for resources of C,
first-class timers. `watching`, pools and organisms are not in it
(they are of Modularity 2015). No GUI example.

### The thesis of 7GUIs (Kiss 2014)

- Method: the seven tasks in each approach, compared by a subset of
  the Cognitive Dimensions of Notations (abstraction level, closeness
  of mapping, hidden dependencies, error-proneness, diffuseness,
  viscosity); an analysis by the author, no study with users.
- Compared: "Java/JavaFX on the object-oriented side, and
  Scala/ScalaFX, Scala.Rx, ReactFX and Elm on the functional side".
- Conclusion: "the toolkit in general played the most crucial role in
  our case studies ... Paradigm and language differences played a more
  minor role." A warning for our evaluation: with 7GUIs alone the
  environment may weigh more than the language.
- It asks for what we would bring: "it would be also worthwhile to
  focus or find more negative GUI programming examples to uncover or
  dismiss some potential limitations of (Siblings of) FRP for example
  with respect to a static dependency graph."

### Sequential code for interfaces, before us

| Work | Fact |
|---|---|
| Cardelli and Pike, Squeak, SIGGRAPH 1985 | "a user interface implementation language that exploits this concurrency rather than hiding it ... based on concurrent programming constructs but can be compiled into a conventional sequential language". The oldest ancestor found. |
| Syme, Petricek, Lomov, F# async, PADL 2011 | "cooperative resumption-based asynchronous tasks are a perfect match for GUI programming" (`let! me = Async.AwaitObservable wnd.LeftButtonDown`); the cancellation is a token passed implicitly, and "finally clauses are run". |
| Leijen, TyDe 2017 | "block-scoped interleaving, cancellation, and timeout's that are lacking in other major asynchronous frameworks". Not about GUIs. |
| Madsen, Lhoták, Tip, OOPSLA 2017 | Events: "lost events and event races, and results in code that is hard to understand and debug"; and "promises are complex and error-prone in their own right". |

Not obtained, so not used: Edwards 2009; Bierman et al. 2012; Okur et
al. 2014; Myers 1991 ("eliminating the spaghetti of call-backs");
Kambona et al. 2013.

## References

- L. Cardelli, R. Pike. Squeak: a Language for Communicating with
  Mice. SIGGRAPH 1985.
- D. Leijen. Structured Asynchrony with Algebraic Effects. TyDe 2017
  (read as MSR-TR-2017-21).
- M. Madsen, O. Lhoták, F. Tip. A Model for Reasoning About JavaScript
  Promises. OOPSLA 2017.
- D. Syme, T. Petricek, D. Lomov. The F# Asynchronous Programming
  Model. PADL 2011.
- C. Vidal. Programmation web réactive. PhD thesis, Université Côte
  d'Azur, 2018. HAL tel-01900619.
- S. Baxter, R. Nigam, J. Politz, S. Krishnamurthi, A. Guha. Putting in
  all the stops: execution control for JavaScript. PLDI 2018,
  pp. 30-45. DOI 10.1145/3192366.3192370.
- A. Chlipala. Ur/Web: A Simple Model for Programming the Web. POPL
  2015, pp. 153-165. DOI 10.1145/2676726.2677004.
- E. Cooper, S. Lindley, P. Wadler, J. Yallop. Links: Web Programming
  Without Tiers. FMCO 2006, LNCS, pp. 266-296.
  DOI 10.1007/978-3-540-74792-5_12.
- Y. Khoo, M. Hicks, J. Foster, V. Sazawal. Directing JavaScript with
  arrows. DLS 2009, pp. 49-58. DOI 10.1145/1640134.1640143.
- E. Kiss. Comparison of Object-Oriented and Functional Programming for
  GUI Development. Master's thesis, Leibniz Universität Hannover, 2014.
  7guis.github.io/7guis
- S. Krishnamurthi, P. Hopkins, J. McCarthy, P. Graunke, G. Pettyjohn,
  M. Felleisen. Implementation and use of the PLT Scheme Web server.
  Higher-Order and Symbolic Computation 20(4):431-460, 2007.
  DOI 10.1007/s10990-007-9008-y.
- L. O'Connor, O. Wickström. Quickstrom: Property-based acceptance
  testing with LTL specifications. PLDI 2022. arXiv:2203.11532.
- C. Queinnec. The influence of browsers on evaluators or,
  continuations to program web servers. ICFP 2000, pp. 23-33.
  DOI 10.1145/351240.351243.
- C. Queinnec. Inverting back the inversion of control or,
  continuations versus page-centric programming. SIGPLAN Notices
  38(2):57-64, 2003. DOI 10.1145/772970.772977.
- G. Radanne, J. Vouillon, V. Balat. Eliom: A Core ML Language for
  Tierless Web Programming. APLAS 2016, LNCS, pp. 377-397.
  DOI 10.1007/978-3-319-47958-3_20.
- J. Vouillon. Lwt: a cooperative thread library. ML Workshop 2008,
  pp. 3-12. DOI 10.1145/1411304.1411307.
- J. Vouillon, V. Balat. From bytecode to JavaScript: the Js_of_ocaml
  compiler. Software: Practice and Experience 44(8):951-972, 2014.
  DOI 10.1002/spe.2187.
- G. Berry, G. Gonthier. The Esterel synchronous programming language:
  design, semantics, implementation. Science of Computer Programming
  19(2):87-152, 1992. DOI 10.1016/0167-6423(92)90005-V.
- G. Berry. The Esterel v5 Language Primer, Version v5_91. 2000.
- G. Berry, M. Serrano. HipHop.js: (A)Synchronous Reactive Web
  Programming. PLDI 2020, pp. 533-545. DOI 10.1145/3385412.3385984.
- G. Berry, C. Nicolas, M. Serrano. HipHop: a synchronous reactive
  extension for Hop. PLASTIC 2011, pp. 49-56.
  DOI 10.1145/2093328.2093337.
- G. Berry, M. Serrano. Hop and HipHop: Multitier Web Orchestration.
  2014. arXiv:1312.0078. (The venue is to confirm.)
- C. Vidal, G. Berry, M. Serrano. Hiphop.js: a language to orchestrate
  web applications. SAC 2018, pp. 2193-2195. DOI 10.1145/3167132.3167440.
  (Three pages.)
- E. Czaplicki, S. Chong. Asynchronous Functional Reactive Programming
  for GUIs. PLDI 2013, pp. 411-422. DOI 10.1145/2491956.2462161.
- R. El Sibaïe, E. Chailloux. Synchronous-Reactive Web Programming.
  REBLS 2016, pp. 9-16. DOI 10.1145/3001929.3001931.
- L. Mandel, M. Pouzet. ReactiveML: a reactive extension to ML. PPDP
  2005, pp. 82-93. DOI 10.1145/1069774.1069782. (Text not read.)
- F. Sant'Anna, N. Rodriguez, R. Ierusalimschy, O. Landsiedel,
  P. Tsigas. Safe System-level Concurrency on Resource-Constrained
  Nodes. SenSys 2013. DOI 10.1145/2517351.2517360.
- F. Sant'Anna, R. Ierusalimschy, N. Rodriguez. Structured Synchronous
  Reactive Programming with Céu. Modularity 2015, pp. 29-40.
  DOI 10.1145/2724525.2724571.
- F. Sant'Anna, R. Ierusalimschy, N. Rodriguez, S. Rossetto, A. Branco.
  The Design and Implementation of the Synchronous Language Céu. ACM
  TECS 16(4), 2017. DOI 10.1145/3035544. (Read as the preprint.)
- F. Sant'Anna. Structured Synchronous Reactive Programming for Game
  Development. Case Study: On Rewriting Pingus from C++ to Céu. SBGames
  2018.
- R. Santos, G. Lima, F. Sant'Anna, N. Rodriguez. CÉU-MEDIA: Local
  Inter-Media Synchronization Using CÉU. WebMedia 2016, pp. 143-150.
  DOI 10.1145/2976796.2976856. (Text not read.)
- E. Bainomugisha, A. Lombide Carreton, T. Van Cutsem, S. Mostinckx,
  W. De Meuter. A survey on reactive programming. ACM Computing Surveys
  45(4), 2013. DOI 10.1145/2501654.2501666.
- A. Courtney, C. Elliott. Genuinely Functional User Interfaces.
  Haskell Workshop, 2001. (Pages not verified.)
- J. Edwards. Coherent reaction. OOPSLA 2009 Companion, pp. 925-932.
  DOI 10.1145/1639950.1640058.
- C. Elliott, P. Hudak. Functional reactive animation. ICFP 1997,
  pp. 263-273. DOI 10.1145/258948.258973.
- D. Harel. Statecharts: a visual formalism for complex systems.
  Science of Computer Programming 8(3):231-274, 1987.
  DOI 10.1016/0167-6423(87)90035-9.
- I. Maier, T. Rompf, M. Odersky. Deprecating the Observer Pattern.
  Technical report EPFL-REPORT-148043, 2010.
- L. Meyerovich, A. Guha, J. Baskin, G. Cooper, M. Greenberg,
  A. Bromfield, S. Krishnamurthi. Flapjax: a programming language for
  Ajax applications. OOPSLA 2009, pp. 1-20. DOI 10.1145/1640089.1640091.
- S. Parent. A Possible Future of Software Development. Slides, Adobe
  Software Technology Lab, 2007 (file dated 2008).
  stlab.cc/legacy/figures/2008_07_25_google.pdf
- G. Salvaneschi, S. Amann, S. Proksch, M. Mezini. An empirical study
  on program comprehension with reactive programming. FSE 2014,
  pp. 564-575. DOI 10.1145/2635868.2635895.
- G. Salvaneschi, S. Proksch, S. Amann, S. Nadi, M. Mezini. On the
  Positive Effect of Reactive Programming on Software Comprehension: An
  Empirical Study. IEEE TSE 43(12):1125-1143, 2017.
  DOI 10.1109/TSE.2017.2655524.
- J. Barnett et al. (eds.). State Chart XML (SCXML): State Machine
  Notation for Control Abstraction. W3C Recommendation, 2015-09-01.
- J. McCall, J. Groff, D. Gregor, K. Malawski. SE-0304: Structured
  Concurrency. Swift Evolution.
