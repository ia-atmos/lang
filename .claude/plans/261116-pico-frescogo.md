# pico-frescogo: the scoreboard of FrescoGO! in Atmos, over pico

Plan for the repository `ia-atmos/pico-frescogo`, session
`pico-frescogo`. Written in the session that maintains the compiler
(`ia-atmos/lang`). The author discusses the open questions in the
session of the port, the serial port above all: nothing here decides
them.

## Context

- FrescoGO! is a scoreboard for presentations of frescobol: two
  athletes, a radar that measures the speed of each hit, a referee at
  the keyboard, a clock, sounds for each range of speed, and a report
  at the end. It is a program of the author.
- The original is one Processing sketch, `placar/placar.pde`, 1710
  lines of Java, version 5.3.0, at `1d2edb8`. The repository has only
  `master`.
- The port is to Atmos, over the compiler `ia-atmos/lang` (branch
  `main`; read `doc/manual.md` there before writing code) and over the
  environment of pico (`ia-atmos/env-pico`: `pico.atm`, built as the
  dedicated compiler `atmos-pico`).
- It differs from the other ports: it is not a game with a world of
  entities. It is a small state machine in time (rest, serve,
  sequence, fall, end), driven by three sources of input (the radar,
  the keyboard, the clock), with a screen that is redrawn from the
  state. That is the kind of program the language is for.
- **The caveat: it needs events from a serial port.** The radar
  speaks over a serial line, and no environment of this implementation
  has one. See "The serial port".

## What the original is made of

| Part | Where (`placar.pde`) | Lines | Note |
|------|----------------------|-------|------|
| Configuration | `setup`, 783-903; `data/conf.json` | 120 | A JSON file with 25 fields: times, limits, the rules of the score, the serial device, the names |
| The flow of a presentation | `go_esquenta`, `go_reinicio`, `go_saque`, `go_queda`, `go_terminando`, 196-340 | 145 | The transitions; the last one writes the report and a screenshot |
| The score | `_jogo_tempo`, `_jogo_lado`, `jogo_kmh`, `jogo_equ`, `jogo_calc`, 342-612 | 270 | Pure functions over the list of sequences of hits: no input, no screen |
| The radar | `radar_mock`, `radar_check`, `radar_be`, `radar`, 615-775 | 160 | Packets of 23 bytes that start with `0x88` (the format `bE`), at 9600 baud; a filter of repetitions and directions; a mock with no hardware |
| Sounds | `sound`, 904-937 | 35 | 15 files, one per range of speed and per moment |
| The keyboard | `keyPressed`, `keyReleased`, `trata_nome`, 939-1132 | 195 | Keys held for 2 seconds, the editing of the names, the modes |
| The loop | `draw`, 1134-1212 | 80 | Each frame: read the radar, apply the automatic fall, update, draw |
| Drawing | `draw_debug`, `draw_jogo`, `draw_logo`, `draw_nome`, `draw_ultima`, `draw_lado`, `draw_lado_medias`, 1214-1710 | 500 | Full screen, two fonts, images, bars |

What it takes from Processing: `fullScreen`, images and TTF fonts,
`processing.sound` (WAV, OGG, MP3, AIFF), `processing.serial`,
`loadJSONObject`, `saveStrings`, `saveFrame`, the date and the time of
the day, `millis`.

## Settled

Rules of the author for all sessions:

- Commit and push straight to the default branch (`master` here), in
  fine-grained commits: one small commit per change, pushed as you go.
  No other branch and no pull request, unless he asks.
- Keep `.claude/plans/TODO.md`: decisions that wait for him (each with
  a recommended option), work known and not done, what was reported
  and he did not acknowledge.
- A decision that belongs to him is asked, one at a time, with a
  recommended option.
- Talk to him in Portuguese, short, leading with the outcome.
  Documents and code in English. The texts of the program itself (the
  screen, the report) stay in Portuguese, as in the original.
- **Fidelity, then expressiveness**: first one to one with the
  original; once it is fully faithful, it may change in favor of
  expressiveness, as long as the intent is not affected.
- The events of an environment have no prefix (`:key.dn`), and an
  environment has a compiler of its own (`atmos-pico`): programs are
  built with it.
