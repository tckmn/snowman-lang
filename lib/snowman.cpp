#include "snowman.hpp"
#include <iostream>
#include <stdexcept>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <numeric>

#define HSH1(a) ((long)a)
#define HSH2(a,b) (((long)a)*256 + ((long)b))
#define HSH3(a,b,c) (((long)a)*256*256 + ((long)b)*256 + ((long)c))

#define ROT2(a,b) v = vars[a]; vars[a] = vars[b]; vars[b] = v;
#define ROT3(a,b,c) v = vars[a]; vars[a] = vars[b]; vars[b] = vars[c]; vars[c] = v;

#define TOG_ACT(n) activeVars[n] = !activeVars[n]
#define ROT_ACT() b = activeVars[0]; activeVars[0] = activeVars[3]; activeVars[3] = activeVars[5]; activeVars[5] = activeVars[6]; activeVars[6] = activeVars[7]; activeVars[7] = activeVars[4]; activeVars[4] = activeVars[2]; activeVars[2] = activeVars[1]; activeVars[1] = b;

// constructor/destructor are boring
Snowman::Snowman(): activeVars{false} { srand(time(nullptr)); }
Snowman::~Snowman() {}

// execute string of code
void Snowman::run(std::string code) {
    std::vector<std::string> tokens = Snowman::tokenize(code);
    for (std::string s : tokens) eval_token(s);
}

// static method to convert string of code into tokens (individual
// instructions)
std::vector<std::string> Snowman::tokenize(std::string code) {
    std::vector<std::string> tokens;
    std::string token;
    for (char& c : code) {
        if (token[0] >= '0' && token[0] <= '9' && !(c >= '0' && c <= '9')) {
            tokens.push_back(token);
            token = "";
        }

        if (token[0] >= '0' && token[0] <= '9') {
            // c is guaranteed to be a digit
            token += c;
        } else if (token[0] >= 'a' && token[0] <= 'z') {
            // two-letter operator in progress
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                token += c;
                tokens.push_back(token);
                token = "";
            } else {
                std::cerr << "panic at tokenize: letter operator terminated "
                    "prematurely?" << std::endl;
                exit(1);
            }
        } else if (token[0] >= 'A' && token[0] <= 'Z') {
            // three-letter operator in progress
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                token += c;
                if (token.length() == 3) {
                    tokens.push_back(token);
                    token = "";
                }
            } else {
                std::cerr << "panic at tokenize: letter operator terminated "
                    "prematurely?" << std::endl;
                exit(1);
            }
        } else if (token[0] == '"') {
            // string literal in progress
            // TODO handle escaping (here and below)
            token += c;
            if (c == '"') {
                tokens.push_back(token);
                token = "";
            }
        } else if (token[0] == ':') {
            // block literal in progress
            token += c;
            int nest_depth = 0;
            bool string_mode = false;
            for (char& tc : token) {
                if (tc == ':' && !string_mode) ++nest_depth;
                else if (tc == ';' && !string_mode) {
                    if (nest_depth == 0) {
                        std::cerr << "panic at tokenize: invalid block "
                            "nesting?" << std::endl;
                        exit(1);
                    } else --nest_depth;
                } else if (tc == '"') {
                    string_mode = !string_mode;
                }
            }
            if (nest_depth == 0) {
                tokens.push_back(token);
                token = "";
            }
        } else if (token.length() == 0) {
            // nothing currently in progress; start new token
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') || (c == '"') || (c == ':')) {
                // some token that is longer than one character
                token += c;
                // allow token to continue to be added to
            } else if (c >= '!' && c <= '~') {
                // single character token
                token += c;
                tokens.push_back(token);
                token = "";
            } else {
                // ignore (whitespace or non-printable ASCII)
            }
        } else {
            std::cerr << "panic at tokenize: token started with unknown value?"
                << std::endl;
            exit(1);
        }
    }
    if (token.length() != 0) tokens.push_back(token);
    return tokens;
}

