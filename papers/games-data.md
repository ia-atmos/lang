# Games: the data of the ports

Collected on 2026-11-20 by the session paper-games from the documents of the ports; nothing was measured again. Draft, to be reviewed by the author.

How to read this file:

- Every number is copied from a file of a port, cited as `repo/path`.
  The commit of each repository is in the table of Subjects.
- A few numbers come from the plans of the ports in the compiler
  repository, cited as `lang/.claude/plans/...` (working tree of
  `/home/user/lang` on 2026-11-20). They are plans, written before the
  ports, and count differently from the documents of the ports.
- "(computed)" marks what this session derived: a ratio, the sum of a
  column, or a count of rows. Nothing else was derived.
- "not reported" means that no document of the port gives it.
- `sdl-snxkx` is a duplicate of `sdl-snkrx` (same commit) and is left
  out.

## 1. Subjects

| Port | Original game | Original language and engine | Size of the original | Scope of the port | Commit | Date of the commit |
|------|---------------|------------------------------|----------------------|-------------------|--------|--------------------|
| doom | Doom (id Software, 1993), through Chocolate Doom at `a2dfd51`, with the shareware `DOOM1.WAD` 1.9 (`doom/README.md`) | C; the engine of Doom itself, over SDL2 | `engine/` is a copy of 301 files (`doom/README.md`); `src/doom/`: 59 files of C, 45637 lines; "about 11 thousand lines, of 45 thousand" are control outside the saved game (`doom/.claude/plans/261113-doom.md`) | The engine stays in C. In Atmos: the loop of frames, the screen wipe, the intermission, the finale, the flow of the game (title, demos, levels), the face of the status bar (`doom/README.md`). No sound, no network | `096bf19` | 2026-11-18 |
| env-ray | The 23 games of `raysan5/raylib-games` at `318688c`: 12 classics and 11 others (`env-ray/README.md`) | C over raylib (tag `6.2` for both sides) | Per game, 143 to 4689 lines of code (`env-ray/doc/table.md`); plan: classics 4054 lines, the others 17819 lines and 58 MB of assets (`lang/.claude/plans/261112-raylib.md`) | All 23 games (`env-ray/README.md`) | `0d8b477` | 2026-11-20 |
| ray-lessmilk | The twelve games of "12 games in 12 weeks" by Thomas Palef, lessmilk.com, 2014 (`ray-lessmilk/README.md`) | JavaScript over Phaser: 1.1.3 for games 1 to 9, 1.1.6 for game 10, 2.0.1 for game 11, 2.0.2 for game 12 (`ray-lessmilk/doc/fidelity.md`) | 3287 lines and 37669 tokens in the twelve games (`ray-lessmilk/doc/table.md`); plan: 3388 lines of JavaScript (`lang/.claude/plans/261112-lessmilk.md`) | The twelve games per `doc/table.md`; the table "State" of the README lists eleven (no Connected); see Inconsistencies | `d8c293b` | 2026-11-19 |
| sdl-flare | Flare, `flareteam/flare-engine@56c747a` with `flareteam/flare-game@af6eee6` (`sdl-flare/README.md`) | C++ (`-std=c++98`) over SDL2 (`sdl-flare/doc/bench.md`) | Engine: 57843 lines of C++, 120 files (`lang/.claude/plans/261111-flare.md`); not in the documents of the port | A slice, milestones F0 to F5: INI reader and mods, engine, maps (24 maps of `alpha_demo`), the hero, enemies, powers, hazards, effects, loot, path finding. Not yet: the HUD and menus (F6), the Empyrean Campaign (`sdl-flare/README.md`) | `af5a1f4` | 2026-11-20 |
| sdl-pingus | Pingus. The target is the 2018 rewrite of the game logic from C++ to Céu, `fsantanna/pingus@036852a`; the timings and levels are compared with the official game, `Pingus/pingus@ffe3851` (`sdl-pingus/README.md`) | C++ (official); Céu over the C++ engine (2018 rewrite) | Official, only the units that were ported: 12794 lines and 77370 tokens of game logic, 652 of options, 7838 of engine; Céu: 5233 lines of game logic (`sdl-pingus/doc/loc.md`). Plan: official 53394 lines in `src/`; Céu 77 files, 8326 lines; C++ called by Céu 22106 lines (`lang/.claude/plans/261110-pingus.md`) | The complete game: P0 to P9 done; 522 of 523 levels offered; 21 actions, 22 objects; not done: input configuration, controllers (`sdl-pingus/README.md`) | `87d0229` | 2026-11-20 |
| sdl-snkrx | SNKRX by a327ex, `a327ex/SNKRX@6b93a64` (`sdl-snkrx/README.md`) | Lua over LÖVE 11 (LÖVE 11.5 with LuaJIT 2.1 in the benchmark, `sdl-snkrx/doc/bench.md`) | Game: 10381 lines, 134827 tokens; engine: 4433 lines, 41735 tokens (`sdl-snkrx/doc/case-study.md`); other numbers in `doc/bench.md`, see Size | "The game as played": menu, shop, 25 levels, 54 characters, 16 classes, 82 passives, options, save, tutorial, credits. Left out: Steam (`sdl-snkrx/README.md`) | `ae9c69f` | 2026-11-19 |
| sdl-tux | SuperTux, tag `v0.7.0` (`c11dfb2`) (`sdl-tux/README.md`, `sdl-tux/tools/bench/README.md`) | C++ with embedded Squirrel scripts, SDL renderer | 106697 lines of C++ in `src/`; data 324 MB; 110 levels (`sdl-tux/.claude/plans/261113-sdl-tux.md`) | A slice of World 1 (31 levels, 393 sprites, 3 tilesets as data; scripts 193 of 196). T0, T1, T2, T4 (small Tux) done; T3 and T5 in course (`sdl-tux/README.md`) | `dcd65d8` | 2026-11-20 |
| env-pico | Not a port of one game: a binding of `pico-sdl` with three example games; `exs/storm` is a port of `ceu-sdl-storm` (`env-pico/README.md`, `env-pico/exs/storm/README.md`) | Céu over SDL for `storm`; not reported for the others | not reported | `birds` (a tutorial in 11 steps), `rocks`, `storm` (`env-pico/exs/README.md`) | `69d9a7d` | 2026-11-20 |

## 2. Size

Question: is the port smaller than the original?

