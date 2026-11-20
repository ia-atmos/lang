References checked on the web on 2026-11-20 by the session
paper-games. 'Confirmed' means the source at the URL was read on that
day. Draft, to be reviewed by the author.

Conventions used below:

- **confirmed**: recalled data matched the source that was read.
- **corrected**: source was read and differs from what was recalled.
- **not found**: no primary source could be read (see last section).
- "Crossref" means the record `https://api.crossref.org/works/<DOI>`
  was read; publisher landing pages (ACM DL, IEEE Xplore, Wiley,
  Elsevier) answered 403 to this session and were NOT read. DBLP was
  also unreachable (bot check). Page ranges are as Crossref gives them.
- Quotes are verbatim from the text extracted from the page/PDF.

# 1. Sweeney, POPL 2006

**Status: corrected (wording).**

- Tim Sweeney. "The Next Mainstream Programming Language: A Game
  Developer's Perspective". Invited talk, POPL 2006 (33rd ACM
  SIGPLAN-SIGACT Symposium on Principles of Programming Languages),
  Charleston, SC, January 2006.
- Proceedings entry (abstract only): Tim Sweeney, "The next mainstream
  programming language", Conference Record of POPL 2006, p. 269.
  DOI 10.1145/1111037.1111061 (Crossref; note the short title there).
- POPL'06 page lists it as: "The Next Mainstream Programming Language:
  A Game Developer's Perspective. Tim Sweeney. (ppt)".
  Read: <https://www.cs.princeton.edu/~dpw/popl/06/>
- Slides (PDF copy of the ppt, 67 pages), read:
  <https://ics.uci.edu/~wscacchi/GameIndustry/Tim-POPL-2006.pdf>

Exact wording, slide titled "Performance" (page 20 of 67 in that PDF):

> When updating 10,000 objects at 60 FPS, everything is
> performance-sensitive
> But:
> – Productivity is just as important
> – Will gladly sacrifice 10% of our performance
>   for 10% higher productivity
> – We never use assembly language

So the recalled "would gladly give up 10% of performance for 10% more
productivity" is a paraphrase. Correct quote: "Will gladly sacrifice
10% of our performance for 10% higher productivity" (no subject on the
slide; bullet under "Productivity is just as important").

Also on the slides: a table giving Game Simulation 10% of CPU budget
vs. Numeric Computation 90% (slide "Three Kinds of Code"); the Pingus
paper (below) cites this as "about half the complexity ... only 10% of
the CPU budget". I read the 10%/90% row; I did not verify the "half the
complexity" figure against the slide table cell by cell.

# 2. Pingus paper, Céu papers, Atmos

## 2.1 Pingus paper — **confirmed** (title/venue), details added

- Francisco Sant'Anna (single author; UERJ). "Structured Synchronous
  Reactive Programming for Game Development — Case Study: On Rewriting
  Pingus from C++ to Céu". SBGames 2018 (XVII Brazilian Symposium on
  Computer Games and Digital Entertainment), Foz do Iguaçu, Brazil,
  Oct 29 – Nov 1, 2018. Computing Track – Full Papers.
- Two published versions:
  - SBC proceedings: "SBC – Proceedings of SBGames 2018 — ISSN:
    2179-2259", pp. 591–600 (page numbers read in the PDF footers).
    Read: <https://www.sbgames.org/sbgames2018/files/papers/ComputacaoFull/186841.pdf>
  - IEEE proceedings: "2018 17th Brazilian Symposium on Computer Games
    and Digital Entertainment (SBGames)", DOI
    10.1109/SBGAMES.2018.00036 (Crossref). Crossref page field is
    garbled ("240-24009"); probably starts at p. 240 — end page NOT
    verified (IEEE Xplore not readable).
- Correction to recollection: "and others" is wrong, the paper has one
  author. Title in the PDF uses a line break, Crossref uses " - ".
- The paper names **four** control-flow patterns (Sec. IV, verbatim):
  1. "Finite State Machines: Event occurrences lead to transitions
     between states and trigger actions comprising the behavior of a
     game entity."
  2. "Continuation Passing: The completion of a long-lasting activity
     in the game may carry a continuation, i.e., some action to execute
     next."
  3. "Dispatching Hierarchies: Entities form a dispatching hierarchy in
     which a container that receives a stimulus automatically forwards
     it to its managed children."
  4. "Lifespan Hierarchies: Entities form a lifespan hierarchy in which
     a terminating container entity automatically destroys its managed
     children."
