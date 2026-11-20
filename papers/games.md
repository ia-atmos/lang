# Paper outline

Working theme: structured concurrency mechanisms for games, evaluated on
ports of existing games to Atmos.

## 1. Introduction

- Context: games.
- Problem: control and logic of games.
- State of the art:
    - dynamic engines: LÖVE, Pygame, Phaser;
    - static: handmade C/C++.
- Proposal: structured concurrency, applied in ports.
- Evaluation: qualitative discussion and quantitative numbers
  (LOC, memory, CPU).

## 2. Structured concurrency

- General description.
- Languages and the synchronous hypothesis.
- Atmos.
- Patterns from the Pingus paper, now with examples from the ports.

## 3. Case Studies

Holds the evaluation and the discussion (no separate section).

- Table with measures and general screenshots.
- CPU is reported as two separate numbers: the whole frame, and the game
  logic alone.
- Simple static: games of raylib (`ia-atmos/env-ray`, 23 games).
  Comparable to Lessmilk in size, with originals in a static language.
- Simple dynamic: Lessmilk (`ia-atmos/ray-lessmilk`). Focus on learning
  and exploration.
- Complete dynamic: SNKRX (`ia-atmos/sdl-snxkx`). Gains in expressiveness
  even here, and even more in memory and CPU.
  Note of the session (2026-11-20, to be reviewed): as measured, the
  gain in CPU is small (1.07 to 1.22 times the ticks of the original,
  with the sound of both mixed), and memory is not a gain (both peaks
  are decoded sounds). The gain to claim is in tokens (68%) and in
  control state. See `papers/games-data.md`.
- Static: Pingus (`ia-atmos/sdl-pingus`), Flare (`ia-atmos/sdl-flare`).
  Expectation of the author, to be measured: the loss against C++ stays
  below 10-20%, which would already be an unexpectedly positive result.
  Argument to use: Sweeney, on accepting 10% of performance in exchange
  for better productivity (POPL 2006 talk; quote to verify).
- Specific engines: Doom (`ia-atmos/doom`).
  One paragraph on its saved game, as a limit of structured
  concurrency: the save prevented tasks in the simulation, which stays
  in C. It points to the paper on snapshots (`papers/snapshots.md`),
  which tells the whole story.

- Change study with AI (idea of the author, to be designed): the cost of
  the same change in the original and in the port, e.g. adding a new
  weapon to Doom. Measures: tokens spent by the AI, lines of code
  changed, and scattering of the change (files, functions, and hunks
  touched).

Other repository with ports, not in the paper so far: `ia-atmos/env-pico`
(binding of pico, three games). The compiler is `ia-atmos/lang`.

## 4. Related work

- FRP in games? (open)
- Godot-style engines, scripting languages, coroutines, Blueprints.
- Coroutines and async/await in games, framed as a trend that is still
  incomplete: the industry moves toward sequential code for control,
  while synchronous languages exist since the 1980s and already offer
  what is missing (parallel compositions, abortion, finalization).
  Examples to verify and cite:
    - Unity: coroutines (`IEnumerator`, `yield`, `StartCoroutine`), and
      async/await (`Awaitable`, UniTask);
    - Lua: coroutines in LÖVE (timer and tween libraries with scripts
      over coroutines), Roblox (`task.wait`, `task.spawn`), Defold;
    - Godot: `await` on signals in GDScript;
    - Unreal: latent actions in Blueprints (`Delay`).

## 5. Conclusion

---

# Alternative organization (proposed, not decided)

Difference: sections 2 and 3 of the outline above are split by role.
The patterns leave the language section and become the qualitative core,
organized by pattern and not by port. The ports become the quantitative
part, organized by question and not by port.

## 1. Introduction

Same as above.

## 2. Control in games

- The problem in concrete form: one running example taken from a port,
  in the original code (update loop, callbacks, explicit state machine).
- Why it is hard: state spread across frames, manual cancellation,
  lifetime of entities.

## 3. Structured concurrency in Atmos

- Synchronous hypothesis and the languages of the family.
- Mechanisms, short: await, parallel compositions, abortion, tasks and
  pools, events, finalization.
- The running example of section 2, rewritten.

## 4. Patterns

- The patterns of the Pingus paper, revised and extended.
- Each pattern: original code from a port, Atmos code, mechanism used,
  in which ports it appears.
- Examples drawn from all ports, so each port shows up where it is the
  best illustration.

## 5. Case Studies

- Subjects: the ports, chosen to cover two axes.

  |         | dynamic original | static original          |
  |---------|------------------|--------------------------|
  | simple  | Lessmilk         | raylib games (`env-ray`) |
  | complex | SNKRX            | Pingus, Flare            |

  Plus Doom as the case of a specific engine.
  Its saved game is one paragraph, as a limit of structured
  concurrency, pointing to the paper on snapshots.
- Method: how the ports were made, how each measure is taken.
- Questions:
    - Q1: do the patterns of section 4 recur across the ports?
      (occurrences per pattern per port)
    - Q2: how does the code change? (LOC, state machines and callbacks
      removed)
    - Q3: what is the cost? (memory, and CPU as whole frame and as
      game logic alone, against the original)
    - Q4: what does a change cost? (same change in original and
      port, made by an AI: tokens, lines changed, scattering)
- Table with all measures, screenshots.
- Threats to validity.

## 6. Related work

## 7. Conclusion