| Port | Original | Port | How it was counted | Source |
|------|----------|------|--------------------|--------|
| doom, intermission | 1219 lines (`wi_stuff.c`) | 622 lines (`wi.atm`), of which 56 are C in the `:prelude` | No blank lines, no comments | `doom/doc/study.md` |
| doom, finale | 528 lines (`f_finale.c`) | 445 lines (`fi.atm`), of which 183 are C | same | `doom/doc/study.md` |
| doom, flow | 125 lines: the "demo loop" of `d_main.c` (88) and the dispatch of `G_Ticker` (37) | 186 lines (`flow.atm`), of which 56 are C | same | `doom/doc/study.md` |
| doom, face of the status bar | 149 lines (`ST_updateFaceWidget`, `ST_calcPainOffset`) | 130 lines (`st.atm`), of which 43 are C | same | `doom/doc/study.md` |
| doom, screen wipe | not reported | not reported | | `doom/doc/study.md` |
| env-ray, 23 games | Per game, see the table below. Sum of the 23 rows (computed): 17854 lines, 146226 tokens | Sum of the 23 rows (computed): 8888 lines, 86766 tokens | `tools/count.py`: no blank lines, no comments; in C, no blocks for the web or Android; in Atmos, no trace; a port counts the files it includes (`classics/classic.atm`) as if inlined, not `ray.atm` | `env-ray/doc/table.md` |
| ray-lessmilk, twelve games | 3287 lines, 37669 tokens | 3534 lines, 28956 tokens: "8% more lines and 23% fewer tokens" | `tools/count.py`: no blank lines, no comments; in Atmos, no trace. JavaScript: all the files of the game in `js/`. Atmos: `games/<game>/`, without the shared `lib/` (2106 lines, 18743 tokens), which does what Phaser did; without `levels.atm` of Dark Blue (229 lines, 1434 tokens) and `map.atm` of The Three Hearts (92 lines, 4579 tokens) | `ray-lessmilk/doc/table.md` |
| sdl-flare | not reported | not reported | | |
| sdl-pingus, game logic | C++ 12794 lines, 77370 tokens; Céu 5233 lines, 38748 tokens | 2732 lines, 20336 tokens: "0.21x the lines and 0.26x the tokens of the C++, and 0.52x the lines and tokens of Céu" | `tools/loc.py`: lines not blank nor comments, preprocessor lines left out; tokens are identifiers, numbers, strings, tags, operators. Only the units that were ported; left out of all three: the editor, the recording of demos, the add-on, font and sprite screens, the configuration files | `sdl-pingus/doc/loc.md` |
| sdl-pingus, 17 actions in common | C++ 1320 lines, 7073 tokens; Céu 754 lines, 5985 tokens | 424 lines, 2877 tokens: "0.41x its C++ ... and 0.48x its Céu" by tokens | same | `sdl-pingus/doc/loc.md` |
| sdl-pingus, options | C++ 652 lines, 4811 tokens; Céu 598 lines, 4615 tokens | 177 lines, 1496 tokens: "0.31x the tokens of the C++" | same | `sdl-pingus/doc/loc.md` |
| sdl-pingus, engine | C++ 7838 lines, 47054 tokens; Céu 532 lines, 3365 tokens | 1219 lines, 10335 tokens (C and Atmos); "not like for like" | same | `sdl-pingus/doc/loc.md` |
| sdl-pingus, per part against Céu | Céu: engine 528, pingu core 517, actions (17) 879, objects (10) 864, game screen 937, other screens and main 3368 | Atmos: 407, 214, 423, 223 (plus 315 for 12 new objects), 607, 872; C of the port `engine/pingus.h` 727 lines against 22106 lines of C++ called by the Céu code | `grep -v` of blank lines and of comment lines, `wc -l` | `sdl-pingus/doc/loc.md` |
| sdl-snkrx, game (case study) | 10381 lines, 134827 tokens | 8647 lines, 83761 tokens: 83% and 62% | "counted without comments or blank lines"; the original is "the ten Lua files of the game" | `sdl-snkrx/doc/case-study.md` |
| sdl-snkrx, game without `main.lua` and `data.atm` | 8422 lines, 113682 tokens | 6982 lines, 69111 tokens: 83% and 61% | same | `sdl-snkrx/doc/case-study.md` |
| sdl-snkrx, engine (case study) | 4433 lines, 41735 tokens (`engine/` without `engine/external`) | 2128 lines, 18826 tokens: 48% and 45% | same | `sdl-snkrx/doc/case-study.md` |
| sdl-snkrx, game (bench) | 10374 lines, 134786 tokens (nine files named) | 4755 lines, 43606 tokens (`src/`, `main.atm`); "the port covers a part of the game (S0 to S7)" | `tools/count.py`: no blanks, no comments | `sdl-snkrx/doc/bench.md` |
| sdl-snkrx, engine (bench) | 4433 lines, 40853 tokens | 2080 lines, 18476 tokens (`eng/`, Atmos and C) | same | `sdl-snkrx/doc/bench.md` |
| sdl-tux | 106697 lines of C++ (`src/`, no blanks nor comments), the whole game | not reported; `game/player.h`, the whole `Player` in C, has 3654 lines | | `sdl-tux/.claude/plans/261113-sdl-tux.md`, `sdl-tux/.claude/plans/TODO.md` |
| env-pico | not reported | not reported | | |

Notes of the ports on the measure:

- ray-lessmilk: "Tokens are the measure that matters: lines depend on
  the layout" (`ray-lessmilk/doc/table.md`).
- sdl-pingus: "Tokens are a fairer measure than lines"; the pingu core
  has the largest gain, 0.15x the tokens of the C++
  (`sdl-pingus/doc/loc.md`).
- sdl-snkrx: 13.0 tokens per line in the Lua against 9.7 in the port;
  1647 lines of the port hold only `}` and the like (19%), against 1467
  lines of only `end` in the original (14%)
  (`sdl-snkrx/doc/case-study.md`).
- doom: of the 57 static variables of the intermission, 25 hold
  graphics (150 lines); the port asks the WAD by name at each draw, "a
  choice that C could make as well, and not a gain of the language"
  (`doom/doc/study.md`).

### env-ray, per game (`env-ray/doc/table.md`, copied)

| Game | Lines C | Lines Atmos | Tokens C | Tokens Atmos | Control C | Control left | Runs | Same |
|------|--------:|------------:|---------:|-------------:|----------:|-------------:|-----:|------|
| snake | 181 | 123 | 1509 | 958 | 5 | 0 | 2 (1220 frames, 16 screens) | relaxed |
| floppy | 143 | 119 | 1054 | 699 | 4 | 0 | 2 (2746 frames, 25 screens) | yes |
| gold_fever | 178 | 164 | 1470 | 1239 | 6 | 1 | 2 (1369 frames, 27 screens) | relaxed |
| arkanoid | 207 | 149 | 2005 | 1312 | 4 | 0 | 2 (22977 frames, 23 screens) | yes |
| asteroids_survival | 241 | 181 | 2262 | 1470 | 3 | 0 | 2 (4531 frames, 22 screens) | yes |
| space_invaders | 271 | 198 | 1903 | 1297 | 9 | 1 | 2 (5477 frames, 26 screens) | relaxed |
| missile_commander | 343 | 162 | 2796 | 1443 | 10 | 1 | 2 (6812 frames, 34 screens) | relaxed |
| gorilas | 351 | 174 | 2994 | 1627 | 6 | 0 | 2 (1008 frames, 44 screens) | relaxed |
| asteroids | 402 | 261 | 3940 | 2176 | 7 | 1 | 2 (1249 frames, 25 screens) | yes |
| pang | 446 | 209 | 4072 | 1791 | 8 | 2 | 2 (2998 frames, 38 screens) | yes |
| platformer | 466 | 169 | 3277 | 1770 | 4 | 0 | 2 (1634 frames, 43 screens) | relaxed |
| tetris | 577 | 340 | 4370 | 2902 | 11 | 2 | 2 (10000 frames, 26 screens) | relaxed |
| raylife | 196 | 180 | 1506 | 1411 | 1 | 1 | 2 (1400 frames, 33 screens) | yes |
| wave_collector | 679 | 274 | 4998 | 2251 | 7 | 1 | 3 (13723 frames, 46 screens) | relaxed |
| repair | 672 | 385 | 5601 | 3855 | 5 | 1 | 2 (3350 frames, 45 screens) | relaxed |
| cat_vs_roomba | 857 | 334 | 7312 | 3556 | 5 | 1 | 2 (13420 frames, 31 screens) | relaxed |
| light_my_ritual | 901 | 281 | 7087 | 2455 | 5 | 2 | 2 (8700 frames, 34 screens) | relaxed |
| transmission | 1125 | 652 | 6898 | 5579 | 6 | 3 | 2 (7036 frames, 37 screens) | yes |
| drturtle | 293 | 223 | 2416 | 1888 | 4 | 1 | 2 (3700 frames, 17 screens) | relaxed |
| just_do | 1454 | 706 | 9350 | 6535 | 5 | 4 | 2 (4801 frames, 47 screens) | yes |
| skully_escape | 2685 | 404 | 18429 | 4097 | 6 | 3 | 3 (11514 frames, 60 screens) | relaxed |
| retro_maze_3d | 497 | 435 | 4185 | 3355 | 5 | 4 | 3 (7092 frames, 58 screens) | relaxed |
| koala_seasons | 4689 | 2765 | 46792 | 33100 | 8 | 6 | 8 (8854 frames, 62 screens) | yes |

### ray-lessmilk, per game (`ray-lessmilk/doc/table.md`, copied)

