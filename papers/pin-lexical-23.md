# Dynamic Allocation with Runtime Lexical Memory Management

> Markdown conversion of the author's draft `lexical.tex`, from
> `fsantanna-no/ceu-lexial-23` (commit `90abe88`, 2026-04-01; the draft
> itself is from 2023, on dyn-lex). Converted by the session `paper-pin`
> on 2026-11-28, at the author's request. The text is the author's,
> unchanged; only the LaTeX markup was converted. Placeholders and notes
> that were in the source (`TODO`, bare outlines, and the commented-out
> block at the end) are kept as they were, marked as such.

*Anonymous* (double-blind submission template, SBC)

## Abstract

*(empty in the source)*

## 1. Introduction

In structured programming, the hierarchical lexical format of the source code
determines the control flow of programs, which is based on sequences,
conditionals, loops, and subroutines.
In addition, programmers rely on the nesting of blocks of such commands to
better organize and reason about the program [Dijkstra 1968; Dahl et al. 1972].
Blocks also determine the scope of variables, limiting their visibility
and lifetime, which helps to reason, not only about control flow, but also
about the memory layout of programs.

Regarding memory management, structured programming typically provides local
variables that use the stack to hold fixed-size data types, and references to
dynamically-allocated values (or objects) that use the heap to hold
variable-size data types.
The stack provides automatic memory management that is easy to reason, since it
deallocates memory in blocks that respect the lexical structure of the program.
However, the life cycle of dynamic objects have no correspondence with the
lexical structure, diverging from the basic principle of structured
programming.
Nevertheless, the heap is essential to express resizable and multi-referenced
data structures, such as vectors and graphs, and is also a requirement to move
objects between scopes.

To deal with the mismatch between the life cycle of objects against the lexical
structure of programs, programming languages offer extra mechanisms to manage
the heap.
For instance, lower-level languages, such as C and Zig, rely on explicit
allocation primitives, while higher-level languages, such as Python and Java,
rely on garbage collectors.

More sophisticated type systems can reconcile heap allocation with the program
structure.
In Cyclone [Grossman et al. 2002], lexical regions behave like local heaps that
are attached to blocks.
An object is explicitly allocated into a region, which is deallocated as a
whole with all containing objects when its enclosing block terminates.
In Rust [Klabnik and Nichols 2023], objects are owned by a unique lexical
reference.
When the owner goes out of scope, the object is automatically deallocated.
The type systems of Cyclone and Rust ensure that no references can escape the
owning lexical scope, providing compile-time safety with minimum runtime
costs.
Nevertheless, they impose restrictions on the expressiveness of the languages.
For instance, objects in Cyclone can be shared freely, but cannot move between
scopes, while objects in Rust can safely move between scopes, but cannot share
ownership.

In this work, we explore the design of Dyn-Lex, a dynamic language with runtime
lexical memory management for heap-allocated objects.
Objects in Dyn-Lex have the following characteristics:

- (a) they are always attached to exactly one block;
- (b) they are automatically released from memory along with their holding
  blocks;
- (c) they can be shared freely, except to outer scopes;
- (d) they can be explicitly moved to inner blocks, except when they have
  multiple references; and
- (e) they can be explicitly moved to outer blocks with no restrictions.

Therefore, Dyn-Lex can express arbitrary cyclic graphs with nodes residing at
different blocks, and move them between scopes when appropriate.
A garbage collector is still required to manage long-lasting blocks that create
and reassign too many objects.
However, like dynamic typing, all safety checks happen at runtime, which incur
in significant space and time overhead.
Nevertheless, our hypothesis is that like dynamic typing, it is reasonable to
pay these costs to improve programming expressiveness.
In our case, the gain in expressiveness manifests as the possibility to reason
about dynamic memory management in lexical terms with less restrictions.

*TODO (in the source): costs and results*

In summary, as main contributions we defend/show evidence of/substantiate that
(a) lexically scoped objects help to reason about memory usage, that
(b) some allocation patterns cannot be statically inferred, and that
(c) the runtime overhead is not prohibitive, thus justifying the design of
Dyn-Lex.

The rest of the paper is organized as follows:
Section 2 describes the design of Dyn-Lex...
Section 3 evaluates our implementation...
Section 4 discusses related work...
Section 5 concludes the paper.

## 2. Design of Dyn-Lex

Dyn-Lex is a dynamically typed language with lexical memory management as its
key aspect.
The surface of Dyn-Lex is that of a standard procedural language, with blocks,
assignments, sequences, conditionals, loops, and functions.
Variables must be explicitly declared to visually determine their scopes.
All commands in Dyn-Lex are expressions that evaluate to a value.
Listing 1 prints the sum from `0` to `5`.

