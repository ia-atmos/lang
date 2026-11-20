Drafts written on 2026-11-20 by the session paper-games, with no input of the author. Independent of the organization of the paper (#1). To be reviewed.

> **Correction, 2026-11-20 (SNKRX).** The numbers of SNKRX in this
> file come from `sdl-snkrx` at `ae9c69f` and are superseded by
> `07f5905` (`doc/bench.md`, `doc/case-study.md`), read on the same
> day. The old benchmark ran the port with no audio while the original
> decoded all its sounds. With the sound of the port mixed (port
> `7639dd3`, 3 runs, medians):
>
> - ticks per second, port against original: 60 against 56 (menu), 71
>   against 66 (shop), 113 against 93 (level 1), 79 against 65
>   (stress): 1.07 to 1.22 times, not 1.8 to 3.0;
> - CPU: 83% to 91% of the original, not 56% to 71%;
> - peak memory: 546 against 946 MB, not 131. The 946 MB are 103 sounds
>   decoded whole at start (359 MB of samples); the port does the same
>   (131 MB muted). Neither peak is about the language: memory is not
>   to be cited as a gain;
> - logic alone (no window, `-O2`, 1500 ticks, 3 runs): 0.089 ms per
>   tick in level 1, 2.77 in the stress, port only;
> - size, one count: the game has 9455 lines and 91671 tokens against
>   10381 and 134827 of the original (91% and 68%); the engines, 2209
>   and 19370 against 4428 and 40796. The 4755 and 8647 lines were two
>   stages of the port.
>
> The SNKRX cells and sentences below are to be rewritten from the new
> documents.

# Conventions of this file

- Every number is followed by its source in brackets. A path such as
  `[sdl-pingus/doc/bench.md]` is relative to the repository of the
  port (`ia-atmos/<name>`); `[plans/<file>]` is a file of
  `.claude/plans/` in `ia-atmos/lang`.
- "(computed here)" marks a sum or ratio that this draft derived from
  a cited table, and that the source does not state itself.
- "(to confirm with the author)" marks what the repositories do not
  establish.
- Code excerpts are copied from the files named above them, at the
  state of the clones read on 2026-11-20. Some lines of the excerpts
  are longer than 72 columns, as in the sources.
- Nothing was built or run for this draft: all numbers are those that
  the ports document.

# 1. Subjects

The evaluation uses six ports of existing games to Atmos. Each port
lives in its own repository, with the same set of documents: a
comparison of code by behavior, a table of state variables
(`doc/state.md`), a description of how the port is compared with its
original (`doc/fidelity.md`), measures (`doc/bench.md`), and the
findings about the compiler (`doc/compiler.md`).

## 1.1 Pingus

Pingus is a free puzzle game (GPL-3) written in C++ by Ingo Ruhnke and
others [sdl-pingus/README.md, plans/261110-pingus.md]. In 2018 its
game logic was rewritten in Céu, as the case study "Structured
Synchronous Reactive Programming for Game Development: On Rewriting
Pingus from C++ to Céu" (SBGames 2018) [sdl-pingus/README.md]. That
rewrite has 77 files and 8326 lines of Céu, and still calls 22106
lines of the original C++ for the display, the collision and ground
maps, and the readers of data [plans/261110-pingus.md]. The port
targets the Céu rewrite (`fsantanna/pingus@036852a`) and is measured
against the official game (`Pingus/pingus@ffe3851`)
[sdl-pingus/README.md, sdl-pingus/doc/bench.md]. It is a third point
for the 2018 study: C++, Céu, Atmos [sdl-pingus/README.md].

Unlike the Céu rewrite, the port is complete: it does not link the
original C++ [sdl-pingus/doc/loc.md]. The game logic is in Atmos: 21
pingu actions, one file each; the 10 world objects of the Céu rewrite
and 12 more; the screens (menu, start, result, story, credits, levels,
worldmap, options) [sdl-pingus/README.md]. The game offers 522 of the
523 levels of the data [sdl-pingus/README.md]. One C header,
`engine/pingus.h`, of 727 lines, takes the place of the C++ that the
Céu code called: images, fonts, sounds, collision and ground maps
[sdl-pingus/doc/loc.md]. The mover of a pingu is also in C
(`pg_move`), because positions and velocities of the original are
32-bit floats and gravity accumulates in `float`
[sdl-pingus/doc/fidelity.md]. The data files were converted once from
s-expressions to Atmos data by a Python script of 380 lines, which
replaces the s-expression reader of the original (936 lines of C++)
[sdl-pingus/doc/loc.md]. On the game logic that all three versions
have, the port has 2732 lines against 12794 of C++ and 5233 of Céu,
and 20336 tokens against 77370 and 38748 [sdl-pingus/doc/loc.md].

Pingus is the best illustration of the five control-flow patterns of
the 2018 study, since each case exists in three languages: the port
documents seven cases (two finite state machines, two cases of
continuation passing, dispatching hierarchies, lifespan hierarchies,
signaling) [sdl-pingus/doc/README.md]. It is also the port with the
strongest check of fidelity (Section 2.3). The example below is the
double click on the Armageddon button: the second click must come
within 1 second of the first [sdl-pingus/doc/armageddon.md]. The C++
class keeps two state variables, `pressed` and `press_time`, shared by
two callbacks [sdl-pingus/doc/armageddon.md].

Céu, `ceu/pingus/screens/game/input.ceu`, as quoted in
[sdl-pingus/doc/armageddon.md]:

```
loop do
    await component.component.on_primary_button_pressed;
    watching 1s do
        await component.component.on_primary_button_pressed;
        break;
    end
end
...
emit outer.game.go_armageddon;
```

Atmos, `sdl-pingus/game/hud.atm`:

```
task Armageddon (g) {
    pin r = [x=W-44, y=H-65, w=38, h=60]
    pin pos = [x=r.x+19, y=r.y+30]
    spawn Sprite("core/buttons/hbuttonbgb", r)
    do {
        spawn Sprite("core/buttons/armageddon_anim", pos, [anchor=:center, still=true, frame=7])
        loop {
            await Click(r)
            watching 1s {
                await Click(r)
                break
            }
        }
    }
    spawn Sprite("core/buttons/armageddon_anim", pos, [anchor=:center])
    emit @(:parent) :game.armageddon
    await(false)
}
```

The still image of the button lives in the `do` block of the loop, and
ends with it; the animated image follows
[sdl-pingus/doc/armageddon.md].

## 1.2 SNKRX

SNKRX is an arcade roguelite by a327ex: a snake of heroes that attack
by themselves, in an arena with waves of enemies, with a shop between
levels [sdl-snkrx/README.md]. It is written in Lua over LÖVE 11
(`a327ex/SNKRX@6b93a64`), the game and the author's own engine on top
of LÖVE [sdl-snkrx/README.md, plans/261111-snkrx.md]. Its source was
released in 2021, with the code under MIT [plans/261111-snkrx.md].
The game has 10381 lines and 134827 tokens of Lua, and its engine
(`engine/` without `engine/external`) 4433 lines and 41735 tokens
[sdl-snkrx/doc/case-study.md]. Sequences in the original are timers
with callbacks: the eight files of the game call the timer 422 times
(`after` 223, `every` 83, `every_immediate` 18, `tween` 63, `cooldown`
32, `during` 3) [sdl-snkrx/doc/case-study.md].

The port is written from scratch, over SDL2, after reading a partial
port by a student (`kboltiz/SNKRX@18c593b`) [sdl-snkrx/README.md]. It
covers the game as played: the menu, the shop, the 25 levels, all 54
characters and 16 classes, the 82 passives, new game plus, the
options, the music, the save, the tutorial, and the credits; Steam is
left out [sdl-snkrx/README.md]. The game is in Atmos (8647 lines,
83761 tokens), and so is most of the engine (2128 lines, 18826 tokens,
Atmos and C) [sdl-snkrx/doc/case-study.md]. The physics is Box2D v3,
in C, kept by decision of the author [sdl-snkrx/README.md,
plans/261111-snkrx.md]; the plan puts shapes, text, sounds, and the
broad phase of collisions in one C header over SDL2, and "everything
else in Atmos" [plans/261111-snkrx.md]. The timers of the original
became 370 `await`s in 27 files, with 59 `par`, 647 `spawn`, and 51
`emit` [sdl-snkrx/doc/case-study.md]. Of 70 variables of the original
that guard a phase or compute a time, 36 are gone in the port; the
other 34 remain as state that one task shows to another
[sdl-snkrx/doc/state.md].

SNKRX is the best illustration of a complete game in a dynamic
language whose control is written as timer callbacks, and of the
measures against a JIT-compiled original (Section 2.4). The example is
a timer with a tag, which the original restarts by registering a new
timer under the same tag.

Lua, `enemies.lua:724-727`, as quoted in [sdl-snkrx/doc/study.md]:

```lua
function Seeker:slow(amount, duration)
  self.slowed = amount
  self.t:after(duration, function() self.slowed = false end, 'slow')
end
```

Atmos, `sdl-snkrx/src/seeker.atm`:

```
task SeekerSlows (e) {
    loop {
        var s = await :Slowed until it.to == e
        loop {
            set e.slowed = s.amount
            set s = par :any {
                await(s.duration * 1s)
                nil
            } with {
                await :Slowed until it.to == e
            }
            until !s
        }
        set e.slowed = nil
    }
}
```

The restart is a `par :any` between the time and the next request; the
field `slowed` remains, since the movement reads it
[sdl-snkrx/doc/study.md].

Note: the outline names the repository `ia-atmos/sdl-snxkx`; the README
of the clone read here is titled `sdl-snkrx` [sdl-snkrx/README.md].
Which name is cited is to confirm with the author.

## 1.3 Flare

Flare (Free Libre Action Roleplaying Engine) is a 2D action RPG by
Clint Bellanger, Justin Jacobs (dorkster), and contributors
[sdl-flare/README.md]. The engine is C++ (C++98 by choice) over SDL2,
with no scripting language: the game is defined by text files in the
style of INI [plans/261111-flare.md]. The engine is GPL-3 and the game
data CC-BY-SA 3.0 [sdl-flare/README.md]. The engine has 57843 lines of
C++ in 120 files, of which 14544 are menus; an entity has a state
enum `cur_state`, used 83 times, and the headers have 34 `Timer`
members and 122 enums [plans/261111-flare.md]. The original has no
tests, no replays, and no recording of input [plans/261111-flare.md].

The port is a slice, at `flare-engine@56c747a` with
`flare-game@af6eee6` [sdl-flare/README.md]. The milestones closed are
F0 to F5: the INI reader and the mods, the engine, the isometric map,
the hero, and then enemies, powers, hazards, effects, loot, and path
finding [sdl-flare/README.md]. The game over menu, the combat text,
and the rest of the HUD are a later milestone (F6), and the Empyrean
Campaign with its menus is the second version [sdl-flare/README.md].
The data is read in the original INI format, by a reader of the port,
so that the original mods work unchanged [sdl-flare/doc/decisions.md].
In C stay the drawing of the map, the collision map, and path finding
(`engine/flare.h`, `engine/map.h`, `engine/collision.h`,
`engine/astar.h`) [sdl-flare/README.md]; the plan gives the reason:
collisions and path finding "are the hot loops", and a direct
translation "would be slow and would bring back the per-frame scans"
[plans/261111-flare.md]. The size of the port is not reported in its
documents (to be counted, with the tool used for the other ports).

Flare is the best illustration of plain C++ classes that are updated
every frame (state enums, timers as members, managers that walk
lists) [plans/261111-flare.md], and of where the conversion to tasks
stops: the timers of the enemy AI and the perception flags remain
state, because the original tests them in a fixed order each frame,
with draws of the random generator between the tests, and a task that
awakes on its own would change the order of the draws
[sdl-flare/doc/state.md]. The example is `set_dir_timer` of the hero
(`Avatar`), in the original a timer ticked per frame and reset when
the keys are released [sdl-flare/doc/state.md]. The documents of the
port do not quote the C++ for this case.

Atmos, `sdl-flare/src/hero.atm`:

```
task Turning (h) {
    set pub = true
    loop {
        await :hero.turned
        set pub = false
        ;; set_dir_timer.tick() in the prologue of each frame, and reset
        ;; when the move keys are not pressed
        loop i in 6 {
            await :hero.prologue
            until !hero_pressing_move(h)
        }
        set pub = true
    }
}
```

## 1.4 Lessmilk

The Lessmilk games are the twelve games of Thomas Palef's challenge
"12 games in 12 weeks" (lessmilk.com, 2014), in JavaScript over Phaser
[ray-lessmilk/README.md]. Games 1 to 9 use Phaser 1.1.3, game 10 uses
1.1.6, game 11 uses 2.0.1, and game 12 uses 2.0.2
[ray-lessmilk/doc/fidelity.md]. The twelve games have 3287 lines and
37669 tokens of JavaScript, from 144 to 403 lines each
[ray-lessmilk/doc/table.md]. They have the common shape of a small
game over a framework: states with `create` and `update`, tweens
chained by callbacks, timers compared with the clock every frame,
groups of sprites, and flags [plans/261112-lessmilk.md].

The port covers the twelve games, over raylib, through the environment
of the raylib port (Section 1.5) [ray-lessmilk/README.md]. It starts
from an earlier port by a student, Jéssica Oliveira
(`jessicacoliveira/atmos-lessmilk@3e24458`), written for the reference
Atmos over `pico-sdl`: its task structure, its modules for motion and
particles, and its analysis of each game [ray-lessmilk/README.md]. The
games are in Atmos: 3534 lines and 28956 tokens, that is, 8% more
lines and 23% fewer tokens than the JavaScript
[ray-lessmilk/doc/table.md]. The shared modules of `lib/`, which do
what Phaser did (motion, particles, physics), are also Atmos, and are
counted apart: 2106 lines and 18743 tokens
[ray-lessmilk/doc/table.md]. The originals run in Chromium with no
screen, stepped frame by frame, to be compared with the port
[ray-lessmilk/README.md].

Lessmilk is the best illustration of the simple dynamic case, and of
what fidelity costs at this size: where the game shows what a recycled
sprite of Phaser keeps from its last life, the port keeps the groups
of the original as slots rather than a task per entity (Pixel War,
Connected, I Hate Rabbits) [ray-lessmilk/doc/state.md,
ray-lessmilk/doc/from-reference.md]. The example is from Run!, whose
original keeps the time of the next enemy in a field and compares it
with the clock in every update.

JavaScript, as quoted in [ray-lessmilk/doc/games/run.md]:

```js
if (this.game.time.now > this.enemyTime)
    this.newEnemy();            // which sets this.enemyTime = now + 500
```

Atmos, `ray-lessmilk/games/run/play.atm`:

```
    ;; an enemy now and then every 500 ms (enemyTime)
    spawn {
        await :update
        loop {
            spawn @enemies Enemy(new_enemy())
            await Wait(500)
        }
    }

    ;; a point now and then every second (scoreTime)
    spawn {
        await :update
        loop {
            set score += 1
            set label_score.s = "score: " ++ tostring(score)
            await Wait(1000)
        }
    }
```

Run! has 4 control variables in the JavaScript and 0 in the port
[ray-lessmilk/doc/table.md].

## 1.5 The raylib games

`raysan5/raylib-games` (commit `318688c`) is a collection of 23 games
in C by Ramon Santamaria and the co-authors credited in each one,
under the zlib license [env-ray/README.md]. Twelve are classics of the
arcade, each in one file with no assets; eleven were made for game
jams and for release, with art and sound [plans/261112-raylib.md,
env-ray/README.md]. The originals range from 143 lines (floppy) to
4689 lines (koala_seasons) [env-ray/doc/table.md], 17854 lines and
146226 tokens in all (computed here from [env-ray/doc/table.md]).
They are written in the style of most small C games: one function runs
once per frame and decides what to do from counters of frames, flags
of phases, and an enum of screens [env-ray/doc/case-study.md].

The port covers all 23 games, over raylib itself (tag `6.2`): the
original and the port draw, play sound, and read input through the
same library [env-ray/README.md]. The games are in Atmos, 8888 lines
and 86766 tokens in all (computed here from [env-ray/doc/table.md]);
the environment `ray.atm` is not counted [env-ray/doc/table.md]. What
stays in C is the library, and one board: the 250000 cells of
`raylife` are C in the port too, in a prelude
[env-ray/doc/bench.md]. Floats of 32 bits are rounded where the C
rounds them (`ray.f32`), against the doubles of Atmos
[env-ray/doc/case-study.md]. Of 134 control variables of the C, the
ports leave 38 according to [env-ray/doc/case-study.md]; the column
"Control left" of [env-ray/doc/table.md] sums to 35 (computed here).
The difference is to be resolved before either number is used.

The raylib games are the best illustration of the simple static case:
the same patterns over 23 small programs of several genres, with the
original and the port over one library, so that the measures compare
the logic and nothing else. They also show where the patterns stop:
flags that many parts of one frame read and write stay data
[env-ray/doc/case-study.md]. The example is the step of the snake,
every 5 frames.

C, as quoted in [env-ray/doc/case-study.md]:

```c
static int framesCounter = 0;
...
if ((framesCounter%5) == 0)
{
    for (int i = 0; i < counterTail; i++)
    {
        if (i == 0)
        {
            snake[0].position.x += snake[0].speed.x;
            snake[0].position.y += snake[0].speed.y;
            allowMove = true;
        }
        else snake[i].position = snakePosition[i-1];
    }
}
...
framesCounter++;
```

Atmos, `env-ray/classics/snake.atm`:

```
        loop {
            val head = snake@0
            vector.insert(snake, 0, [x=head.x+g.speed.x, y=head.y+g.speed.y])
            val tail = vector.remove(snake)
            val h = snake@0
            val wall = (h.x > (W-OX)) || (h.y > (H-OY)) || (h.x < 0) || (h.y < 0)
            val self = loop i in (1, #snake - 1) {
                until same(h, snake@i)
            }
            until wall || self
            emit :moved [tail=tail]
            await(5, :frame)
        }
```

A paused game does not count the frames of `await(5, :frame)`, with no
code for it [env-ray/doc/case-study.md].

## 1.6 Doom

Doom is by id Software (1993); its source code was released in 1997,
and under the GNU GPL version 2 in 1999 [doom/README.md]. The port
uses Chocolate Doom, by Simon Howard and contributors, at the commit
`a2dfd51`: `engine/` is a copy of the 301 files that the build of Doom
reads [doom/README.md]. `src/doom/` of Chocolate Doom has 59 files of
C and 45637 lines [plans/261113-doom.md]. The engine is specialized
for one game: a BSP renderer, a blockmap for collision, fixed-point
arithmetic, a WAD format [plans/261113-doom.md].

The port does not rewrite the engine. The simulation of a level (the
objects and their table of states, the weapons, the doors and lifts)
stays in C; what goes to Atmos is the control around it: the loop of
frames, the screen wipe, the intermission, the finale, the flow of the
game (title, demos, levels), and the face of the status bar
[doom/README.md]. Three files of the engine are changed, and three
leave the build; every change is listed in [doom/doc/seam.md]. The
reason for the border is the saved game. The table of states has 967
rows, and the position of an object in it is a number, which can be
written to a file; a task suspended in an `await` is a point in the
code, with its local variables [plans/261113-doom.md]. The saved game
writes every object with the number of its row, and the fields of
every door, lift, floor, ceiling, and light; so to keep saved games is
to keep the whole simulation as it is, and the author decided to keep
them [plans/261113-doom.md]. Two ways around were considered and not
taken: a save as a replay of the input, and a snapshot of the tasks by
the runtime [plans/261113-doom.md] (the latter is the subject of
`papers/snapshots.md`). The parts in Atmos compare as follows: the
intermission, 1219 lines of C against 622, of which 56 are C in a
prelude; the finale, 528 against 445, of which 183 are C; the flow,
125 against 186, of which 56 are C; the face, 149 against 130, of
which 43 are C [doom/doc/study.md].

Doom is the best illustration of the limits. First, the saved game, as
above. Second, a state machine that is polled at each tic, whose
states are conditions of the simulation, stays a state machine in
Atmos: the face of the status bar is "the same machine", in one loop
[doom/doc/study.md]. The port documents, for each part, what reads
worse than the C [doom/doc/study.md]. Where states follow each other
in time, the gain is clear. The statistics of the intermission are, in
C, a variable `sp_state` from 1 to 10 tested in a chain of `if` at
each tic [doom/doc/study.md].

C, as quoted in [doom/doc/study.md]:

```c
if (sp_state == 2) {
    cnt_kills[0] += 2;
    if (!(bcnt&3)) S_StartSound(0, sfx_pistol);
    if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills) {
        cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
        S_StartSound(0, sfx_barexp);
        sp_state++;
    }
}
else if (sp_state == 4) { /* the same, for the items */ }
else if (sp_state == 6) { /* the same, for the secrets */ }
...
else if (sp_state & 1) {
    if (!--cnt_pause) { sp_state++; cnt_pause = TICRATE; }
}
```

Atmos, `doom/wi.atm` (the first lines of a `watching` block that goes
on with the counters of the time and closes 13 lines below):

```
    watching :wi.accel {
        await(35, :wi.tic)
        await WI_Count(cnt, "kills", WI.kills)
        await(35, :wi.tic)
        await WI_Count(cnt, "items", WI.items)
        await(35, :wi.tic)
        await WI_Count(cnt, "secret", WI.secret)
        await(35, :wi.tic)
```

The key that skips the counters was a test at the top of the C
function; it is now the `watching` around the sequence
[doom/doc/study.md].

## 1.7 Not in the outline so far

### SuperTux

SuperTux is a free 2D platform game (GPL-3, free art) in C++17, over
SDL2 and OpenGL [plans/261113-sdl-tux.md]. Its `src/` has 106697
lines; state is kept by hand in 348 uses of state enums and `m_state`
and 108 `Timer` members [plans/261113-sdl-tux.md]. The original embeds
a second language, Squirrel, for scripts that wait: 89 of its 110
levels have scripts [plans/261113-sdl-tux.md]. The game advances in
fixed steps of 15 ms, with a seed that the configuration can fix
[plans/261113-sdl-tux.md].

The port (`ia-atmos/sdl-tux`) is a slice of the tag `v0.7.0`, in
course: the data of World 1 is converted to Atmos data (31 levels, the
worldmap, 393 sprites, 3 tilesets, and 193 of 196 scripts); the small
Tux and the collisions run with no screen; objects as tasks and the
interpreter of the scripts are in course [sdl-tux/README.md]. Thirty
scripted runs, of up to 3000 ticks, give the trace of the official
game line by line [sdl-tux/README.md]. Most of the port is C so far:
the logic of the player is `game/player.h`, translated from C++17 to
C99 "statement by statement, keeping the 32-bit float arithmetic of
the original operation by operation" [sdl-tux/game/player.h], with
3654 lines, and the collisions are `game/coll.h`, with 2702 lines; the
Atmos files of `game/` have 2835 lines, of 11503 in all (raw lines,
counted here with `wc -l`). The reason recorded is the 32-bit reals:
the player and the collisions stay in C until the compiler has them,
and a build option (`ATMOS_REAL=float`) was done on 2026-11-19
[sdl-tux/.claude/plans/TODO.md]. SuperTux would illustrate two points
that no port of the outline has: an original with an embedded
scripting language for sequences, and the cost of float fidelity as a
language feature. The example is a coin, whose resources are released
by a `defer` whatever ends the task.

Atmos, `sdl-tux/game/session.atm`:

```
task Coin (life, objs, x, y) {
    val id = obj_new(x, y, 32, 32, COL.TOUCHABLE, KIND.OTHER)
    val sprite = "images/objects/coin/coin.sprite"
    val l = look_new(sprite, x, y, LAYER_OBJECTS - 1)
    `tg_looks[$l.u.i].obj = $id.u.i;`
    set pub = [resp=HIT.ABORT]
    set life.byid@id = task
    defer {
        set life.byid@id = nil
        `tg_obj_del($id.u.i);`
        `tg_look_del($l.u.i);`
    }
    await :hit until (it@0 == PLAYER) && `:boolean tg_touches_player($id.u.i, -0.1f)`
    set status.coins = math.min(status.coins + 1, 9999)
    val px, py = `:number tg_objs[$id.u.i-1]->bbox.x1`, `:number tg_objs[$id.u.i-1]->bbox.y1`
    spawn @objs BouncyCoin(px, py, false, sprite)
}
```

### pico

`ia-atmos/env-pico` is not a port of one game: it is a 2D graphics
library for Atmos, a binding over the C library `pico-sdl`, with three
games as examples: `birds`, a tutorial in 11 steps; `rocks`, a
spaceship shooter for 2 players; and `storm`, a port of
`ceu-sdl-storm` [env-pico/README.md]. The repository documents no
comparison with an original, no table of state variables, and no
measures; the authorship and the origin of `birds` and `rocks` are not
stated in the files read (to confirm with the author). Its use for the
paper would be as a source of short examples written directly in the
language, not as a subject of the evaluation. The example is the
"press enter" screen of `rocks`: a blinking text, aborted by the key.

Atmos, `env-pico/exs/rocks/main.atm`:

```
    watching :key.dn [key=:Return] {
        ;;;
        ;; Spawns the blinking message, and awaits a :Return press.
        ;;;
        loop {
            ;; 500ms on
            watching 500ms {
                val p = [ '%', x=0.5, y=0.5, h=0.075 ]
                loop on :draw {
                    pico.set.pencil [ color=:white ]
                    pico.output.draw.text("= PRESS ENTER TO START =", p)
                }
            }
            ;; 500ms off
            await 500ms
        }
    }
```

# 2. Method

## 2.1 Who wrote the ports

The ports were written by AI coding agents, under the direction of the
author of the language. The repositories support this claim to
different degrees:

- Flare states it: "This port is written with an AI assistant (Claude
  Code), with Francisco Sant'Anna directing the work and reviewing it.
  The assistant writes the code, the tests, the tools, and the
  documents of this repository" [sdl-flare/README.md].