| # | Game | Phaser | Lines JS | Lines Atmos | Lines % | Tokens JS | Tokens Atmos | Tokens % | Control JS | Control left | Runs | Same |
|--:|------|--------|---------:|------------:|--------:|----------:|-------------:|---------:|-----------:|-------------:|-----:|------|
| 1 | Run! | 1.1.3 | 144 | 142 | -1 | 1502 | 1025 | -32 | 4 | 0 | 3 (3900 frames, 12 games) | yes |
| 2 | Pixel War | 1.1.3 | 198 | 238 | +20 | 2188 | 1546 | -29 | 5 | 1 | 2 (6000 frames, 9 games) | yes |
| 3 | Box Jump | 1.1.3 | 185 | 214 | +16 | 2746 | 2291 | -17 | 3 | 2 | 2 (5400 frames, levels 1 to 4, 56 deaths) | yes |
| 4 | Man vs Penguins | 1.1.3 | 308 | 306 | -1 | 3566 | 2503 | -30 | 6 | 3 | 2 (8400 frames, many deaths) | yes |
| 5 | Fill the Holes | 1.1.3 | 363 | 416 | +15 | 4875 | 4015 | -18 | 3 | 2 | 2 (6659 frames, 6 levels solved) | yes |
| 6 | Princess Quest | 1.1.3 | 403 | 366 | -9 | 4306 | 2916 | -32 | 5 | 4 | 2 (8387 frames, 5 games) | yes |
| 7 | Connected | 1.1.3 | 334 | 357 | +7 | 3386 | 2633 | -22 | not counted | not counted | 2 (13033 frames, 3 games) | yes |
| 8 | Save the City | 1.1.3 | 176 | 194 | +10 | 1896 | 1598 | -16 | 2 | 1 | 2 (2162 frames, 2 cities gone, a restart) | yes |
| 9 | I Hate Rabbits | 1.1.3 | 252 | 247 | -2 | 3161 | 2250 | -29 | 3 | 1 | 2 (9200 frames, 5 games) | yes |
| 10 | Dark Blue | 1.1.6 | 307 | 421 | +37 | 3268 | 3252 | 0 | 4 | 3 (+1) | 3 (9900 frames, levels 1 and 2, 30 deaths) | yes |
| 11 | The Three Hearts | 2.0.1 | 331 | 339 | +2 | 3479 | 2590 | -26 | 3 | 3 | 2 (1800 frames, a death, a key, four doors, a heart) | yes |
| 12 | Crazy Snake | 2.0.2 | 286 | 294 | +3 | 3296 | 2337 | -29 | 4 | 3 (+1) | 2 (9800 frames, score 52, 2 games) | yes |

### sdl-pingus, per unit (`sdl-pingus/doc/loc.md`, copied)

| Unit | C++ lines | Céu lines | Atmos lines | C++ tokens | Céu tokens | Atmos tokens |
|------|-----------|-----------|-------------|------------|------------|--------------|
| action basher | 133 | 94 | 62 | 653 | 649 | 391 |
| action blocker | 80 | 40 | 16 | 453 | 345 | 136 |
| action bomber | 64 | 36 | 34 | 413 | 317 | 255 |
| action bridger | 192 | 134 | 47 | 929 | 954 | 292 |
| action climber | 83 | 33 | 22 | 455 | 313 | 127 |
| action digger | 93 | 52 | 39 | 453 | 374 | 236 |
| action drown | 37 | 11 | 4 | 172 | 104 | 24 |
| action exiter | 40 | 12 | 5 | 181 | 116 | 26 |
| action faller | 114 | 69 | 40 | 714 | 557 | 333 |
| action floater | 58 | 17 | 12 | 281 | 143 | 67 |
| action jumper | 41 | 28 | 11 | 230 | 237 | 84 |
| action laser_kill | 37 | 12 | 4 | 176 | 67 | 27 |
| action miner | 87 | 64 | 34 | 516 | 576 | 254 |
| action slider | 58 | 36 | 20 | 350 | 296 | 147 |
| action splashed | 38 | 13 | 6 | 184 | 105 | 35 |
| action waiter | 40 | 15 | 5 | 180 | 99 | 29 |
| action walker | 125 | 88 | 63 | 733 | 733 | 414 |
| action angel | 40 | --- | 11 | 230 | --- | 104 |
| action boarder | 71 | --- | 28 | 470 | --- | 237 |
| action smashed | 37 | --- | 4 | 177 | --- | 27 |
| action superman | 40 | --- | 8 | 218 | --- | 81 |
| pingu core | 1773 | 663 | 221 | 10047 | 5257 | 1529 |
| world objects, weather | 1767 | 744 | 540 | 13600 | 5579 | 4436 |
| game: world, panel, playfield, level, ground | 2965 | 663 | 664 | 17883 | 4914 | 4838 |
| screens: menu, start, result, story, credits, levels, worldmap | 4781 | 2409 | 832 | 27672 | 17013 | 6207 |
| **total, game logic** | **12794** | **5233** | **2732** | **77370** | **38748** | **20336** |
| options (all but the renderer) | 652 | 598 | 177 | 4811 | 4615 | 1496 |
| engine (C++ engine / Céu engine / C + Atmos) | 7838 | 532 | 1219 | 47054 | 3365 | 10335 |

## 3. Control state

Question: how many state variables, flags, timers, state machines, and
callbacks did the port remove?

| Port | In the original | In the port | What stayed, as the port says | Source |
|------|-----------------|-------------|-------------------------------|--------|
| doom, intermission | 57 variables of the file (`static`), of which 25 hold graphics | 3 (`WI`, `WI_POINTER`, `WI_FRAGS`) | `sp_state`, `cnt_pause`, `acceleratestage`, `dm_state`, `ng_state`, `state` are gone; `WI_POINTER` and `WI_FRAGS` are globals that keep quirks of the original | `doom/doc/study.md` |
| doom, flow | 3 variables named (`demosequence`, `pagetic`, `pagename`) and a `switch` on `gameaction` | none | The decisions (next level, victory) stay in C, in `G_DoCompleted` | `doom/doc/study.md` |
| doom, finale | `finalestage`, `finalecount` | counts not reported | The cast is "still that machine, with the same six variables"; "three more globals for quirks of the original" | `doom/doc/study.md` |
| doom, face | `priority`, `lastattackdown`, `lastcalc`, `oldhealth`, `st_facecount` | `prio`, `count`, `rampage`, local to the task `Face` | "the same machine"; six variables of `st_stuff.c` lost their `static` | `doom/doc/study.md` |
| doom, wipe | `static boolean wipe`, the array `y[]`, the flag `done` | counts not reported | An `await :frame` after `await :all cols` whose reason needs a comment | `doom/doc/study.md` |
| env-ray, 23 games | 134 control variables | 38 left (`doc/case-study.md`); the column "Control left" of `doc/table.md` sums to 35 (computed); see Inconsistencies | Flags that the C reads at many points of one frame and changes at others; koala_seasons leaves 6 of its 8 | `env-ray/doc/case-study.md`, `env-ray/doc/table.md`, list in `env-ray/doc/state.md` |
| ray-lessmilk | Per game, 2 to 6 control variables; Connected "not counted"; no total given | Per game, 0 to 4 left; Dark Blue and Crazy Snake add 1 flag each ("(+1)") | Flags read and written in the same update, in the order of the original; groups of recycled slots | `ray-lessmilk/doc/table.md`, list in `ray-lessmilk/doc/state.md` |
| sdl-flare | No counts. Enums `cur_state` of the hero and of the enemies, timers, flags are listed by name | No counts. The enums became tasks per action; effects became a task each | Timers of the enemy and of the hero (`cooldown`, `casting`, ...) are "still state"; the perception flags stay as fields; hazards stay a list walked per frame | `sdl-flare/doc/state.md` |
| sdl-pingus, 17 actions | C++ 24; Céu 5 | 0 | | `sdl-pingus/doc/state.md` |
| sdl-pingus, 10 world objects of Céu | C++ 13; Céu 3 | 0 | | `sdl-pingus/doc/state.md` |
| sdl-pingus, panel and game | C++ 9; Céu 4 | 0 | What remains mutable is listed as data (`p.act`, `g.next`, `late` of the clock, ...) | `sdl-pingus/doc/state.md` |
| sdl-snkrx | 70 variables that guard a phase or compute a time (with three groupings counted as one each) | 36 gone; 34 became 34 variables: 18 fields that other tasks read, 6 counts, 6 locals of a task, 4 fields of the run or of the HUD | "none guards a phase of the task that sets it" | `sdl-snkrx/doc/state.md`, `sdl-snkrx/doc/case-study.md` |
| sdl-tux | Whole game: 348 uses of state enums and `m_state`, and 108 `Timer` members, in `badguy/`, `object/`, `supertux/` | not reported | The timers and phases of `Player` "are still flags and timers in the struct" in C | `sdl-tux/.claude/plans/261113-sdl-tux.md`, `sdl-tux/.claude/plans/TODO.md` |
| env-pico | not reported | not reported | | |