*Outline (in the source):*

- do/escape
- minimal language
- sugar free
- table summary

**Listing 1.** Calculates the sum from `0` to `5` in Dyn-Lex.

```
var sum = func (i) {    // sum from 0 to i
    var s = 0
    loop {
        if i == 0 {
            break(s)    // final loop expression
        }
        set s = s + i
        set i = i - 1
    }                   // loop evaluates to s
}
println(sum(5))         // --> 15
```

As basic types, Dyn-Lex supports booleans, characters, numbers, tags, and a
`nil` unit type.
Tags (aka symbols or atoms) represent unique human-readable values.
In a boolean context, all values are considered to be true, except `false`
and `nil`.
As compound types, Dyn-Lex supports
tuples for fixed-sized heterogeneous types,
vectors for variable-sized homogeneous types, and
dictionaries for key-value mappings.
A string in Dyn-Lex is a mutable vector of characters.
As we discuss further, functions can represent a limited form of closures.
Listing 2 illustrates values of all types in Dyn-Lex:

**Listing 2.** Basic and compound types of Dyn-Lex.

```
true   false        // booleans
'a'    '\n'         // characters
1      10.25        // numbers
:x     :KEY_A       // tags
nil                 // nil

[1, true]           // tuple
#['a','b','c']      // vector (string "abc")
@[(:x,1), (:y,2)]   // dictionary

func (v) { v*2 }    // function
```

Regarding operations between expressions, Dyn-Lex supports function calls and
standard arithmetic, comparison and logical operators.
In addition, to deal with collections, Dyn-Lex supports index and length
operators.
Listing 3 illustrates all supported operations in Dyn-Lex:

**Listing 3.** Supported operations in Dyn-Lex.

```
10 + 20             // 3
10 > 20             // false
10 and 20           // 20
sum(5)              // 15
#[10,20,30][1]      // 20 (tuple/vector indexing)
#([10,20,30])       // 3  (tuple/vetor length)
@[(:x,10)][:x]      // 10 (dictionary indexing)
```

### 2.1 Lexical Scoping Intuition

The scope of all variables in Dyn-Lex are restricted to their enclosing lexical
blocks, which determine not only their visibility, but also the allocation
lifetime of the values they hold.

For variables that hold values of basic types, the scoping behavior is
straightforward:
when a block starts, it allocates space for all variables (typically in the
stack), such that they can hold their values in entirety;
when the variable is accessed or assigned, the relevant value is fully
copied from source to destination, such that no dependency between them
persists;
when the block terminates, the variables loose visibility and all space can
be safely deallocated, since no dependencies with the values remain
active.

However, the semantics of compound types is more complex for two reasons:
(1) they cannot fit in the space of a basic type because they either have
variable size or carry extra meta information (e.g., tuple size); and
(2) users expect to share compound values such that they can be modified
and accessed through references from multiple places.
As examples of the latter, compound values, which we also refer as objects, can
be modified by setters, and nodes in graphs can point to each other.
For these reasons, an object is allocated externally to blocks (typically in
the heap), and only a reference to it is held in variables.
Now, when a variable holding a reference is accessed or assigned, it creates
dependencies between the source and destination, since they share a copy of the
same reference.
As a consequence, if the source and destination are declared at different
scopes, it is not clear to which block the value must be lexically attached.
For this reason, most programming languages do not attach the lifetime of
objects to a lexical scope, resorting to manual deallocation or garbage
collection.

Listings 4 and 5 compare basic and compound
types to illustrate how the lexical scope of compound types cannot be
determined properly, since a reference to the object escapes the scope in which
it is first created.
Listing 5 generates a runtime error in Dyn-Lex, complaining that
the assignment `x=y` involve incompatible scopes.

**Listing 4.** Basic type escape.

```
var x
do {
  var y = 10 // basic type
  set x = y  // copy 10 to x
}
println(x)
```

**Listing 5.** Compound type escape.

```
var x
do {
  var y = [10] // compound type
  set x = y // ref to [10] escapes
}
println(x)
```

As described in the Introduction, objects in Dyn-Lex have the following
characteristics:
(a) they are always attached to exactly one block;
(b) they are automatically released from memory with their holding blocks;
(c) they can be shared freely, except to outer scopes;
(d) they can be explicitly moved to inner scopes, except if multi
referenced; and
(e) they can be explicitly moved to outer scopes with no restrictions.

Listing 5 illustrates items (a), (b), and (c).
Regarding (a), the object is only attached to the `do` block that
creates it.
Regarding (b), the `do` block deallocates the object on termination.
Regarding (c), it is safe to share the object with `y`, declared in the
scope of the `do` block, but not with `x`, which is declared in the
outer scope.
Because the object is deallocated at the end of the block, accessing it
afterwards as `x` would constitute an *use-after-free* violation.
This is the reason why the example raises an error.

