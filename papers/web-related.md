# Related work for the paper on the web: facts and their sources

For `papers/web.md`, item `paper-web#4`. Each fact about another system
is here with the source it was read from on 2026-11-26, or is marked
**unverified**. What is not here is from memory and does not go into
the paper. A quote is the text of the page as fetched.

States of this file: sections 1 to 6 are checked; sections 7 and 8
wait for their search.

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

Waits for its search.

## 8. Task sets and sizes (7GUIs, TodoMVC, WebAssembly)

Waits for its search.

## References

- G. Berry, G. Gonthier. The Esterel synchronous programming language:
  design, semantics, implementation. Science of Computer Programming
  19(2):87-152, 1992. DOI 10.1016/0167-6423(92)90005-V.
- G. Berry. The Esterel v5 Language Primer, Version v5_91. 2000.
- G. Berry, M. Serrano. HipHop.js: (A)Synchronous Reactive Web
  Programming. PLDI 2020, pp. 533-545. DOI 10.1145/3385412.3385984.
- G. Berry, C. Nicolas, M. Serrano. HipHop: a synchronous reactive
  extension for Hop. PLASTIC 2011, pp. 49-56.
  DOI 10.1145/2093328.2093337. (Text not read.)
- G. Berry, M. Serrano. Hop and HipHop: Multitier Web Orchestration.
  2014. arXiv:1312.0078. (The venue is to confirm.)
- C. Vidal, G. Berry, M. Serrano. Hiphop.js: a language to orchestrate
  web applications. SAC 2018, pp. 2193-2195. DOI 10.1145/3167132.3167440.
  (Three pages; text not read.)
- E. Czaplicki, S. Chong. Asynchronous Functional Reactive Programming
  for GUIs. PLDI 2013, pp. 411-422. DOI 10.1145/2491956.2462161. (Text
  not read.)
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
  TECS 16(4), 2017. DOI 10.1145/3035544. (Text not read.)
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
