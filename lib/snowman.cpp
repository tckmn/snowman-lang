#include "snowman.hpp"
#include <iostream>   // std::cout, std::cerr, std::endl
#include <ctime>      // time (for seeding RNG)
#include <cstdlib>    // rand, srand
#include <cmath>      // abs, fmod, pow, ceil, floor, round
#include <algorithm>  // find
#include <regex>      // obvious
// included from snowman.hpp: <vector>, <string>, <map>, <stdexcept>

#define DEBUG

#define HSH1(a) ((long)a)
#define HSH2(a,b) (((long)a)*256 + ((long)b))
#define HSH3(a,b,c) (((long)a)*256*256 + ((long)b)*256 + ((long)c))

#define ROT2(a,b) v = vars[a]; vars[a] = vars[b]; vars[b] = v;
#define ROT3(a,b,c) v = vars[a]; vars[a] = vars[b]; vars[b] = vars[c]; vars[c] = v;

#define TOG_ACT(n) activeVars[n] = !activeVars[n]
#define ROT_ACT() b = activeVars[0]; activeVars[0] = activeVars[3]; \
    activeVars[3] = activeVars[5]; activeVars[5] = activeVars[6]; \
    activeVars[6] = activeVars[7]; activeVars[7] = activeVars[4]; \
    activeVars[4] = activeVars[2]; activeVars[2] = activeVars[1]; \
    activeVars[1] = b;

const std::string DIGITS = "0123456789abcdefghijklmnopqrstuvwxyz";
const int TOBASE_PRECISION = 10; // number of digits after decimal point
const double TOBASE_EPSILON = 0.00001; // if the decimal part is less than
                                       // this, it will be treated as an int

// constructor/destructor
Snowman::Snowman(): activeVars{false}, activePermavar(0),
        savedActiveState{false}, debugOutput(false) {
    srand(time(nullptr));
}
Snowman::~Snowman() {}

// execute string of code
void Snowman::run(std::string code) {
    std::vector<std::string> tokens;
    try {
        tokens = Snowman::tokenize(code);
    } catch (SnowmanException& se) {
        std::cerr << "SnowmanException thrown at tokenize" << std::endl;
        std::cerr << "  what():  " << se.what() << std::endl;
        // all exceptions are fatal because then we have no tokens to run
        std::cerr << "fatal error, aborting" << std::endl;
        return;
    }
    for (std::string s : tokens) {
        try {
            evalToken(s);
        } catch (SnowmanException& se) {
            std::cerr << "SnowmanException thrown at evalToken" << std::endl;
            std::cerr << "  what():  " << se.what() << std::endl;
            if (se.fatal) {
                std::cerr << "fatal error, aborting" << std::endl;
                return;
            } else {
                std::cerr << "non-fatal error, continuing" << std::endl;
                continue;
            }
        }
        if (debugOutput) {
            std::cout << "<[T]> " << s << std::endl;
            std::cout << "<[D]> " << debug();
        }
    }
}

// static method to convert string of code into tokens (individual
// instructions)
std::vector<std::string> Snowman::tokenize(std::string code) {
    std::vector<std::string> tokens;
    std::string token;
    bool comment = false, escaping = false;
    for (char& c : code) {
        if (comment) {
            if (c == '\n') comment = false;
            else continue;
        }

        if ((token[0] != '"' && token[0] != ':') && !(c >= '!' && c <= '~')) {
            // ignore non-printable-ASCII outside of string/block literals
            continue;
        }

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
                throw SnowmanException("at tokenize: letter operator "
                    "terminated prematurely?", true);
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
                throw SnowmanException("at tokenize: letter operator "
                    "terminated prematurely?", true);
            }
        } else if (token[0] == '"') {
            // string literal in progress
            token += c;
            if (c == '"') {
                if (escaping) {
                    token.erase(token.length() - 2, 1); // get rid of backslash
                    escaping = false;
                } else {
                    tokens.push_back(token);
                    token = "";
                }
            } else if (c == '\\') {
                if (escaping) {
                    token.erase(token.length() - 2, 1); // get rid of backslash
                    escaping = false;
                } else {
                    escaping = true;
                }
            } else if (escaping) escaping = false;
        } else if (token[0] == ':') {
            // block literal in progress
            token += c;
            int nest_depth = 0;
            bool string_mode = false, escaping = false;
            for (char& tc : token) {
                if (tc == ':' && !string_mode) ++nest_depth;
                else if (tc == ';' && !string_mode) {
                    if (nest_depth == 0) {
                        throw SnowmanException("at tokenize: invalid block "
                            "nesting?", true);
                    } else --nest_depth;
                } else if (tc == '"' && !escaping) {
                    string_mode = !string_mode;
                }
                escaping = (tc == '\\' && string_mode);
            }
            if (nest_depth == 0) {
                tokens.push_back(token);
                token = "";
            }
        } else if (token[0] == '=') {
            // permavar switch in progress
            token += c;
            if ((c == '+') || (c == '!')) {
                tokens.push_back(token);
                token = "";
            } else if (c != '=') {
                throw SnowmanException("at tokenize: invalid permavar name?",
                    true);
            }
        } else if (token.length() == 0) {
            // nothing currently in progress; start new token
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') || (c == '"') || (c == ':') ||
                    (c == '=')) {
                // some token that is longer than one character
                token += c;
                // allow token to continue to be added to
            } else /*if (c >= '!' && c <= '~')*/ {
                // single character token
                // (printable ascii is already guaranteed from if-continue
                //  above)
                if ((c == '/') && (tokens.size() > 0) &&
                        (tokens[tokens.size()-1] == "/")) {
                    // comment
                    tokens.pop_back();
                    comment = true;
                } else {
                    token += c;
                    tokens.push_back(token);
                    token = "";
                }
            }
        } else {
            throw SnowmanException("at tokenize: token started with unknown"
                "value?", true);
        }
    }
    if (token.length() != 0) tokens.push_back(token);
    return tokens;
}