The control columns per game of env-ray and ray-lessmilk are in the
tables copied in section 2.

## 4. Mechanisms and patterns

Question: which mechanisms replace the control state, and how often?

Only sdl-snkrx counts occurrences in the port. The others name the
patterns and show one case of each.

| Port | Patterns or mechanisms named | Occurrences reported | Source |
|------|------------------------------|----------------------|--------|
| doom | A variable that became control (`await Wipe()`); a task per column of the wipe; a sequence of awaits under `watching` for the counters; one task `WI_Count` for three copies of code; a task per animation; `watching :title.next` per step of the title; the flow as a loop of awaits | Not counted. Removed from the engine: `D_RunFrame` (48 lines), the "demo loop" (138 lines), the loop of actions of `G_Ticker` (36 lines); `watching :title.next` is repeated "the same five times" | `doom/doc/study.md`, `doom/doc/seam.md` |
| env-ray | Nine sections: 1 a counter of frames becomes an await; 2 a flag that allows something once becomes a sequence; 3 a flag of activity becomes the life of a task; 4 pause and game over become a toggle and a block; 5 the enum of screens becomes tasks in sequence; 6 the transition becomes two loops; 7 copies of one file become one task and tables; 8 where the pattern stops; 9 what fidelity cost | Not counted per pattern. Pattern 4 is written once for the 12 classics; pattern 6 replaces five variables; pattern 7: seven rooms of skully_escape, seven copies of one file of 400 lines, become one task | `env-ray/doc/case-study.md` |
| ray-lessmilk | Comparisons with the clock become awaits; states of Phaser become a loop of awaits; a task per enemy in a pool; tweens as the task `Motion` | Not counted in the port. In the originals: `Linear.None` 35 tweens, `Bounce.Out` 10, `Quadratic.In` 1; Phaser 1.1.3, "used by nine of the twelve games, has no timers" | `ray-lessmilk/doc/games/run.md`, `ray-lessmilk/doc/fidelity.md` |
| sdl-flare | A task per action of the hero and of the enemy; a task per effect in a pool of its entity | not reported | `sdl-flare/doc/state.md` |
| sdl-pingus | The five patterns of the 2018 study, in seven cases: finite state machines (2 cases), continuation passing (2), dispatching hierarchies (1), lifespan hierarchies (1), signaling mechanisms (1); a table maps 14 constructs of Céu to Atmos (computed count of rows) | Not counted | `sdl-pingus/doc/README.md` |
| sdl-snkrx | Six patterns for the timers: `after` nested as a sequence; `every` as a loop; `cooldown` as a task; a tagged timer restarted as `par :any`; a tagged timer from many places as a token; a timer that undoes an effect as a task with `defer` | Original: 422 calls of the timer in the eight files of the game: `after` 223, `every` 83, `every_immediate` 18, `tween` 63, `cooldown` 32, `during` 3; 5 cancels by tag; 417 lines call a timer and 167 more close a callback (6% of its lines). Port: 370 `await` in 27 files, 59 `par`, 647 `spawn`, 51 `emit`; 51 pools, 98 `spawn { }`; 1172 lines with `await`, `par`, `spawn`, `watching`, `emit`, `defer`, or `loop ... on` (14%) | `sdl-snkrx/doc/case-study.md` |
| sdl-tux | Objects of a sector as tasks in a pool; one task `BadGuy` for the base, with the overrides as local functions that test the class | not reported | `sdl-tux/.claude/plans/TODO.md` |
| env-pico | not reported | not reported | |

## 5. Fidelity

Question: is the port the same game as the original?

| Port | Method | Result | Source |
|------|--------|--------|--------|
| doom | A test platform with no window, one tic per frame, input from a script; one line per frame with a checksum of the simulation and a checksum of the 320x200 screen; the recorded trace comes from the engine all in C. Tests: the three demos, `title`, `newgame`, `level`, `save`, `skip`, `dm`, `net`, `finale`, `endgame`, `death` | "The thirteen tests are identical". Demo lengths: 5026, 3836, 2134 tics. Not verified: episodes 2 to 4, Doom 2, frags above zero, more than one player, the secret exit, the cast of the finale ("ported by reading the C, and have never run"), more than one tic of the clock per frame in the wipe | `doom/README.md`, `doom/doc/study.md`, `doom/doc/bench.md`, `doom/.claude/plans/TODO.md` |
| env-ray | raylib patched with a test mode (fixed seed, fixed time step, scripted input, screenshots), with no screen; the originals patched to print a trace, one line per frame; traces compared line by line and screens pixel by pixel | 55 scripted runs (`README.md`). Per game: 2 to 8 runs, 1008 to 22977 frames, 16 to 62 screens (`doc/table.md`). 9 games "yes" and 14 "relaxed" (computed counts of the column "Same"): a relaxed port is identical up to the branch `faithful` (`e0b6a01`) and then changed on purpose; the differences are listed per game, for example snake: 42 lines of the trace of `play` and 26 of `self`, 3 of 12 and 1 of 4 screens differ. Sum of the column "Runs" (computed): 55 runs, 145611 frames, 819 screens | `env-ray/README.md`, `env-ray/doc/table.md`, `env-ray/doc/fidelity.md` |
| ray-lessmilk | The originals run in Chromium with no screen, driven by Playwright, with a seeded `Math.random` and a clock stepped by frame; the port has the same clock and generator; one trace line per frame, compared line by line; the same run file drives both. Text is "compared by eye, not pixel by pixel" | Every game in the table: "yes", on 2 or 3 runs each, 1800 to 13033 frames per game (`doc/table.md`). "All 31 runs of the twelve games" give the same traces on the site of today as on the copy of 2014 (`doc/fidelity.md`). Not reached in The Three Hearts: the second and third hearts, the end, the last door | `ray-lessmilk/doc/table.md`, `ray-lessmilk/doc/fidelity.md` |
| sdl-flare | The original patched (`tools/bench/original.patch`): seeded generator in the place of `rand()`, scripted input by logic frame, a trace of events (map, spawn, state, hit, power, loot) and optionally of the hero in every frame; traces compared line by line; screens compared pixel by pixel with no HUD and no hero | "30 scripted runs (walks, a fight, duels and stands in 13 maps) give the same trace as the original, with its draws"; the 24 maps of `alpha_demo` give the same pixels; all text files of `alpha_demo` and `fantasycore` read exactly as the original reads them. Differences: "None in the runs that the port plays" | `sdl-flare/README.md`, `sdl-flare/doc/fidelity.md` |
| sdl-pingus | The official game patched to print the same trace as the port (a line per change of action: time, pingu, action, x, y); the 159 demos of the data replayed in both and the traces compared | 149 of the 154 demos that the official game plays give the same trace ("every pingu, every change of action, at the same tick and the same pixel, up to the tick 39000"); 5 differ, by one pixel or one tick, the first at tick 1105; 5 demos do not load in the official game. History: 39, then 142, then 145, then 149. Levels solved by the demos in the port: 12 solved, 141 failed, 6 with no result; "only 38 of the 154 ... save any pingu" in the official game | `sdl-pingus/doc/fidelity.md` |
| sdl-snkrx | The same scripted input and seed in both; traces of both programs (`tst/original/run1.txt`, `tools/compare.py`); "faithful by events and formulas, not tick by tick (Box2D 3.1 against the 2.3 of LÖVE)" | With a click at tick 62, the shop starts at tick 113 in both. The first marker of level 1 comes 269 ticks after the arena starts in the port (`doc/case-study.md`) or 270 (`doc/study.md`), 272 in the original; its enemies 3 ticks earlier, 6 ticks apart in both. No count of identical runs | `sdl-snkrx/doc/study.md`, `sdl-snkrx/doc/case-study.md`, `sdl-snkrx/.claude/plans/TODO.md` |
| sdl-tux | The official game patched with a test mode (seeds, scripted input, a trace per tick of the player and per change of state of a badguy); traces compared line by line; frames compared pixel by pixel | "Thirty scripted runs, of up to 3000 ticks, give the trace of the official game line by line" (`README.md`); "40 runs identical" and "29 runs identical with the lines `B` and `E` of every badguy" (`TODO.md`). A frame of `journey_begins` is identical pixel by pixel, "but for the blocks and the badguys"; 219 pixels differ at the edge of one tilemap, and 207 in another case | `sdl-tux/README.md`, `sdl-tux/.claude/plans/TODO.md`, `sdl-tux/tools/bench/README.md` |
| env-pico | not reported | not reported | |

