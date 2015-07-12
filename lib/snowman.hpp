#ifndef __SNOWMAN_HPP__
#define __SNOWMAN_HPP__

#include <vector>
#include <string>

struct Variable {
    Variable(): undefinedVal(true) { type = UNDEFINED; }
    Variable(const Variable& v) {}
    Variable& operator=(const Variable& v) {}
    ~Variable() {}
    enum { UNDEFINED, NUM, ARRAY, BLOCK } type;
    union {
        bool undefinedVal;
        double numVal;
        std::vector<Variable> arrayVal;
        std::string blockVal;
    };
};

class Snowman {
    private:
        static std::vector<std::string> tokenize(std::string code);
    public:
        Snowman();
        ~Snowman();
        void run(std::string code);
        Variable vars[8];
};

#endif