- Abstract: "We show gains in productivity for four behaviors in Pingus
  through a qualitative analysis". Conclusion: "we rewrote about 1/4 of
  the whole codebase (9.186 from 39.362 lines of code)".
- It quotes Sweeney exactly as in section 1 above.
- The author's longer web report with the same title (not the paper)
  says instead: "We selected 9 representative game behaviors ... We
  also categorized these examples in 5 abstract C++ control-flow
  patterns". Do not mix the two (4 patterns in the paper, 5 in the web
  report). Read: <https://fsantanna.github.io/pingus/>

## 2.2 Céu papers (all via Crossref unless noted)

- **confirmed** — F. Sant'Anna, N. Rodriguez, R. Ierusalimschy,
  O. Landsiedel, P. Tsigas. "Safe System-level Concurrency on
  Resource-Constrained Nodes". SenSys 2013 (11th ACM Conf. on Embedded
  Networked Sensor Systems), pp. 1–14. DOI 10.1145/2517351.2517360.
  (The author's page lists the SenSys'13 paper under another title,
  "Safe Concurrent Abstractions for Wireless Sensor Networks"; the
  Crossref/ACM title is the one above.)
- **confirmed** — F. Sant'Anna, R. Ierusalimschy, N. Rodriguez.
  "Structured Synchronous Reactive Programming with Céu". Modularity
  2015 (14th Int. Conf. on Modularity), pp. 29–40.
  DOI 10.1145/2724525.2724571. This is the paper on dynamic Céu: the
  author's page describes it as 'main paper about the "dynamic Céu":
  organisms, pointers and references'. No separate paper on
  organisms/pools was found; "Dynamic Organisms in Céu" is a 2014 talk
  (Strange Loop / Emerging Languages Camp / Future of Programming
  Workshop), per the same page.
- **confirmed** — R. C. M. Santos, G. F. Lima, F. Sant'Anna,
  R. Ierusalimschy, E. H. Haeusler. "A Memory-Bounded, Deterministic
  and Terminating Semantics for the Synchronous Programming Language
  Céu". LCTES 2018 (19th ACM SIGPLAN/SIGBED), pp. 1–18.
  DOI 10.1145/3211332.3211334.
- **confirmed** (addition) — F. Sant'Anna, R. Ierusalimschy,
  N. Rodriguez, S. Rossetto, A. Branco. "The Design and Implementation
  of the Synchronous Language Céu". ACM Trans. Embedded Computing
  Systems 16(4), pp. 1–26, 2017. DOI 10.1145/3035544. (Author's page:
  "contrasting and comparing with Esterel".)
- **confirmed** (addition) — R. C. M. Santos, G. F. Lima,
  F. Sant'Anna, N. Rodriguez. "Céu-Media" [full title on author's
  page: "Céu-Media: Local Inter-Media Synchronization Using Céu"].
  WebMedia 2016, pp. 143–150. DOI 10.1145/2976796.2976856.
- Listed on the author's page only (no DOI checked): REBLS 2014
  "Structured Reactive Programming with Céu" (superseded by
  Modularity'15); REBLS 2015 "Reactive Traversal of Recursive Data
  Types"; PhD thesis "Safe System-level Concurrency on
  Resource-Constrained Nodes with Céu" (PUC-Rio, 2013; Crossref DOI
  10.17771/pucrio.acad.28739).
- Read: <https://ceu-lang.org/chico/> and Crossref records.

## 2.3 Atmos — **not found** (no publication)