## 6. Tests

| Port | Test suite | Result | Source |
|------|-----------|--------|--------|
| doom | 13 tests, each a trace of a run against the trace of the engine all in C; `make test-c` runs the same on the engine with no Atmos | "The thirteen tests are identical"; which branches each test reaches "was not measured" | `doom/README.md`, `doom/doc/study.md` |
| env-ray | 55 scripted runs, against stored results (`tst/run.sh`) | Identical to the originals, or to the stored results of the relaxed ports | `env-ray/README.md`, `env-ray/doc/fidelity.md` |
| ray-lessmilk | "the 33 tests give the same traces" (`doc/compiler.md`; 26 tests at an earlier date); 31 runs of the games (`doc/fidelity.md`) | Same traces. Run! with the address sanitizer: its three runs (3900 frames, 12 games) with no error and no leak | `ray-lessmilk/doc/compiler.md`, `ray-lessmilk/doc/fidelity.md` |
| sdl-flare | The scripted runs of `tst/runs.txt` (30 per the README), and tests of the INI reader and of the animations; `ASAN=1 tst/run.sh` | Same traces. The address sanitizer on the fight run "reports 132 KB in 1030 allocations" not freed, cause not found | `sdl-flare/README.md`, `sdl-flare/doc/todo.md` |
| sdl-pingus | `tst/run.sh`, about 1 minute: the data, the engine, screenshots, the game played with the mouse, 25 scripted levels; `tst/demos.sh`, about 15 minutes: the 159 demos | "The address sanitizer finds no error and no leak in the tests" | `sdl-pingus/README.md`, `sdl-pingus/doc/compiler.md` |
| sdl-snkrx | "40 tests", with traces and screenshots | "40 tests green" (at `3a6ffe9`) | `sdl-snkrx/.claude/plans/TODO.md`, `sdl-snkrx/doc/compiler.md` |
| sdl-tux | `tst/run.sh`; the scripted runs of `tst/runs.txt` | "the suite is identical" with 32-bit reals | `sdl-tux/README.md`, `sdl-tux/.claude/plans/TODO.md` |
| env-pico | `make tests`: tests of the binding in `tst/`; size not reported | not reported | `env-pico/README.md` |

## 7. Performance

Question: what does the port cost in memory and CPU?

| Port | Baseline | Machine and conditions | Memory | CPU, whole frame | CPU, game logic alone | Frames with no cap | Source |
|------|----------|------------------------|--------|------------------|-----------------------|--------------------|--------|
| doom | The engine all in C (`out/doom-c-tst`) | Intel Xeon at 2.10 GHz (a cloud container), gcc 13.3, `-O2`; no window, no sound; best of 7 runs, 7 ms of start taken out; error of about 5% | not reported | With rendering of the 320x200 buffer, C against Atmos (seam with nothing ported, 9 runs): demo1 295 against 303 ms, demo2 222 against 237 ms, demo3 132 against 134 ms | With no rendering: demo1 22 against 25 ms, demo2 21 against 20 ms, demo3 13 against 14 ms: "No difference above the error of the measure". After the face was ported: 33 ms (C) against 44 ms (Atmos) on demo1 (`TODO.md`, item 51) | Baseline in C: 209, 202, 142 thousand tics/s with no rendering; 16, 15, 14 thousand tics/s with rendering | `doom/doc/bench.md`, `doom/.claude/plans/TODO.md` |
| env-ray | The original C games, same raylib, both at `-O2` | Intel Xeon at 2.10 GHz; test mode of raylib, software renderer into memory, no frame cap, no sound device; one scripted run cut at 1200 frames; median of 3 | Peak resident set per game, 12.6 to 52.9 MB in C and 12.6 to 54.3 MB in Atmos; raylife 22.4 MB in C against 15.2 MB in Atmos | CPU per frame per game, 0.66 to 33.85 ms in C and 0.64 to 34.16 ms in Atmos; ratio per game 0.93 to 1.20; "the sum of the CPU per frame of the ports is 1.02 times that of the originals" | Not separated. Stress of space_invaders (150 frames, median of 3), CPU per frame, Atmos/C: 0.99 at 10 enemies, 0.98 at 100, 1.34 at 1000, 1.70 at 5000; each enemy adds 0.75 µs per frame in C and 1.39 µs in the port | Per game, 29 to 1446 fps in C and 29 to 1546 fps in Atmos | `env-ray/doc/bench.md` |
| ray-lessmilk | not reported | not reported (`doc/table.md` points to a `bench.md` that does not exist) | not reported | not reported | not reported | not reported | `ray-lessmilk/doc/table.md` |
| sdl-flare | Only the original is measured (F0, 2026-11-11) | 4 cores of an Intel Xeon at 2.10 GHz, g++ 13.3 `-O2`, software renderer, dummy video and audio; 600 logic frames; two runs | Original: 73.3 MB (title) to 489.5 MB (`frontier_plains`). Port: not reported | Original: CPU 4.48 to 9.71 s capped for 600 frames. Port: not reported | Port: only `ini.load`, 0.002 to 0.006 s per map file | Original: wall 3.96 to 8.71 s for 600 frames uncapped. Port: not reported | `sdl-flare/doc/bench.md` |
| sdl-pingus | The official game at `ffe3851`, release build, `sdl` renderer | Container with no screen, Ubuntu 24.04, 4 cores, SDL2 2.30 software renderer, sound off; 10 s each; cap of 40 fps unless lifted | Port against official: menu 41.3 against 39.5 MB; tutorial level 37.5 against 34.9 MB; 100 pingus 37.3 against 34.8 MB ("1.05x ... in the menu and 1.07x in a level") | CPU in 10 s at 40 fps, port against official: menu 0.97 against 1.09 s; level 0.88 against 1.02 s; 100 pingus 0.92 against 0.91 s. A frame costs 1.46 ms in the port and 1.18 ms in the official game | Not separated in the game. Micro benchmark: a wake of a task costs about 55 ns (2000 tasks, 1000 events, 0.11 s) | Frames in 10 s, port against official: level 6850 against 8472; 100 pingus 6999 against 8957 ("1.2x to 1.3x the frames of the port") | `sdl-pingus/doc/bench.md` |
| sdl-snkrx | The original on LÖVE 11.5 with LuaJIT 2.1 | 4 cores of an Intel Xeon at 2.10 GHz, 16 GB; Xvfb 960x540, OpenGL over Mesa llvmpipe (renders on the CPU, in threads), every frame drawn, no cap; median of 3 runs; port at `b2c2089` | Peak, port against original: menu 130.6 against 946.2 MB; shop 131.7 against 946.1; level 1 131.6 against 946.2; stress 140.8 against 946.9. "Why the original needs that much was not investigated" | CPU of the process (with the threads of llvmpipe), port against original: menu 10.24 against 18.13 s; shop 11.17 against 18.72; level 1 29.13 against 40.86; stress 29.58 against 42.37 ("56% to 71%") | Port only, 1500 ticks: level 1 logic 0.23 s, draw 0.35 s, render 8.10 s; stress logic 3.90 s, draw 0.56 s, render 8.57 s; about 0.15 ms of logic per tick with few objects, about 4 ms with the crowd | Ticks per second, port against original: menu 161 against 54; shop 176 against 64; level 1 172 against 91; stress 113 against 63 ("3.0 ... 2.8 ... 1.9 ... 1.8" times) | `sdl-snkrx/doc/bench.md` |
| sdl-tux | Only the official game is measured (2026-11-14) | Container, 4 cores, software renderer over the dummy video driver, window of 1280x800 | Official: 142.2 to 142.5 MB in `journey_begins`, 223.6 to 223.7 MB in `crystal_mine`. Port: not reported | Official, 2000 ticks with no pacing, drawing: 7.89 s of CPU; a frame costs about 3.6 ms to draw. Port: not reported | Official, 2000 ticks with no drawing: 0.75 s of CPU. Port: not reported | Official: 276 frames per second with no pacing. Port: not reported | `sdl-tux/tools/bench/README.md` |
| env-pico | not reported | not reported | not reported | not reported | not reported | not reported | |

