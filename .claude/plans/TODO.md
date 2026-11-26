# TODO

Everything that is pending in the session that maintains the compiler
and coordinates the other sessions: asked and not decided, reported and
not acknowledged, or known and not done. Keep it current: remove an item
when it is settled, add one whenever something is left open.

The numbers are the ones used in the conversation with the author, and
do not change: an item is `#N` (see `ISSUES.md` for every number and its
state, and "Standing rules" for the convention). Numbers that are
missing here were settled or moved.

Last update: 2026-11-24. Owned by the session `atmos` since 2026-11-24
(`#119`, handoff `261124-atmos.md`); section A below predates the moves
recorded in `ISSUES.md`, which is the reference for what is open.

## A. Decisions waiting for an answer

Each has the option recommended by this session.

### The language

| # | Decision | Recommended |
|---|----------|-------------|
| 1 | An `await` over a pool with a condition ("a pingu enters my zone") | Do it. Design it from the cases of SNKRX and Flare, which now exist |
| 2 | Phases inside a tick, instead of relying on the order of creation | Not in the language: a convention with two events, as the ports did |
| 3 | 32-bit reals. **Decided by the author on 2026-11-16** (in the session `sdl-tux`, answering "1" to "the compiler gets 32-bit reals"), so that the enemies and blocks of SuperTux are plain Atmos. The form is still open: a build option where every real is a C `float`, or an explicit type `float32`. What Tux needs: each operation between two reals rounds to `float`; a literal `0.1` converts as `0.1f`; integers do not change; `math` uses the `f` functions; `%.9g` prints a float exactly. Its test: `vy = vy + (10*100)*dt` with `dt = 1.0/(1000.0/15.0)` reads 15.000001, 30.0000019, 45.0000038 | The build option (`ATMOS_REAL=float`): one rule for the whole program, no second type in the language, and the runtime only changes the C type of a real. To ask the author, then do |
| 4 | An `emit` that names its receiver, instead of `emit @2` | Not to change: pass the receiver as an argument, and document it |
| 6 | The event of termination of transparent blocks | Remove it. It changes the semantics, and two golden tests must be adapted |
| 33 | Time emitted to a pool or a task (slow motion in SNKRX) | Not now: a global factor and `:tick` solve it there |
| 63 | Two limits of the check of spawns (item 36, done): a `pin t = if c { spawn T() }` that is the last statement of a loop body dies at each step and is not flagged; and the check is lexical, so a task that ends in its start through an await that is satisfied at once is flagged (4 of the tests were so) | Extend the check to a `pin` of a task that ends its block. Keep it lexical. On 2026-11-16 the author approved narrowing the check to a spawn that is the very last statement of its block (`c33e02b`, by the session `spawn-check`): a spawn followed by another spawn is no longer flagged, since the second may awake the first. He also decided that the main chunk stays a block of the rule. The session `spawn-check` is done and archived, at his request. What is left of this item is the first limit (a `pin` of a task that ends a loop body) |
| 69 | Make the six exit names keywords of the lexer: `break`, `skip`, `return`, `escape`, `throw`, `abort`, as `until` and `while` already are. Today they are identifiers that the parser and the resolver treat apart, with two exceptions added by hand (accepted as the key of a constructor, rejected in a declaration). Proposed by the session `skip`, which the author asked to pass it on (2026-11-16) | Do it, after the migration of `pin` settles: small (lexer, the parser where it tests `EXITS`, the expected output of the `lex` tests and of `dcl-err-reserved-*`), and it removes the two exceptions. The AST and the coder do not change |
| 65 | `e@1.d` and `t@0.x` do not compile: `1.d` is read as a number (`invalid number`), so it must be written `(t@0).x` or `t@(0).x`. Met by `env-iup`, by Flare and Lessmilk in their reports, by this session in two tests, and by `pico-1oom` (2026-11-17), which indexes vectors of tables everywhere. Listed in the ambiguities of the manual since 2026-11-17 | A dot belongs to a number only when a digit follows it, so `t@0.x` is `(t@0).x`. Small (the lexer); no program that compiles today changes meaning |
| 45 | A snapshot of the tasks by the runtime, to save and restore a program. Possible in this backend (a task is a record, not a C stack); native values need hooks from each binding | To talk about. Doom is the use case |
| 64 | An architecture for programs in Atmos, proposed by the author on 2026-11-14: one global snapshot (a table) from which everything is spawned; a fixed set of command events (`:cmd.*`) as the only way to change it; so a run can be recorded, resumed, and replayed in any direction and speed from the first snapshot, the commands, and periodic snapshots; later, distributed over a blockchain such as Freechains. It is the conclusion of the saved game of Doom (state that persists is data, and tasks are a function of it) made into a rule | In discussion, in a session of its own (`paper-snapshots`, formerly `snapshots`), where he asked whether it makes sense and whether the pattern has a name. Related to item 45 |
| 46 | The vocabulary of the events of an environment. **Settled by the author on 2026-11-17, in the session `envs`**: one vocabulary for all, that of pico: `:quit`; `:key.dn`/`:key.up` with `key` (name) and `code` (number); `:mouse.motion`, `:mouse.button.dn`/`.up` with `x`, `y`, `button`; `:frame [n, us]` once per frame for the logic, then `:draw`; events of a handle carry `h`. `raylib` renames its binding and games, Lessmilk follows. Left with `envs`: the order inside a frame | Done here; followed by `envs` |
| 60 | An `emit` to one task (`emit @t (e)`) whose target raises an exception: the exception goes to the task that emitted, and the task that holds the target never sees it. With a broadcast of the same event it goes to the holder. The reference does the same. Found when an environment emitted to one task from C; for that case there is now `atm_emit_to`, which delivers the exception to the holder | To the holder in both, as in a broadcast: where an exception goes should not depend on how the event was emitted. It departs from the reference |
| 58 | `test` blocks to test a game (asked by the author in the session `env-pico`). Today a run with no screen is driven from outside, by environment variables that each binding reads in its `main`, with other names in each | A convention of the bindings, not of the language: one call that puts the environment in the test mode (fixed frames, no real time, no sound), and the rest is plain Atmos in the `test` block (`await(10, :frame)`, an `emit` of an input event, an assertion, a screenshot by a function; the run ends with the block). Polled input needs a function of the binding to set it. To try first in one environment |
| 59 | How a program selects its environment. The author asked (2026-11-14, in the session of pico) for a compiler of its own per environment (`atmos-pico`, `atmos-iup`), made from this compiler, with `include` still working, and with the examples and documents using the dedicated one. Done here: `make env` builds one, and the option `-e` does the same by hand. Open: it still needs the directory of this compiler, a C compiler, and the development files of the library; a download that needs none of them is another, larger thing | To confirm the mechanism. Each environment then builds its `atmos-<env>` with it and uses it in its examples |