- What is found in the compiler goes to its session
  (`ceu-atmos`, session_011eECogKVQ7ZzmWCoMST2wK) by message, with a
  small program that reproduces it.

## The serial port

To be discussed with the author in the session of the port. What is
known, and the options.

**What the original does.** It opens the device once (the last one of
the list, or the one in `conf.json`), and each frame, in `draw`, asks
whether 23 bytes are available, reads one packet, and filters it. So
the serial port is *polled once per frame*, and never blocks.

**What exists here.**

- An environment is one file in Atmos with C in native literals,
  which defines `main` and runs the loop of a library. A program has
  one `main`: two environments cannot be combined today. This is the
  open item 57 of the list of the compiler ("how two environments
  live in one program"), met by `env-uv` and `env-iup`.
- **The primary and secondary architecture of lua-atmos is not
  available in this implementation.** There, each environment
  registers a table with `step` and `quit`; the first one that is
  loaded is the primary (it owns the clock and may sleep), and the
  others are secondary (their `step` polls with no wait, and emits no
  time). `lua-atmos/env-pico` and `env-socket` support both modes.
  Here, the only step in that direction is in `env-uv`, which offers
  one step of its loop as a function for another environment to
  drive.

**Options.**

| | Option | What it takes | Note |
|---|--------|---------------|------|
| A | The serial port as a small library of the port: natives over POSIX (`open`, `termios`, a read that does not block), polled by a task once per frame | Nothing from the compiler nor from pico | One to one with the original, which polls in `draw`. No event comes from outside: the task that polls is the source |
| B | A new environment, `env-posix` (file descriptors, the serial line, signals, processes), which emits `:serial` events | A way to combine it with pico: the primary and secondary modes, to be designed in the compiler (item 57) | The general answer, and this port is its first real case. Only for POSIX: the Mac and Linux, not Windows |
| C | `env-uv` with a serial line (a `uv_poll` over the descriptor) | The same combination as B, plus libuv | Portable to Windows only with more work (libuv does not open serial lines) |

A recommendation, for the author to decide: **A first**, to have the
port running and faithful, and use it as the case to design **B**
with him, since the way a second environment enters a program is his
to define. If B is chosen, the shape that lua-atmos already has is
the natural model: an environment file may define a step and a quit
instead of a `main`, the dedicated compiler receives a list of
environments, and the primary calls the steps of the others in its
loop.

In any option, **a mock comes first**: the original has
`radar_mock`, and the tests need a radar with no hardware. A scripted
radar (speeds and directions at given frames, as the scripted keys of
`PICO_ATM_KEYS`) gives deterministic tests, and the real serial line
is only tested by hand, by the author, with the device.

## Fidelity

There is no trace to compare with yet, and the original may not run
in the container (Processing, Java, a screen). In this order:

- **The score is pure.** `jogo_calc` and the functions under it take
  the list of sequences and the configuration and give the numbers of
  each athlete. They can be taken out of the sketch into a plain Java
  program, with no Processing, that reads a script of hits and prints
  the numbers. With a JDK, that is the oracle for the 270 lines that
  matter most: the same scripts, the same numbers, in Atmos.
- **The report is text.** `go_terminando` writes it with
  `saveStrings`: the same script of inputs must give the same file.
- **The filter of the radar** (`radar_be`) is a function of the
  packets and their times: the same bytes at the same times, the same
  accepted speeds.
- **The screen**: only if the original can be run with a virtual
  display (`processing-java`, `Xvfb`), compare screenshots at given
  moments. If not, the screen is checked by the author, by eye, and
  the tests fix the screenshots of the port from then on.
- A test mode with no person: fixed frames, a script of keys and of
  radar packets, a trace, screenshots (pico has `PICO_ATM_FRAMES`,
  `_KEYS`, `_SHOT`).

Installing packages (a JDK, Processing) needs the author's
permission.

## Milestones

- [ ] F0: repository: readme of the port (what it is, and that it is
      made with AI), `.claude/plans/TODO.md`, the setup that fetches
      the compiler and `env-pico` at fixed commits and builds
      `atmos-pico`. A window opens with the logo. The check of what
      runs in the container: pico with no screen, a JDK, Processing.
- [ ] F1: the oracle of the score: the functions of the score in
      plain Java, a format for scripts of hits, the first scripts with
      their expected numbers.
- [ ] F2: the configuration and the score in Atmos, identical to the
      oracle.
- [ ] F3: the flow of a presentation, with the keyboard only (the
      manual mode: the arrows mark the hits), the clock, and the
      sounds. The report, identical for the same script.
- [ ] F4: the screen: the game mode and the debug mode.
- [ ] F5: the radar: the filter over a scripted source of packets,
      then the serial line by the option that the author chose. The
      automatic mode (falls detected by silence).
- [ ] F6: the editing of the names, the remaining keys, the launcher.
      A full presentation, by the author, with the device.
- [ ] F7: after full fidelity: the same program written for
      expressiveness, and what changed.

## Questions that the port must answer early

Not decided here. They are the author's, in the session.

- **The serial port**: the options above.
- **The configuration**: the original reads JSON, and the language
  loads its own literals (`X.load`), not JSON. Either a small reader
  of JSON (in Atmos, or the C of a library), or `conf.json` becomes a
  file of the language. The first keeps the files of the users.
- **The sounds**: four formats (WAV, OGG, MP3, AIFF). Check what pico
  plays, and convert what it does not.
- **What pico lacks**: full screen, the size of a text, a screenshot
  from the program (the report saves one), the date and the time of
  the day for the names of the files. Ask the session of pico
  (`pico-sdl`, session_011eECoikzsmBiPNXNF7ffpf) before working
  around.
- **The directory `placar/`**: whether the port lives beside the
  original (`atmos/`) or replaces it. Beside, while fidelity is
  checked.

## What the other ports taught

Read "What the Pingus port taught" in `ia-atmos/lang`,
`.claude/plans/261111-snkrx.md`. And since 2026-11-15 the rules of
`pin` are new (`doc/manual.md`, "Pin" and "Pub";
`.claude/plans/261115-pin-migration.md`). What matters most here:

- A dynamic value that was just created is declared with `pin`:
  `pin conf = load(...)`, `pin seq = []`. A `val` only refers to a
  value that has an owner; a variable does not own (`set x = [...]`
  is an error: change the table in place, or keep it as a field of a
  pinned table). `vector.clear(t)` empties a vector, and
  `vector.insert` gives the element it stored.
- This program is mostly one large state (the list of sequences, the
  numbers of the two athletes): keep it in one pinned table of the
  main block, and let the tasks refer to it.
- A task spawned in the body of a loop, of an `if`, or of a branch of
  `match` is aborted when that block ends. The states of the flow
  (rest, serve, sequence, fall) are a sequence of awaits in one task,
  not tasks spawned from handlers.
- A key held for 2 seconds is `watching :key.up { await 2s ; ... }`.
  A fall by 3.5 seconds of silence is an await of the time that
  restarts at each hit. These are the places where the original keeps
  timers by hand (`KEY_TIMER`, `RADAR_AUTO_INICIO`): one to one
  first, then as awaits, in F7.
- Integers and reals are distinct (`7 // 2` is `3`, `7 / 2` is
  `3.5`); Java rounds and truncates in its own ways (`(vel + 5) /
  10` on integers): reproduce each.
- The time passes in pieces: a `:clock` await awakes at each timer
  that expires. What must run once per frame awaits `:frame` (check
  the name in `pico.atm`).
- C that the program calls fails by assigning to `st`; the names with
  the prefixes `a_`, `A_`, and `atm_` are reserved.

## Risks

- **The serial line cannot be tested here**: no device in the
  container. A pseudo-terminal (`openpty`) fed with recorded packets
  tests the code of the line; the real device is the author's.
- **No oracle for the screen**, if Processing does not run here.
- **The rules of the score change between versions** (the last
  commits of the original change them): port `1d2edb8`, and say so in
  the readme.
- **Windows**: the original runs wherever Processing does. A serial
  line over POSIX leaves Windows out: say it, and ask.

## Workflow

- Nothing of the serial port is built before the author chooses.
- Commits straight to `master`, small, pushed as you go.
- A commit only when the tests of the repository pass.
- Keep `.claude/plans/TODO.md` current.
- Findings for the compiler go to its session by message.