- Each port has a plan, written in the session that maintains the
  compiler and "sent to the session" that executes it in the
  repository of the port [plans/261111-flare.md,
  plans/261111-snkrx.md, plans/261112-raylib.md,
  plans/261112-lessmilk.md, plans/261113-doom.md,
  plans/261113-sdl-tux.md]. The plans separate what the session
  settles alone (design and fidelity decisions, written down) from
  what needs the author (installing packages, scope beyond the plan,
  what to do next) [plans/261111-flare.md].
- In the clones read here (shallow, one commit each), the last commit
  of every port has the author name "Claude" (git log of the clones).
- The READMEs of the other ports (Pingus, SNKRX, Lessmilk, raylib,
  Doom, SuperTux) do not state how they were written: to confirm with
  the author, together with which models and how many sessions.

Two ports start from earlier work by people. The Pingus port follows
the Céu rewrite of 2018, by the author of Atmos
[sdl-pingus/README.md]. The Lessmilk port starts from a student's port
of the twelve games, and the SNKRX port read a student's partial port
as a starting point [ray-lessmilk/README.md, sdl-snkrx/README.md].

## 2.2 Fidelity, then expressiveness

The ports follow one rule, set by the author: once a port is fully
faithful to its original, it may relax exactness in favor of
expressiveness, as long as the intent of the original author is not
affected; until then, a difference is a defect [plans/261117-envs.md,
plans/261114-pico-1oom.md]. The rule has two consequences for the
evaluation.

