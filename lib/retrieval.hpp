#include "snowman.hpp"

// here be dragons
// thou art forewarned

template<> class Snowman::Retrieval<tNum> {
    public:
    tNum a;
    Retrieval(Snowman* sm, bool consume) {
        a = sm->retrieve(Variable::NUM, consume).numVal;
    }
};

template<> class Snowman::Retrieval<tNum, tNum> {
    public:
    tNum a, b;
    Retrieval(Snowman* sm, bool consume) {
        a = sm->retrieve(Variable::NUM, consume).numVal;
        b = sm->retrieve(Variable::NUM, consume, 1).numVal;
    }
};

template<> class Snowman::Retrieval<tArray*> {
    private: bool consume;
    public:
    tArray* a;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::ARRAY, consume).arrayVal;
    }
    ~Retrieval() {
        if (consume) delete a;
    }
};

template<> class Snowman::Retrieval<tArray*, tBlock*> {
    private: bool consume;
    public:
    tArray* a;
    tBlock* b;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::ARRAY, consume).arrayVal;
        b = sm->retrieve(Variable::BLOCK, consume, 1).blockVal;
    }
    ~Retrieval() {
        if (consume) { delete a; delete b; }
    }
};

template<> class Snowman::Retrieval<tArray*, tArray*> {
    private: bool consume;
    public:
    tArray *a, *b;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::ARRAY, consume).arrayVal;
        b = sm->retrieve(Variable::ARRAY, consume, 1).arrayVal;
    }
    ~Retrieval() {
        if (consume) { delete a; delete b; }
    }
};

template<> class Snowman::Retrieval<tArray*, tArray*, tBlock*> {
    private: bool consume;
    public:
    tArray *a, *b;
    tBlock* c;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::ARRAY, consume).arrayVal;
        b = sm->retrieve(Variable::ARRAY, consume, 1).arrayVal;
        c = sm->retrieve(Variable::BLOCK, consume, 2).blockVal;
    }
    ~Retrieval() {
        if (consume) { delete a; delete b; delete c; }
    }
};

template<> class Snowman::Retrieval<tArray*, tArray*, tArray*> {
    private: bool consume;
    public:
    tArray *a, *b, *c;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::ARRAY, consume).arrayVal;
        b = sm->retrieve(Variable::ARRAY, consume, 1).arrayVal;
        c = sm->retrieve(Variable::ARRAY, consume, 2).arrayVal;
    }
    ~Retrieval() {
        if (consume) { delete a; delete b; delete c; }
    }
};

template<> class Snowman::Retrieval<tArray*, tNum> {
    private: bool consume;
    public:
    tArray* a;
    tNum b;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::ARRAY, consume).arrayVal;
        b = sm->retrieve(Variable::NUM, consume, 1).numVal;
    }
    ~Retrieval() {
        if (consume) delete a;
    }
};

template<> class Snowman::Retrieval<tArray*, tNum, tNum, tArray*> {
    private: bool consume;
    public:
    tArray *a, *d;
    tNum b, c;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::ARRAY, consume).arrayVal;
        b = sm->retrieve(Variable::NUM, consume, 1).numVal;
        c = sm->retrieve(Variable::NUM, consume, 2).numVal;
        d = sm->retrieve(Variable::ARRAY, consume, 3).arrayVal;
    }
    ~Retrieval() {
        if (consume) { delete a; delete d; }
    }
};

template<> class Snowman::Retrieval<tBlock*> {
    private: bool consume;
    public:
    tBlock* a;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::BLOCK, consume).blockVal;
    }
    ~Retrieval() {
        if (consume) delete a;
    }
};

template<> class Snowman::Retrieval<tBlock*, tNum> {
    private: bool consume;
    public:
    tBlock* a;
    tNum b;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::BLOCK, consume).blockVal;
        b = sm->retrieve(Variable::NUM, consume, 1).numVal;
    }
    ~Retrieval() {
        if (consume) delete a;
    }
};

template<> class Snowman::Retrieval<tBlock*, tBlock*> {
    private: bool consume;
    public:
    tBlock *a, *b;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::BLOCK, consume).blockVal;
        b = sm->retrieve(Variable::BLOCK, consume, 1).blockVal;
    }
    ~Retrieval() {
        if (consume) { delete a; delete b; }
    }
};

template<> class Snowman::Retrieval<tBlock*, tBlock*, Variable> {
    private: bool consume;
    public:
    tBlock *a, *b;
    Variable c;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(Variable::BLOCK, consume).blockVal;
        b = sm->retrieve(Variable::BLOCK, consume, 1).blockVal;
        c = sm->retrieve(-1, consume, 2);
    }
    ~Retrieval() {
        if (consume) { delete a; delete b; c.mm(); }
    }
};

template<> class Snowman::Retrieval<Variable> {
    private: bool consume;
    public:
    Variable a;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(-1, consume);
    }
    ~Retrieval() {
        if (consume) a.mm();
    }
};

template<> class Snowman::Retrieval<Variable, Variable> {
    private: bool consume;
    public:
    Variable a, b;
    Retrieval(Snowman* sm, bool consume): consume(consume) {
        a = sm->retrieve(-1, consume);
        b = sm->retrieve(-1, consume, 1);
    }
    ~Retrieval() {
        if (consume) { a.mm(); b.mm(); }
    }
};

// this one's special!
template<> class Snowman::Retrieval<bool> {
    private:
    Variable a;
    public:
    bool b;
    Retrieval(Snowman* sm) {
        a = sm->retrieve(-1, true, -1);
        b = Snowman::toBool(a);
    }
    ~Retrieval() {
        a.mm();
    }
};