Other measures:

- env-ray, binary size: 786 to 1644 KB in C, 1998 to 3393 KB in Atmos
  (`env-ray/doc/bench.md`).
- sdl-pingus, binary stripped: 1.5 MB for the port, 3.0 MB for the
  official game; time to the first frame 93 ms; `X.load` of the largest
  level 2 ms; build 24 s (`sdl-pingus/doc/bench.md`).
- sdl-pingus, the port alone: 100 pingus in fast forward 56.8 MB and
  1.98 s of CPU in 10 s; uncapped 4958 frames in 10 s; armageddon at
  5 s 62.0 MB (`sdl-pingus/doc/bench.md`).
- sdl-snkrx, a profile of the stress with `callgrind`, on a port whose
  units did not attack: 52% of the instructions under `separate` and
  23% under `phys_step`; the two are "about 70% of the logic of the
  stress" (`sdl-snkrx/doc/bench.md`).
- sdl-flare, the original: binary of 2235736 bytes patched; build of
  43 s (`sdl-flare/doc/bench.md`).
- sdl-tux, the official game: binary of 10.4 MB (8.8 MB stripped); the
  largest level loads with `X.load` in 18 ms in the port
  (`sdl-tux/tools/bench/README.md`, `sdl-tux/.claude/plans/TODO.md`).

### env-ray, per game (`env-ray/doc/bench.md`, copied)

| Game | Run | Frames | CPU/frame C | CPU/frame Atmos | Atmos/C | fps C | fps Atmos | CPU at 60 fps C | CPU at 60 fps Atmos | Memory C | Memory Atmos | Binary C | Binary Atmos |
|------|-----|-------:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|----:|
| snake | play | 760 | 0.66 | 0.64 | 0.97 | 1446 | 1546 | 4% | 4% | 12.6 | 12.6 | 790 | 2010 |
| floppy | play | 1200 | 1.90 | 1.90 | 1.00 | 519 | 525 | 11% | 11% | 12.6 | 12.6 | 786 | 1998 |
| gold_fever | play | 1009 | 3.39 | 3.60 | 1.06 | 292 | 275 | 20% | 22% | 12.6 | 12.6 | 790 | 2030 |
| arkanoid | lose | 1200 | 2.92 | 2.71 | 0.93 | 340 | 366 | 18% | 16% | 12.6 | 12.6 | 790 | 2027 |
| asteroids_survival | seed3 | 1200 | 1.15 | 1.08 | 0.94 | 807 | 927 | 7% | 6% | 12.7 | 12.6 | 790 | 2031 |
| space_invaders | play | 1200 | 0.87 | 1.03 | 1.18 | 1085 | 958 | 5% | 6% | 12.6 | 12.6 | 790 | 2039 |
| missile_commander | play | 1200 | 1.62 | 1.55 | 0.95 | 576 | 639 | 10% | 9% | 12.6 | 12.6 | 790 | 2039 |
| gorilas | play | 648 | 2.86 | 3.03 | 1.06 | 347 | 327 | 17% | 18% | 12.6 | 12.7 | 790 | 2038 |
| asteroids | die | 577 | 1.08 | 1.05 | 0.97 | 861 | 944 | 6% | 6% | 12.7 | 12.7 | 790 | 2067 |
| pang | play | 1200 | 1.00 | 1.13 | 1.13 | 948 | 876 | 6% | 7% | 12.6 | 12.7 | 790 | 2047 |
| platformer | play | 734 | 1.67 | 1.64 | 0.98 | 594 | 606 | 10% | 10% | 12.6 | 12.7 | 790 | 2043 |
| tetris | play | 1200 | 1.08 | 1.29 | 1.20 | 913 | 763 | 6% | 8% | 12.7 | 12.7 | 790 | 2115 |
| raylife | play | 800 | 2.51 | 2.73 | 1.09 | 396 | 364 | 15% | 16% | 22.4 | 15.2 | 790 | 2006 |
| wave_collector | lose | 1200 | 11.08 | 11.20 | 1.01 | 90 | 88 | 66% | 67% | 44.1 | 44.9 | 1363 | 2128 |
| repair | play | 1200 | 18.39 | 18.31 | 1.00 | 53 | 54 | 110% | 110% | 36.5 | 35.9 | 1363 | 2149 |
| cat_vs_roomba | play | 1200 | 16.65 | 17.36 | 1.04 | 60 | 57 | 100% | 104% | 25.7 | 26.3 | 1368 | 2216 |
| light_my_ritual | start | 1200 | 18.19 | 18.38 | 1.01 | 54 | 53 | 109% | 110% | 43.0 | 43.3 | 1367 | 2180 |
| transmission | play | 1200 | 8.12 | 8.20 | 1.01 | 122 | 121 | 49% | 49% | 28.8 | 29.4 | 1373 | 2165 |
| drturtle | play | 1200 | 33.85 | 33.83 | 1.00 | 29 | 29 | 203% | 203% | 29.3 | 29.8 | 1356 | 2059 |
| just_do | play | 1200 | 10.81 | 11.00 | 1.02 | 92 | 90 | 65% | 66% | 17.0 | 17.7 | 1383 | 2291 |
| skully_escape | play | 1200 | 9.57 | 9.77 | 1.02 | 104 | 100 | 57% | 59% | 32.2 | 32.9 | 1407 | 2270 |
| retro_maze_3d | play | 1200 | 6.76 | 6.73 | 1.00 | 147 | 147 | 41% | 40% | 13.7 | 14.2 | 1644 | 2395 |
| koala_seasons | play | 1200 | 33.16 | 34.16 | 1.03 | 30 | 29 | 199% | 205% | 52.9 | 54.3 | 1452 | 3393 |

## 8. Limits and negative findings

What each port says reads poorly, did not move, or stayed in C.

### doom (`doom/doc/study.md`, `doom/doc/seam.md`, plan)

- The simulation stays in C because of the saved game: "the position
  of an object is a number, and a number can be written to a file. A
  task suspended in an `await` is a point in the code, with its local
  variables." `p_saveg.c` writes the objects with the number of their
  row, so "to keep saved games is to keep the whole simulation as it
  is" (`doom/.claude/plans/261113-doom.md`).
- The face of the status bar did not become tasks: "a state machine
  that is polled at each tic, whose states are conditions of the
  simulation, stays a state machine in Atmos. Tasks help when the
  states follow each other in time ... not when they are chosen anew
  at each tic."
- The cast of the finale "reads no better than C".
- "The 'you are here' that blinks": one line of C against a loop with
  two awaits; "The line of C is better."
