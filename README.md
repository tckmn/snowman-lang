# snowman-lang

An [esoteric programming language](http://esolangs.org), with the primary
design goal of being as confusing and hard to read as possible. See
[`doc/snowman.md`](https://github.com/KeyboardFire/snowman-lang/blob/master/doc/snowman.md)
for documentation.

[Example programs are inside the `examples`
directory.](https://github.com/KeyboardFire/snowman-lang/tree/master/examples)
As a simple example, here's a [ROT13](https://en.wikipedia.org/wiki/ROT13)
program:

    }vg:*#96nG|#110nL,bO|#64nG'(#78nL('>
    bO,oR|:#13nA;:#109nG|#123nL,bO|#77nG
    '(#91nL('>bO,oR|:#13nS;:#;bI;bI;aMsP

C++ interpreter is inside `lib`. To compile, `cd lib` and `make release` (or
just `make` for the debug build, which will be the default until the first
non-beta version).

## TODO

- write tests!

- return something other than empty string for EOF (with `vg`)

- add an option to suppress STDERR output (or do it by default and require it
  to be explicitly enabled)

- put these TODO items in a better place than the README
