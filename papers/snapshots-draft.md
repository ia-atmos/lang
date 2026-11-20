# Drafts for `papers/snapshots.md`

> Status: DRAFT, written by the `paper-snapshots` session on 2026-11-20
> under the rule "work with no input" (`atmos-lang#95`), to be confirmed
> by the author (`paper-snapshots#8`). Only the sections that do not
> depend on the open items `#1` to `#5`: Section 3 (the architecture and
> its example) and Section 9 (detecting hidden state). The code is
> quoted from `papers/snapshots/board.atm`, which compiles and runs on
> `main`; its output is `papers/snapshots/expected.txt`. The rule
> numbers R1 to R8 are those of the outline, still open (`#4`).

## 3. The Architecture

A program in this architecture has four parts.
The *snapshot* is one table of plain data: numbers, strings, tags, and
tables of them, with no functions, tasks, or cycles.
It is what `X.dump` writes and `X.load` reads back.
Everything that the program must remember across a restart is in it, and
the program can spawn all of its tasks from it.
The *commands* are events whose tags descend from `:cmd`, such as
`:cmd.add` and `:cmd.move`.
They are the only way to change the snapshot.
The *disposable tasks* are everything else: they read the snapshot, react
to input, animate, and emit commands, but never write to the snapshot.
The *journal* is the first snapshot, the commands in the order they were
applied, and a snapshot taken every few commands.

From these parts come four operations.
To *save* is to dump the table between two reactions.
To *resume* is to load the last snapshot, spawn the tasks from it, and
apply the commands that came after it.
To *go to an instant* is to load the nearest snapshot before it and apply
the commands up to it.
To go *backwards*, at any speed, is a sequence of such jumps.

We illustrate the architecture with notes on a board.
The user adds a note with a key and a click, drags a note to move it, and
deletes one with another key and a click.
The snapshot holds the notes, keyed by an id, and the next id:

```
[seq=9, next=5, notes=[@(2)=[x=60, y=60], @(3)=[x=30, y=20], @(4)=[x=40, y=40]]]
```

A single function, the *applier*, changes the table.
It receives the table and a command, and either applies the command and
returns `true`, or rejects it and writes nothing:

```
func apply (S, c) {
    match c {
        :cmd.add => {
            val id = S.next
            set S.next = id + 1
            set S.notes@(id) = [x=c.x, y=c.y]
            true
        }
        :cmd.move => {
            val n = S.notes@(c.id)
            if n == nil => false => do {
                set n.x = c.x
                set n.y = c.y
                true
            }
        }
        <...>   ;; :cmd.del
    }
}
```

One task awaits every command, since an await on `:cmd` matches all the
tags below it.
It is the only writer of the table.
It applies the command, appends it to the journal as data, takes a
snapshot every `EVERY` commands, and announces that the table changed:

```
spawn {
    loop c on :cmd {
        if apply(S, c) {
            set S.seq = S.seq + 1
            set journal@+ = X.dump(c)
            if S.seq % EVERY == 0 {
                set snaps@+ = [seq=S.seq, s=X.dump(S)]
            }
            emit @:global :applied [cmd=c]
        }
    }
}
```

A note is a task.
It lives while its row does, and a drag in progress lives in its local
variable `p`.
Only the drop leaves the task, as a command:

```
task Note (S, id) {
    set pub = id
    watching :applied until S.notes@(id) == nil {
        loop {
            await :mouse.down until inside(S.notes@(id), it)
            var p = nil
            watching :mouse.up {
                loop m on :mouse.move {
                    set p = m
                }
            }
            if p {
                emit @:global :cmd.move [id=id, x=p.x, y=p.y]
            }
        }
    }
}
```

The drag is the kind of state that structured concurrency keeps where it
is used, in a local and in the position of an `await`.
It is also the kind of state that a restart may lose: a drag that is cut
in the middle is simply not a move.

The board spawns one note for each row, in the order of the ids, and one
more after each `:cmd.add`.
A restart aborts the board, replaces the table, and spawns the board
again:

```
func restart (text) {
    loop _,b in boards {
        abort b
    }
    set S = X.load(text)
    spawn @boards Board(S)
}
```

The abortion of the board finalizes its notes and every drag in progress.
The assignment to `S`, a `pin var` of the outermost block, releases the
old table once nothing refers to it.

The program feeds a script of input, as the test modes of the ports of
games to Atmos do, and then checks four claims.
The journal has nine commands and four snapshots (at 0, 3, 6, and 9).

- *Resume*: the last snapshot plus the commands after it give the final
  table.
- *Go to an instant*: for every instant from 0 to 9, the nearest snapshot
  before it plus the commands up to it give the table recorded at that
  instant.
- *Same tasks* (R6): the notes spawned along the run, by commands, are
  those spawned from the final table (ids 2, 3, and 4).
- *No hidden state* (R4): see Section 9.

All four hold.
The replays call the applier alone: no task is spawned to replay, which
is what makes a jump in time cheap.

## 9. Detecting Hidden State

The architecture holds only if no task keeps state that the table does
not know and that affects a later command (R4).
Such state is easy to write by accident, because it is the natural style
of structured concurrency: a counter in a local, a phase in the position
of an `await`.

Determinism gives a test for it.
Run the same input twice: once without interruption, and once with a
restart at chosen points, where the program dumps the table, aborts its
tasks, loads the dump, and spawns its tasks again.
If no task holds hidden state, both runs end with the same table.

The points must be chosen.
A restart in the middle of a gesture loses the gesture, by design: the
drag of a note is disposable, and the input that follows the cut, the
rest of the drag, no longer means a move.
So the test cuts only between gestures.
The example marks these points in its input script, with `:cut`, and the
run with restarts cuts at each of them: ten restarts in a run of nine
commands.
The two runs end with the same table.

To see that the test finds hidden state, the example has a switch
`HIDDEN`.
With it, each note counts its drags in a local, and shifts each drop by
that count:

```
var drags = 0
<...>
val dx = if HIDDEN => drags => 0
set drags = drags + 1
emit @:global :cmd.move [id=id, x=p.x+dx, y=p.y]
```

The count outlives a command and affects the next one, and the table does
not hold it.
A restart sets it back to zero.
With the switch on, the two runs end with different tables, and the test
reports it.

The test needs no oracle: any recorded run of the program, with points
between gestures, is a test.
It is the same use of determinism as the recorded games of Age of Empires,
which began as a tool to reproduce bugs (Bettner and Terrano 2001), and as
the demos of Doom in the port to Atmos.
What it cannot do is prove the absence of hidden state: it only finds
the state that the recorded input exercises.
