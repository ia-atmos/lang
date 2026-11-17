# Snapshots and Commands: Structured Reactive Programs over a Permissionless Consensus

Proposed name of the architecture: **snapshots and commands**.

The author asked on 2026-11-16 that the title has one of: decentralized,
blockchain, permissionless, consensus, and left the choice open. The
title above is a placeholder. Alternative titles:

- State is Data, Tasks are Disposable: Snapshots and Commands for
  Decentralized Reactive Programs
- Tasks as a Function of Data: Decentralized Reactive Programs that
  Survive Reordering
- Deterministic Reactive Programs on a Blockchain: Snapshots, Commands,
  and Automatic Branch Resolution
- System Prevalence Meets Structured Concurrency, without a Server

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
The same journal makes the program decentralized: peers that agree on the
order of the commands reach the same table, with no server.
Over a permissionless consensus the order may change after the fact, when
branches meet; a peer then goes back to a snapshot and applies the commands
again in the new order, with no code from the programmer.
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
    - decentralized execution over a permissionless consensus, with
      reordering resolved by the architecture
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
    - one path for "die", "load", and "go back"
    - by the rules of pin (`papers/pin.md`, Section 4), cited by name
      and not restated here
    - rule "end": the block finalizes all that it pinned, in reverse
      order, with its `defer`s
    - rule "store": tasks hold rows, since a row belongs to an outer
      block; rows cannot hold tasks, so the table keeps no task alive
    - rules "result" and "pin": the only door out of a level is a fresh
      value stored in the table, which is, by definition, state that
      persists
    - what the rules do not see: global state inside a C binding
    - load is one assignment: `pin S` at the top, then
      `set S = X.load(...)`; the old snapshot is finalized when its last
      reference goes (rules of pin as changed on 2026-11-17)
        - tasks of the old level still hold rows of the old snapshot:
          abort the level first, and the old table goes at once
        - so the order of a restart is: abort, replace, spawn
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
- Not a new kind of restriction: Doom already forbids a save outside a
  level
    - the port keeps the screen wipe and the intermission in tasks, with
      state in locals and in the `await` where each task is suspended
    - no save covers them, and none has to
    - checkpoints widen the same window to the level itself
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

## 8. Decentralized Execution

> Added on 2026-11-16, from the author: commands are reordered by the
> consensus, and the resolution of branches is to be automatic. He
> approved the mechanism on the same day: back to a snapshot, and apply
> the commands again in the new order, with those that no longer apply
> rejected by all peers. The rest of the section is a proposal.

- From one machine to many
    - peers with the same snapshot and the same order of commands reach
      the same table: a replicated state machine
    - the journal is the only thing that travels
- A permissionless consensus as the journal: Freechains
    - a command is a post in a chain
    - posts form a graph: a partial order
    - the consensus gives a total order, which is the order of the journal
    - no server, no fixed set of members
- The problem: the order is not final
    - a peer applies a command when it arrives, before the others agree
    - when branches meet, commands may enter before ones already applied
- Automatic branch resolution
    - find the first position where the old and the new order differ
    - go back to the nearest snapshot before it
    - apply the commands again, in the new order
    - abort and spawn the tasks from the resulting table
    - the same operation as "go to an instant" of Section 3
- What a command must be, to survive a reordering
    - valid or rejected by the table alone (R8): a command that no longer
      applies in the new order is rejected by every peer alike
    - an intention, not a difference of state: "move note 3 to x,y", not
      "the table is now T"
- What the user sees
    - a state that is tentative until the consensus settles
    - a command of the user undone by a branch that arrives later
    - open: how to show it, and when a command is final
- What the tick is, among peers
    - time is local and disposable; only commands are shared
    - an action game among peers needs the tick as a command: lockstep,
      out of scope
- Snapshots among peers
    - a snapshot is valid for a position of the order that is final
    - a new peer starts from a snapshot and the commands after it
- Cost: the depth of a reordering, times the cost of a command

## 9. Detecting Hidden State

- The risk: a local variable that the table does not know
- The test
    - at any point: dump, abort all, load, spawn
    - the same commands must give the same table afterwards
- Automatic under determinism: any recorded run is a test
- What a compiler could check
    - writes to the table outside the applier
    - open

## 10. Implementation

- The applier, the journal, and the snapshot in Atmos
- Cost
    - a dump of the whole table
    - a replay from the last snapshot
    - the interval between snapshots as the trade-off
- The shape of the data: no cycles, references as ids
- Versions: a journal against changed code
    - snapshots age better than journals

## 11. Case Studies

- To be chosen with the author; none follows the architecture yet
- Doom (`ia-atmos/doom`; facts reported by its session on 2026-11-15)
    - the saved game as the origin of the idea
        - told in full here; the paper on games (`papers/games.md`)
          has one paragraph that points to this one
        - the simulation stays in C because a save must cover it: 967
          rows of states, 448 with an action, 137 types of object
        - Atmos has the control around it: frames, wipe, intermission
    - the oracle: ten runs, 34,770 frames, compared frame by frame
        - three demos of the original (10,996 tics), seven runs with
          scripted input
        - per frame: a checksum of the simulation and one of the screen
        - identical between the engine in C and the program in Atmos
    - state in tasks where no save can happen
        - the intermission: counters local to a task, the phase is an
          `await` (C: `static` variables and a state from 1 to 10)
        - the wipe: one task per column, its position a local (C: an
          array)
    - state that had to be global: what C kept from one intermission to
      the next
    - planned, not started: one door and one monster as tasks, with no
      saved games
    - after the faithful port: checkpoints, and things as tasks instead
      of rows of the state table
