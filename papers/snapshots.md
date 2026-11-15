# State is Data, Tasks are Disposable: Snapshots and Commands for Structured Reactive Programs

Proposed name of the architecture: **snapshots and commands**.

Alternative titles:

- Snapshots and Commands: A Persistence Architecture for Synchronous Programs
- Tasks as a Function of Data: Saving, Resuming, and Replaying Reactive Programs
- System Prevalence Meets Structured Concurrency

> Status: outline, first version, 2026-11-15. The idea is the author's
> (item 64 of `.claude/plans/TODO.md`, 2026-11-14). Nothing here is
> implemented or measured: the sketches were not compiled, and no port
> follows the architecture yet. What waits for the author and for other
> sessions is in `.claude/plans/TODO-paper-snapshots.md`.

## Abstract

Structured concurrency lets a reactive program keep its state where it is
used: in the local variables and in the program counter of tasks.
This is what makes the code readable, and also what makes the program hard
to save: a running task is not data.
We propose an architecture that keeps both.
All state that must survive a restart lives in one table of plain data, the
snapshot, from which the program can spawn every task it needs.
The table changes only in reaction to a fixed set of command events.
Everything else, such as animations, timers, input in progress, and
communication, lives in tasks and is disposable: a restart throws it away.
From the first snapshot, the journal of commands, and periodic snapshots, a
run can be saved, resumed, and replayed, forwards and backwards.
The architecture is known in databases as system prevalence, and resembles
event sourcing and the Elm architecture.
Our point is what a synchronous language adds to it: reactions are atomic,
so a snapshot needs no transaction; execution is deterministic, so a replay
needs no further discipline; and blocks own tasks, so a restart is the
abortion of a block.
For action games, we restrict saves to checkpoints, which keeps the state
between two checkpoints in tasks.
We present the architecture in Atmos, its rules, a test that detects hidden
state, and case studies.

## 1. Introduction

- Problem: to save and resume a reactive program
    - a save, a crash, an undo, a replay, a second device
- Tension
    - structured concurrency moves state into tasks: locals and the program
      counter
    - a task is not data: it cannot be written to a file
- Three known ways out, each with a cost
    - snapshot of the tasks by the runtime: closures, pools, native values
    - replay of all input since the start: grows with time, breaks when
      the code changes
    - tasks that can be entered in the middle: the code is a state machine
      again
- Thesis: separate what persists from what is disposable, by design
    - what persists is data, in one table
    - tasks are a function of the table
    - the table changes only by commands
- Contributions
    - the architecture and its rules
    - what the synchronous model gives for free
    - checkpoints as the form for action games
    - a test for hidden state, automatic under determinism
    - case studies

## 2. Background

- Atmos in one page: tasks, `await`, `emit`, `par`, `watching`, pools
- Synchronous and deterministic
    - tasks yield only at an `await`
    - an `emit` runs to completion
    - tasks awake in a fixed order
    - time is an event, not a clock that tasks read
- Data: tables, `X.dump` and `X.load`, with no code and no cycles
- Tags as hierarchies: `:cmd.move` is matched by an await on `:cmd`
- Blocks own tasks and values: abortion, `defer`, `pin`
    - refer to the paper on pin (`papers/pin.md`)

## 3. The Architecture

- Four parts
    - snapshot: one table of data, from which everything is spawned
    - commands: `:cmd.*` events, the only writers of the table
    - disposable tasks: read the table, emit commands, never write
    - journal: first snapshot, commands in order, periodic snapshots
- Operations
    - save: dump the table between two reactions
    - resume: load the last snapshot, spawn, apply the commands after it
    - go to an instant: the nearest snapshot before it, then the commands
    - backwards at a speed: a sequence of such jumps, with a cache
- A small example: notes on a board
    - a note is a row; a drag is disposable; the drop is `:cmd.move`
    - one task applies commands and writes the journal
- Figure: the table, the applier, the tasks, the journal

## 4. The Rules

- R1. The table holds data only: what `X.dump` writes
- R2. Only the applier of commands writes to the table
- R3. The applier is a function of the table and the command
    - no clock, no input, no random number that is not in one of them