First, the code that is measured is constrained by the original: a
port keeps what the original does by accident (globals never reset, a
counter shared by two things, the drawing that changes the state) when
dropping it would change a frame [env-ray/doc/case-study.md].

Second, relaxations are explicit. In the raylib port, the branch
`faithful` is the last version identical on all runs; each relaxation
is a commit of its own, with the difference that it causes listed per
game [env-ray/doc/fidelity.md]. In the table of that port, 14 of the
23 games are marked "relaxed" and 9 "yes" (counted here from
[env-ray/doc/table.md]). Several relaxations fix what the port's
document describes as defects of the C: the original Tetris never
clears a full line, because of the start of a loop
[env-ray/doc/fidelity.md].

## 2.3 How a port is compared with its original

The procedure was fixed after the first port, as a list of lessons
[plans/261111-flare.md], and has the same parts in every port.

**The original, instrumented.** The original is built in the same
container as the port, with a patch that prints a trace in the exact
format of the port. In Pingus, the patch prints a line per change of
action (`time pingu id action x y`), and a second mode prints one
pingu at each tick, to find the first tick where the two games differ
[sdl-pingus/doc/fidelity.md]. Reading the C++ found 3 differences; the
traces found about 15 [plans/261111-flare.md]. In Flare, the patch
also replaces `rand()` by a seeded generator, reads scripted input,
and can print every call to the generator; the port must call its
generator at the same places, including draws that have no visible
effect, such as the choice of a sound of steps with the audio off
[sdl-flare/doc/fidelity.md]. In Lessmilk, the originals run in
Chromium with no screen, with a harness loaded before Phaser: a seeded
`Math.random`, a clock that moves only when a frame is stepped, and a
2D canvas that can be read back [ray-lessmilk/doc/fidelity.md]. In the
raylib games, the test mode is a patch to the library itself, so the
original and the port link the same code [env-ray/doc/fidelity.md]. In
Doom, the reference is the engine all in C, built from the same tree
with `c/main.c` in the place of the Atmos program [doom/README.md].