- The test modes of the ports: a run is a list of inputs
    - every port has one: scripted input, fixed frames, traces and
      screenshots compared
    - evidence for determinism, not for the architecture
- A game by turns: 1oom (planned, not started)
    - its saved game is the whole state, in a documented format, and a
      run is reproducible from a seed
- Flare (`ia-atmos/sdl-flare`; reported by its session on 2026-11-16)
    - the original saves stats, inventory, campaign, map and position,
      and cooldowns; the port has no save yet
    - the tasks of a map are already created from its file at each load
    - state in tasks that a save of tables would miss: a turn timer as
      the counter of a loop, the current action as which task runs, the
      rest of an effect as the progress of its task
    - a candidate for checkpoints: a save at the entry of a map
- An application that is not a game: an editor
- Measures: size of the snapshot, commands per minute, time to resume,
  hidden state found by the test

## 12. Limitations

- Action games with a save at any point
- Rule R6 has two paths to keep equal
- Rule R4 is not checked by the compiler
- Journals and changes of code
- State in memory: the table must fit
- Among peers: a command of the user may be undone later, and the cost
  of a deep reordering is not bounded by the architecture
- Determinism ends at native code
    - in Doom the screen differed between runs at first: the engine
      reads beyond the end of sprite columns, and pointers of the
      allocator reached the screen as pixels

## 13. Related Work

- System prevalence: Prevayler (Wuestefeld 2001); the pattern language
  "Prevalent Systems" (Johnson and Wuestefeld, SugarLoafPLoP 2014)
    - the closest: memory, journal of commands, periodic snapshots
    - its rules match ours: transactions are Commands (GoF), run one at
      a time to completion, are deterministic, keep I/O outside, and
      take the time from the transaction, not from the clock
    - no tasks; the clients are outside the prevalent system
    - its examples, each a related work for us
        - Smalltalk-80: the image is a snapshot, the changes file a
          journal (Goldberg 1984)
        - an in-memory database with a journal and one lock
          (Garcia-Molina et al. 1984)
        - Akka persistent actors: state changes only by messages,
          journal and snapshot, migration of an actor
        - LMAX: 6 million transactions per second (Fowler 2011)
        - Croquet: replicated 3D worlds; actions are transactions sent
          through a serializer, rendering is a query done locally
          (Smith et al. 2003); the closest to Section 8
        - Age of Empires and StarCraft: a save is a snapshot, a replay
          is a journal, multiplayer is replication; stated there with
          no citation
- Deterministic lockstep in games: the primary sources
    - Age of Empires: Bettner and Terrano, "1500 Archers on a 28.8:
      Network Programming in Age of Empires and Beyond", GDC 2001 and
      Gamasutra, 2001-03-22
        - read in full on 2026-11-17
        - every machine runs the whole simulation; only commands travel,
          each scheduled for a future turn (latency up to 250 ms is not
          noticed)
        - a player issues a command every 1.5 to 2 s on average: the
          journal is small compared with the work of the simulation
        - random numbers seeded and synchronized; "the same number of
          calls to random" was the hard part for the programmers
        - out of sync: checksums of the world, the objects, the paths;
          small differences multiply (a deer, then a villager)
        - recorded games (AoE2) began as "I could really use this for
          debugging": deterministic, so a recording is a repro case
          that "plays out the exact same way every time"
        - a side benefit: a client that runs differently is "out of
          sync" and stops, so cheating is hard
        - nothing on saved games: the claim of Johnson and Wuestefeld
          that a save is a snapshot has no source yet
        - the recorded game (.mgx) is a header with a snapshot of the
          initial state and the commands of each turn; a patch breaks
          old recordings (community format specs, not the developers)
    - StarCraft: no paper by Blizzard found
        - Wyatt (Blizzard), HandmadeCon 2015 interview: Warcraft 2 and
          StarCraft wait for the packets of every player each turn
        - replays: every command with its frame, replayed by the engine;
          old replays break with new patches (Liquipedia, OpenBW)
        - StarCraft II: "a deterministic state machine": initial state
          plus a list of events (sc2reader documentation)
    - the lesson for us: a journal is tied to the version of the code
      (Section 10, Versions)
- Event sourcing and CQRS
    - records facts; ours records intentions, and needs determinism
- Command and Memento patterns: the two parts, not the architecture
- The Elm architecture and Redux
    - one state, actions, time travel
    - the view is a pure function; our tasks have a life of their own
- Replicated state machines: Raft
    - a log of commands, snapshots to compact it
    - a fixed set of members, an order that is final
- Optimistic replication: Bayou, Time Warp
    - tentative writes, undone and applied again when the order settles
    - the closest to Section 8
- Blockchains and smart contracts
    - a state machine over a consensus, with reorganizations of the chain
    - ours: a whole reactive program, not a contract, and no currency
- CRDTs and local-first software
    - commands that commute, so that the order does not matter
    - the alternative we do not take: it constrains the data, ours the
      order
- Deterministic lockstep and replays in games; rollback netcode
    - records raw input, not commands
- Record and replay debugging: rr, QEMU, TARDIS
    - machine level; no separation of what matters
- Snapshots of processes and continuations
    - the way we do not take
    - a snapshot of the tasks by the runtime is possible in Atmos (a
      task is a record, not a C stack), and is not designed
- Synchronous languages: Esterel, Céu
    - determinism used for testing and replay

## 14. Future Work

- Action games among peers: the tick as a shared command
- Permissions: which peer may emit which command
- A check in the compiler for rules R2 and R4
- Undo as an operation on the journal
- Migration of journals between versions

## 15. Conclusion

- What persists is data; tasks are a function of it
- A synchronous language makes the old pattern cheap
- Checkpoints give tasks back to action games
- The same journal, over a permissionless consensus, removes the server