- The decisions of the flow stay in `G_DoCompleted`, which reads
  static tables of `g_game.c`.
- "An action with no task is an error at run time", where the `switch`
  of C ignores it.
- Each step of the title is wrapped in the same `watching`, five
  times.
- Quirks of the original became visible globals (`WI_POINTER`,
  `WI_FRAGS`, `FI_END`).
- The `main` stays in the engine: `D_DoomMain` cannot return.
- The order of drawing is the order in which the tasks are attached,
  "and it had to be thought about".
- With no rendering, the program became slower than the engine in C
  after the face was ported: 44 against 33 ms on demo1; the likely
  cause is "not measured apart" (`doom/.claude/plans/TODO.md`, 51).

### env-ray (`env-ray/doc/case-study.md`, `env-ray/doc/fidelity.md`)

- "the flags that are a state shared by many parts of one frame stay
  data, and the order of the frame stays explicit": drturtle keeps
  `gameraMode`; koala_seasons keeps the frame of the C, "one function
  of 1700 lines over 250 statics", as functions over a table.
- Pattern 7 (one task and tables for seven copies) "is not specific to
  Atmos; it is what a port by hand allows".
- Fidelity forced the ports to keep accidents of the C: globals never
  reset, the init of the next screen under the old one, a counter
  shared by two things, drawing that changes the state, 32-bit floats.
- The board of raylife (250000 cells) is C in the port too.
- At 1000 and 5000 entities, a task per enemy costs 1.34 and 1.70
  times the C (`env-ray/doc/bench.md`).
- The frames are mostly drawing in software, which "hides the cost of
  the logic" (`env-ray/doc/bench.md`).

### ray-lessmilk (`ray-lessmilk/doc/state.md`, `doc/table.md`)

- The ports have more lines than the originals (8% more); Dark Blue
  has 37% more lines and the same tokens.
- Groups of recycled sprites stay slots, not a task per life: "A task
  per life with a new sprite would not be the same game" (Pixel War);
  the same in Connected, Man vs Penguins, I Hate Rabbits
  (`doc/from-reference.md`).
- Flags stay where the original reads and writes them in one update,
  in order; The Three Hearts leaves 3 of 3; two ports add a flag.
- A timer stays a comparison in the update in Man vs Penguins: a loop
  with `await Wait` in another task would draw its random number in
  another order.
- The shared `lib/` (2106 lines) redoes what Phaser did, with its
  frame order, its late tweens, its bodies one frame behind, and its
  32-bit rounding (`doc/fidelity.md`).

### sdl-flare (`sdl-flare/doc/state.md`, `doc/compiler.md`)

- "What stays as state is what the original tests in a fixed order
  each frame, with draws of the generator between the tests: making it
  a task that awakes on its own would change the order of the draws."
- The port must call the generator at the same hidden places as the
  original (`doc/fidelity.md`, "Hidden draws").
- Pinned values cannot reach the tables of a module: three shapes that
  the port wanted were rewritten with arguments.

### sdl-pingus (`sdl-pingus/doc/bench.md`, `doc/fidelity.md`, `doc/loc.md`)

- Uncapped, the official game draws 1.2x to 1.3x the frames of the
  port; "a fixed cost per frame, not a cost per pingu".
- 5 of 154 demos still differ, by one pixel or one tick.
- The mover is in C (`pg_move`), for 32-bit floats.
- The engine comparison "is not like for like".
- A bug made half of the rain drops never end, and "the cost grew with
  the time"; a spawn inside an `if` left the backgrounds of 66 levels
  still, "and no trace saw it" (`doc/compiler.md`).
- Open: fast forward and border of the worldmap, the climber center,
  the bridger offset, input configuration (`doc/todo.md`).

### sdl-snkrx (`sdl-snkrx/doc/case-study.md`, `doc/bench.md`)

- The first version spawned the attacks in a loop or an `if`, "and
  they never ran; no trace showed it".
- Time is global: a pool cannot have its own clock.
- A contact of the physics is broadcast to all the tasks that await
  one, with a filter; with 200 enemies, this and the separation "are
  most of the logic of a tick".
- 34 of 70 variables remain, as state that one task shows to another.
- The lines fall less than the tokens (83% against 62%).
- Not tick by tick: Box2D 3.1 against 2.3; a timer splits the time of
  a tick, so the physics may take two steps in a tick
  (`.claude/plans/TODO.md`, items 3 and 4).

### sdl-tux (`sdl-tux/.claude/plans/TODO.md`)

- `game/coll.h`: the collision system and `Physic` stay in C, with
  callbacks, as in the C++, for 32-bit reals.
- `game/player.h`: the whole `Player` is in C (3654 lines); its timers
  and phases "are still flags and timers in the struct".
- In the original the drawing changes the game (Tux stops growing when
  the animation of his sprite ends), so the traces are recorded
  drawing.
- When several scripts wake in the same step, the port wakes them in
  the order of their tasks, the original in the order of their times.

### env-pico

not reported

## 9. Migration to the rules of `pin`

All of this section: measured under earlier rules of pin, not to cite.

| Port | What the port reports | Source | Mark |
|------|-----------------------|--------|------|
| sdl-pingus | 128 lines changed only from `val`/`var` to `pin`; 15 places rewritten; of the 159 traces of the demos, 156 as before | `sdl-pingus/doc/compiler.md` | measured under earlier rules of pin, not to cite |
| sdl-snkrx | The commit `5c489ea`: 617 lines changed, of which 433 only from `val` or `var` to `pin`; about 60 places needed a rewrite; 35 warnings at the second migration | `sdl-snkrx/doc/compiler.md`, `sdl-snkrx/.claude/plans/TODO.md` | measured under earlier rules of pin, not to cite |
| ray-lessmilk | Second migration: 20 values became `val` (10 in the library, 10 in the games); two shapes needed a rewrite; 16 of the 22 bodies in an experiment | `ray-lessmilk/doc/compiler.md` | measured under earlier rules of pin, not to cite |
| env-ray | The first migration rewrote the trace in four games | `env-ray/.claude/plans/TODO.md` | measured under earlier rules of pin, not to cite |
| sdl-tux | Second migration: two values given away by their block are `val` | `sdl-tux/.claude/plans/TODO.md` | measured under earlier rules of pin, not to cite |
| doom | Two migrations done; no cost given | `doom/.claude/plans/ISSUES.md` | measured under earlier rules of pin, not to cite |
| sdl-flare, env-pico | not reported | | |

## Gaps

Per port, the measures of the sections above that are missing.

- **doom**: no total of lines for the port or for the part of the
  engine it replaces; no lines for the screen wipe; no tokens; no
  count of control variables for the wipe and the finale; no
  occurrences of mechanisms; no memory; no frames with a window; the
  benchmark is of the seam "with nothing ported" (D8, the final
  benchmark, is not done), and the only later number is in a TODO.
- **env-ray**: no totals of lines and tokens in any file (only rows);
  no CPU of the logic alone (the stress of one game is the nearest);
  the CPU at 60 fps is derived, not measured; no occurrences per
  pattern; `ray.atm` is not counted.
- **ray-lessmilk**: no benchmark at all (memory, CPU, frames); no
  total of control variables; Connected has no control count and no
  section in `doc/state.md`; no occurrences of mechanisms in the port;
  no screens compared pixel by pixel.
- **sdl-flare**: no size of the port, no size of the original in its
  own documents, no tokens; no counts of control state; no mechanisms;
  no performance of the port (only the baseline of the original and
  the INI reader); a slice only (no HUD, no menus).
- **sdl-pingus**: no CPU of the logic alone in the game (only a micro
  benchmark); no occurrences of mechanisms; the control counts cover
  the 17 actions, the 10 objects of Céu and the panel, not the 12 new
  objects nor the screens; no machine model in the benchmark.
- **sdl-snkrx**: no count of runs identical to the original and no
  fidelity document: the comparison is by timings of events; the size
  of the test suite is only in a TODO; the profile of the logic is of
  the port alone, with no equivalent for the original.
