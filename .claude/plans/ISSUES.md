# Items of the session `atmos-lang`

The sequence of `atmos-lang` (`atmos-lang#N` from outside). The session
`atmos` has its own, in `ISSUES-atmos.md` (the author, 2026-11-25:
"Aqui tb deve ser diferente do lang"). `#120` to `#124` were used by
`atmos` by mistake and are now `atmos#1` to `atmos#5`: they are not
reused here, `atmos-lang` continues from `#125`.

Every item, by its number. A number is never reused nor
changed: an item that closes, or that moves to another session, keeps
it. Inside this session an item is `#N`; from another session it is
`atmos-lang#N`. The convention is `#71`, and is in `TODO.md`, "Standing
rules". The details of the open ones are in `TODO.md`.

States: **open** (waits for the author here), **work** (known, not
started, no decision needed), **moved** (another session handles it),
**done**, **closed** (settled with no change).

| # | Item | State |
|---|------|-------|
| 1 | `await` over a pool with a condition ("a pingu enters my zone") | moved: `atmos-pool` |
| 2 | Phases inside a tick, instead of the order of creation | moved: `atmos-pool` |
| 3 | 32-bit reals | done 2026-11-19 (4565af1, the build option `ATMOS_REAL=float`, chosen by the author in `sdl-tux`); what follows moved: `atmos-float` |
| 4 | An `emit` that names its receiver, instead of `emit @2` | moved: `atmos-pool` |
| 5 | An optional `task main` as the entry point | closed 2026-11-17: no (decided in `atmos-envs`) |
| 6 | The event of termination of transparent blocks | done 2026-11-23: `blocks` merged by `atmos-blocks` as 95946fe on the author's "#2a" (atmos-blocks#2); measured by `atmos-lang` on 8801b76 (same code): port diagnostics and C identical, sdl-pingus passes, a loop of `par :all` twice as fast; the ports told |
| 7 | Iterating a tagged table gives the field `tag` | closed: kept, as the reference |
| 8 | `await x` needs `await(x)`; `loop n {}` is not a counted loop | closed: became #31 |
| 9 | Indexing a string is an error (the reference gives `nil`) | closed: kept |
| 10 | The names `:value`, `X.nin`, `include` | closed; later `:value` became `:atmos` and `X.xin` became `X.in` (2026-11-17) |
| 11 | Mouse events in `lib/sdl.atm` | closed 2026-11-16: the library left for `env-sdl` |
| 12 | Tweens and springs as a library | work |
| 13 | The trace of an error with the tasks it crossed | done 2026-11-20 (main f52579e, the branch `lang`; approved by the author, `#99a`) |
| 14 | `X.dump` with indentation | closed: not until someone needs it |
| 15 | Faster compilation (24 s in Pingus) | work |
| 16 | Slimmer `ATask` and `ANode` | closed: not now |
| 17 | The examples of `pico-sdl` | done by `env-pico` |
| 18 | Branch `trails` | done 2026-11-15: deleted by the author |
| 19 | Branch `ccr-566b5373-ag7186` | done 2026-11-15: deleted by the author |
| 20 | Images and sounds of Rocks in this repository | closed; Rocks left for `env-sdl` on 2026-11-16 |
| 21 | Three changes of style in the port of Rocks | closed: kept |
| 22 | Mark the plan of Pingus as done | open: the author tests the game first (`sdl-pingus` gives him the steps) |
| 23 | Pingus still uses workarounds no longer needed | done 2026-11-18 by `sdl-pingus` |
| 24 | The README of Pingus points its plan to an old branch | done 2026-11-15 |
| 25 | The session of SNKRX waits for an answer | closed: moved to the table of sessions |
| 26 | The session of Flare: three items to confirm, a permission | closed: moved to the table of sessions |
| 27 | SuperTux, taken up again as `sdl-tux` | moved: `sdl-tux` |
| 28 | The session of pico waits for an answer | done by `env-pico` |
| 29 | The dynamic slot: SNKRX | closed: chosen |
| 30 | The C/C++ slot: Flare (Doom was added) | closed: chosen |
| 31 | `await x` for a variable or a field | moved: `atmos-args` |
| 32 | `await :all ts` on an empty pool never awakes | done 2026-11-18 (`cab49e2`) |
| 33 | Time emitted to a pool or a task (slow motion) | moved: `atmos-pool` |
| 34 | A constructor nested in a pinned one is not pinned with it | done 2026-11-15 |
| 35 | Performance (dispatch of events, tables, small tables) | work |
| 36 | A `spawn` that dies with its block, silently | done 2026-11-14 (session `spawn-check`) |
| 37 | Doom: how far to go after the flow of the game | moved: `doom` |
| 38 | The paper on structured reactive concurrency in games | moved: `paper-games` |
| 39 | `grab` and `context` (issues 22 and 21 of `atmos-lang/atmos`) | moved: `atmos-grab`, `atmos-context` |
| 40 | The rules of `pin` | moved: `atmos-pin` (2026-11-18) |
| 41 | The escape error does not say what escaped | done 2026-11-15 |
| 42 | The environment for IUP | moved: `env-iup` |
| 43 | The environment for libuv | moved: `env-uv` |
| 44 | `await(n, :e)`, an await with a count | done |
| 45 | A snapshot of the tasks by the runtime | open (to talk about; Doom is the case) |
| 46 | The vocabulary of the events of an environment | done 2026-11-17 (decided in `atmos-envs`; the order inside a frame stays there) |
| 47 | The binding of raylib and its games share one repository | moved: `atmos-envs` (its A5) |
| 48 | A `skip` for loops | done 2026-11-14 (session `skip`) |
| 49 | The picture of the organization and the social preview | open: the author's to do, in the settings of GitHub |
| 50 | (not recorded: used in the conversation before this list had it) | closed |
| 51 | One shared environment for SDL | closed: became #55 |
| 52 | Environments this backend lacks: the browser, embedded boards | moved: `atmos-envs` (its A7); the ESP32 is #70 |
| 53 | (not recorded: used in the conversation before this list had it) | closed |
| 54 | The assets of Lessmilk have no known license | open: ask Thomas Palef (the author's to do) |
| 55 | The three SDL games use the environment `env-sdl` | moved: `env-sdl` |
| 56 | Whether `raylib.atm`, `rl`, `:rl.` follow the name `ray` | moved: `atmos-envs` (decided with #46: one vocabulary; `env-ray` renames) |
| 57 | Two environments in one program | done 2026-11-17 (`atmos.env`, `81986e7`); what is left is in `atmos-envs` |
| 58 | `test` blocks to test a game | moved: `atmos-envs` (its A3); see #72 |
| 59 | How a program selects its environment | moved: `atmos-envs` (its A4) |
| 60 | Where the exception of `emit @t` goes | moved: `atmos-pool` |
| 61 | Master of Orion (`pico-1oom`) | moved: `pico-1oom` |
| 62 | A task whose value is dropped outlives its block | done 2026-11-15 |
| 63 | A `pin` of a task that ends a loop body is not flagged | moved 2026-11-20: `atmos-pin` (asked to prepare it on `pin`) |
| 64 | An architecture of snapshots and commands | moved: `paper-snapshots` |
| 65 | `t@0.x` does not compile | done 2026-11-20 (main f52579e, the branch `lang`; approved by the author, `#99a`) |
| 66 | A paper on `pin` | moved: `paper-pin` |
| 67 | A paper on snapshots and commands | moved: `paper-snapshots` |
| 68 | FrescoGO! over pico | moved: `pico-frescogo` |
| 69 | The six exit names as keywords of the lexer | done 2026-11-20 (main 61979ce, the branch `args`; approved by the author, `#99a`) |
| 70 | Atmos on the ESP32 | moved: `env-esp32` |
| 71 | The numbering of items: `#N`, never reused, for everything put to the author | done 2026-11-18 (this file) |
| 72 | The tests of the projects with `test` blocks and emits: what still needs a trick | open (answered on 2026-11-18; the adoption is `atmos-envs`, its A3) |
| 73 | Two branches with nothing outside `main`, to delete: `lang` `ccr-6e380402-ae5ue6`, `sdl-pingus` `ccr-d59a6b51-1a4san` | open: the author's to do (a session cannot delete a branch) |
| 74 | No port has a `test` block: the twelve drive their tests from outside, with about ninety environment variables | open (remark; follows `#76`) |
| 75 | What a `test` block cannot do alone: keep the environment from opening a window or sound, set polled input, read pixels, reach a C callback | open (the adoption is `atmos-envs`, its A3) |
| 76 | A pilot: one port moves one test from a variable to a `test` block, to measure what is left | done 2026-11-19 (`sdl-pingus#15`, its `tst/bomber-test.atm`): logic, time, frames, keys work with `test` and emits; left: window and audio under `-t` (`atmos-envs` A3), switches read with `getenv` |
| 77 | Merge of `pin` b80bcd1 (`atmos-pin#11`: the warning of a pin given away sees `until c, v` and `while c, v`) | done 2026-11-19 (907fee9; approved by the author) |
| 78 | The trick left by the pilot `#76`: the environment opens a window and the audio also under `-t` | moved: `atmos-envs` (its A3) |
| 79 | What a test may want to set must be a parameter or a global, not `getenv` | open (remark; for the convention of `atmos-envs`) |
| 80 | The 25 sessions of the project awakened at the author's request | done 2026-11-19 |
| 81 | Merge of `pin` (`atmos-pin#1`: `set :out`, and the use of finalized data is an error) | done 2026-11-19 (tip 64005fd merged; approved by the author in `atmos-pin`, "Pode juntar o #1 à main"; suite and 12 ports pass) |
| 82 | The measurement of the branch `args` 6ba7568 on the ports (`atmos-args#3`): 2100 of 2105 trees identical, 5 files of `env-pico` become an error (`key = f [...],` in a table) | done 2026-11-19 (sent to `atmos-args`; the design is its) |
| 83 | The 25 sessions awakened a second time at the author's request | done 2026-11-19 |
| 84 | The second measurement of `args`, at 823d843 (single-atom rule, `atmos-args#8`): trees and generated C identical for all 2105 files of the ports | done 2026-11-19 (sent to `atmos-args`; the merge waits for the author there) |
| 85 | Three findings of the board build of `env-esp32`: no `nanosleep` on the host, a manual example that leaks, warnings of `-Wall` in the generated C | done 2026-11-19 (ee17162 `ATM_NO_CLOCK`, dc94c6f `a_set` in the manual, 7b0bf06 `A_UNUSED`) |
| 86 | Merge of `args` 823d843 (`atmos-args#6`: `f x`, `f t.x`, `f 10`, `await t`) | done 2026-11-19 (approved by the author in `atmos-args`, "Ok, lá"; suite passes after the merge) |
| 87 | The rules of `pin` declared stable by the author (`atmos-pin#9`): told to the 14 ports and the 6 language sessions | done 2026-11-19 (`TODO-pin.md`, section "Stable", on main 71f7d17) |
| 88 | How `#58` was settled: the language side closed with no change, the adoption by the environments is not done | done 2026-11-19 (answered; the author asked for the adoption proposal, requested of `atmos-envs`, its #3) |
| 89 | Configurable real and integer types for other systems (ESP32, Arduino) | moved: `atmos-float` (`261119-float.md`) |
| 90 | Is the await over a pool with a condition (`#1`) still pending? | answered 2026-11-19: yes, in `atmos-pool`, waiting for the author's choice (`#1a` recommended) |
| 91 | Explain `#60` (where the exception of `emit @t` goes) | answered 2026-11-19 with a program run on main; the decision stays `#60`, in `atmos-pool` |
| 92 | A loader of data that accepts names, `loadx` (the author, through `pico-slides`) | done 2026-11-20 by `atmos-loadx`: `X.loadx(s)` on main (d27805c), names as values with a deep copy per use, no spread; suite checked here |
| 93 | The measurement of the branch `float` 3bec041 (`ATMOS_INT`, `ATMOS_TIME`, libraries built on demand) on the ports, default configuration: nothing changes | done 2026-11-20 (sent to `atmos-float`; the merge waits for the author there) |
| 94 | The branch `float` merged by `atmos-float` (692af43: `ATMOS_INT`, `ATMOS_TIME`, libraries on demand): main checked, the 15 ports and the 8 language sessions told | done 2026-11-20 |
| 95 | Work with no input: review every session and propose work that needs no decision of the author; the sessions may also go on with their own suggestions | done 2026-11-20: rule written (`TODO.md`, "Standing rules"); proposals sent to the 28 sessions; here, `#13` and `#65` prepared on the branch `lang` |
| 96 | The cost of a broadcast: about 180 instructions per emit for each task that awaits another event (measured by `doom`, its program e40); an index from the tag to the trails that await it would remove it, keeping the order of awaking | open: after the branches that change `rt/task.c` (pool, grab, context) merge; `doom` also measured its variant with a door and the zombieman as tasks: 711.7 M against 263.4 M instructions, about 40% in the traversal, and 2.2 thousand per directed emit into nested awaits |
| 97 | `atm_emit_to`: an exit raised by a task that the emit ended outside the target was lost (reported by `env-uv`) | done 2026-11-20 (7e3b7b5, case `env-emit-to-02`) |
| 98 | A warning for a binary operator at the start of a line (`x` then `-1` is `x - 1`; reported by `doom`); the row is in the manual, "Ambiguities" | open (the author's; recommended: no warning, the row is enough) |
| 99 | Branches ready and measured, waiting for the author's merge | open: `lang` and `args` merged on 2026-11-20 (`#99a`). Left, his: `float` (the variables renamed; asked there), `pool` (now 9199baf; `#60` no longer for merge, see `#121`; the grid; `find`/`filter` split off to `pool-find` 0c1a6ec, for `atmos-iter`), `context` (merged 2026-11-22 by `atmos-context`, b71fbe4, on the author's "merge"; measured by `atmos-lang`: generated C of the 2197 port files unchanged, cases, golden, 48 configurations and sdl-pingus pass), `grab` (merged 2026-11-22 by `atmos-grab`, f416c03, on the author's "aprovado": `toggle` replaced by `mute`, `stage`, `solo`; the ports to rewrite are `#115`) (breaks a parameter `grab` of `env-sdl`), `envs` (two designs), `pin-out` (`#102`), `oo` 895aedd (clauses by tag, a prototype; no port changes). Merged 2026-11-22: `iter` (by `atmos-iter`, 7c64f69, the author's "Sim para ambos") and `tag-rebased` (by `atmos-lang`, 98e4787, the author's "Sim" in `atmos-tag`; same diagnostics and C on the 2197 port files, sdl-pingus and sdl-tux suites identical; 15 ports told); `blocks` merged 2026-11-23 (95946fe, the author's "#2a") |
| 100 | Merged with no input, as defect fixes, documentation and tests: `atmos-loadx` (the include that is not a file), `atmos-oo` (the table idiom in the manual), `atmos-pin#7` (the two leaks of the cycle collection) and `#12` (a warning, no port gets it), `atmos-envs` (the manual on environments); main 8c46149, suite and sdl-pingus pass; the ports asked to run their suites on it | done 2026-11-20; confirmed by the author on 2026-11-20 (`#100a ok`): the warning of `atmos-pin#12` and the paragraph of `atmos-oo#3` stay |
| 101 | The frame loop of `sdl-snxkx` about 12% slower since the ceiling of 2026-11-17: `a_unname_x` called for every slot of a block at the end of each step | done 2026-11-20 by `atmos-pin` (#14, main 23a063e): the slots are tested inline |
| 102 | `set :out` written in a function or an `if` is cleared when that block ends, silently (`atmos-pin#16`) | the author chose `#16b` (the place lasts until the value ends) in `atmos-pin`, 2026-11-21; `pin` 319c947 measured (no port changes, no cost); the merge into main asked to him here as `#110` |
| 103 | The command line of the language (asked by the author) | moved: `atmos-cli` (`261120-cli.md`) |
| 104 | Explain `find` / `filter` of `atmos-pool` | answered 2026-11-20 |
| 105 | The measurement of `float` 1bb1edf (the variables renamed: `ATMOS_FLOAT`, `ATMOS_INTEGER`, `ATMOS_CLOCK`): the default build is untouched (2197 files); `sdl-tux` (three scripts) and `env-esp32` (its Makefile) set the old names and must change at the merge | done 2026-11-20 merged 2026-11-21 (d19d40f) |
| 106 | A library for iterables, instead of `find`/`filter` as globals; iterators, loops and functions to be organized (asked by the author) | moved: `atmos-iter` (`261120-iter.md`) |
| 107 | Archive `atmos-args` (asked by the author in it) | done 2026-11-20: checked in its transcript, all closed, branch equal to main |
| 108 | The C names renamed too on `float` (`A_TIME_US` to `A_CLOCK_US`, `AReal` to `AFloat`, the `-D` macros): seven repositories would fail to build | done 2026-11-21: `float` merged by `atmos-float` (d19d40f) after the seven were ready (six compatible on their main, sdl-tux pushing at the merge); suite and sdl-pingus pass on it (with env-sdl at 94cb414 or later); every session told |
| 109 | The branch `blocks` d2825ef (the author's `atmos-blocks#1a`: the end of a transparent block is no longer an event; a `par` is woken directly) measured: no port changes (2197 files; no `await true`, `:xtask` or predicate over any event in the ports); sdl-pingus passes; a loop of short blocks with 30 idle tasks about 8 times faster | done 2026-11-21 (sent to `atmos-blocks`; the merge waits for the author there) |
| 110 | Merge `pin` 319c947 (`atmos-pin#16b`) into main | done 2026-11-21 (a80a77f; the author approved it in `atmos-pin`, "Pode juntar a main o #16"; suite and sdl-pingus pass); sdl-flare and sdl-snxkx told |
| 111 | Merge `pin` (`atmos-pin#17`, the ceiling only on the value, and `#3`, `ATMOS_PIN=unchecked|off`), approved by the author in `atmos-pin` | done 2026-11-21 (34c381b), after the warning of the on-demand build was fixed (0228ece); suite from clean and sdl-pingus pass; the ports asked to run their suites |
| 112 | Are `grab` and `context` used in any project? No: both are only on their branches, with reduced copies of the cases | answered 2026-11-21; `#112a` approved by the author: the real cases rewritten on branches of the ports (sdl-snxkx: the shop with `grab`, the stars with `context`; env-iup: the dialog of 7GUIs with `grab`; env-pico: the current layer with `context`), not merged, reported to the sessions of the features. Done 2026-11-21: sdl-snxkx exp-grab 1167084 (the shop no longer buys behind the modal; +8 lines; misfits reported) and exp-context 88da713 (canvas switches 24.72 to 4.86 per frame; +16 lines) |
| 113 | Merge `pin` (`atmos-pin#18`: the collection of unpinned cycles removed in every mode) | done 2026-11-21 (f999c52; the author in `atmos-pin`: "#18. Sim, main", "Checagem sai de todo modo"); suite, 48 configurations and sdl-pingus pass; the ports asked to run under ASAN for leaks |
| 114 | Merge `pin` 443736c: `atmos-pin#19` (a warning for `var g` holding a function that refers to `g`), `#23` (`set :out pub = v`), `#29` (the manual: deterministic reference counting is part of the semantics) | merged 2026-11-22 (author: "merge"), 0b5b8ad: cases 463, golden 443 with ASAN, 48 configurations, no new diagnostic on the 2197 port files, sdl-pingus passes |
| 115 | The ports rewrite for `mute`, `stage`, `solo` (lang main f416c03): 7 sessions told on 2026-11-22 with their lines: env-ray (21 files, 17 toggle lines), sdl-pingus (4 files, 3 lines, 2 emits), sdl-snxkx (2 lines, 6 emits), env-sdl (2 files), env-pico (2 files, 2 emits), ray-lessmilk (a local `mute`), doom (a local `stage`) | done 2026-11-22: all 7 rewrote and pushed (doom 9182b14, env-sdl, env-pico 1e68b0c6, ray-lessmilk c9284df, sdl-snxkx 2b41ab0, sdl-pingus 4aed063, env-ray 4c713f2), each with its suite identical on the new main; checked by `atmos-lang` on the updated clones: no `toggle` left, no syntax error under main, sdl-pingus suite passes |
| 116 | Slices: `#116a` `vector.sub` (a copy), `#116b` a syntax `t@(i, j)`, `#116c` a view with no copy | decided 2026-11-22, the direction `#116c` (his words: "Crie sessão atmos-slices. A ideia é não ter cópia"); moved to `atmos-slices`, material `261122-slices.md` |
| 117 | `atmos-iter` merged `#21a`/`#22b` (a8f067b: `iter.map` raises on nil; a loop may assign the keys it walks) on the author's choice of options, with no explicit approval of the merge | decided 2026-11-22: it stays (his answer: "#117 sim"); measured: 48 configurations, no port C changes |
| 118 | `env-js` or `env-web` | answered 2026-11-24: `env-web` (the author created `ia-atmos/env-web`; session `env-web`) |
| 119 | A new session `atmos` that manages all the others; `atmos-lang` keeps the implementation | decided 2026-11-24 (his answer: "Ok"); session `atmos` created, handoff `261124-atmos.md`; from `#120` this index is kept by `atmos` |