**A test mode with no screen in the port.** Each port runs with no
window, with fixed frames, a fixed seed, scripted keys and mouse,
screenshots at chosen frames, and no frame cap
[sdl-pingus/README.md, env-ray/doc/fidelity.md]. Every test, replay,
and benchmark uses it [plans/261111-flare.md].

**Stored traces and screenshots.** The suite of each port compares its
output with stored results: 25 scripted levels in Pingus, one per
action and per object [sdl-pingus/README.md]; 55 runs over the 23
raylib games, compared in their trace lines and, pixel by pixel, in
their screens [env-ray/README.md, env-ray/doc/fidelity.md]; 13 runs in
Doom, each a line per frame with a checksum of the simulation and a
checksum of the 320x200 screen [doom/README.md, doom/doc/study.md]; 30
runs in Flare, with pixel comparisons of maps, enemies, and hazards
[sdl-flare/README.md]. In Lessmilk, each game has 2 or 3 runs that are
the same as the original on all their trace lines
[ray-lessmilk/doc/table.md]; text is compared by eye, since fonts
differ between the browser and raylib [ray-lessmilk/doc/fidelity.md].

**Replays of the demos of the original.** Pingus ships 159 demos,
recordings of the actions given to the pingus. The official game plays
154 of them, and 149 of the 154 give the same trace in the port:
"every pingu, every change of action, at the same tick and the same
pixel" [sdl-pingus/doc/fidelity.md]. The 5 that differ do so late, by
one pixel or one tick [sdl-pingus/doc/fidelity.md]. Doom replays the
three demos of the shareware data [doom/README.md].

