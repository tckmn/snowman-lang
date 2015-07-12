#include "snowman.hpp"
#include <iostream>

Snowman::Snowman() {
    // nothing
}

Snowman::~Snowman() {
    // nothing
}

void Snowman::run(std::string code) {
    std::vector<std::string> tokens = Snowman::tokenize(code);
    for (std::string s : tokens) std::cout << s << std::endl;
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
            // TODO handle colons/semicolons in strings
            for (char& tc : token) {
                if (tc == ':') ++nest_depth;
                else if (tc == ';') {
                    if (nest_depth == 0) {
                        std::cerr << "panic at tokenize: invalid block "
                            "nesting?" << std::endl;
                    } else --nest_depth;
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