// execute an individual token (called in a loop over all tokens)
void Snowman::eval_token(std::string token) {
    bool consume; // used for letter operators, 2nd and 3rd if blocks below
    if (token[0] >= '0' && token[0] <= '9') {
        // store literal number
        int num;
        try {
            num = std::stoi(token);
        } catch (const std::invalid_argument& e) {
            std::cerr << "panic at eval_token: invalid number?" << std::endl;
            exit(1);
        } catch (const std::out_of_range& e) {
            std::cerr << "panic at eval_token: number out of range?"
                << std::endl;
            exit(1);
        }
        store(Variable((double)num));
        return;
    } else if (token.length() == 2 && token[0] >= 'a' && token[0] <= 'z') {
        // two-letter operator
        if (token[1] >= 'A' && token[1] <= 'Z') {
            consume = true;
            // convert to lowercase
            token[1] = token[1] + ('a' - 'A');
        } else {
            consume = false;
        }
        // handled further below
    } else if (token.length() == 3 && token[0] >= 'A' && token[0] <= 'Z') {
        // three-letter operator
        if ((token[1] >= 'a' && token[1] <= 'z') &&
                (token[2] >= 'A' && token[2] <= 'Z')) {
            consume = true;
            // convert to all uppercase
            token[1] = token[1] - ('a' - 'A');
        } else if ((token[1] >= 'A' && token[1] <= 'Z') &&
                (token[2] >= 'a' && token[2] <= 'z')) {
            consume = false;
            // convert to all uppercase
            token[2] = token[2] - ('a' - 'A');
        } else {
            std::cerr << "panic at eval_token: bad letter function "
                "capitalization?" << std::endl;
            exit(1);
        }
        // handled further below
    } else if (token.length() >= 2 && token[0] == '"') {
        // store literal string-array
        auto vec = new std::vector<Variable>(token.length() - 2);
        for (int i = 1; i < token.length() - 1; ++i) {
            (*vec)[i-1] = Variable((double)token[i]);
        }
        store(Variable(vec));
        return;
    } else if (token.length() >= 2 && token[0] == ':') {
        // store literal block
        auto str = new std::string(token.substr(1, token.length() - 2));
        store(Variable(str));
        return;
    } else if (token.length() == 1 && token[0] >= '!' && token[0] <= '~') {
        // handled below
    } else {
        std::cerr << "panic at eval_token: bad token?" << std::endl;
        exit(1);
    }

    // compute a hash for each token, so that we can use a switch statement
    // below. see also #define'd HSH1, HSH2, and HSH3
    long token_hsh = 0;
    for (char& ch : token) {
        token_hsh *= 256;
        token_hsh += ch;
    }

    // huge switch statement (this contains all operators, letter or otherwise)
    std::vector<Variable> vec; // for convenience with retrieve()
    Variable v; // for variable operators (ROT2, ROT3)
    bool b; // for active variable rotation operators (ROT_ACT)
    switch (token_hsh) {
    case HSH1('/'): // cf
        ROT2(2, 5); break;
    case HSH1('\\'): // ah
        ROT2(0, 7); break;
    case HSH1('_'): // fh
        ROT2(5, 7); break;
    case HSH1('|'): // bg
        ROT2(1, 6); break;
    case HSH1('-'): // de
        ROT2(3, 4); break;
    case HSH1('\''): // bd
        ROT2(1, 3); break;
    case HSH1('`'): // be
        ROT2(1, 4); break;
    case HSH1(','): // eg
        ROT2(4, 6); break;
    case HSH1('.'): // dg
        ROT2(3, 6); break;
    case HSH1('^'): // dbe
        ROT3(1, 3, 4); break;
    case HSH1('>'): // aef
        ROT3(0, 4, 5); break;
    case HSH1('<'): // cdh
        ROT3(2, 3, 7); break;
    case HSH1('('): // af
        TOG_ACT(0); TOG_ACT(5); break;
    case HSH1(')'): // ch
        TOG_ACT(2); TOG_ACT(7); break;
    case HSH1('{'): // bdg
        TOG_ACT(1); TOG_ACT(3); TOG_ACT(6); break;
    case HSH1('}'): // beg
        TOG_ACT(1); TOG_ACT(4); TOG_ACT(6); break;
    case HSH1('~'): // invert all (abcdefgh)
        TOG_ACT(0); TOG_ACT(1); TOG_ACT(2); TOG_ACT(3); TOG_ACT(4); TOG_ACT(5);
        TOG_ACT(6); TOG_ACT(7); break;
    case HSH1('@'): // rotate (done clockwise, abcehgfd -> bcehgfda)
        ROT_ACT(); break;
    case HSH1('%'): // reflect (abcehgfd -> hgfdabce)
        ROT_ACT(); ROT_ACT(); ROT_ACT(); ROT_ACT(); break;
    case HSH1('?'): // mark all as inactive
        activeVars[0] = activeVars[1] = activeVars[2] = activeVars[3] =
            activeVars[4] = activeVars[5] = activeVars[6] = activeVars[7] =
            false; break;
    case HSH1('*'): // retrieve a value, set the current permavar's value to this
        vec = retrieve(-1);
        permavars[activePermavar] = vec[0];
        break;
    case HSH1('#'): // store the current permavar's value
        store(permavars[activePermavar]);
        break;
    case HSH3('N','D','E'): // (n) -> n: decrement
        vec = retrieve(Variable::NUM, 1, consume);
        store(Variable(vec[0].numVal - 1));
        break;
    case HSH3('N','I','N'): // (n) -> n: increment
        vec = retrieve(Variable::NUM, 1, consume);
        store(Variable(vec[0].numVal + 1));
        break;
    case HSH3('N','A','B'): // (n) -> n: absolute value
        vec = retrieve(Variable::NUM, 1, consume);
        store(Variable(vec[0].numVal < 0 ? -vec[0].numVal : vec[0].numVal));
        break;
    case HSH2('n','f'): // (n) -> n: floor
        vec = retrieve(Variable::NUM, 1, consume);
        store(Variable(floor(vec[0].numVal)));
        break;
    case HSH2('n','c'): // (n) -> n: ceiling
        vec = retrieve(Variable::NUM, 1, consume);
        store(Variable(ceil(vec[0].numVal)));
        break;
    case HSH3('N','R','O'): // (n) -> n: round
        vec = retrieve(Variable::NUM, 1, consume);
        store(Variable(round(vec[0].numVal)));
        break;
    case HSH2('n','a'): // (nn) -> n: addition
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal + vec[1].numVal));
        break;
    case HSH2('n','s'): // (nn) -> n: subtraction
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal - vec[1].numVal));
        break;
    case HSH2('n','m'): // (nn) -> n: multiplication
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal * vec[1].numVal));
        break;
    case HSH2('n','d'): // (nn) -> n: division
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal / vec[1].numVal));
        break;
    case HSH3('N','M','O'): // (nn) -> n: modulo
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(fmod(vec[0].numVal, vec[1].numVal)));
        break;
    case HSH2('n','l'): // (nn) -> n: less than
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable((double)(vec[0].numVal < vec[1].numVal)));
        break;
    case HSH2('n','g'): // (nn) -> n: greater than
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable((double)(vec[0].numVal > vec[1].numVal)));
        break;
    case HSH2('n','r'): { // (nn) -> n: range
        vec = retrieve(Variable::NUM, 2, consume);
        int a = round(vec[0].numVal), b = round(vec[1].numVal);
        std::vector<double> rng(b - a);
        std::iota(std::begin(rng), std::end(rng), a);
        auto vrng = new std::vector<Variable>;
        for (double d : rng) vrng->push_back(Variable(d));
        store(Variable(vrng));
        break;
    }
    case HSH2('n','p'): // (nn) -> n: power
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(pow(vec[0].numVal, vec[1].numVal)));
        break;
    case HSH2('a','r'): { // (an) -> a: array repeat
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        int count = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal);
        auto arr = new std::vector<Variable>(vec.size() * count);
        for (int i = 0; i < vec.size() * count; ++i) {
            (*arr)[i] = vec[i % vec.size()];
        }
        store(Variable(arr));
        break;
    }
    case HSH2('a','e'): { // (ab) -> -: each
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        std::string b = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        for (Variable v : vec) {
            store(v);
            run(b);
        }
        break;
    }
    case HSH2('s','p'): // (a) -> -: print an array-"string"
        vec = retrieve(Variable::ARRAY, 1, consume);
        std::cout << arrstring(vec[0]) << std::endl;
        break;
    case HSH2('b','d'): // (b) -> -: do (`:...;bD` is basically the same as `:;:...;bW`, except it's a do-while so the condition isn't tested first)
        vec = retrieve(Variable::BLOCK, 1, consume);
        do {
            run(*vec[0].blockVal);
        } while (Snowman::toBool(retrieve(-1)[0]));
        break;
    case HSH2('n','o'): // (*) -> n: boolean/logical not (returns `1` for `0 :; []`, `0` otherwise)
        vec = retrieve(-1, 1, consume);
        store(Variable((double)(!Snowman::toBool(vec[0]))));
        break;
    case HSH2('t','s'): // (*) -> a: to array-"string"
        vec = retrieve(-1, 1, consume);
        store(stringarr(Snowman::inspect(vec[0])));
        break;
    case HSH2('b','o'): // (**) -> n: boolean/logical and ("bo" = "both" because "an," "ad," and "nd" are all taken)
        vec = retrieve(-1, 2, consume);
        store(Variable((double)(Snowman::toBool(vec[0]) &&
            Snowman::toBool(vec[1]))));
        break;
    case HSH2('o','r'): // (**) -> n: boolean/logical or
        vec = retrieve(-1, 2, consume);
        store(Variable((double)(Snowman::toBool(vec[0]) ||
            Snowman::toBool(vec[1]))));
        break;
    case HSH2('e','q'): // (**) -> n: equal?
        vec = retrieve(-1, 2, consume);
        if (vec[0].type != vec[1].type) {
            store(Variable(0.0));
        } else {
            switch (vec[0].type) {
            case Variable::UNDEFINED:
                store(Variable(1.0));
                break;
            case Variable::NUM:
                store(Variable((double)(vec[0].numVal == vec[1].numVal)));
                break;
            case Variable::ARRAY:
                //store(Variable((double)((*vec[0].arrayVal) ==
                //    (*vec[1].arrayVal))));
                // TODO why doesn't this work?
                break;
            case Variable::BLOCK:
                store(Variable((double)((*vec[0].blockVal) ==
                    (*vec[1].blockVal))));
                break;
            }
        }
        break;
    case HSH2('d','u'): // (*) -> **: duplicate
        vec = retrieve(-1, 1, consume);
        store(vec[0]);
        store(vec[0]);
        break;
    case HSH2('v','n'): // (-) -> -: no-op (do nothing)
        break;
    case HSH2('v','g'): { // (-) -> a: get line of input (as an array-"string")
        std::string line;
        std::getline(std::cin, line);
        store(stringarr(line));
        break;
    }
    case HSH2('v','r'): // (-) -> n: random number [0,1)
        store(Variable((double)rand() / RAND_MAX));
        break;
    default:
        std::cerr << "panic at eval_token: unknown token?" << std::endl;
        exit(1);
    }
}