// execute an individual token (called in a loop over all tokens)
void Snowman::evalToken(std::string token) {
    // used for letter operators, 2nd and 3rd if blocks below
    // (initialized to false because compiler warnings)
    bool consume = false;
    if (token[0] >= '0' && token[0] <= '9') {
        // store literal number
        int num;
        try {
            num = std::stoi(token);
        } catch (const std::invalid_argument& e) {
            store(Variable(0.0));
            throw SnowmanException("at evalToken: invalid number " + token +
                "? using 0 instead", false);
        } catch (const std::out_of_range& e) {
            store(Variable(0.0));
            throw SnowmanException("at evalToken: number " + token + " out of "
                "range, using 0 instead", false);
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
            throw SnowmanException("at evalToken: bad letter function "
                "capitalization, ignoring token", false);
        }
        // handled further below
    } else if (token.length() >= 2 && token[0] == '"') {
        // store literal string-array
        auto vec = new std::vector<Variable>(token.length() - 2);
        for (vvs i = 1; i < token.length() - 1; ++i) {
            (*vec)[i-1] = Variable((double)token[i]);
        }
        store(Variable(vec));
        return;
    } else if (token.length() >= 2 && token[0] == ':') {
        // store literal block
        auto str = new std::string(token.substr(1, token.length() - 2));
        store(Variable(str));
        return;
    } else if ((token[0] == '=') || (token[0] == '+') || (token[0] == '!')) {
        // switch permavar
        activePermavar = (token.length()-1) * 2 +
            (token[token.length()-1] == '!');
        return;
    } else if (token.length() == 1 && token[0] >= '!' && token[0] <= '~') {
        // handled below
    } else {
        throw SnowmanException("at evalToken: unrecognized token?", true);
    }

    // compute a hash for each token, so that we can use a switch statement
    // below. see also #define'd HSH1, HSH2, and HSH3
    long token_hsh = 0;
    for (char& ch : token) {
        token_hsh *= 256;
        token_hsh += ch;
    }

    // and now, time for...
    // THE HUGE SWITCH STATEMENT! (this contains all operators, letter or
    //   otherwise)

    std::vector<Variable> vec; // for convenience with retrieve()
    Variable v; // for variable operators (ROT2, ROT3)
    bool b; // for active variable rotation operators (ROT_ACT)

    switch (token_hsh) {

    /// Rotation operators
    case HSH1('/'): /// cf
        ROT2(2, 5); break;
    case HSH1('\\'): /// ah
        ROT2(0, 7); break;
    case HSH1('_'): /// fh
        ROT2(5, 7); break;
    case HSH1('['): /// af
        ROT2(0, 5); break;
    case HSH1(']'): /// ch
        ROT2(2, 7); break;
    case HSH1('|'): /// bg
        ROT2(1, 6); break;
    case HSH1('-'): /// de
        ROT2(3, 4); break;
    case HSH1('\''): /// bd
        ROT2(1, 3); break;
    case HSH1('`'): /// be
        ROT2(1, 4); break;
    case HSH1(','): /// eg
        ROT2(4, 6); break;
    case HSH1('.'): /// dg
        ROT2(3, 6); break;
    case HSH1('^'): /// dbe
        ROT3(1, 3, 4); break;
    case HSH1('>'): /// aef
        ROT3(5, 4, 0); break;
    case HSH1('<'): /// cdh
        ROT3(2, 3, 7); break;

    /// Active variable operators
    case HSH1('('): /// af
        TOG_ACT(0); TOG_ACT(5); break;
    case HSH1(')'): /// ch
        TOG_ACT(2); TOG_ACT(7); break;
    case HSH1('{'): /// bdg
        TOG_ACT(1); TOG_ACT(3); TOG_ACT(6); break;
    case HSH1('}'): /// beg
        TOG_ACT(1); TOG_ACT(4); TOG_ACT(6); break;
    case HSH1('~'): /// invert all (abcdefgh)
        TOG_ACT(0); TOG_ACT(1); TOG_ACT(2); TOG_ACT(3); TOG_ACT(4); TOG_ACT(5);
        TOG_ACT(6); TOG_ACT(7); break;
    case HSH1('@'): /// rotate (done clockwise, abcehgfd -> bcehgfda)
        ROT_ACT(); break;
    case HSH1('%'): /// reflect (abcehgfd -> hgfdabce)
        ROT_ACT(); ROT_ACT(); ROT_ACT(); ROT_ACT(); break;
    case HSH1('?'): /// mark all as inactive
        activeVars[0] = activeVars[1] = activeVars[2] = activeVars[3] =
            activeVars[4] = activeVars[5] = activeVars[6] = activeVars[7] =
            false; break;
    case HSH1('$'): /// save current state
        // woo, C-like memory management
        memcpy(savedActiveState, activeVars, sizeof(bool)*8);
        break;
    case HSH1('&'): /// restore saved state
        memcpy(activeVars, savedActiveState, sizeof(bool)*8);
        break;

    /// Permavar operators
    case HSH1('*'): /// retrieve a value, set the current permavar's value to this
        vec = retrieve(-1, 1, true, -1);
        permavars[activePermavar] = vec[0];
        break;
    case HSH1('#'): /// store the current permavar's value
        store(permavars[activePermavar]);
        break;

    /// Number operators
    case HSH3('N','D','E'): /// (n) -> n: decrement
        store(Variable(retrieve(Variable::NUM, 1, consume)[0].numVal - 1));
        break;
    case HSH3('N','I','N'): /// (n) -> n: increment
        store(Variable(retrieve(Variable::NUM, 1, consume)[0].numVal + 1));
        break;
    case HSH3('N','A','B'): /// (n) -> n: absolute value
        vec = retrieve(Variable::NUM, 1, consume);
        store(Variable(vec[0].numVal < 0 ? -vec[0].numVal : vec[0].numVal));
        break;
    case HSH2('n','f'): /// (n) -> n: floor
        store(Variable(floor(retrieve(Variable::NUM, 1, consume)[0].numVal)));
        break;
    case HSH2('n','c'): /// (n) -> n: ceiling
        store(Variable(ceil(retrieve(Variable::NUM, 1, consume)[0].numVal)));
        break;
    case HSH3('N','R','O'): /// (n) -> n: round
        store(Variable(round(retrieve(Variable::NUM, 1, consume)[0].numVal)));
        break;
    case HSH3('N','B','N'): /// (n) -> n: bitwise NOT
        store(Variable((double) ~((int)round(retrieve(Variable::NUM, 1,
            consume)[0].numVal))));
        break;
    case HSH3('N','B','O'): /// (nn) -> n: bitwise OR
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable((double) (((int)round(vec[0].numVal)) |
                                 ((int)round(vec[1].numVal)))));
        break;
    case HSH3('N','B','A'): /// (nn) -> n: bitwise AND
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable((double) (((int)round(vec[0].numVal)) &
                                 ((int)round(vec[1].numVal)))));
        break;
    case HSH3('N','B','X'): /// (nn) -> n: bitwise XOR
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable((double) (((int)round(vec[0].numVal)) ^
                                 ((int)round(vec[1].numVal)))));
        break;
    case HSH2('n','a'): /// (nn) -> n: addition
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal + vec[1].numVal));
        break;
    case HSH2('n','s'): /// (nn) -> n: subtraction
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal - vec[1].numVal));
        break;
    case HSH2('n','m'): /// (nn) -> n: multiplication
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal * vec[1].numVal));
        break;
    case HSH2('n','d'): /// (nn) -> n: division
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(vec[0].numVal / vec[1].numVal));
        break;
    case HSH3('N','M','O'): /// (nn) -> n: modulo
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(fmod(vec[0].numVal, vec[1].numVal)));
        break;
    case HSH2('n','l'): /// (nn) -> n: less than
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable((double)(vec[0].numVal < vec[1].numVal)));
        break;
    case HSH2('n','g'): /// (nn) -> n: greater than
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable((double)(vec[0].numVal > vec[1].numVal)));
        break;
    case HSH2('n','r'): { /// (nn) -> n: range
        vec = retrieve(Variable::NUM, 2, consume);
        auto vrng = new std::vector<Variable>;
        for (double i = vec[0].numVal; i < vec[1].numVal; ++i) {
            vrng->push_back(Variable(i));
        }
        store(Variable(vrng));
        break;
    }
    case HSH2('n','p'): /// (nn) -> n: power
        vec = retrieve(Variable::NUM, 2, consume);
        store(Variable(pow(vec[0].numVal, vec[1].numVal)));
        break;
    case HSH2('n','b'): { /// (nn) -> a: to base
        vec = retrieve(Variable::NUM, 2, consume);
        // convert integer part
        int n = floor(vec[0].numVal), base = round(vec[1].numVal);
        bool neg = n < 0;
        if (neg) n = -n;
        std::string nb;
        while (n) {
            nb = DIGITS[n % base] + nb;
            n /= base;
        }
        if (neg) nb = '-' + nb;
        // convert decimal part
        double decimalPart = vec[0].numVal - floor(vec[0].numVal);
        if (decimalPart > TOBASE_EPSILON) {
            nb += '.';
            for (int count = 0; (count < TOBASE_PRECISION) &&
                    (decimalPart > TOBASE_EPSILON); ++count) {
                decimalPart *= base;
                int digit = floor(decimalPart);
                nb += DIGITS[digit];
                decimalPart -= digit;
            }
        }
        store(stringToArr(nb));
        break;
    }

    /// Array operators
    case HSH3('A','S','O'): { /// (a) -> a: sort
        auto arr = new std::vector<Variable>(*retrieve(Variable::ARRAY, 1,
            consume)[0].arrayVal);
        std::sort(arr->begin(), arr->end());
        store(Variable(arr));
        break;
    }
    case HSH3('A','S','B'): { /// (ab) -> a: sort by
        auto arr = new std::vector<Variable>(*retrieve(Variable::ARRAY, 1,
            consume)[0].arrayVal);
        std::string cmp = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        std::sort(arr->begin(), arr->end(),
            [&] (Variable const& a, Variable const& b) {
                store(a);
                store(b);
                run(cmp);
                return Snowman::toBool(retrieve(-1, 1, true, -1)[0]);
            });
        store(Variable(arr));
        break;
    }
    case HSH2('a','f'): { /// (ab) -> *: fold
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        std::string blk = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        store(vec[0]);
        for (vvs i = 1; i < vec.size(); ++i) {
            store(vec[i]);
            run(blk);
        }
        break;
    }
    case HSH2('a','c'): { /// (aa) -> a: concatenate arrays
        vec = retrieve(Variable::ARRAY, 2, consume);
        int s1 = vec[0].arrayVal->size(), s2 = vec[1].arrayVal->size();
        auto arr = new std::vector<Variable>(s1 + s2);
        for (int i = 0; i < s1; ++i) (*arr)[i] = (*vec[0].arrayVal)[i];
        for (int i = 0; i < s2; ++i) (*arr)[s1+i] = (*vec[1].arrayVal)[i];
        store(Variable(arr));
        break;
    }
    case HSH2('a','d'): { /// (aa) -> a: array/set difference
        vec = retrieve(Variable::ARRAY, 2, consume);
        auto arr = new std::vector<Variable>;
        for (vvs i = 0; i < vec[0].arrayVal->size(); ++i) {
            if (std::find(vec[1].arrayVal->begin(), vec[1].arrayVal->end(),
                    (*vec[0].arrayVal)[i]) == vec[1].arrayVal->end()) {
                arr->push_back((*vec[0].arrayVal)[i]);
            }
        }
        store(Variable(arr));
        break;
    }
    case HSH3('A','O','R'): { /// (aa) -> a: setwise or
        vec = retrieve(Variable::ARRAY, 2, consume);
        auto arr = new std::vector<Variable>;
        for (Variable v : *vec[0].arrayVal) {
            if (std::find(arr->begin(), arr->end(), v) == arr->end()) {
                arr->push_back(v);
            }
        }
        for (Variable v : *vec[1].arrayVal) {
            if (std::find(arr->begin(), arr->end(), v) == arr->end()) {
                arr->push_back(v);
            }
        }
        store(Variable(arr));
        break;
    }
    case HSH3('A','A','N'): { /// (aa) -> a: setwise and
        vec = retrieve(Variable::ARRAY, 2, consume);
        auto arr = new std::vector<Variable>;
        for (vvs i = 0; i < vec[0].arrayVal->size(); ++i) {
            if (std::find(vec[1].arrayVal->begin(), vec[1].arrayVal->end(),
                    (*vec[0].arrayVal)[i]) != vec[1].arrayVal->end() &&
                    std::find(arr->begin(), arr->end(), (*vec[0].arrayVal)[i])
                    == arr->end()) {
                arr->push_back((*vec[0].arrayVal)[i]);
            }
        }
        store(Variable(arr));
        break;
    }
    case HSH2('a','r'): { /// (an) -> a: array repeat
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        int count = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal);
        auto arr = new std::vector<Variable>(vec.size() * count);
        for (vvs i = 0; i < vec.size() * count; ++i) {
            (*arr)[i] = vec[i % vec.size()];
        }
        store(Variable(arr));
        break;
    }
    case HSH2('a','j'): { /// (aa) -> a: array join
        vec = retrieve(Variable::ARRAY, 2, consume);
        auto arr = new std::vector<Variable>;
        if (vec[0].arrayVal->size() < 2) {
            store(vec[0]);
        } else {
            for (auto it = vec[0].arrayVal->begin(); it !=
                    std::prev(vec[0].arrayVal->end()); ++it) {
                arr->push_back(*it);
                arr->insert(arr->end(), vec[1].arrayVal->begin(),
                    vec[1].arrayVal->end());
            }
            arr->push_back((*vec[0].arrayVal)[vec[0].arrayVal->size() - 1]);
            store(Variable(arr));
        }
        break;
    }
    case HSH2('a','s'): { /// (aa) -> a: split
        vec = retrieve(Variable::ARRAY, 2, consume);
        auto arr = new std::vector<Variable>, tmp = new std::vector<Variable>;
        for (vvs i = 0; i < vec[0].arrayVal->size(); ++i) {
            if ((*vec[1].arrayVal) ==
                    std::vector<Variable>(vec[0].arrayVal->begin() + i,
                        vec[0].arrayVal->begin() + i +
                        vec[1].arrayVal->size())) {
                arr->push_back(Variable(tmp));
                tmp = new std::vector<Variable>;
                i += vec[1].arrayVal->size() - 1;
            } else {
                tmp->push_back((*vec[0].arrayVal)[i]);
            }
        }
        arr->push_back(Variable(tmp));
        store(Variable(arr));
        break;
    }
    case HSH2('a','g'): { /// (an) -> a: split array in groups of size
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        vvs n = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal);
        auto arr = new std::vector<Variable>, tmp = new std::vector<Variable>;
        for (vvs i = 0; i < vec.size(); ++i) {
            tmp->push_back(vec[i]);
            if (i % n == (n-1)) {
                arr->push_back(Variable(tmp));
                tmp = new std::vector<Variable>;
            }
        }
        if (tmp->size()) arr->push_back(Variable(tmp));
        else delete tmp;
        store(Variable(arr));
        break;
    }
    case HSH2('a','e'): { /// (ab) -> -: each
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        std::string b = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        for (Variable v : vec) {
            store(v);
            run(b);
        }
        break;
    }
    case HSH2('a','m'): { /// (ab) -> a: map
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        std::string b = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        auto v2 = new std::vector<Variable>;
        for (Variable v : vec) {
            store(v);
            run(b);
            v2->push_back(retrieve(-1, 1, consume, -1)[0]);
        }
        store(Variable(v2));
        break;
    }
    case HSH2('a','n'): { /// (an) -> a: every nth element
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        int n = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal);
        auto v2 = new std::vector<Variable>;
        for (vvs i = 0; i < vec.size(); i += n) v2->push_back(vec[i]);
        store(Variable(v2));
        break;
    }
    case HSH3('A','S','E'): { /// (ab) -> a: select
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        std::string b = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        auto v2 = new std::vector<Variable>;
        for (Variable v : vec) {
            store(v);
            run(b);
            if (Snowman::toBool(retrieve(-1, 1, consume, -1)[0])) {
                v2->push_back(v);
            }
        }
        store(Variable(v2));
        break;
    }
    case HSH3('A','S','I'): { /// (ab) -> a: select by index / index of / find index
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        std::string b = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        auto v2 = new std::vector<Variable>;
        for (vvs i = 0; i < vec.size(); ++i) {
            Variable v = vec[i];
            store(v);
            run(b);
            if (Snowman::toBool(retrieve(-1, 1, consume, -1)[0])) {
                v2->push_back(Variable((double)i));
            }
        }
        store(Variable(v2));
        break;
    }
    case HSH3('A','A','L'): { /// (an) -> a: elements at indeces less than n
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        vvs n = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal);
        auto v2 = new std::vector<Variable>(n);
        for (vvs i = 0; i < vec.size() && i < n; ++i) (*v2)[i] = vec[i];
        store(Variable(v2));
        break;
    }
    case HSH3('A','A','G'): { /// (an) -> a: elements at indeces greater than n
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        int n = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal);
        auto v2 = new std::vector<Variable>(vec.size() - n - 1);
        for (vvs i = n + 1; i < vec.size(); ++i) (*v2)[i - n - 1] = vec[i];
        store(Variable(v2));
        break;
    }
    case HSH2('a','a'): /// (an) -> *: element at index
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        store(vec[(int)retrieve(Variable::NUM, 1, consume, 1)[0].numVal]);
        break;
    case HSH2('a','l'): /// (a) -> n: array length
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        store(Variable((double)vec.size()));
        break;
    case HSH2('a','z'): { /// (a) -> a: zip/transpose
        vec = *retrieve(Variable::ARRAY, 1, consume)[0].arrayVal;
        auto vec2 = new std::vector<Variable>;
        // sanity check, also get max size
        vvs maxSize = 0;
        for (vvs i = 0; i < vec.size(); ++i) {
            if (vec[i].type != Variable::ARRAY) {
                throw SnowmanException("at az: array elements are not arrays, "
                        "stopping execution of az", false);
            }
            if (vec[i].arrayVal->size() > maxSize) {
                maxSize = vec[i].arrayVal->size();
            }
        }
        // fill vec2 now
        for (vvs j = 0; j < maxSize; ++j) {
            auto tmp = new std::vector<Variable>;
            for (vvs i = 0; i < vec.size(); ++i) {
                if (vec[i].arrayVal->size() > j) {
                    tmp->push_back((*vec[i].arrayVal)[j]);
                }
            }
            vec2->push_back(Variable(tmp));
        }
        store(Variable(vec2));
        break;
    }
    case HSH3('A','S','P'): { /// (anna) -> a: splice (first argument is array to splice, second is start index, third is length, fourth is what to replace with)
        std::vector<Variable> arr = *retrieve(Variable::ARRAY, 1,
            consume)[0].arrayVal;
        vvs idx = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal),
            len = round(retrieve(Variable::NUM, 1, consume, 2)[0].numVal);
        std::vector<Variable> repl = *retrieve(Variable::ARRAY, 1,
            consume, 3)[0].arrayVal;
        auto result = new std::vector<Variable>;
        for (vvs i = 0; i < idx && i < arr.size(); ++i) {
            result->push_back(arr[i]);
        }
        result->insert(result->end(), repl.begin(), repl.end());
        for (vvs i = idx + len; i < arr.size(); ++i) {
            result->push_back(arr[i]);
        }
        store(Variable(result));
        break;
    }

    /// "String" operators
    case HSH2('s','b'): { /// (an) -> n: from-base from array-"string"
        std::string str = arrToString(retrieve(Variable::ARRAY, 1,
            consume)[0].arrayVal);
        int base = round(retrieve(Variable::NUM, 1, consume, 1)[0].numVal);
        double num = 0;
        bool neg = false;
        if (str[0] == '-') {
            neg = true;
            str = str.substr(1);
        }
        ss dotPos = str.find('.');
        int subPos = str.length() - 1;
        if (dotPos != std::string::npos) {
            str.erase(dotPos, 1);
            subPos = dotPos - 1;
        }
        for (int i = str.length() - 1; i >= 0; --i) {
            char c = (str[i] >= 'A' && str[i] <= 'Z') ? str[i] + ('a' - 'A') :
                str[i];
            ss digit = DIGITS.find(c);
            if ((digit == std::string::npos) || (digit >= (ss) base)) {
                num = 0;
                break;
            } else {
                num += digit * pow(base, subPos - i);
            }
        }
        store(Variable(num * (neg ? -1 : 1)));
        break;
    }
    case HSH2('s','p'): /// (a) -> -: print an array-"string"
        vec = retrieve(Variable::ARRAY, 1, consume);
        std::cout << arrToString(vec[0]);
        break;
    case HSH2('s','m'): { /// (aa) -> a: regex match; first array-"string" is search text, second array-"string" is regex
        std::string str = arrToString(retrieve(Variable::ARRAY, 1, consume)[0]);
        std::regex rgx(arrToString(retrieve(Variable::ARRAY, 1, consume, 1)[0]));
        auto mb = std::sregex_iterator(str.begin(), str.end(), rgx),
             me = std::sregex_iterator();
        auto results = new std::vector<Variable>;
        for (auto it = mb; it != me; ++it) {
            results->push_back(stringToArr(it->str()));
        }
        store(Variable(results));
        break;
    }
    case HSH2('s','r'): { /// (aaa) -> a: regex replace; first array-"string" is string to operate on, second array-"string" is rege, third is replacement text
        std::string str = arrToString(retrieve(Variable::ARRAY, 1, consume)[0]);
        std::regex rgx(arrToString(retrieve(Variable::ARRAY, 1, consume, 1)[0]));
        std::string repl = arrToString(retrieve(Variable::ARRAY, 1, consume, 2)[0]);
        store(Variable(stringToArr(std::regex_replace(str, rgx, repl))));
        break;
    }
    case HSH3('S','R','B'): { /// (aab) -> a: same as `sr` but with a block instead of array-"string"
        std::string str = arrToString(retrieve(Variable::ARRAY, 1, consume)[0]);
        std::regex rgx(arrToString(retrieve(Variable::ARRAY, 1, consume, 1)[0]));
        std::string repl = *retrieve(Variable::BLOCK, 1, consume, 2)[0].blockVal;
        auto rb = std::sregex_token_iterator(str.begin(), str.end(), rgx, {-1,0}),
             re = std::sregex_token_iterator();
        std::string result;
        bool isMatch = false;
        for (auto it = rb; it != re; ++it) {
            if (isMatch) {
                store(stringToArr(*it));
                run(repl);
                result += arrToString(retrieve(Variable::ARRAY, 1, true, -1)[0]);
            } else {
                result += *it;
            }
            isMatch = !isMatch;
        }
        store(stringToArr(result));
        break;
    }

    /// Block operators
    case HSH2('b','r'): { /// (bn) -> -: repeat
        std::string code = *retrieve(Variable::BLOCK, 1, consume)[0].blockVal;
        double count = retrieve(Variable::NUM, 1, consume, 1)[0].numVal;
        for (int i = 0; i < round(count); ++i) run(code);
        break;
    }
    case HSH2('b','w'): /// (bb) -> -: while ("returned" value from second block is simply first non-undefined active variable, which is set to undefined after reading it)
        vec = retrieve(Variable::BLOCK, 2, consume);
        while (1) {
            run(*vec[1].blockVal);
            if (!Snowman::toBool(retrieve(-1, 1, true, -1)[0])) break;
            run(*vec[0].blockVal);
        }
        break;
    case HSH2('b','i'): { /// (bb*) -> -: if/else
        std::string ifBlock = *retrieve(Variable::BLOCK, 1, consume)[0].blockVal;
        std::string elseBlock = *retrieve(Variable::BLOCK, 1, consume, 1)[0].blockVal;
        bool condition = Snowman::toBool(retrieve(-1, 1, consume, 2)[0]);
        if (condition) run(ifBlock);
        else run(elseBlock);
        break;
    }
    case HSH2('b','d'): /// (b) -> -: do (`:...;bD` is basically the same as `:;:...;bW`)
        vec = retrieve(Variable::BLOCK, 1, consume);
        do {
            run(*vec[0].blockVal);
        } while (Snowman::toBool(retrieve(-1, 1, true, -1)[0]));
        break;
    case HSH2('b','e'): /// (b) -> -: execute / evaluate
        vec = retrieve(Variable::BLOCK, 1, consume);
        run(*vec[0].blockVal);
        break;

    /// (Any type) operators
    case HSH2('n','o'): /// (*) -> n: boolean/logical not (returns `1` for `0 :; []`, `0` otherwise)
        vec = retrieve(-1, 1, consume);
        store(Variable((double)(!Snowman::toBool(vec[0]))));
        break;
    case HSH2('w','r'): { /// (*) -> a: wrap in array
        vec = retrieve(-1, 1, consume);
        auto wrapped = new std::vector<Variable>(1);
        (*wrapped)[0] = vec[0];
        store(Variable(wrapped));
        break;
    }
    case HSH2('t','s'): /// (*) -> a: to array-"string"
        vec = retrieve(-1, 1, consume);
        store(stringToArr(Snowman::inspect(vec[0])));
        break;
    case HSH2('b','o'): /// (**) -> n: boolean/logical and ("bo" = "both" because "an," "ad," and "nd" are all taken)
        vec = retrieve(-1, 2, consume);
        store(Variable((double)(Snowman::toBool(vec[0]) &&
            Snowman::toBool(vec[1]))));
        break;
    case HSH2('o','r'): /// (**) -> n: boolean/logical or
        vec = retrieve(-1, 2, consume);
        store(Variable((double)(Snowman::toBool(vec[0]) ||
            Snowman::toBool(vec[1]))));
        break;
    case HSH2('e','q'): /// (**) -> n: equal?
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
                store(Variable((double)((*vec[0].arrayVal) ==
                    (*vec[1].arrayVal))));
                break;
            case Variable::BLOCK:
                store(Variable((double)((*vec[0].blockVal) ==
                    (*vec[1].blockVal))));
                break;
            }
        }
        break;
    case HSH2('d','u'): /// (*) -> **: duplicate
        vec = retrieve(-1, 1, consume);
        store(vec[0]);
        store(vec[0]);
        break;

    /// "Void" operators
    case HSH2('v','n'): /// (-) -> -: no-op (do nothing)
        break;
    case HSH2('v','g'): { /// (-) -> a: get line of input (as an array-"string")
        std::string line;
        std::getline(std::cin, line);
        store(stringToArr(line));
        break;
    }
    case HSH2('v','r'): /// (-) -> n: random number [0,1)
        store(Variable((double)rand() / RAND_MAX));
        break;
    case HSH2('v','t'): /// (-) -> n: time (seconds since epoch)
        store(Variable((double)time(nullptr)));
        break;
    default:
        throw SnowmanException("at evalToken: unrecognized token?", true);

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

