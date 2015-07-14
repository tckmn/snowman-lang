# Snowman language documentation

This is the documentation page for the Snowman
[esoteric programming language](http://esolangs.org).

## Design goals

- Be as confusing as possible.<sup>1</sup>

<sup>1: while still being feasible to learn and write a simple program in a few
minutes.</sup>

Snowman was named as such because it has 8 variables, and a snowman looks like
an 8. (It's also a golf term, and I was introduced to esolangs via [Programming
Puzzles & Code Golf Stack Exchange](http://codegolf.stackexchange.com).)

## Variables

Snowman has eight "normal" variables. They can be visualized as follows:

    a b c
    d   e
    f g h

The variable operators work according to this "layout."

Variables can be of the following types:

- undefined (all 8 initially start out as this)
- number
- block
- array

Additionally, zero or more variables are marked as "active variables." The
effect of this will be covered in more detail later. Initially, there are no
active variables.

## Operators

### Variable operators

These are operators that have to do with manipulating the 8 variables in
Snowman. They are all ASCII characters that are not upper- or lowercase
letters.

The following operators rotate variables; for example, `/` switches the `c`
variable with the `f` variable. They are all symbols that consist of only
straight lines that can be drawn in one stroke.

    a b c
    d   e
    f g h

- `/`: cf
- `\`: ah
- `_`: fh
- `|`: bg
- `-`: de
- `'`: bd
- `` ` ``: be
- `,`: eg
- `.`: dg
- `^`: dbe
- `>`: aef
- `<`: cdh

*(`[` and `]` do not currently do anything)*

Rotation is done counterclockwise. For example, `>` causes the `a` variable to
take on the previous value of the `e` variable, `e` becomes what `f` was, and
`f` becomes what `a` was.

The following operators toggle which variables are marked as "active;" for
example, `(` will mark variables `a` and `f` as active if they are currently
inactive, and inactive if they are active. They are all symbols that have curvy
lines.

    a b c
    d   e
    f g h

- `(`: af
- `)`: ch
- `{`: bdg
- `}`: beg
- `~`: invert all (abcdefgh)
- `@`: rotate (done clockwise, abcehgfd -> bcehgfda)
- `%`: reflect (abcehgfd -> hgfdabce)
- `?`: mark all as inactive
- `$`: save current state
- `&`: restore saved state

The following operators have to do with permavars. These are variables that you
can freely store to / retrieve from.

Note: in the following documentation, "store" is defined as follows: place in
first undefined active variable slot. A "store" operation does nothing if all
active variables are full or there are no active variables.

Similarly, a "retrieve" does the opposite. It selects the first non-undefined
active variable in the same way, sets it to undefined, and returns what its
value was.

- `*`: retrieve a value, set the current permavar's value to this
- `#`: store the current permavar's value

To switch the current permavar, use the `=+!` characters. Permavar names are
zero or more `=`, then a `+` or `!`. So, possible peramvar names are `+`, `!`,
`=+`, `=!`, `==+`, `==!`, `===+`, etc. The permavar starts as `+` by default.

The following operators have to do with literals:

- `:...;`: store literal block
- `"..."`: store literal string-array ("strings" are just arrays of ASCII
  codes)
- (one or more digits): store literal number

Literal arrays don't exist. You can create empty arrays with `""` and push to
them with letter operators.

## Letter operators

Letter operators are all either two or three letters long. They are listed in
the following format:

- `foo` (args) -> rtn: desc

where `foo` is the operator, (args) is a list of the types of the arguments the
operator takes (`n` = number, `b` = block, `a` = array, `*` = any, `-` = void),
rtn is a list of the types of the operator's return values, and desc is a short
description.

Arguments to a letter operator are the list of active variables. If any active
variables are undefined, it is an error/UB, unless they are "trailing
undefineds" (which is used for returning values without consuming the
arguments or returning more values than there are arguments; see below).

If there are less than n non-undefined active variables, behavior is undefined.
An error may occur. So don't do that!

Different capitalization invokes different effects when calling a
letter-operator. This is documented in the table below:

    aB -> 2-letter, consume
    ab -> 2-letter, do not consume
    AbC -> 3-letter, consume
    ABc -> 3-letter, do not consume

Letter operators called with the "do-not-consume" strategy return values by
placing them in order inside the undefined active variable slots. If there are
not enough or too many undefined active variables, undefined behavior is
invoked.

This means that if you are calling a letter operator via "do-not-consume" that
takes two arguments and returns one value, you must have at least three active
variables.

If you call a letter operator with the "consume" strategy, it will consume its
arguments (set them to undefined) before returning a value. This allows you to
call a letter operator that takes two arguments and returns one value with only
two active variable slots, although you will lose the original variables. In
this case, the second active variable slot remains undefined and there is no
error.

If there is a type mismatch (if you give arguments of the wrong type to the
operator), undefined behavior is invoked.

### Numbers

- `nde` (n) -> n: decrement
- `nin` (n) -> n: increment
- `nab` (n) -> n: absolute value
- `nf` (n) -> n: floor
- `nc` (n) -> n: ceiling
- `nro` (n) -> n: round
- `nbn` (n) -> n: bitwise NOT
- `nbo` (nn) -> n: bitwise OR
- `nba` (nn) -> n: bitwise AND
- `nbx` (nn) -> n: bitwise XOR
- `na` (nn) -> n: addition
- `ns` (nn) -> n: subtraction
- `nm` (nn) -> n: multiplication
- `nd` (nn) -> n: division
- `nmo` (nn) -> n: modulo
- `nl` (nn) -> n: less than
- `ng` (nn) -> n: greater than
- `nr` (nn) -> a: range
- `np` (nn) -> n: power
- `nb` (nn) -> a: to base

### Arrays

- `aso` (a) -> a: sort
- `asb` (ab) -> a: sort by
- `af` (ab) -> a: fold
- `ac` (aa) -> a: concatenate arrays
- `ad` (aa) -> a: array/set difference
- `aor` (aa) -> a: setwise or
- `aan` (aa) -> a: setwise and
- `ar` (an) -> a: array repeat
- `aj` (aa) -> a: array join
- `as` (aa) -> a: split
- `ag` (an) -> a: split array in groups of size
- `ae` (ab) -> -: each
- `am` (ab) -> a: map
- `an` (an) -> a: every nth element
- `ase` (ab) -> a: select
- `asi` (ab) -> n: select by index / index of / find index
- `aal` (an) -> a: elements at indeces less than n
- `aag` (an) -> a: elements at indeces greater than n
- `aa` (an) -> \*: element at index
- `al` (a) -> n: array length
- `az` (a) -> a: zip/transpose
TODO: unfold?

#### "String" operators

- `sb` (an) -> n: from-base from array-"string"
- `sp` (a) -> -: print an array-"string"
- `se` (a) -> a: eval (Ruby) code in array-"string", return array-"string" of
  result
- `sm` (aa) -> a: regex match; first array-"string" is search text, second
  array-"string" is regex
- `sr` (aaa) -> a: regex replace; first array-"string" is string to operate on,
  second array-"string" is regex, third is replacement text
- `srb` (aab) -> a: same as `sr` but with a block instead of array-"string"

### Blocks

- `br` (bn) -> -: repeat
- `bw` (bb) -> -: while ("returned" value from second block is simply first
  non-undefined active variable, which is set to undefined after reading it)
- `bi` (bbn) -> -: if/else
- `bd` (b) -> -: do (`:...;bD` is basically the same as `:;:...;bW`)

### Any

- `no` (\*) -> n: boolean/logical not (returns `1` for `0 :; []`, `0`
  otherwise)
- `wr` (\*) -> a: wrap in array
- `ts` (\*) -> a: to array-"string"
- `bo` (\*\*) -> n: boolean/logical and ("bo" = "both" because "an," "ad," and
  "nd" are all taken
- `or` (\*\*) -> n: boolean/logical or
- `eq` (\*\*) -> n: equal?
- `du` (\*) -> \*\*: duplicate

### Void

- `vn` (-) -> -: no-op (do nothing)
- `vg` (-) -> a: get line of input (as an array-"string")
- `vr` (-) -> n: random number [0,1)

## Other characters

Whitespace and non-printable-ASCII characters are all stripped before
processing your Snowman program. This means that `1 1` will not have the
desired effect (it will store 11). To store two 1's, use `1vn1` or `1vN1`.

## Example programs

Hello World (`sP` can also be `sp`):

    ("Hello, World!"sP

Cat program (exits on empty input):

    (:vGsp;bD

FizzBuzz (newlines are extraneous):

    )1vn100nR:du*_/3NmO0eq)(#5NmO0eq
    }~(~%@bo(%nO?_/)#%@{%@tS?)aRsP@@
    "Fizz"_aRsP\"Buzz"aRsP?);aE