- **sdl-tux**: no `doc/`; no size of the port; no control counts; no
  mechanisms; no performance of the port (only the official game); a
  slice in course.
- **env-pico**: nothing of the above: no original measured, no size,
  no control state, no fidelity, no benchmark. Only `exs/storm` is a
  port.

## Inconsistencies

Inside one port:

- **env-ray, control variables left**: `doc/case-study.md` and
  `.claude/plans/TODO.md` (item 24) say 38 of 134; the column "Control
  left" of `doc/table.md` sums to 35, and "Control C" to 134
  (computed). `doc/state.md` has 38 lines with the mark `**left**`
  (counted by this session), so the two may count different things
  (variables against rows).
- **env-ray, identical or relaxed**: `README.md` says "each identical
  to its original on its scripted runs (55 runs)"; `doc/table.md`
  marks 14 of the 23 games "relaxed" (computed count), with traces and
  screens that differ on purpose.
- **env-ray, drturtle**: `doc/fidelity.md` gives the final file as 485
  lines; `doc/table.md` gives 293 lines of code for the C (one is raw,
  the other with no blanks nor comments; not stated).
- **ray-lessmilk, number of games**: "twelve games" everywhere, but
  the table "State" of `README.md` has eleven rows (no Connected), and
  `doc/state.md` has no section for Connected.
- **ray-lessmilk, number of runs**: "all 31 runs of the twelve games"
  (`doc/fidelity.md`); "26 tests" and then "33 tests"
  (`doc/compiler.md`); the column "Runs" of `doc/table.md` sums to 26
  (computed).
- **ray-lessmilk, lines of a port**: Run! has 142 lines in
  `doc/table.md` and 139 in `doc/games/run.md`; Box Jump 214 and 207
  (`doc/games/box_jump.md`).
- **ray-lessmilk, benchmark**: `doc/table.md` refers to `bench.md`,
  which is not in the repository.
- **sdl-flare, number of runs**: `README.md` says 30 scripted runs;
  `tst/runs.txt` lists 31 names (counted by this session).
- **sdl-pingus, lines**: the two tables of `doc/loc.md` differ for the
  same parts: actions (17) Céu 879 and Atmos 423 in the first, 754 and
  424 in the second; engine in Atmos 407 (plus 727 of C) in the first,
  1219 (C and Atmos) in the second. Different tools (`grep`, then
  `tools/loc.py`) and different groupings.
- **sdl-pingus, fast forward**: `doc/bench.md` says "fast forward runs
  4 ticks per 20ms" and later "fast forward (5 ticks per frame)".
- **sdl-pingus, demos**: `doc/todo.md` still says the traces match "in
  142 demos"; `doc/fidelity.md` says 149.
- **sdl-snkrx, size**: `doc/bench.md` and `doc/case-study.md` give
  different numbers for the same things. Original game: 10374 lines
  and 134786 tokens (nine files named), or 10381 and 134827 ("ten Lua
  files"); original engine: 4433 lines in both, but 40853 or 41735
  tokens; port: 4755 lines and 43606 tokens (S0 to S7), or 8647 and
  83761 (whole game); engine of the port 2080 and 18476, or 2128 and
  18826. The timers are counted "in the eight files of the game".
- **sdl-snkrx, which port was measured**: the setup of `doc/bench.md`
  says the port is "at `b2c2089` (the whole game, S9)"; its section on
  size says the port "covers a part of the game (S0 to S7 ...)". Part
  of the profile is "measured on the port of S7, whose units did not
  attack"; `.claude/plans/TODO.md` item 11 says the S7 stress numbers
  "are not to be compared".
- **sdl-snkrx, a timing**: the first marker of level 1 comes 269 ticks
  after the arena starts (`doc/case-study.md`) or 270 (`doc/study.md`).
- **sdl-snkrx, sections**: `doc/case-study.md` says `study.md` has
  "twelve behaviors side by side"; `doc/study.md` has 14 sections.
- **sdl-tux, number of runs**: `README.md` says thirty runs of up to
  3000 ticks; `.claude/plans/TODO.md` says 30, 40, and 29 with
  badguys, in different items; `tst/runs.txt` lists 69 runs (counted
  by this session).
- **doom, lines**: the plan gives `wi_stuff` 1850, `f_finale` 738,
  `f_wipe` 302 lines, "measured on `linuxdoom-1.10`"; `doc/study.md`
  gives 1219 and 528 for Chocolate Doom with no blanks nor comments.
- **doom, benchmark**: `doc/bench.md` says "No difference above the
  error of the measure" (seam with nothing ported);
  `.claude/plans/TODO.md` item 51 says 44 against 33 ms later. The
  baseline of demo1 is 24 ms in one table and 22 ms in the next.

Across ports, not comparable as they are:

- **Lines**: all drop blank lines and comments, but with four tools:
  `tools/count.py` in env-ray, ray-lessmilk, and sdl-snkrx (each
  repository has its own copy; not checked to be the same), `grep` and
  `tools/loc.py` in sdl-pingus, and a method not named in doom. Tokens
  exist in four ports only (env-ray, ray-lessmilk, sdl-pingus,
  sdl-snkrx).
- **What the port side includes**: doom counts the C of the `:prelude`
  inside the Atmos files; env-ray leaves `ray.atm` out; ray-lessmilk
  leaves out a `lib/` of 2106 lines that redoes Phaser, and two data
  files; sdl-pingus counts game logic apart from the engine and leaves
  the converter out; sdl-snkrx counts game and engine apart, with the
  data tables in.
- **What the original side includes**: whole small games (env-ray,
  ray-lessmilk), only the units that were ported (sdl-pingus, doom),
  the whole game against a port that covered a part (sdl-snkrx,
  `doc/bench.md`).
- **Control state**: each port defines it its own way: "variables that
  only control the flow" (env-ray, ray-lessmilk), classes control,
  period, dual, action (sdl-pingus), "guard a phase or compute a time"
  with groupings counted as one (sdl-snkrx), static variables of a
  file, graphics included (doom). sdl-pingus reports 0 left with a
  list of what remains "as data"; sdl-snkrx reports 34 left with the
  same kind of list.
- **Fidelity**: frame by frame with screens pixel by pixel (env-ray,
  doom), trace line by line (ray-lessmilk, sdl-flare, sdl-tux), a line
  per change of action (sdl-pingus), timings of a few events
  (sdl-snkrx). 14 ports of env-ray were then relaxed on purpose.
- **Baselines of performance**: the same engine with and without
  Atmos (doom), the same library under both (env-ray), another build
  of the game over the same SDL renderer (sdl-pingus), another
  language, runtime, and physics library (sdl-snkrx: LuaJIT and LÖVE
  with Box2D 2.3 against C and Box2D 3.1). The factors of sdl-snkrx
  (1.8 to 3.0 times the ticks, 946 against 131 MB) are not of the same
  kind as the others.
- **Renderer**: all benchmarks draw in software on the CPU (rlsw,
  SDL software, llvmpipe in threads), so the whole frame is mostly
  drawing; sdl-snkrx's CPU time includes the threads of llvmpipe.
- **Units**: ms per frame and fps (env-ray), s of CPU in 10 s and
  frames in 10 s (sdl-pingus), ticks per second and s of CPU per run
  (sdl-snkrx), ms per demo and tics per second (doom). Frame caps
  differ: none (env-ray, sdl-snkrx), 40 fps (sdl-pingus).
- **Runs and statistics**: best of 7 or 9 (doom), median of 3
  (env-ray, sdl-snkrx), one run of 10 s (sdl-pingus, "noise ... about
  10%"), two runs (sdl-flare).
- **Logic alone**: only sdl-snkrx splits logic, draw, and render (port
  only); doom's "no rendering" is the nearest in another port; the
  outline of the paper (`papers/games.md`) asks for both numbers for
  every port.
- **Machine**: "Intel Xeon at 2.10 GHz" in a container for doom,
  env-ray, sdl-flare, sdl-snkrx; sdl-pingus and sdl-tux say only
  "4 cores". Measured on different days and commits of the compiler.
