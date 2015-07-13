#include "snowman.hpp"
#include <iostream>
#include <stdexcept>

#define HSH1(a) ((long)a)
#define HSH2(a,b) (((long)a)*256 + ((long)b))
#define HSH3(a,b,c) (((long)a)*256*256 + ((long)b)*256 + ((long)c))

// constructor/destructor are boring
Snowman::Snowman(): activeVars{false} {}
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
    switch (token_hsh) {
    case HSH1('('): // af
        activeVars[0] = !activeVars[0];
        activeVars[5] = !activeVars[5];
        break;
    case HSH2('s','p'): // (a) -> -: print an array-"string"
        vec = retrieve(Variable::ARRAY);
        std::cout << arrstring(vec[0]) << std::endl;
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

std::vector<Variable> Snowman::retrieve(int type, int count) {
    // for definition of "retrieve", see doc/snowman.md
    // (this implementation is a bit different, because it's also used for
    // gathering letter operator arguments)
    // default value of count is 1
    std::vector<Variable> vec;
    for (int i = 0; i < 8; ++i) {
        if (activeVars[i]) {
            if ((vars[i].type != Variable::UNDEFINED) &&
                    (type == -1 || vars[i].type == type)) {
                vec.push_back(vars[i]);
                vars[i] = Variable(); // set to undefined
                if (vec.size() == count) return vec;
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