std::vector<Variable> Snowman::retrieve(int type, vvs count, bool consume, int skip) {
    // for definition of "retrieve", see doc/snowman.md
    // (also used for gathering letter operator arguments)
    // default value of count is 1
    // default value of consume is true
    // default value of skip is 0
    // if skip is -1, any amount of variables will be skipped (ex. retrieve(-1,
    //   1, false, -1) will get you the first non-undefined variable)
    std::vector<Variable> vec;
    for (int i = 0; i < 8; ++i) {
        if (activeVars[i]) {
            if (skip > 0) {
                --skip;
                continue;
            }
            if ((vars[i].type != Variable::UNDEFINED) &&
                    (type == -1 || vars[i].type == type)) {
                vec.push_back(vars[i]);
                if (consume) vars[i] = Variable(); // set to undefined
                if (vec.size() == count) return vec;
            } else if (skip == -1) {
                continue;
            } else {
                throw SnowmanException("at retrieve: wrong type, stopping "
                    "execution of operator", false);
            }
        }
    }
    if (vec.size() < count) {
        throw SnowmanException("at retrieve: not enough variables, stopping "
            "execution of operator", true);
    }
    return vec;
}

std::string Snowman::arrToString(Variable arr) {
    // convert std::vector<Variable[.type==Variable::NUM]> to std::string
    if (arr.type != Variable::ARRAY) {
        throw SnowmanException("at arrToString: bad argument?", true);
    }
    std::string s;
    for (Variable v : *arr.arrayVal) {
        if (v.type == Variable::NUM) {
            s += (char)v.numVal;
        } else {
            throw SnowmanException("at arrToString: bad argument?", true);
        }
    }
    return s;
}

Variable Snowman::stringToArr(std::string str) {
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
    case Variable::NUM: {
        char buf[64];
        sprintf(buf, "%.*G", 16, v.numVal);
        return std::string(buf);
    }
    case Variable::ARRAY: {
        std::string s = "[";
        for (Variable v2 : *v.arrayVal) {
            s += Snowman::inspect(v2) + " ";
        }
        if (s.length() == 1) s = "[]";
        else s[s.length()-1] = ']';
        return s;
    }
    case Variable::BLOCK:
        return ":" + (*v.blockVal) + ";";
    default: throw SnowmanException("at inspect: impossible type?", true);
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
    default: throw SnowmanException("at toBool: impossible type?", true);
    }
}

std::string Snowman::debug() {
    std::string s;
    for (int i = 0; i < 8; ++i) {
        s += "{" + (activeVars[i] ? std::string("*") : std::string("")) + " " +
            Snowman::inspect(vars[i]) + " } ";
    }

    for (const auto& pv : permavars) {
        s += std::string(pv.first / 2, '=') + (pv.first % 2 == 0 ? "+" : "!") +
            "=" + Snowman::inspect(pv.second) + " ";
    }

    s[s.length()-1] = '\n';
    return s;
}
