#include "snowman.hpp"
#include <iostream>
#include <stdexcept>

Snowman::Snowman() {
    // nothing
}

Snowman::~Snowman() {
    // nothing
}

void Snowman::run(std::string code) {
    std::vector<std::string> tokens = Snowman::tokenize(code);
    for (std::string s : tokens) eval_token(s);
}

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
            token += c;
            if (c == '"') {
                tokens.push_back(token);
                token = "";
            }
        } else if (token[0] == ':') {
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
            if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') || (c == '"') || (c == ':')) {
                token += c;
                // allow token to continue to be added to
            } else if (c >= '!' && c <= '~') {
                token += c;
                tokens.push_back(token);
                token = "";
            } else {
                // ignore
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

void Snowman::eval_token(std::string token) {
    if (token[0] >= '0' && token[0] <= '9') {
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
    } else if (token.length() == 2 && token[0] >= 'a' && token[0] <= 'z') {
        // TODO
    } else if (token.length() == 3 && token[0] >= 'A' && token[0] <= 'Z') {
        // TODO
    } else if (token.length() >= 2 && token[0] == '"') {
        std::vector<Variable> vec(token.length() - 2);
        for (int i = 1; i < token.length() - 1; ++i) {
            vec[i-1] = Variable((double)token[i]);
        }
        store(Variable(vec));
    } else if (token.length() >= 2 && token[0] == ':') {
        store(Variable(token.substr(1, token.length() - 2)));
    } else if (token.length() == 1 && token[0] >= '!' && token[0] <= '~') {
        // TODO
    } else {
        std::cerr << "panic at eval_token: bad token?" << std::endl;
        exit(1);
    }
}

void Snowman::store(Variable v) {
    // TODO
}