**Formulas and timings, where a trace cannot match.** SNKRX is not
compared tick by tick: positions come from Box2D, and choices from 675
calls to the random generator [plans/261111-snkrx.md]. The plan
compares timings of events, formulas against values printed by the
original, the tables of content, and distributions over many runs
[plans/261111-snkrx.md]. The port reports that the menu reaches the
shop in tick 113 in both, and that the first marker of level 1 comes
269 ticks after the arena starts in the port and 272 in the original
[sdl-snkrx/doc/case-study.md].

**Numbers as the original.** Atmos numbers are doubles; the C and C++
originals use 32-bit floats. The ports either move the arithmetic to
C (the mover of Pingus [sdl-pingus/doc/fidelity.md]; the player of
SuperTux [sdl-tux/game/player.h]) or round explicitly (`ray.f32`
[env-ray/doc/case-study.md]). In Lessmilk, the port follows the
minified Phaser that the games load, not its source: the minifier
reorders one expression, which rounds differently, and a trace
differed after 568 frames [ray-lessmilk/doc/fidelity.md].

## 2.4 How each measure is taken

**Lines and tokens.** Lines are counted with no blank lines and no
comments, in all ports [sdl-pingus/doc/loc.md,
env-ray/doc/table.md, ray-lessmilk/doc/table.md,
sdl-snkrx/doc/bench.md, doom/doc/study.md]. Tokens (identifiers,
numbers, strings, tags, operators) are counted by a script per port
(`tools/loc.py`, `tools/count.py`) [sdl-pingus/doc/loc.md,
env-ray/doc/table.md]. The trace code of a port is left out
[env-ray/doc/table.md]. In the raylib port the table is generated from
the sources: "no number here is typed by hand" [env-ray/doc/table.md].
The ports report tokens next to lines because the two diverge
(Section 3.1). What each count includes differs per port and is stated
in each: Pingus counts per unit only what all versions have, and
leaves out the editor and the recording of demos
[sdl-pingus/doc/loc.md]; the raylib and Lessmilk counts leave out the
environment and the shared modules [env-ray/doc/table.md,
ray-lessmilk/doc/table.md].