We now introduce the `drop` primitive that permits to move objects across
scopes.
A `drop(v)` operation does two things:
(1) it detaches the referenced object from its current block, and
(2) it removes the reference held in the given variable, setting it to
`nil`.
On a further assignment, the detached object is attached to the scope of its
destination variable.
Listing 6 and 7 illustrate items (d) and (e),
using the `drop` primitive.

**Listing 6.** Move to inner scope.

```
var o1 = [10]
var p1 = [20]
var p2 = p1  // multi ref
do {
    var o2 = drop(o1) // OK
    var p3 = drop(p1) // NO
}
println(o1) // OK: nil
println(p2) // NO: use-after-free
```

**Listing 7.** Move to outer scope.

```
var v = do {
    var u = #[]
    set u[#u] = 1
    set u[#u] = 2
    set u[#u] = 3
    drop(u) // OK
}
println(v)  // OK: #[1,2,3]
```

*TODO (in the source):*

```
val g = func (v) {
    println(v)
}
val f = func (v) {
    g(v)
    g(v)
}
f([1])
```

Listing 6 creates two objects: `[10]` is only shared with
`o1`, while `[20]` is shared with `p1` and `p2`.
In the inner scope, `o2=drop(o1)` reattaches the object `[10]` to the
scope of `o2`, setting `o1=nil`.
Now, the object is deallocated when the inner scope terminates, and the
`println(o1)` in the outer scope would print `nil` correctly.
However, `p3=drop(p1)` fails due to item (d), since `p2` in the outer
scope still holds a reference to the dropped object in the inner scope.
If the reattachment succeeded, the `println(p2)` in the outer scope would
constitute an *use-after-free* violation.
This is the reason why item (d) forbids to move a multi-referenced object to an
inner scope.

Listing 7 creates a vector in the inner scope, populates it, and
drops it before escaping to the outer scope.
According to item (e), this code is perfectly valid, and the vector is
reattached to the outer scope.
Note that the `drop(u)` is still required to be explicit about the change
in scope.

Listings 4–7 just give an intuition on the
semantics of Dyn-Lex.
However, we did not discuss more complex programs that involve indexing or
mixing objects with multiple scopes.
For instance, Listing 8 creates a tuple that mixes the scopes
of `a` and `b`, passing it to a function that creates a cycle, to
finally drop it as a whole to the outer scope.
The example is in accordance with items (a)–(e) and executes successfully, but
we now need to explain the lexical rules in more detail.

**Listing 8.** A complex example in Dyn-Lex.

```
var f = func (v) {
    set v[2] = v
    v
}
var a = [1]
var c = do {
    var b = [2]
    drop(f([a,b,nil]))  // tuple
}
println(c)  // OK: [[1],[2],*]
```

### 2.2 Lexical Scoping Semantics

When an object is first allocated through a constructor (e.g., `[x,10]`)

*Outline (in the source):*

```
var v = [x]

var v = [nil]
set v[0] = x
```

- FLEETING
- CONSTRUCTING
- MUTABLE
- IMMUTABLE

- indexes
- collections

## 3. Performance Evaluation

*(empty in the source)*

## 4. Related Work

*(empty in the source)*

## 5. Conclusion

*(empty in the source)*

## Notes (commented out in the source)

The block below was inside `\begin{comment} ... \end{comment}` at the end of
`lexical.tex`, and is kept verbatim.