### Projects and repositories

| # | Decision | Recommended |
|---|----------|-------------|
| 37 | Doom: how far to go after the flow of the game (the status bar, the messages, the menus, the automap). The other three questions of the plan were answered on 2026-11-13 with the recommended options (Chocolate Doom, a copy with a first untouched commit, the shareware data fetched by a script), and the session may start | To decide when the wipe, the intermission, the finale, and the flow are done |
| 42 | The environment for IUP: the plan is sent (`261113-env-iup.md`). It waits for the author on three items: a thin and generic binding, the seven tasks of 7GUIs as what to build, and how IUP gets into the container. Its first milestone is only a check that IUP builds and opens a dialog with no screen | The three as recommended in the plan |
| 43 | The environment for libuv: the plan is sent (`261113-env-uv.md`). It waits for the author on three items: the order of the features, the samples of `ceu-libuv` as tests, and who owns the main loop (the environment, with a step function for a GUI to drive it later) | The three as recommended in the plan |
| 47 | The binding of raylib and its 23 games share one repository (`env-ray`), and Lessmilk keeps a copy of the binding, pinned at a commit | Split the games out (`ray-games`), so that the binding is used and not copied |
| 55 | The three games over SDL (Pingus, SNKRX, Flare) wrote three platform layers of their own, and none uses `lib/sdl.atm`. The author asked to make them use the environment they are supposed to use. Of the 87 functions of SDL that the three use, 24 are common to all, and `lib/sdl.atm` covers 32. Each engine also has C of its game, which stays (the pathfinding and the maps of Flare, the physics of SNKRX, the collision maps of Pingus) | In course: the author created `ia-atmos/env-sdl`, and its session has the plan `261113-env-sdl.md`. Five items of the plan wait for him there: one file `sdl.atm`, the prefix of the events, one test mode, `lib/sdl.atm` leaving the compiler (and where `exs/rocks/` goes), and how a game gets the environment |
| 56 | Whether the file `raylib.atm`, its table `rl`, and the prefix `:rl.` of its events follow the new name of the repository (`ray`) | Not studied. Related to item 46 |
| 27 | SuperTux, taken up again as `sdl-tux`: the plan is sent (`261113-sdl-tux.md`), to a new session. It waits for the author on four items: the tag `v0.7.0`, the slice (World 1, in two steps), the scripts (those of the levels as data that a small task interprets, the long scenes as Atmos code), and the SDL renderer only | The four as recommended in the plan |
| 57 | How two environments live in one program (a GUI and the network: two loops, one `main`). `env-uv` and `env-iup` meet it, and now `pico-frescogo` (pico and a serial line). lua-atmos has an answer: each environment registers a `step` and a `quit`; the first one loaded is the primary (it owns the clock and may sleep), the others are secondary (they poll with no wait) | Not designed here. The model of lua-atmos is the natural one: an environment file may define a step and a quit instead of a `main`, the dedicated compiler receives a list of environments, and the primary calls the steps of the others. To decide with the first real case (item 68). **A first pair works** (2026-11-16): `env-uv` and `env-iup` in one program (`tst/pair/iup-echo.atm` of `env-uv`: a button of IUP sends a text to an echo server of libuv). How: `uv.atm` built with `-DUVA_NO_MAIN` defines no `main` and emits no time, and `iup.atm` calls its poller at each tick and its closer after `atm_quit`, through weak symbols. Both sessions say it only answers this pair, and propose the general form in the runtime: a registry where an environment that does not own `main` registers a poller and a closer (`atm_poll_add`, `atm_close_add`), called by the one that owns it (`atm_poll()`), and a rule for who owns `main` and the time. It is the `step` and `quit` of lua-atmos |
| 49 | The picture of the organization and the social preview of the repository `lang` | Upload `doc/logo/atmos-icon.png` and `doc/logo/atmos-1280x640.png` (the author's to do, in the settings of GitHub) |
| 61 | Master of Orion (`ia-atmos/pico-1oom`, a fork of 1oom): the session exists and has its initial instructions (`261114-pico-1oom.md`); the author commands it there, one to one with fidelity first. Three things it must hear from him before any work: the original project forbids code made by AI (the port stays in the fork, and nothing goes back); the game needs the LBX files of an original copy, which cannot be fetched; and which branch to port | His to decide, in that session |
| 68 | FrescoGO! (`ia-atmos/pico-frescogo`, a program of the author: a Processing sketch of 1710 lines, the scoreboard of frescobol with a radar on a serial line). To be ported over pico. The plan is `261116-pico-frescogo.md`, sent to a new session, `pico-frescogo`, on 2026-11-16. Its caveat: no environment here has a serial port, and two environments cannot be combined in one program (item 57); the primary and secondary modes of lua-atmos do not exist in this implementation | The author discusses it in that session. Recommended there: the serial line as a small library of the port, polled once per frame as the original does, and use the case to design a second environment (`env-posix`) with him |
| 54 | The assets of Lessmilk have no known license; the author chose to copy them | Ask Thomas Palef |

## B. Work known and not done

Waiting for the author's "ok":

| # | Work | Note |
|---|------|------|
| 6 | Remove the event of termination of transparent blocks | See A |
| 22 | Mark the plan of Pingus as done, with the link to the port | Waits for the author, who wants to test the game himself first (2026-11-18): the session `sdl-pingus` was asked to give him a step by step, from the clones on |

Not started, with no decision needed:

| # | Work | Note |
|---|------|------|
| 12 | Tweens and springs as a library | To harvest from what SNKRX (`eng/anim.atm`) and Lessmilk (`lib/motion.atm`) wrote |
| 13 | The trace of an error with the tasks it crossed. The author said to skip it (2026-11-11). New evidence from the environment for libuv: an error raised inside a task of a library shows only the lines of the library (`uv.atm:N (throw)`), and not the line of the `await` in the program of the user, which is the one needed | Add one line per await that the error crosses, ` |  file:N (await)`, as a call already has. Small, and it changes the expected output of the tests that print such traces |
| 15 | Faster compilation (24 s in Pingus, a single C file) | To measure in Flare first |
| 35 | Performance. Three ports show the same profile: the dispatch of events to many tasks, the access to tables, and, the clearest, the construction of small tables (about 0.5 us each, in the Storm of pico) | Not now: SNKRX runs at almost twice the speed of its original . Measured by `env-uv` (20000 round trips of an echo, in ms): a broadcast with a filter costs 374, 2084, 7880 for 10, 100, 1000 connections; an emit to one task (`atm_emit_to`) costs 196, 312, 465. The handle as a field of the event (`:read [h=s]`) costs within the noise of the measure (at most 10 to 15 percent), measured again on one machine on 2026-11-17; a first number of 1.7 times compared two machines and was wrong. The author then chose bare tags for env-uv, since each event has one receiver |
| 38 | The paper, "Structured Reactive Concurrency in Games". Moved to this repository on 2026-11-15, by the author: the outline is `papers/games.md`, the list of its session is `.claude/plans/TODO-paper-games.md`, and a new session, `paper-games`, writes them. They came from `ia-atmos/paper-src-games` at `7ff8f88` (session `src-games`), which the author removes | After the ports close: they are its evaluation. Not of this session: pull before any push |
| 66 | A second paper, on `pin` as a memory mechanism (an idea of the author, 2026-11-15). A session of its own, `paper-pin`, writes the outline in this repository, `papers/pin.md` (in `main` since 2026-11-15, and its session pushes there: `8bb293a`; its list is `.claude/plans/TODO-paper-pin.md`): a name, the idea, why it is relevant as a memory mechanism, and why it only applies to structured programming | Not of this session. It is a third session that writes in `ia-atmos/lang`: pull before any push |
| 67 | A third paper, on the architecture of snapshots and commands (item 64; asked by the author in the session `paper-snapshots` on 2026-11-15). That session writes the outline, `papers/snapshots.md`, and keeps `.claude/plans/TODO-paper-snapshots.md`. Its closest relative is system prevalence (Prevayler); for action games the author restricts saves to checkpoints | Not of this session |
| 52 | Environments that the families of Atmos and Céu have and this backend does not: the browser (through WebAssembly; SDL and raylib support it) and embedded boards | Ideas, not asked for |
| 70 | Atmos on the ESP32. `ia-atmos/env-esp32` is a copy of `pgvalle-research/atmos-esp32` (Pedro Valle: the Lua implementation on the board, Lua 5.5 over ESP-IDF, with bindings in C for GPIO, PWM, ADC, ESP-NOW, IR, NVS, and environments in Lua, the clock as primary). A session `env-esp32` was created on 2026-11-18 with the plan `261118-env-esp32.md`; the author discusses there what to do. Related to item 52 | His to decide, in that session |

## C. Reported and not acknowledged

Done by this session with a choice that the author did not confirm. Each
may be changed.

- **The order of awaking, in the manual.** The tasks of a block in the
  order they were attached; nested tasks and pools before the body of
  their task; a task only reacts to an emit that starts after its
  await. It states what the runtime and the reference do.
- **A loop over a pool** visits a task that its body spawns into the
  pool, and does not visit one that its body aborts.
- **`await(n, p)`**: a count lower than 1 is satisfied at once, with
  `nil`; the value is the one of the last time; a counted pattern cannot
  spawn a task; the parentheses are optional only in `watching` and
  `loop on`.
- **A `:clock` await awakes at each piece of the time**, and so many
  times per frame when timers are pending (as in the reference). Now in
  the manual, after the port of Storm took one `:clock` for one frame.
- **A native literal fails by assigning to `st`**, and the names with
  the prefixes `a_`, `A_`, and `atm_` are reserved.
- **A task that an await spawns gives its final value away**
  (`pin s = await T()` works; found by `env-uv`). The task that comes
  as the second result no longer holds the value: `val v, t = await
  T()` then `t.ret` is `nil`. A task awaited by its reference keeps
  it.
- **A compiler dedicated to an environment** is this compiler built
  with defaults (`make env`), not a wrapper nor a library: the file of
  the environment is included before the program, and its flags go to
  the C compiler before those of the user.
- **An uncaught table shows as its constructor** in the message of
  the error (`:Err.io [msg="..."]`), cut at 200 characters, and no
  longer as `table: 0x...`.
- **The names of the exit statements** (`break`, `skip`, `return`,
  `until`, ...) cannot be declared, as `pub`, and may be the names of
  fields, as the keywords (`[skip=true]`).
- **A `spawn` that is aborted at the end of its block is a compile
  error**, not a warning (item 36, done on 2026-11-14 by a session of
  its own; the author was asked and has not answered). It applies when
  the spawn has no pool, its value is not used, only spawns follow it
  in the block, and its task may await.
- **A bare `pin x` is a `pin val x`** (2026-11-17): it may not be
  assigned; only `pin var` may. The author approved `pin val` and `pin
  var`, and did not say whether the bare form stays. Kept, so that the
  ~1770 pins of the ports keep compiling; the few that are assigned
  (Flare 1, SNKRX 1, by a scan) become `pin var`.
- **A `pin` parameter is sugar** for `pin x = x` at the top of the body
  (the AST shows it so), and works in `func`, `task`, and lambdas.
- **`math.atan(y, x)`** takes the point, as in Lua 5.4.
- **`pub`** is rejected as the name of a variable.
- **The README** follows the one of `atmos-lang/atmos`. It has no badge
  of tests and no list of versions: this repository has no CI and no
  releases.
- **The logos** in `doc/logo/` are rendered from `atmos.svg` of
  `atmos-lang/etc`. To upload them as the picture of the organization
  and as the social preview of the repository is the author's to do
  (item 49).

## Session names (renamed by the author's decision, 2026-11-18)

| Session | Old title | ID |
|---|---|---|
| `atmos-lang` (this one) | `ceu-atmos` | session_011eECogKVQ7ZzmWCoMST2wK |
| `atmos-pin` | `Pin rules discussion` | session_011eECogSDhkVfyt7qme5DLh |
| `env-pico` | `pico-sdl` | session_011eECoikzsmBiPNXNF7ffpf |
| `env-ray` | `raylib` | session_011eECohyySVpgZiuUF17qc2 |
| `env-iup` | `iup` | session_011eECoiJLW3LfTrKdHtNFvg |
| `env-uv` | `uv` | session_011eECoeuP2uc6Uge65NF8uL |
| `sdl-pingus` | `pingus` | session_011eECoent86e4ta3otsCoEa |
| `sdl-snxkx` | `snxkx` | session_011eECohRKf757Ydp2i9w7Ng |
| `sdl-flare` | `flare` | session_011eECoegYsZr7nSPxcugGtt |
| `ray-lessmilk` | `lessmilk` | session_011eECohyasjWkvCHdxGxJ6D 42, 43, 27, and 55) |
| `raylib` | A permission to change the remote of its checkout of the compiler (harmless meanwhile: the old address redirects) |
| `flare` | Three items of its plan to confirm, and a permission to install packages (not checked again since 2026-11-11) |
| `atmos-oo` | Created on 2026-11-18, after the method call (`::`) was removed (`b4424ff`): whether and how to declare and dispatch functions of the same name by tag. Material `261118-oo.md`; list `TODO-oo.md`; branch `oo` |
| `atmos-float` | Created on 2026-11-19, after `ATMOS_REAL=float` (`4565af1`, `atmos-lang#3`): the numeric types of a build, and whether integers and reals should be configurable for other systems. Material `261119-float.md`; list `TODO-float.md`; branch `float`; session_011eECog5kQDX9i1eN1gKzDJ |
| `atmos-loadx` | Created on 2026-11-19, asked by the author through `pico-slides`: a loader of data that accepts names. Done on `main` (2026-11-20): `X.loadx([dir,] s)` with `TITLE = [...]` definitions, a name used as a value (a copy), `include "file"`; no spread; `X.load` unchanged; manual "Load with Names"; all 23 items closed. Material `261119-loadx.md`; list `TODO-loadx.md`; branch `loadx`; session_011eECoiJyx2tmrJg6HgoknZ. Done and archived on 2026-11-23 (the author asked); left as an option only: a limit on the depth of a chain of includes |
| `atmos-cli` | Created on 2026-11-20, asked by the author: the command line of the language (options, configuration, environments, install, cache, tests). Material `261120-cli.md`; list `TODO-cli.md`; branch `cli`; session_011eECoijhygAGqD1jDZ3pWZ |
| `atmos-blocks` | Created on 2026-11-20, for `atmos-lang#6`: whether a transparent block emits an event when it ends, and the wait of a `par` that rides on it. Material `261120-blocks.md`; list `TODO-blocks.md`; branch `blocks`; session_011eECoipBcPZnUN2BbGAyuB. Done and archived on 2026-11-23 (the author asked): `blocks` merged as 95946fe |
| `atmos-iter` | Created on 2026-11-20, asked by the author after `find`/`filter` of `atmos-pool`: iterators, loops and functions over iterables, to organize that part of the language. Material `261120-iter.md`; list `TODO-iter.md`; branch `iter`; session_011eECofmVokL6pRNJNTDjhd |
| `atmos-tag` | Created on 2026-11-22, asked by the author through `atmos-iter` (its `#17`): whether the tag of a table should have a more primitive form, out of the keys. Material `261122-tag.md`; list `TODO-tag.md`; branch `tag-rebased` (the remote `tag` is stale, cut from an old main); session_011eECogjajEpy59YjN42Max. Done and archived on 2026-11-22 (the author asked): #1 to #5 merged as 98e4787 |
| `atmos-slices` | Created on 2026-11-22, asked by the author in `atmos-lang` (its `#116`, direction `#116c`): slices of vectors with no copy. Material `261122-slices.md`; list `TODO-slices.md`; branch `slices`; session_011eECoensrrB23ggBu4daLd |
| `atmos-threads` | Created on 2026-11-22, asked by the author in `atmos-lang`: threads (none today; `thread` is only a reserved word; the reference has `thread { ... }`). Material `261122-threads.md`; list `TODO-threads.md`; branch `threads`; session_011eECohRAjCia6Hb5cAcrz2 |
| `atmos-cc` | Created on 2026-11-23, asked by the author in `atmos-lang`: the time of a build with includes (separate compilation, `--verbose`, `--cc-` flags). Material `261123-cc.md` (Pingus: Atmos to C 0.42 s, cc 5.9 / 20.7 / 34.8 s at -O0/-O1/-O2); list `TODO-cc.md`; branch `cc`; session_011eECoibvvH58hfgFezzYvM |
| `atmos-ppp` | Created on 2026-11-23, asked by the author in `atmos-lang`: whether the tip markers `@#` and `@+` are still relevant, and `+=` on vectors. Material `261123-ppp.md` (ports: 359 `set t@+ = v`, 7 `t@#`, 74 `vector.insert`, 0 `++=`); list `TODO-ppp.md`; branch `ppp`; session_011eECogRQbTZU7Bn4bZysFK |
| `env-web` | Created on 2026-11-24, asked by the author in `atmos-lang` (its `#118`: `env-web` rather than `env-js`): Atmos in the browser, SDL and Rocks through Emscripten, the DOM. Repository `ia-atmos/env-web`; material `261124-env-web.md` here; session_011eECoffhJTecG6BZSq3GQi |
| `atmos` | Created on 2026-11-24, asked by the author in `atmos-lang` (its `#119`, "Ok"): the manager of all sessions, his main point of contact; owns this `TODO.md` and `ISSUES.md` from now on; `atmos-lang` keeps the implementation, the measurements and the merges. Handoff `261124-atmos.md`; session_011eECohhJbc4GMpAj3yyeEo |
| `pico-slides` | Not created here (seen on 2026-11-19): presentations as one tagged table over pico, repository `ia-atmos/pico-slides`; session_011eECogEdSaHMwZpk7NGmgk. Reads all its documents with `X.loadx` (its 39 tests load, reported 2026-11-20) |
| `atmos-grab`, `atmos-context` | Created on 2026-11-18, one per issue of `atmos-lang/atmos` (22 and 21): item 39 moved there. Material `261118-grab.md` and `261118-context.md` (the real cases of the ports); lists `TODO-grab.md` and `TODO-context.md` |
| `atmos-pin` | Since 2026-11-18 everything about `pin` is there, by the author's decision: item 40 and the pin choices of section C moved (`261118-pin-handoff.md`). It implements on the branch `pin`; this session only measures on the ports, migrates the tests, and tells the ports |
| `atmos-args` (was `args`; its branch stays `args`) | Created on 2026-11-18, to discuss one rule for arguments with and without parentheses (calls, statements, `await`): item 31 moved there. Material `261118-args.md`; list `TODO-args.md` |
| `atmos-pool` (was `pool`; its branch stays `pool`) | Created on 2026-11-18, to discuss awaits and emits over pools and tasks: items 1, 2, 4, 33, 60 move there (31 went to `args`, 32 was done here). Its material is `261118-pool.md`; its list, `TODO-pool.md` |
| `atmos-envs` (was `envs`) | Created on 2026-11-17 (session_011eECofYWTNPACTtUjHN4p6), to discuss the environments with the author: items 46, 47, 52, 56 to 60, 68 move there. Its material is `261117-envs.md`; its list, `TODO-envs.md` |