- No academic publication on Atmos was found (Crossref, arXiv author
  query, web search, author's page). Only the repositories exist:
  - <https://github.com/atmos-lang/atmos> — README read (raw): "The
    Programming Language Atmos — Structured Event-Driven Concurrency";
    "Atmos is a programming language that reconciles Structured
    Concurrency and Event-Driven Programming"; "Atmos is inspired by
    synchronous programming languages like Céu and Esterel. Atmos
    compiles to Lua and relies on lua-atmos for its concurrency
    runtime." Stable branch v0.8 on the day read.
  - <https://github.com/lua-atmos/atmos> — README read (raw): "Atmos
    is a programming library for Lua that reconciles Structured
    Concurrency and Event-Driven Programming".
  - Announcement (ceu-lang list), read:
    <https://groups.google.com/g/ceu-lang/c/oqxaFaxWINw>
- Cite as software (URL + version), not as a paper. "Successor of
  Céu" is not a phrase I found in a source; the README says "inspired
  by".

# 3. Synchronous languages and structured concurrency

## 3.1 Synchronous languages (Crossref) — all **confirmed**

- G. Berry, G. Gonthier. "The Esterel synchronous programming
  language: design, semantics, implementation". Science of Computer
  Programming 19(2):87–152, 1992. DOI 10.1016/0167-6423(92)90005-V.
- N. Halbwachs, P. Caspi, P. Raymond, D. Pilaud. "The synchronous data
  flow programming language LUSTRE". Proceedings of the IEEE
  79(9):1305–1320, 1991. DOI 10.1109/5.97300.
- P. Le Guernic, T. Gautier, M. Le Borgne, C. Le Maire. "Programming
  real-time applications with SIGNAL". Proceedings of the IEEE
  79(9):1321–1336, 1991. DOI 10.1109/5.97301.
- A. Benveniste, P. Caspi, S. A. Edwards, N. Halbwachs, P. Le Guernic,
  R. de Simone. "The synchronous languages 12 years later".
  Proceedings of the IEEE 91(1):64–83, 2003.
  DOI 10.1109/JPROC.2002.805826.
- (addition) F. Gretz, F.-J. Grosch. "Blech, Imperative Synchronous
  Programming!". FDL 2018 (Forum on Specification & Design Languages),
  pp. 5–16. DOI 10.1109/FDL.2018.8524036. (Extended version: LNEE,
  Springer 2019, pp. 161–186, DOI 10.1007/978-3-030-31585-6_9.)

## 3.2 Structured concurrency

- **confirmed** — Nathaniel J. Smith. "Notes on structured
  concurrency, or: Go statement considered harmful". Blog post, dated
  "Wed 25 April 2018". Introduces Trio's nursery: "the nursery block
  doesn't exit until all the tasks inside ...".
  Read: <https://vorpus.org/blog/notes-on-structured-concurrency-or-go-statement-considered-harmful/>
- **confirmed** — Martin Sústrik. "Structured Concurrency". 250bpm
  blog, "February 7th, 2016". Quote: "The rest of the article will
  explore the question of cancellation and will propose a model of
  'structured concurrency' to deal with it." and "If block B is nested
  in block A, the lifetime of B cannot exceed the lifetime of A."
  Read: <https://sustrik.github.io/250bpm/blog:71> (the old address
  250bpm.com/blog:71 now returns 404; 250bpm.com is a Substack).
  His library: libdill, "Structured Concurrency for C"
  (<https://sustrik.github.io/libdill/structured-concurrency.html>,
  seen in search results only, not read).
- **confirmed** — Trio (Python). Reference docs have sections "The
  nursery API" and "Cancel scopes and nurseries".
  Read: <https://trio.readthedocs.io/en/stable/reference-core.html>
- **confirmed** — Kotlin coroutines. Official docs: "Kotlin coroutines
  rely on a principle called structured concurrency ... A parent
  coroutine waits for its children to complete before it finishes. If
  the parent coroutine fails or gets canceled, all its child
  coroutines are recursively canceled too."
  Read: <https://kotlinlang.org/docs/coroutines-basics.html>
- **confirmed** — Swift SE-0304 "Structured concurrency". Authors:
  John McCall, Joe Groff, Doug Gregor, Konrad Malawski. Status:
  "Implemented (Swift 5.5)". Read:
  <https://github.com/swiftlang/swift-evolution/blob/main/proposals/0304-structured-concurrency.md>
- **corrected/updated** — Java. The current JEP is **JEP 543:
  Structured Concurrency** (authors Alan Bateman, Viktor Klang, Ron
  Pressler), Status "Integrated", Release 28; "We here propose to
  finalize Structured Concurrency in JDK 28, without further change."
  History per that JEP: incubator JEP 428 (JDK 19), JEP 437 (JDK 20);
  preview JEP 453 (JDK 21), 462 (22), 480 (23), 499 (24), 505 (25,
  static factory methods replace constructors of
  `StructuredTaskScope`), 525 (26), 533 (27). So after seven previews
  the API is final in JDK 28 (status "Integrated", i.e. JDK 28 itself
  may not be released yet — check before submission).
  Read: <https://openjdk.org/jeps/543>, <https://openjdk.org/jeps/0>

# 4. Coroutines and async/await in game engines

General caution: "lacks X" claims are mostly arguments from silence
(the documentation offers no such construct). Where a doc passage
supports the claim it is quoted; otherwise it is marked UNSUPPORTED.