**State variables.** Each port lists the variables of the original
that live across frames and classifies them; a variable "that only
controls the flow" (a counter of frames, a timestamp compared with the
clock, a flag that guards a phase, a current state) is a control
variable, and the table reports how many the original has and how many
the port leaves [env-ray/doc/state.md, ray-lessmilk/doc/state.md,
sdl-pingus/doc/state.md, sdl-snkrx/doc/state.md,
sdl-flare/doc/state.md]. The variables that remain are listed with the
reason.

**Memory and CPU.** Both programs run in the same container, one after
the other, with the same scripted input and seed, and no screen.

- Pingus: 10 s per scenario, with `tools/measure.py` (peak memory, CPU
  time, frames), on the software renderer of SDL2 with the dummy video
  driver, sound off, with and without the cap of 40 frames per second;
  the noise between runs is about 10% [sdl-pingus/doc/bench.md].
- SNKRX: each run is a process measured by `wait4` (clock, CPU time of
  all threads, peak resident memory), three runs, the median; both
  programs on a virtual display with OpenGL rendered on the CPU
  (llvmpipe), every frame drawn, no cap [sdl-snkrx/doc/bench.md].
- raylib games: one scripted run per game, cut at 1200 frames, median
  of 3 runs, with no frame cap, on the software renderer of raylib;
  CPU per frame, frames per second, peak resident memory, and size of
  the binary [env-ray/doc/bench.md].
- Doom: tics per second on the three demos, the best of 7 runs, with
  the time to start (7 ms) taken out, with and without rendering; the
  runs with no rendering take 15 to 25 ms on a clock with a grain of
  1 ms, an error of about 5% [doom/doc/bench.md].
- Flare: only the baseline of the original is measured so far (600
  logic frames, two runs) [sdl-flare/doc/bench.md]. Lessmilk: memory
  and CPU are announced "once measured", and no `doc/bench.md` exists
  in the clone [ray-lessmilk/doc/table.md].

**Whole frame and game logic.** The outline asks for CPU as two
numbers. The ports show why: with a software renderer, the frame is
mostly drawing. Three ports separate the logic. SNKRX has a profile
mode that splits a run into logic, drawing, and rendering: in level 1,
0.23 s, 0.35 s, and 8.10 s; with 200 more enemies, 3.90 s, 0.56 s, and
8.57 s [sdl-snkrx/doc/bench.md]. Doom reports each demo with and
without rendering [doom/doc/bench.md]. The raylib port has a stress
test that scales the number of enemies of one game from 10 to 5000,
each a task in the port and a slot of an array in the C: each enemy
adds 0.75 µs per frame in the C and 1.39 µs in the port
[env-ray/doc/bench.md]. Pingus reports a micro benchmark of the wake
of a task, about 55 ns [sdl-pingus/doc/bench.md]. A uniform measure of
the logic alone, across all ports and their originals, does not exist
yet.