```
>>> string=32 | udata=56 | udata=40 | proto=128
>>> upval=40 | table=56 | closure=48

>>> tuple=56, vector=72, dict=64, clos=80

dyn-lex respects the lexical structure of the program also when dealing with dynamic memory allocation. When a dynamic value is first assigned to a variable, it becomes attached to the block in which the variable is declared, and the value cannot escape that block in further assignments or as return expressions. This is valid not only for collections (tuples, vectors, and dictionaries), but also for closures. This restriction ensures that terminating blocks deallocate all memory at once. More importantly, it provides static means to reason about the program. To overcome this restriction, dyn-lex also provides an explicit drop operation to deattach a dynamic value from its block.


The type system of Cyclone ensures that no references can escape the scope of
regions to prevent dangling references.
Rust..., but multiple borrowed references.
The type system of Rust also ensures that no borrowed references can escape the
owner scope.
 (akin to explicit deallocation primitives).

In addition to standard structured mechanisms, \lex supports dynamic data
structures (tuples, vectors, and dictionaries), exceptions, deferred
expressions, and a limited form of closures.
%
costs mem
    - hold = (tp,block,nxt)
    - _var_
costs run
    - all assigns

Patina: A Formalization of the Rust Programming Language
https://pling.jondgoodwin.com/post/cyclone/

- webserver, across multiple requests, stack ripping
    - even if fixed size, still requires the heap

- problem with heap
    - references
    - escapes
- state of the art
    - move semantics
    - but no sharing
    - cyclone, rust
        - problem of callbacks
            - still holds for this work
- Ceu
    - restore reasoning
    - func, upval, noupval
    - drop semantics
- Tradeoff
    - memory cost
    - runtime cost
    - still requires GC
    - single ref

 y second remark is that our intellectual powers are rather
geared to master static relations and that our powers to visualize
processes evolving in time are relatively poorly developed. For
that reason w e should do (as wise programmers aware of our
limitations) our utmost to shorten the conceptual gap between
the static program and the dynamic process, to m a k e the cor-
respondence between the program (spread out in text space) and
the process (spread out in time) as trivial as possible.

currently two alternatives:
    - malloc/free
    - GC
    - ownership semantics
        - move, borrow
            - implicit
        - Each value in Rust has an owner.
        - There can only be one owner at a time.
        - When the owner goes out of scope, the value will be dropped.

none enforced by the lexical structure
```

## References

From `lexical.bib`.

- [Dahl et al. 1972] Ole-Johan Dahl, Edsger W. Dijkstra, and C. A. R. Hoare.
  *Structured Programming*. Academic Press, 1972.
- [Dijkstra 1968] Edsger W. Dijkstra. Letters to the editor: go to statement
  considered harmful. *Communications of the ACM*, 11(3):147–148, 1968.
- [Grossman et al. 2002] Dan Grossman, Greg Morrisett, Trevor Jim, Michael
  Hicks, Yanling Wang, and James Cheney. Region-based memory management in
  Cyclone. In *Proceedings of PLDI'02*, pages 282–293, 2002.
- [Klabnik and Nichols 2023] Steve Klabnik and Carol Nichols. *The Rust
  Programming Language*. No Starch Press, 2023.

## Appendix: notes on Rust (`rust.txt`)

The repository also has `rust.txt`, the author's notes (in Portuguese)
comparing the design with Rust, kept verbatim.

```
- so pode ter uma referencia
    - impossivel ciclos
        - impossivel lista duplamente encadeada
- nos de arvores nao podem ter owners diferentes

Rust = Ceu
  &x = x
   x = drop(x)


- descobri as diferencas
    1. ce nao tem foco em mut: concorrencia/write-while-reading/iterator
        - problema ortogonal
    2. ADTs em rust sao ruins
    3. Rust nao consegue mover partes de estruturas recursivas
        - var z: (Nat,Nat) = ...
        - var x: Nat = z.1          -- z.1=$
        - isso por conta do set auto nil
    x. keyword move
    4. Ce terá pools (pool xs[]:Nat ; var x:Nat in xs = ...)
        4.1 Limita escopo da memória
            - o sistema de ownership não respeita escopo mínimo, sempre posso retornar.
            - confusão com entendimento da memória em determinado momento
            - ter um ponteiro único nao ajuda necessariamente a entender
        4.2 Permite bounded pool
    X. Conseguiria guardar ponteiros na raiz da struct que quando houver qq free,
       eu percorro a raiz e elimino o ptr
    5. Growable. Só posso escrever em subarvores vazias, nunca substituir um no e remover
       a sua subarvore. Isso iria delasalocar nós com ponteiros pendentes.
       A verificação é em runtime, feito na atribuicao. Só pode ser assim pois
       ponteiros que já foram extremidades deixam de ser assim que há uma attr
        - agora vou poder ter ponteiros guardados na propria árvore ou até mesmo
          fora em escopos menores ou iguais
    - tipos de recs:
        - immutable
            - nao precisa de clone (pode share os nós sem move)
        - mutable
            - growable
                - pode guardar ponteiros
            - movable
                - pode mover sub arvores
                  (transfer ownership of subtree)
    - empty value:
        - movable: o teste do grow é deito em cima do valor empty
        - mutable: o valor original após o move vira o empty

https://radekvit.medium.com/move-semantics-in-c-and-rust-the-case-for-destructive-moves-d816891c354b

The reason why bit copies are always enough for a move operation in Rust is that Rust does not support self-referential structs in its safe subset. The borrowing rules in Rust make it impossible for a struct to borrow from its own fields (unless you reach for raw pointers and unsafe). Such structs would require their move operations to adjust these references after moving the resources from the original object, but without them, there is no real need to execute arbitrary code on moves.
```