## 4.1 Unity coroutines — **confirmed**

- Manual, "Coroutines" (Unity 6000.0 docs): "Coroutines are methods
  with an IEnumerator return type and a yield return statement
  included somewhere in the body." Started with `StartCoroutine`.
  Read: <https://docs.unity3d.com/6000.0/Documentation/Manual/coroutines.html>
- Abortion: "To stop a coroutine, use StopCoroutine and
  StopAllCoroutines. A coroutine also stops if: The value of
  GameObject.activeSelf becomes false for the GameObject the script is
  attached to. The MonoBehaviour script is destroyed with a call to
  Destroy. Note: Disabling the MonoBehaviour script by setting enabled
  to false doesn't stop coroutines." Same text in
  <https://docs.unity3d.com/6000.0/Documentation/ScriptReference/MonoBehaviour.StopCoroutine.html>
  ("Stops the first coroutine named methodName, or the coroutine
  stored in routine running on this behaviour.")
- So abortion is by explicit handle/name or tied to the owner
  object, not to a lexical block.
- Leak warning (same manual page): "Coroutines retain references to
  their owner and captured variables. Ensure they end or are stopped
  with MonoBehaviour.StopCoroutine to avoid leaks."
- Finalization: UNSUPPORTED. Neither page says what happens to
  `finally` blocks or other cleanup when a coroutine is stopped; the
  docs are silent. The common claim that `finally` does not run on
  `StopCoroutine` was not found in official docs; do not state it
  without testing or another source.
- Parallel composition with join: UNSUPPORTED by a passage; the pages
  read show no par/join construct (argument from silence).

## 4.2 Unity `Awaitable` — **confirmed**

- `UnityEngine.Awaitable` ("public class Awaitable : IEnumerator").
  The API page exists in the 2023.1 docs and returns 404 in the 2022.3
  docs, which supports "introduced in Unity 2023.1" (no release note
  was read).
  Read: <https://docs.unity3d.com/6000.0/Documentation/ScriptReference/Awaitable.html>,
  <https://docs.unity3d.com/2023.1/Documentation/ScriptReference/Awaitable.html>
  (HTTP 200), same path under 2022.3 (HTTP 404).
- "Awaitable is usually a preferable alternative to .NET Task for
  asynchronous code in Unity. ... instances of the Awaitable class are
  pooled and therefore not safe to await multiple times in the same
  method."
- Cancellation: method `Cancel` — "Cancels the awaitable. If the
  awaitable is being awaited, the awaiter receives a
  System.OperationCanceledException." Static methods listed:
  BackgroundThreadAsync, EndOfFrameAsync, FixedUpdateAsync,
  FromAsyncOperation, MainThreadAsync, NextFrameAsync,
  WaitForSecondsAsync — i.e. no WhenAll/WhenAny combinator in the
  class (from the method list read).
- "Unity doesn't automatically stop code running in the background
  when you exit Play mode. To cancel a background operation on exiting
  Play mode, use Application.exitCancellationToken". Read:
  <https://docs.unity3d.com/6000.0/Documentation/Manual/async-awaitable-continuations.html>

## 4.3 UniTask — **confirmed**

- Cysharp/UniTask, <https://github.com/Cysharp/UniTask> (README read
  raw). "Struct based UniTask and custom AsyncMethodBuilder to achieve
  zero allocation".
- Offers join combinators: "utilities like UniTask.WhenAll,
  UniTask.WhenAny, UniTask.WhenEach".
- Cancellation is cooperative, by token passing: "For propagate
  Cancellation, all async method recommend to accept CancellationToken
  cancellationToken at last argument, and pass CancellationToken from
  root to end."; "When cancellation is detected, all methods throw
  OperationCanceledException and propagate upstream."
  `GetCancellationTokenOnDestroy` ties a token to a GameObject.
- Supported "lack": cancellation is not lexical/automatic; it must be
  threaded manually through every call (quote above).

## 4.4 Godot GDScript — **confirmed**

- 4.x, "Awaiting signals or coroutines": "The await keyword can be
  used to create coroutines which wait until a signal is emitted
  before continuing execution. Using the await keyword with a signal
  or a call to a function that is also a coroutine will immediately
  return the control to the caller."
  Read: <https://docs.godotengine.org/en/stable/tutorials/scripting/gdscript/gdscript_basics.html>