- R4. A task holds no state that outlives a command and affects a later one
- R5. Every task can be spawned from its row, in any valid state of the row
- R6. Spawning and aborting in reaction to a command gives the same tasks
  as spawning from the resulting table
- R7. Only commands from outside enter the journal
    - a command emitted by the applier is not recorded
- R8. A command is validated before it writes
- What each rule protects, and what breaks without it

## 5. What the Synchronous Model Gives

- Atomicity: a reaction runs to completion
    - the table is consistent between commands
    - no transaction, no lock
- Determinism: same table and same commands, same result
    - in other languages this is the costly part of the pattern
- Time as an event
    - a faithful replay re-emits time between commands
    - animations come back; raw input does not
- Structured abortion: a restart is the end of a block
    - nothing disposable survives it
    - one path for "die", "load", and "go back"
- Hierarchical tags: one await on `:cmd` sees every command

## 6. Time, Randomness, and the Outside

- Is the tick a command?
    - yes, if the state that persists changes with time
    - then the journal is a recording of input
- Random numbers
    - resolved before the command and carried in it, or
    - a seed in the table
- Communication and devices
    - the task that talks to the outside is disposable
    - what it learns enters as a command
- Two kinds of replay
    - logical: commands only, state to state
    - faithful: commands with their instants, plus time

## 7. Checkpoints: Action Games

- The hard case: state that persists changes at every tick
- The restriction: a save happens only at a checkpoint
- Consequences
    - the snapshot is small: level, checkpoint, inventory, seed
    - commands are few: checkpoint, death, level, lasting pickups
    - all between two checkpoints is disposable
    - a level starts from each checkpoint, not from any state
    - the state between checkpoints can live in tasks
- A second, optional layer: input since the last checkpoint
    - a demo that starts at the checkpoint
    - replay, rewind, tests
- What is lost: a save at any point
- Design decisions made explicit: which pickups survive a death

## 8. Detecting Hidden State

- The risk: a local variable that the table does not know
- The test
    - at any point: dump, abort all, load, spawn
    - the same commands must give the same table afterwards
- Automatic under determinism: any recorded run is a test
- What a compiler could check
    - writes to the table outside the applier
    - open

## 9. Implementation

- The applier, the journal, and the snapshot in Atmos
- Cost
    - a dump of the whole table
    - a replay from the last snapshot
    - the interval between snapshots as the trade-off
- The shape of the data: no cycles, references as ids
- Versions: a journal against changed code
    - snapshots age better than journals

## 10. Case Studies

- To be chosen with the author; none follows the architecture yet
- Doom
    - the saved game as the origin of the idea
    - the demos of the original as the oracle
    - after the faithful port: checkpoints, and things as tasks instead of
      rows of the state table
- A game with checkpoints or by turns, among the ports
- An application that is not a game: an editor
- Measures: size of the snapshot, commands per minute, time to resume,
  hidden state found by the test

## 11. Limitations

- Action games with a save at any point
- Rule R6 has two paths to keep equal
- Rule R4 is not checked by the compiler
- Journals and changes of code
- State in memory: the table must fit

## 12. Related Work

- System prevalence: Prevayler
    - the closest: memory, journal of commands, periodic snapshots
    - no tasks, no user interface
- Event sourcing and CQRS
    - records facts; ours records intentions, and needs determinism
- Command and Memento patterns: the two parts, not the architecture
- The Elm architecture and Redux
    - one state, actions, time travel
    - the view is a pure function; our tasks have a life of their own
- Replicated state machines: Raft
    - a log of commands, snapshots to compact it
- Deterministic lockstep and replays in games; rollback netcode
    - records raw input, not commands
- Record and replay debugging: rr, QEMU, TARDIS
    - machine level; no separation of what matters
- Snapshots of processes and continuations
    - the way we do not take
- Synchronous languages: Esterel, Céu
    - determinism used for testing and replay

## 13. Future Work

- Distribution
    - a total order of commands among peers is a replicated state machine
    - Freechains as the journal
    - when the order changes: back to a snapshot, apply again
- A check in the compiler for rules R2 and R4
- Undo as an operation on the journal
- Migration of journals between versions

## 14. Conclusion

- What persists is data; tasks are a function of it
- A synchronous language makes the old pattern cheap
- Checkpoints give tasks back to action games
