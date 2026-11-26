# Related work for the paper on the web: facts and their sources

For `papers/web.md`, item `paper-web#4`. Each fact about another system
is here with the source it was read from on 2026-11-26, or is marked
**unverified**. What is not here is from memory and does not go into
the paper. A quote is the text of the page as fetched.

States of this file: sections 3 to 6 are checked; sections 1, 2, 7 and
8 wait for their searches.

## 1. Synchronous languages on the web (HipHop.js, others)

Waits for its search.

## 2. The lineage (Esterel, Céu) and Elm

Waits for its search.

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