- No handle to a suspended coroutine in 4.x: "Unlike yield in previous
  Godot versions, you cannot obtain the function state object. This is
  done to ensure type safety."
- Cancellation: the section documents no way to cancel/abort a pending
  `await` (no occurrence of cancel/abort as an API in the page).
  "Cannot be cancelled" is therefore an argument from silence plus the
  quote above (no state object to act on) — phrase it that way.
- 3.x, "Coroutines with yield": "GDScript offers support for
  coroutines via the yield built-in function. Calling yield() will
  immediately return from the current function, with the current
  frozen state of the same function as the return value. Calling
  resume() on this resulting object will continue execution".
  Read: <https://docs.godotengine.org/en/3.6/tutorials/scripting/gdscript/gdscript_basics.html>
- Parallel/join, finalization: no construct in either section
  (silence).

## 4.5 Unreal Engine — **partly not verified**

- Epic's documentation site is rendered by JavaScript and its content
  could not be read by this session (old docs.unrealengine.com: 403).
  From a search-engine snippet only (NOT read at source): Blueprint
  node Delay / `UKismetSystemLibrary::Delay` — "Perform a latent
  action with a delay (specified in seconds). Calling again while it
  is counting down will be ignored."; Retriggerable Delay — "Calling
  again while it is counting down will reset the countdown to
  Duration." URLs returned by the search:
  <https://dev.epicgames.com/documentation/en-us/unreal-engine/BlueprintAPI/Utilities/FlowControl/Delay>,
  <https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/Engine/UKismetSystemLibrary/Delay>
- Whether Unreal itself has first-party C++ coroutine support: NOT
  verified either way (cannot prove absence from docs I could not
  read).
