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

C++ interpreter is inside `lib`. To compile, `cd lib` and either `make` or
`g++ main.cpp snowman.cpp snowman.hpp -o snowman -std=c++11`.
