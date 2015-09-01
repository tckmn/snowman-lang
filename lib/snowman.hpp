#ifndef __SNOWMAN_HPP__
#define __SNOWMAN_HPP__

#include <stdexcept>
#include <vector>
#include <string>
#include <map>

struct Variable;

typedef bool tUndefined;
typedef double tNum;
typedef std::vector<Variable> tArray;
typedef std::string tBlock;

typedef tArray::size_type vvs;
typedef tBlock::size_type ss;

class SnowmanException: public std::runtime_error {
    public:
        SnowmanException(std::string msg = "Snowman error occurred",
                bool fatal = false): std::runtime_error(msg), fatal(fatal) {}
        bool fatal;
};

struct Variable {
    // constructors
    Variable(): undefinedVal(true) { type = UNDEFINED; }
    Variable(tUndefined x): undefinedVal(x) { type = UNDEFINED; }
    Variable(tNum x): numVal(x) { type = NUM; }
    Variable(tArray* x): arrayVal(x) { type = ARRAY; }
    Variable(tBlock* x): blockVal(x) { type = BLOCK; }

    // destructor
    ~Variable() {}

    // copy constructor
    Variable(const Variable& v) {
        switch (v.type) {
        case UNDEFINED: type = UNDEFINED; undefinedVal = false; break;
        case NUM: type = NUM; numVal = v.numVal; break;
        case ARRAY: type = ARRAY; arrayVal = new tArray(*v.arrayVal); break;
        case BLOCK: type = BLOCK; blockVal = new tBlock(*v.blockVal); break;
        }
    }

    // assignment operator
    Variable& operator=(const Variable& v) {
        switch (v.type) {
        case UNDEFINED: type = UNDEFINED; undefinedVal = false; break;
        case NUM: type = NUM; numVal = v.numVal; break;
        case ARRAY: type = ARRAY; arrayVal = new tArray(*v.arrayVal); break;
        case BLOCK: type = BLOCK; blockVal = new tBlock(*v.blockVal); break;
        }
        return *this;
    }

    // operators
    bool operator==(const Variable& v) const {
        if (type != v.type) return false;
        switch (v.type) {
        case UNDEFINED: return true;
        case NUM: return numVal == v.numVal;
        case ARRAY: return arrayVal == v.arrayVal;
        case BLOCK: return blockVal == v.blockVal;
        default: throw SnowmanException("at Variable::operator==: impossible"
                    "type?", true);
        }
    }
    bool operator<(const Variable& v) const {
        if (type != v.type) return type < v.type;
        switch (v.type) {
        case UNDEFINED: return false;
        case NUM: return numVal < v.numVal;
        case ARRAY: return arrayVal < v.arrayVal;
        case BLOCK: return blockVal < v.blockVal;
        default: throw SnowmanException("at Variable::operator<: impossible"
                     "type?", true);
        }
    }

    // manage memory (use when modifying value)
    // BE VERY CAREFUL when calling this function
    void mm() {
        switch (type) {
        case ARRAY: delete arrayVal; break;
        case BLOCK: delete blockVal; break;
        default: break;
        }
    }

    // the actual data
    enum { UNDEFINED, NUM, ARRAY, BLOCK } type;
    union {
        tUndefined undefinedVal;
        tNum numVal;
        tArray* arrayVal;
        tBlock* blockVal;
    };
};

// used for subroutines
struct VarState {
    Variable vars[8];
    bool activeVars[8];
};

class Snowman {
    private:
        // internal evaluation methods
        void evalToken(std::string token);
        void store(Variable v);
        Variable retrieve(int type, bool consume = true, int skip = 0);

        template<typename T = tUndefined, typename U = tUndefined,
            typename V = tUndefined, typename W = tUndefined> class Retrieval{};

        // utility methods having to do with the language itself
        static std::string arrToString(tArray arr);
        static Variable stringToArr(std::string str);
        static std::string inspect(Variable str);
        static bool toBool(Variable v);

        // command line args
        tArray args;

        // variables and permavars
        Variable vars[8];
        bool activeVars[8];
        std::vector<VarState> subroutines;
        std::map<int, Variable> permavars;
        int activePermavar;
        bool savedActiveState[8];

    public:
        // constructor / destructor
        Snowman();
        ~Snowman();

        // for manipulating a string of code
        static std::vector<std::string> tokenize(std::string code);
        void run(std::string code);

        // command line args
        void addArg(std::string arg);

        // debugging (also used for REPL)
        std::string debug();
        bool debugOutput;

        // version
        const static int MAJOR_VERSION = 1;
        const static int MINOR_VERSION = 0;
        const static int PATCH_VERSION = 2;
};

#endif