- **confirmed** — UE5Coro is a third-party plugin:
  <https://github.com/landelare/ue5coro> (README read raw): "UE5Coro
  implements C++20 coroutine support for Unreal Engine 5 with a focus
  on gameplay logic"; "Latent coroutines will automatically track
  their target UObject, and clean up early if needed." The README now
  carries an archival note ("UE5Coro is complete, with no known bugs
  at the time of archival").

## 4.6 Roblox `task` library — **confirmed**

- Read: <https://create.roblox.com/docs/reference/engine/libraries/task>
  (and its `.md` version).
- `task.spawn(functionOrThread, ...)` returns a thread; `task.wait`:
  "Yields the current thread until the given duration (in seconds) has
  elapsed, then resumes the thread on the next Heartbeat step."
- Abortion exists by handle: `task.cancel(thread)` — "Cancels a thread
  and closes it, preventing it from being resumed manually or by the
  engine's scheduler." Limitation, verbatim: "Note that threads may be
  in a state where it is impossible to cancel them. For example, the
  currently executing thread and threads that have resumed another
  coroutine may not be cancelled. If this is the case, an error will
  be generated."
- Join / finalization on cancel: not documented on that page
  (silence).

## 4.7 LÖVE and Lua libraries

- LÖVE itself — **partly verified**. The official wiki (love2d.org)
  answered 403 (bot check) and was not read. The community API table
  `love2d-community/love-api` (`love_api.lua`, read raw) lists the
  modules audio, data, event, filesystem, font, graphics, image,
  joystick, keyboard, math, mouse, physics, sound, system, thread,
  timer, touch, video, window — no coroutine/task module. LÖVE
  programs use plain Lua coroutines or libraries.
- hump.timer — **confirmed**: `Timer.script(func)`: "Execute a
  function that can be paused without causing the rest of the program
  to be suspended. func will receive a function - wait - to do
  interrupt the script (but not the whole program) as only argument."
  Supported "lack": `Timer.after/every/during/tween` are documented
  with "Returns: The timer handle. See also Timer.cancel()", while
  `Timer.script` documents no return value, so no documented way to
  cancel one script (only `Timer.clear()` of the whole instance).
  Read: <https://hump.readthedocs.io/en/latest/timer.html>
- knife — **corrected**: the coroutine module is `knife.convoke`
  ("Flatten async code with coroutines"), not knife.timer
  ("Create timers and tweens") nor knife.chain ("Flatten async code
  with chained functions"). Read:
  <https://github.com/airstruck/knife> (readme.md, readme/convoke.md)
- flux — **corrected**: "A fast, lightweight tweening library for
  Lua"; tweens with chained callbacks (`oncomplete`, `after`); it
  offers NO coroutine scripts. Read: <https://github.com/rxi/flux>
- coil (addition) — **confirmed**: "A tiny cooperative threading
  module for Lua. Coil is based around coroutines"; `coil.add(fn)`,
  `coil.wait(...)`; abortion by handle: "A task can be stopped and
  removed at any point by calling its :stop() method"; task groups
  that are updated/paused together. Read: <https://github.com/rxi/coil>

## 4.8 Defold — **confirmed**

- The Lua manual has a "Coroutines" section: "Functions execute from
  beginning to end and there is no way to stop them midway through.
  Coroutines allow you to do that ... When a coroutine yields it
  returns control back to the caller but remembers its execution point
  so it can continue from there later on." The example uses plain
  `coroutine.create` / `coroutine.resume` / `coroutine.yield` driven
  from `update()`; no engine-level scheduler, join or cancel is shown.
  Read: <https://defold.com/manuals/lua/>

## 4.9 KorGE — **confirmed**

- "Korge integrates tweens and easings, and it is fully integrated
  with coroutines for your coding pleasure."; `suspend fun
  View?.tween(...)`; an "animator" with `sequence`/`parallel` blocks;
  "cancel(), cancels the animation not running the rest of the
  elements and keeping the properties as they are right now."
  Read: <https://docs.korge.org/views/animation/>,
  <https://docs.korge.org/io/async/> (`delay`, `withTimeout`).
- Being Kotlin coroutines, 3.2 (Kotlin structured concurrency)
  applies; nothing KorGE-specific about scopes was read.

## 4.10 Naughty Dog state scripts

- **confirmed** — Jason Gregory (Naughty Dog). "State-Based Scripting
  in Uncharted 2: Among Thieves". Game Developers Conference 2009.
  Slides read: <https://www.gameenginebook.com/resources/gdc09-statescripting-uncharted2.pdf>
  (footers say "Game Developers Conference 2009 — Monday, April 6,
  2009", which is after the conference; treat as export date. No GDC
  Vault entry was found.)
  Relevant slides, verbatim:
  - "Tracks: (on ...) blocks contain one or more tracks. A track is a
    bit like a thread or fiber. Tracks can be put to sleep — Wait for
    duration ... Wait for an action to be done ... Tracks can be
    synchronized via signals" (slide 29).
  - "Changing States: Transitions to other states via (go
    "state-name") command. State transitions cause current state's
    (on ...) blocks to be aborted. Use (on (exit) ...) block for
    clean-up" (slide 27).
  Note for the paper: this system DOES have parallel tracks, abortion
  on state change, and an exit block for cleanup; it is closer to the
  synchronous style than the engine coroutines above. Do not list it
  as lacking abortion/finalization.
- **confirmed** — Jason Gregory. "Game Engine Architecture, Third
  Edition". CRC Press / A K Peters, 2018. DOI 10.1201/9781315267845
  (Crossref; contents not read). Crossref also lists a 2026 record
  with the same title (DOI 10.1201/9781003371519), presumably a newer
  edition — check which one to cite.

# 5. FRP and other work on languages for game logic

- **confirmed** — Evan Czaplicki, Stephen Chong. "Asynchronous
  Functional Reactive Programming for GUIs". PLDI 2013, pp. 411–422.
  DOI 10.1145/2491956.2462161.
- **confirmed** — Evan Czaplicki. "A Farewell to FRP" (subtitle
  "Making signals unnecessary with The Elm Architecture"). elm-lang.org
  news, 2016-05-10 (Elm 0.17). Quote: "Elm 0.17 is out today, and it
  introduces *subscriptions*"; "all the toughest concepts in Elm
  (signals, addresses, and ports) could collapse into ...". Read (page
  source): <https://github.com/elm/elm-lang.org/blob/master/pages/news/farewell-to-frp.elm>;
  public address <https://elm-lang.org/news/farewell-to-frp>.
- **confirmed** — Antony Courtney, Henrik Nilsson, John Peterson. "The
  Yampa Arcade". Haskell Workshop 2003 (Proc. 2003 ACM SIGPLAN
  Workshop on Haskell), pp. 7–18. DOI 10.1145/871895.871897.
- **confirmed** — Conal Elliott, Paul Hudak. "Functional Reactive
  Animation". ICFP 1997, pp. 263–273. DOI 10.1145/258948.258973.
- **confirmed** — Casanova:
  - G. Maggiore, A. Spanò, R. Orsini, M. Bugliesi, M. Abbadi,
    E. Steffinlongo. "A formal specification for Casanova, a language
    for computer games". EICS 2012, pp. 287–292.
    DOI 10.1145/2305484.2305533.
  - G. Maggiore, A. Spanò, R. Orsini, G. Costantini, M. Bugliesi,
    M. Abbadi. "Designing Casanova: A Language for Games". LNCS,
    Springer 2012, pp. 320–332. DOI 10.1007/978-3-642-31866-5_27
    (conference name not in the Crossref record).
  - M. Abbadi, F. Di Giacomo, A. Cortesi, P. Spronck, G. Costantini,
    G. Maggiore. "Casanova: A Simple, High-Performance Language for
    Game Development". LNCS, Springer 2015, pp. 123–134.
    DOI 10.1007/978-3-319-19126-3_11 (conference name not in record).
- **corrected** (title) — SGL: Walker White, Alan Demers, Christoph
  Koch, Johannes Gehrke, Rajmohan Rajagopalan. "Scaling Games to Epic
  Proportions". SIGMOD 2007, pp. 31–42. DOI 10.1145/1247480.1247486.
- **confirmed** (partly) — Robert Nystrom. "Game Programming
  Patterns". Web edition, "© 2009-2021 Robert Nystrom". Chapters read:
  "Update Method" and "Game Loop" (both under Sequencing Patterns),
  "Component" (Decoupling Patterns):
  <https://gameprogrammingpatterns.com/update-method.html>,
  <https://gameprogrammingpatterns.com/game-loop.html>,
  <https://gameprogrammingpatterns.com/component.html>.
  The "State" chapter page answered with a bot check and was not
  read. Print data "Genever Benning, 2014" is taken from the Pingus
  paper's reference [14], not from the publisher.
- **confirmed** — Behavior trees: Michele Colledanchise, Petter Ögren.
  "Behavior Trees in Robotics and AI: An Introduction". CRC Press,
  2018. DOI 10.1201/9780429489105 (Crossref gives the title without
  the subtitle).
- **confirmed** — Coroutines (general, not game specific): Ana Lúcia
  de Moura, Roberto Ierusalimschy. "Revisiting Coroutines". ACM TOPLAS
  31(2), pp. 1–31, 2009. DOI 10.1145/1462166.1462167.
- Céu-based game work: only the Pingus paper (2.1). Its related-work
  section contrasts FRP and SSRP: "We believe that FRP is more
  suitable for data-intensive applications, while SSRP, for
  control-intensive applications."

# 6. Code scattering metrics

- **confirmed** — G. Kiczales, J. Lamping, A. Mendhekar, C. Maeda,
  C. Lopes, J.-M. Loingtier, J. Irwin. "Aspect-Oriented Programming".
  ECOOP 1997, LNCS (vol. 1241 not in the Crossref record), pp.
  220–242. DOI 10.1007/BFb0053381.
- **confirmed** — M. Eaddy, T. Zimmermann, K. D. Sherwood, V. Garg,
  G. C. Murphy, N. Nagappan, A. V. Aho. "Do Crosscutting Concerns
  Cause Defects?". IEEE Trans. Software Engineering 34(4):497–515,
  2008. DOI 10.1109/TSE.2008.36. (That it defines "degree of
  scattering" was NOT checked in the text; paper not readable.)
- **confirmed** (exact title/venue supplied) — Cláudio Sant'Anna,
  Alessandro Garcia, Christina Chavez, Carlos Lucena, Arndt von Staa.
  "On the Reuse and Maintenance of Aspect-Oriented Software: An
  Assessment Framework". Anais do XVII Simpósio Brasileiro de
  Engenharia de Software (SBES 2003), pp. 15–30.
  DOI 10.5753/sbes.2003.23850. (That it defines the concern diffusion
  metrics was NOT checked in the text. Note: this Sant'Anna is Cláudio,
  not Francisco.)

# 7. Venues

## 7.1 The Art, Science, and Engineering of Programming

All from the official site, read:
<https://programming-journal.org/>, `/cfp/`, `/timeline/`,
`/submission/`.

- Scope: "accepts papers that advance knowledge of programming. Almost
  anything about programming is in scope, but in each case there
  should be a clear relevance to the act and experience of
  programming." Perspectives: Art, Science (Theoretical), Science
  (Empirical), Engineering. "Art: knowledge and technical skills
  acquired through practice and personal experiences. Examples include
  libraries, frameworks, languages, APIs, programming models and
  styles, programming pearls, and essays about programming."
  "Engineering: ... Examples include measurements of artifacts'
  properties, development processes and tools, and quality assurance
  methods." The CFP does not use the words "case study" or
  "experience report".
- "Papers submitted as 'The Art' should include a very solid
  contextualization of the work, and, when applicable, they should
  include the artifacts themselves."
- Length: "The main part of the paper should not exceed 22 pages (in
  the provided style of the journal), but there is no limit for
  bibliography and appendices."
- Cycle per volume (AoE): Issue 1 — submission October 1, first
  notification December 1, revised January 1, final January 15,
  publication February 15. Issue 2 — submission February 1, first
  notification April 1, publication June 15. Issue 3 — submission
  June 1, first notification August 1, publication October 15.
- The site banner still reads "Upcoming Submission Deadline: October
  1, 2026" (already past on the day read); latest volume listed is
  Volume 11, Issue 1. By the regular cycle the next deadlines would be
  February 1, 2027 and June 1, 2027 — inferred from the cycle, not
  announced on the page.
- Diamond open access, no author fees; overlay on arXiv.
- Metrics — UNOFFICIAL, NOT VERIFIED (Scopus and Scimago answered 403;
  values below come from third-party pages seen only as search
  snippets): CiteScore 2.5, SJR 0.261 for 2022 (TU Dresden research
  portal); "2024 CiteScore 1.7, Q4, SJR 0.223" (slogix.in);
  "best quartile Q4" (researchbite.com). ISSN 2473-7321.

## 7.2 Software: Practice and Experience (Wiley)

- Official Wiley pages answered 403; scope NOT read at the official
  source. UNOFFICIAL (search snippets of researchr.org and
  myhuiban.com, which reproduce journal text): aimed at practitioners
  sharing experience with designing, implementing and evaluating
  software techniques and tools; accepts experience reports / case
  studies ("detailed accounts of completed software projects").
  ISSN 0038-0644 (print), 1097-024X (online) (editage.com snippet).
- Deadlines: none found (continuous submission is usual for this
  journal, not verified).
- Metrics — UNOFFICIAL, NOT VERIFIED: SJR 0.688, Q2, H-index 84
  (search snippet of a scimagojr.com ranking page, year stated by the
  search summary as 2025; page itself not read); CiteScore 4.8, year
  unstated (researcher.life). No CiteScore percentile found.

# Could not verify

- DBLP records for any item (site behind a bot check); ACM DL, IEEE
  Xplore, Elsevier, Springer and Wiley landing pages (403). All
  DOI-based data above therefore rests on Crossref metadata only.
- End page of the IEEE version of the Pingus paper (Crossref page
  field garbled, "240-24009").
- Any academic publication on Atmos (none found); the phrase
  "successor of Céu" in a source.
- A separate Céu paper on organisms/pools other than Modularity 2015.
- Unity: behaviour of `finally`/cleanup when a coroutine is stopped
  (official docs silent); a release note stating that `Awaitable`
  arrived in 2023.1 (only inferred from 2023.1 = 200, 2022.3 = 404).
- Godot: an explicit statement that an `await` cannot be cancelled
  (docs silent; only "you cannot obtain the function state object").
- Unreal: all official documentation (JavaScript-rendered; only search
  snippets); whether the engine has first-party C++ coroutine support;
  latent-action cancellation semantics.
- LÖVE official wiki (403); the "no coroutine API" claim rests on the
  community `love-api` module table.
- Nystrom, "State" chapter page and the print edition's publisher
  data (ISBN, publisher) from a primary source.
- Gregory's GDC 2009 talk date and a GDC Vault entry; contents of
  "Game Engine Architecture" on state scripts (book not read).
- "Coroutines in Lua" (de Moura, Rodriguez, Ierusalimschy, J.UCS
  2004): Crossref lookup failed; not confirmed.
- Any Esterel-based game paper; any canonical academic ECS reference;
  any paper specifically on coroutines for game scripting.
- Eaddy et al. and Sant'Anna et al. 2003: that the texts define
  "degree of scattering" / "concern diffusion" metrics (bibliographic
  data confirmed, contents not read).
- Software: Practice and Experience: official scope text, deadlines,
  CiteScore percentile, Scimago quartile from the official pages.
  ‹Programming›: Scopus/Scimago figures from official pages.
- Sweeney: the "half the complexity" figure attributed to him by the
  Pingus paper (only the 10% / 90% CPU row was read on the slides).