void Snowman::store(Variable val) {
    // for definition of "store", see doc/snowman.md
    for (int i = 0; i < 8; ++i) {
        if (activeVars[i] && vars[i].type == Variable::UNDEFINED) {
            vars[i].set(val);
            return;
        }
    }
}

std::vector<Variable> Snowman::retrieve(int type, int count, bool consume, int skip) {
    // for definition of "retrieve", see doc/snowman.md
    // (also used for gathering letter operator arguments)
    // default value of count is 1
    // default value of consume is true
    // default value of skip is 0
    std::vector<Variable> vec;
    for (int i = 0; i < 8; ++i) {
        if (activeVars[i]) {
            if (skip) {
                --skip;
                continue;
            }
            if ((vars[i].type != Variable::UNDEFINED) &&
                    (type == -1 || vars[i].type == type)) {
                vec.push_back(vars[i]);
                if (consume) vars[i] = Variable(); // set to undefined
                if (vec.size() == count) return vec;
                // TODO should trailing non-undefined's be ok?
            } else {
                std::cerr << "panic at retrieve: wrong type?" << std::endl;
                exit(1);
            }
        }
    }
    if (vec.size() < count) {
        std::cerr << "panic at retrieve: not enough variables?" << std::endl;
        exit(1);
    }
    return vec;
}

