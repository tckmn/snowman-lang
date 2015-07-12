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
}

std::vector<std::string> Snowman::tokenize(std::string code) {
    std::vector<std::string> tokens;
    std::string token;
    for (char& c : code) {
        if (token[0] >= 'a' && token[0] <= 'z') {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                token += c;
                tokens.push_back(token);
                token = "";
            } else {
                // ERROR TODO
            }
        } else if (token[0] >= 'A' && token[0] <= 'Z') {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                token += c;
                if (token.length() == 3) {
                    tokens.push_back(token);
                    token = "";
                }
            } else {
                // ERROR TODO
            }
        } else if (token[0] == '"') {
            token += c;
            if (c == '"') {
                tokens.push_back(token);
                token = "";
            }
        } else if (token.length() == 0) {
            token += c;
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '"')) {
                // allow token to continue to be added to
            } else if (c >= '!' && c <= '~') {
                tokens.push_back(token);
                token = "";
            } else {
                // ERROR TODO
            }
        } else {
            // ERROR TODO
        }
    }
    return tokens;
}