# 3. Threats to validity

## 3.1 Construct validity

**Lines of code as a proxy for expressiveness.** Lines and tokens
disagree, in both directions of interest. The Lessmilk ports have 8%
more lines and 23% fewer tokens than the JavaScript
[ray-lessmilk/doc/table.md]; the SNKRX port has 83% of the lines and
62% of the tokens of the game in Lua [sdl-snkrx/doc/case-study.md].
The SNKRX port measured why: Lua is denser (13.0 tokens per line
against 9.7), a timer with its callback often fits in one line, and
19% of the lines of the port hold only a closing brace
[sdl-snkrx/doc/case-study.md]. Neither measure isolates control: in
SNKRX, the lines that call a timer or close a callback are 6% of the
original, and "about four fifths of both" programs are formulas,
tables, and drawing [sdl-snkrx/doc/case-study.md].

**Reductions that are not due to the language.** Part of each
reduction comes from decisions that any rewrite could take. In
skully_escape, 2685 lines of C become 404 [env-ray/doc/table.md]
mostly because seven copies of one file become one task and a table
per room, which "is not specific to Atmos; it is what a port by hand
allows" [env-ray/doc/case-study.md]. In the intermission of Doom, 150
lines of C that load and free graphics disappear by "a choice that C
could make as well, and not a gain of the language"
[doom/doc/study.md]. In Pingus, the data reader of the original is
replaced by a converter in Python that is not counted, and the engines
are "not like for like" [sdl-pingus/doc/loc.md]. In Lessmilk and
raylib, the shared modules and the environment are left out of the
count of each game [ray-lessmilk/doc/table.md, env-ray/doc/table.md].

**State variables as a measure.** The classification of a variable as
control or data is made by the same agents that wrote the port, under
a rule stated in each `doc/state.md`, with no second rater. The counts
are not yet consistent: for the raylib games, 38 control variables
left in one document and 35 by the sum of the table
[env-ray/doc/case-study.md, env-ray/doc/table.md]; one Lessmilk game
(Connected) is "not counted" [ray-lessmilk/doc/table.md].

**Fidelity is not uniform.** The claim "the same game" has a different
meaning per port. Pingus is tick-exact on 149 of 154 demos, and the
rest differ by details of float arithmetic and of the order of deaths
in a tick [sdl-pingus/doc/fidelity.md]. SNKRX is not tick-exact by
design (positions come from the physics library, and choices from 675
random calls) and is compared by timings and formulas
[plans/261111-snkrx.md]. Of
the raylib games, 14 of 23 were relaxed on purpose after being
identical (counted here from [env-ray/doc/table.md]). Everywhere,
identity is established on the scripted runs only: the ports list what
no run reaches (two keys in the same frame in snake
[env-ray/doc/fidelity.md]; in Doom, the cast, the scroll, and the skip
of the finale "were ported by reading the C, and have never run",
since the shareware data has only episode 1 [doom/doc/study.md]).
Sound is ported but not checked: the container has no sound device
[env-ray/doc/fidelity.md].

**Fidelity and expressiveness pull against each other.** Being
identical frame by frame forces a port to keep accidental behavior of
the original [env-ray/doc/case-study.md], and in Flare it keeps timers
and flags as state so that the random generator is called in the
original order [sdl-flare/doc/state.md]. The measured code is
therefore not the code that a programmer would write in Atmos from
scratch, and the bias goes against the language. The relaxed ports go
the other way, and their differences are listed
[env-ray/doc/fidelity.md].

## 3.2 Internal validity

**Who made the ports.** The ports were written by AI agents directed
by the designer of the language (Section 2.1), with the explicit goal,
in every plan, of "no state variables for control"
[plans/261110-pingus.md, plans/261111-flare.md,
plans/261111-snkrx.md]. The reduction of control variables is thus a
design rule of the ports, checked in review [sdl-pingus/doc/state.md],
and not an outcome observed in independent programmers. Three ports
also had a model to follow: the Céu rewrite of Pingus, and the student
ports of Lessmilk and SNKRX (Section 2.1). The comparison documents,
including the judgments of what "reads worse" [doom/doc/study.md],
were written by the same agents.

**The language changed during the evaluation.** The ports reported
findings to the compiler, and some were fixed while the ports were
made: three during the SNKRX port [sdl-snkrx/doc/case-study.md]; one
commit of the compiler changed the order of a loop over a pool and,
with it, the traces of three Pingus demos
[sdl-pingus/doc/fidelity.md]; a build option for 32-bit reals was
added for SuperTux [sdl-tux/.claude/plans/TODO.md]. The language under
evaluation is in part a product of the evaluation.

**C kept in the ports.** Every port keeps C for hot loops and for
exact float arithmetic, and the measures of the port include it:
`engine/pingus.h` with 727 lines and the mover of a pingu
[sdl-pingus/doc/loc.md, sdl-pingus/doc/fidelity.md]; the collision
map, the drawing of the map, and path finding in Flare
[sdl-flare/README.md]; Box2D in SNKRX [sdl-snkrx/README.md]; the board
of raylife [env-ray/doc/bench.md]; 183 of the 445 lines of the finale
of Doom [doom/doc/study.md]; in SuperTux, the whole player and the
collisions (Section 1.7). A CPU or memory result of a port is a result
of Atmos plus this C, and a line count that reports only the Atmos
files understates the port.

**Different runtimes on each side.** Only the raylib games and Doom
keep the library and engine constant between original and port. In
SNKRX, the original runs on LÖVE 11.5 with LuaJIT and OpenAL, and the
port on SDL2 with its own build of Box2D v3 (the version of the physics
inside LÖVE is not stated in the repositories); the peak memory is
946 MB against 131 to 141 MB, and "why the original needs that much
was not investigated" [sdl-snkrx/doc/bench.md]. The speed ratio of 1.8
to 3.0 [sdl-snkrx/doc/bench.md] cannot be attributed to the language
alone. In Lessmilk, the original would run in a browser. In Pingus,
the engines differ (Section 3.1).