std::string Snowman::arrstring(Variable arr) {
    // convert std::vector<Variable[.type==Variable::NUM]> to std::string
    if (arr.type != Variable::ARRAY) {
        std::cerr << "panic at arrstring: bad argument?" << std::endl;
        exit(1);
    }
    std::string s;
    for (Variable v : *arr.arrayVal) {
        if (v.type == Variable::NUM) {
            s += (char)v.numVal;
        } else {
            std::cerr << "panic at arrstring: bad argument?" << std::endl;
            exit(1);
        }
    }
    return s;
}

Variable Snowman::stringarr(std::string str) {
    auto vec = new std::vector<Variable>;
    for (char& c : str) {
        vec->push_back(Variable((double)c));
    }
    return Variable(vec);
}

std::string Snowman::inspect(Variable v) {
    switch (v.type) {
    case Variable::UNDEFINED:
        return "";
    case Variable::NUM:
        return std::to_string(v.numVal);
    case Variable::ARRAY: {
        std::string s;
        for (Variable v2 : *v.arrayVal) {
            s += Snowman::inspect(v2); // TODO: have a separator or something?
        }
        return s;
    }
    case Variable::BLOCK:
        return *v.blockVal;
    }
}

bool Snowman::toBool(Variable v) {
    switch (v.type) {
    case Variable::UNDEFINED:
        return false;
    case Variable::NUM:
        return v.numVal != 0;
    case Variable::ARRAY:
        return (*v.arrayVal).size() != 0;
    case Variable::BLOCK:
        return (*v.blockVal).size() != 0;
    }
}
