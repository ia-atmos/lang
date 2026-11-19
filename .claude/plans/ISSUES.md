# Items of the session `atmos-lang`

Every item of this session, by its number. A number is never reused nor
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
| 3 | 32-bit reals (decided to do; the form is open: a build option or a type) | open |
| 4 | An `emit` that names its receiver, instead of `emit @2` | moved: `atmos-pool` |
| 5 | An optional `task main` as the entry point | closed 2026-11-17: no (decided in `atmos-envs`) |
| 6 | The event of termination of transparent blocks | open (recommended: remove) |
| 7 | Iterating a tagged table gives the field `tag` | closed: kept, as the reference |
| 8 | `await x` needs `await(x)`; `loop n {}` is not a counted loop | closed: became #31 |
| 9 | Indexing a string is an error (the reference gives `nil`) | closed: kept |
| 10 | The names `:value`, `X.nin`, `include` | closed; later `:value` became `:atmos` and `X.xin` became `X.in` (2026-11-17) |
| 11 | Mouse events in `lib/sdl.atm` | closed 2026-11-16: the library left for `env-sdl` |
| 12 | Tweens and springs as a library | work |
| 13 | The trace of an error with the tasks it crossed | open (recommended: one line per await) |
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
| 63 | A `pin` of a task that ends a loop body is not flagged | open (recommended: extend the check) |
| 64 | An architecture of snapshots and commands | moved: `paper-snapshots` |
| 65 | `t@0.x` does not compile | open (recommended: a dot belongs to a number only before a digit) |
| 66 | A paper on `pin` | moved: `paper-pin` |
| 67 | A paper on snapshots and commands | moved: `paper-snapshots` |
| 68 | FrescoGO! over pico | moved: `pico-frescogo` |
| 69 | The six exit names as keywords of the lexer | open (recommended: do it, after `atmos-args` merges) |
| 70 | Atmos on the ESP32 | moved: `env-esp32` |
| 71 | The numbering of items: `#N`, never reused, for everything put to the author | done 2026-11-18 (this file) |
| 72 | The tests of the projects with `test` blocks and emits: what still needs a trick | open (answered on 2026-11-18; the adoption is `atmos-envs`, its A3) |
| 73 | Two branches with nothing outside `main`, to delete: `lang` `ccr-6e380402-ae5ue6`, `sdl-pingus` `ccr-d59a6b51-1a4san` | open: the author's to do (a session cannot delete a branch) |
| 74 | No port has a `test` block: the twelve drive their tests from outside, with about ninety environment variables | open (remark; follows `#76`) |
| 75 | What a `test` block cannot do alone: keep the environment from opening a window or sound, set polled input, read pixels, reach a C callback | open (the adoption is `atmos-envs`, its A3) |
| 76 | A pilot: one port moves one test from a variable to a `test` block, to measure what is left | open: approved 2026-11-19, asked of `sdl-pingus` |
| 77 | Merge of `pin` b80bcd1 (`atmos-pin#11`: the warning of a pin given away sees `until c, v` and `while c, v`) | done 2026-11-19 (907fee9; approved by the author) |
