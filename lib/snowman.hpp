#ifndef __SNOWMAN_HPP__
#define __SNOWMAN_HPP__

#include <vector>
#include <string>
#include <map>

struct Variable {
    // constructors
    Variable(): undefinedVal(true) { type = UNDEFINED; }
    Variable(bool x): undefinedVal(x) { type = UNDEFINED; }
    Variable(double x): numVal(x) { type = NUM; }
    Variable(std::vector<Variable>* x): arrayVal(x) { type = ARRAY; }
    Variable(std::string* x): blockVal(x) { type = BLOCK; }

    // destructor
    ~Variable() {}

    // essentially operator='s, except specialer
    void set(bool x) { mm(); type = UNDEFINED; undefinedVal = x; }
    void set(double x) { mm(); type = NUM; numVal = x; }
    void set(std::vector<Variable>* x) { mm(); type = ARRAY; arrayVal = x; }
    void set(std::string* x) { mm(); type = BLOCK; blockVal = x; }
    void set(Variable& v) {
        mm();
        switch (v.type) {
        case UNDEFINED: set(v.undefinedVal); break;
        case NUM: set(v.numVal); break;
        case ARRAY: set(v.arrayVal); break;
        case BLOCK: set(v.blockVal); break;
        }
    }

    // manage memory (use when modifying value)
    // BE VERY CAREFUL when calling this function
    void mm() {
        switch (type) {
        case ARRAY: delete arrayVal; break;
        case BLOCK: delete blockVal; break;
        }
    }

    enum { UNDEFINED, NUM, ARRAY, BLOCK } type;
    union {
        bool undefinedVal;
        double numVal;
        std::vector<Variable>* arrayVal;
        std::string* blockVal;
    };
};

class Snowman {
    private:
        static std::vector<std::string> tokenize(std::string code);
        void eval_token(std::string token);
        void store(Variable v);
        std::vector<Variable> retrieve(int type, int count = 1, bool consume =
            true, int skip = 0);
        static std::string arrstring(Variable arr);
        static Variable stringarr(std::string str);
        static std::string inspect(Variable str);
        static bool toBool(Variable v);
        Variable vars[8];
        bool activeVars[8];
        std::map<int, Variable> permavars;
        int activePermavar;
    public:
        Snowman();
        ~Snowman();
        void run(std::string code);
        std::string debug();
        bool debugOutput;
        const static int MAJOR_VERSION = 0;
        const static int MINOR_VERSION = 0;
        const static int PATCH_VERSION = 0;
};

#endif