Each session keeps its own `.claude/plans/TODO.md`, in its repository,
since 2026-11-13: what waits for the author in each is there.

The repositories of the organization were renamed on 2026-11-13, and
every session was told its new name: `env-pico`, `env-iup`, `env-uv`,
`env-ray`, `ray-lessmilk`, `sdl-pingus`, `sdl-snxkx`, `sdl-flare`,
`paper-src-games`, `sdl-tux`; `doom` and `lang` kept theirs. The titles
of the sessions still have the old names.

## Session ids

To message a session with `send_message` (archived: `atmos-args`, `atmos-tag`, `atmos-blocks`, `atmos-loadx`, `env-esp32`, `atmos-ppp`, `atmos-float`, `atmos-grab`).

| Session | Id | Topic |
|---|---|---|
| `atmos-lang` | session_011eECogKVQ7ZzmWCoMST2wK | the compiler; measurements on the ports; merges |
| `atmos-pin` | session_011eECogSDhkVfyt7qme5DLh | the rules of `pin` |
| `atmos-pool` | session_011eECoj2p8VQMfaFidx4Nfx | awaits and emits over pools |
| `atmos-envs` | session_011eECofYWTNPACTtUjHN4p6 | the API of environments |
| `atmos-grab` | session_011eECohuNsFwANSFccPJ1vr | `grab` and `grabbable` (archived 2026-11-26 by `atmos`, on the author's words in `atmos`: "arquivar atmos-grab entao"; no open items, `atmos#20`) |
| `atmos-context` | session_011eECofmJeKkrNWJ2M57peX | `context` |
| `atmos-oo` | session_011eECoewYWvj9UCiBWahwfQ | functions by tag |
| `atmos-float` | session_011eECog5kQDX9i1eN1gKzDJ | the numeric options of a build (archived 2026-11-26 by `atmos`, on the author's words there: "ok. Acho que pode pedir pra sessao pai atmos arquivar aqui?"; all on `main`, index `TODO-float.md`; `atmos-float#12` and `#8` carried by `atmos`, `atmos#19`) |
| `atmos-loadx` | session_011eECoiJyx2tmrJg6HgoknZ | `X.loadx` |
| `atmos-cli` | session_011eECoijhygAGqD1jDZ3pWZ | the command line |
| `atmos-blocks` | session_011eECoipBcPZnUN2BbGAyuB | the end of a transparent block |
| `atmos-iter` | session_011eECofmVokL6pRNJNTDjhd | iterators, loops and functions |
| `atmos-tag` | session_011eECogjajEpy59YjN42Max | the tag of a table |
| `atmos-slices` | session_011eECoensrrB23ggBu4daLd | slices with no copy |
| `atmos-threads` | session_011eECohRAjCia6Hb5cAcrz2 | threads |
| `atmos-cc` | session_011eECoibvvH58hfgFezzYvM | the time of a build |
| `atmos-ppp` | session_011eECogRQbTZU7Bn4bZysFK | the tip markers `@#` `@+` (archived 2026-11-26 by `atmos-lang`, on the author's "Peça para o pai arquivar"; `++=` in place and `string.join` merged; the branch `ppp` stays, to delete) |
| `env-web` | session_011eECoffhJTecG6BZSq3GQi | environment: the browser |
| `atmos` | session_011eECohhJbc4GMpAj3yyeEo | the manager of all sessions |
| `sdl-pingus` | session_011eECoent86e4ta3otsCoEa | port: Pingus |
| `sdl-snxkx` | session_011eECohRKf757Ydp2i9w7Ng | port: SNKRX |
| `sdl-flare` | session_011eECoegYsZr7nSPxcugGtt | port: Flare |
| `sdl-tux` | session_011eECoijTqoBzMrEaknmW9P | port: SuperTux |
| `ray-lessmilk` | session_011eECohyasjWkvCHdxGxJ6D | port: Lessmilk games |
| `doom` | session_011eECoiGQuUE9LUpGJZ7yFf | port: Doom |
| `pico-1oom` | session_011eECoiEGgQ6ZBFHUCoxt77 | port: 1oom |
| `pico-frescogo` | session_011eECoitGg7fJwfZh8JDNnR | port: FrescoGO! |
| `pico-slides` | session_011eECogEdSaHMwZpk7NGmgk | presentations over pico |
| `env-sdl` | session_011eECogZjerUUUp2BmsSRgW | environment: SDL |
| `env-pico` | session_011eECoikzsmBiPNXNF7ffpf | environment: pico-sdl |
| `env-ray` | session_011eECohyySVpgZiuUF17qc2 | environment: raylib, with its classics |
| `env-iup` | session_011eECoiJLW3LfTrKdHtNFvg | environment: IUP |
| `env-uv` | session_011eECoeuP2uc6Uge65NF8uL | environment: libuv |
| `env-esp32` | session_011eECoh5ihcYSug4LCqgMmr | environment: ESP32 boards (archived 2026-11-26 by `atmos-lang`, on the author's words there: "Pedro vai controlar esse repo. Acho que podemos arquivar aqui? Peça para sessao pai arquivar se for o caso"; all pushed up to 3e7ebd5; `atmos#12`) |
| `paper-pin` | session_011eECofALi31oSimzbLxYZp | paper on `pin` |
| `paper-games` | session_011eECogKyRYMVJJfbmpHYv9 | paper on the game ports |
| `paper-snapshots` | session_011eECoiWKpJwsuKH43TLunQ | paper on snapshots and commands |
| `paper-media` | session_011eECoiQwcXzsrFhTMMYGr7 | paper on media |
| `paper-web` | session_011eECoei7PA2fA6jzUtWpqz | paper on Atmos in the browser (created 2026-11-26 by `atmos`, `atmos#17`; material `261126-paper-web.md`) |

## Standing rules from the author, for all sessions

- Commit and push straight to `main`, in fine-grained commits.
- **Items are `#N`** (2026-11-18, `#71`). Everything a session puts to
  the author has a number: a decision, a question, a remark, a defect, a
  piece of work. A number is unique in its session, taken in order,
  never reused and never changed: an item that closes or moves keeps
  it. `#N` is reserved for this: nothing else in a session is
  enumerated with bare numbers (no "1, 2, 3" for a local list in a
  paragraph or a reply: bullets, or letters under an item, `#75a`, for
  its options and its steps). From another session an item is
  `session#N` (`atmos-pin#12`); an item that moves keeps the number of
  the session where it was born. Each session keeps the index of its
  numbers (title and state): here, `ISSUES.md`; elsewhere, a section of
  its TODO. Identifiers that sessions already use (`A3`, `F6`, `E5`)
  stay valid for what was written with them.
- **Language sessions work on branches** (2026-11-18): a session that
  discusses a part of the language (`atmos-xxx`, `args`, `pool`)
  implements it in the compiler itself, on a branch named after it
  (`args`, `pool`, `grab`, `context`, `pin`); its plans and TODO still
  go straight to `main`. Before merging: bring the latest `main` into
  the branch and run the full suite (`tst/run.sh`, `make test-rt
  test-env`, with `ATMOS_ASAN=1`); ask `atmos-lang` to measure the
  impact on the ports when the change touches existing programs; merge
  only after the author approved the design and with the suite passing
  over the current `main`. After the merge, `atmos-lang` migrates the
  tests if needed and tells the ports.
- **Work with no input** (2026-11-20, `#95`). The author, in
  `atmos-lang`: "I want you to review all sessions and propose them
  changes that need no input. The sessions may also work suggest by
  themselves. No input form me. Go ahead". As read by `atmos-lang` (the
  reading is its, to be corrected by him): a session does now the work
  that needs no decision of his, proposed to it or its own; where a
  decision of his is pending and the session has a recommendation, it
  may go on with the recommendation if that is reversible and stays in
  its own repository or branch, and records it as an item marked "by
  the session, to be confirmed", so that he can review and undo it; it
  does not ask him and does not wait. Not covered: the merge of a
  language change into `main` of `ia-atmos/lang` (still his approval:
  the branch is left ready and measured), anything outward or hard to
  undo (upstream, tags, releases, deleting originals or history,
  installs that need his approval), and the limits a session already
  has. A defect fix that changes no rule of the language goes to `main`
  through `atmos-lang`, after the suite and the measurement on the
  ports.
- **Screenshots at each big step** (2026-11-26, said in `pico-1oom`):
  "A cada passo grande, me mostre alguns screenshots." For every
  project with a screen: at a milestone or a feature that changes what
  is on screen, the session shows him a few screenshots in its report.
- **A pair of Atmos excerpts per group of changes** (2026-11-26, said
  in `atmos`): "Para todos os projetos, alem dos screenshots, mostrar
  também trechos de código Atmos idiomaticos e ruins. Um par por grupo
  de mudanças." Every project (ports and environments): in a report to
  him, each group of changes comes with one idiomatic excerpt and one
  bad one (awkward, verbose, a workaround), with file and line.
  Added the same day: "1. Numerar 2. Justificar os ruins": each excerpt
  takes a number of the session's `#N` sequence, and each bad one says
  why it is bad and what would make it better.
- Each session keeps a `.claude/plans/TODO.md` (2026-11-13).
- The events of an environment have no prefix (2026-11-13).
- **General instruction for the ports** (2026-11-26, confirmed in
  `pico-1oom`: "Confirmo, pode enviar"; `pico-1oom#29`, `atmos#16`). It
  does not replace "Fidelity, then expressiveness"; it says what fidelity
  means ("Fidelidade não é compatibilidade de formatos. É algo interno
  sobre o comportamento observável somente durante a execução").
  1. Freedom: fidelity is of the behaviour observable while the program
     runs, not of the shape of the code nor of file formats; no line by
     line translation.
  2. Idioms: idiomatic Atmos (tables, lambdas, `if`/`ifs`/`match` as
     expressions, strings and tables, the standard library); no integer
     widths, pointer emulation or memory management of the original,
     except where the behaviour needs them.
  3. External files: the formats the program reads and writes for itself
     (configuration, logs, saves) may change to native forms (`X.load`,
     `X.dump`); the exception is the data of the original (WADs, LBX).
  4. Bugs: do not reproduce bugs of safety or tied to the language of the
     original; fix the original in a local copy for the checks only, and
     record each one.
  5. Each port answers the author in its session: how much it follows
     1-4, what it will redo, a grade from 1 to 10.
- **Fidelity, then expressiveness** (2026-11-14, said in the session
  of Lessmilk): once a port is fully faithful to its original, it may
  relax the constraints of exactness in favor of expressiveness, as
  long as the intent of the original author is not affected. It holds
  for every port, after it reaches full fidelity. The plans say that a
  difference is never an accepted variation: that stands until full
  fidelity, and not after.

## E. Housekeeping of this session

- If the container restarts, the checkout comes back with the old
  address (`fsantanna/ceu-atmos`): point the remote to `ia-atmos/lang`
  before any push.
- The cloud environment of this session is still named `atmos-lang`, and
  does not reach `lessmilk.com` (the request to add the host could not
  be approved from the phone).

## Settled, for the record

7, 9, 10, 14, 16, 20, 21 (kept as they are); 8 (became 31); 17 and
28 (the session `env-pico` wrote the binding and its examples); 25 and
26 (moved to D); 29 and 30 (the games were chosen; Doom was added); 44
(`await(n, p)`, done); 48 (`skip`, done by a session of its own on
2026-11-14: no value, parentheses optional; the author chose no
dedicated error for `continue`); 36 (the check of spawns, done on
2026-11-14; it found 5 live defects in three ports); the names of the repositories (first part of 47); 34, 41, and 62 (done with the new rules of `pin` on 2026-11-15: a nested constructor is pinned with the outer one; the message of an escape says the type and the line of the pin; an unpinned task that is dropped is aborted at once); 18, 19, and 24 (on 2026-11-15 the author deleted every branch but `main`: `trails`, the two `ccr-*`, and `claude/paper-pin`, none with work missing from `main`; the README of Pingus already pointed its plan to `main`; a session cannot delete a branch, the push is refused with 403);
5 (no `task main`: decided by the author on 2026-11-17, in the session `atmos-envs`; the file stays the main task); 23 (the session `sdl-pingus` removed its workarounds in the two migrations of `pin`, 2026-11-17 and 18);
51 (became 55); 11 (mouse in `lib/sdl.atm`: the library left this repository on 2026-11-16, by the author's decision, with Rocks; both live in `ia-atmos/env-sdl`).

## Next week (the author, 2026-11-23)

> Vamos continuar novos passos após créditos da próxima semana. Iremos
> acordar todos esperando créditos. Também iremos perguntar quais
> projetos ainda tem código C embarcado. Não agora. Aguarde.

The pause is over (2026-11-25, `atmos#7`): the sessions were awakened
(`atmos#4`) and told to resume. Still to ask, when he says: which C code
each project still embeds.

## Tomorrow (the author, 2026-11-24)

> Todo tomorrow:
> - env-js with sdl support
> - Can we run rocks there?
> - what about dom? Want to revive old plans

What exists, as found on 2026-11-24 (nothing done yet):
- `lua-atmos/env-js` and `atmos-lang/web`: the browser environment of
  the reference (Atmos in Lua, active in Mar/2026), which runs a program
  passed in the URL; `web` is an editor with a "Run" button
  (`261117-envs.md`, the table of environments; `261117-envs-atmos.md`
  read `env-js` against our API).
- Rocks: `ia-atmos/env-sdl` `exs/rocks/` (with images, sounds, a font,
  and a pause test) and `ia-atmos/env-pico` `exs/rocks/`.
- The compiler emits C; a browser build would go through Emscripten
  (SDL2 has an Emscripten port). This container has `node` but no
  `emcc`.
- Moved to the session `env-web` (2026-11-24). The "old plans" of the DOM: to find (in `lua-atmos/env-js`,
  `atmos-lang/web`, or the author's older repositories) before
  proposing.