**Rendering hides the logic.** All measures use software rendering in
a container. In Pingus, "the frame time is spent in the software
renderer of SDL, not in the tasks" [sdl-pingus/doc/bench.md]; in the
raylib games the sum of the CPU per frame of the ports is 1.02 times
that of the originals, and the drawing "hides the cost of the logic"
[env-ray/doc/bench.md]. Where the logic is isolated, the port is
slower: 1.70 times the C with 5000 enemies [env-ray/doc/bench.md];
uncapped, the official Pingus draws 1.2 to 1.3 times the frames of the
port [sdl-pingus/doc/bench.md]. Results at the frame cap are an upper
bound on parity, not evidence of it.

**Measurement error and port bugs.** The noise between runs is about
10% in Pingus [sdl-pingus/doc/bench.md] and about 5% in Doom
[doom/doc/bench.md], with 2 to 9 runs per number
[sdl-flare/doc/bench.md, doom/doc/bench.md]. A benchmark is only
as valid as the port: in the first stress runs of SNKRX the units of
the port never attacked, because of a task that ended with its block,
and no trace showed it [sdl-snkrx/doc/bench.md,
sdl-snkrx/doc/case-study.md]. The test mode of raylib also changes the
originals: without the patch, the arrow keys are never released
[env-ray/doc/fidelity.md].

## 3.3 External validity

**Selection.** The games were chosen by the author; all are open-source
games that build in a container (the criteria of the choice are to
confirm with the author). The subjects are 2D games, with two
exceptions: one raylib game in 3D (retro_maze_3d), drawn by the
library [env-ray/doc/fidelity.md], and Doom, whose renderer and
simulation stay in C [doom/README.md]. No 3D engine is written in the
language. No subject uses a general engine with an editor (Unity,
Godot, Unreal), which is where the coroutines discussed in the related
work live.

**No network, and no sound in some builds.** The Doom port is built
with no sound and no network [doom/README.md]; its test `net` is a
cooperative game of one player [doom/README.md]. No port has
networked play. SNKRX leaves out Steam [sdl-snkrx/README.md].

**Size and completeness.** By count, small games dominate: 35 of the
games are raylib or Lessmilk games [env-ray/README.md,
ray-lessmilk/README.md]; the largest of their originals has 4689
lines, and 19 of the 23 raylib originals have fewer than 1000 (counted
here from [env-ray/doc/table.md]). Their results should not be
averaged with those of the large ports. Of the large ports, two are
complete (Pingus, SNKRX) and two are slices: Flare stops before the HUD and the menus, which are
14544 of the 57843 lines of the engine [sdl-flare/README.md,
plans/261111-flare.md]; Doom moves the control around the simulation,
which the plan estimated at about 11 thousand of 45 thousand lines,
and so far has part of it (not the menus, the messages, nor the
automap) [plans/261113-doom.md, doom/README.md]. The parts left out of
Flare are not the same kind of code as the parts ported.

**Ports, not new games.** A port preserves the architecture of the
original where fidelity demands it (the frame order of koala_seasons
is kept "as functions over a table of statics"
[env-ray/doc/case-study.md]). The results say how existing control
code translates, not how a game designed for the language would look,
nor what it costs to learn the language.

**No user study.** No programmer other than the agents and the author
wrote or read the code under measurement. Claims about readability
rest on code excerpts and on the documents of the ports. The change
study of the outline (the same change made in the original and in the
port by an AI) is not designed yet, and would itself measure an AI
rather than a person.

## 3.4 Reliability

**What supports replication.** Each port pins the commit of its
original, of the compiler, and of the libraries
[sdl-pingus/README.md, sdl-snkrx/doc/bench.md, env-ray/README.md,
doom/README.md], keeps the patch of the original in the repository
with the script that builds it, and stores the expected traces and
screenshots. The suites run with no screen.

**What limits it.**

- Data that is not in the repositories: the shareware `DOOM1.WAD` is
  fetched by a script [doom/README.md]; the art of Flare is read from
  a checkout of `flare-game` [sdl-flare/README.md]; the Lessmilk
  originals come from a copy of the site kept by a third party, which
  has the assets of two games only, the rest coming from the student
  port [ray-lessmilk/doc/fidelity.md].
- Measures taken in one cloud container, on one processor model
  (an Intel Xeon at 2.10 GHz) [sdl-snkrx/doc/bench.md,
  env-ray/doc/bench.md, doom/doc/bench.md], with software rendering; a
  real window, sound, and macOS were not checked
  [sdl-pingus/README.md].
- Documents of one port that disagree, because they were written at
  different stages: the size of the SNKRX port is 4755 lines in the
  benchmark document, written when the port covered a part of the
  game, and 8647 in the case study [sdl-snkrx/doc/bench.md,
  sdl-snkrx/doc/case-study.md]; the timer calls of the original are
  375 in the plan and 422 in the case study [plans/261111-snkrx.md,
  sdl-snkrx/doc/case-study.md]; the README of Lessmilk lists 11 games
  in its table of state, without Connected, which the table of the
  port has [ray-lessmilk/README.md, ray-lessmilk/doc/table.md]. The
  paper should take every number from one regenerated table per port,
  at a commit that it cites.
- The numbers of this draft were read from the documents of the ports,
  not measured again.

# 4. Open points for the author

- How the ports were written, beyond what Flare states: models,
  number of sessions, and the share of the author's review (2.1).
- The name of the SNKRX repository to cite (1.2).
- raylib: 38 or 35 control variables left (1.5).
- Flare, Lessmilk, SuperTux: no measure of memory and CPU of the port
  against the original yet (2.4); Flare: no line count of the port.
- A uniform measure of "game logic alone" for the table of the paper
  (2.4).
- Whether SuperTux and pico enter the paper, and in what role (1.7).
- The variant of Doom with a door type and one monster as tasks,
  described in the plan [plans/261113-doom.md]: its state was not
  found in the clone.
