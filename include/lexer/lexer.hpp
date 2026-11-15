#pragma once
#include "lexer/token.hpp"
#include <string>
#include <vector>

namespace pin {

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> scan();

private:
    const std::string& source;
    std::vector<Token> tokens;
    size_t start = 0;
    size_t current = 0;
    int line = 1;
    int column = 1;
    int startColumn = 1;

    bool ended() const;
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);
    void add(TokenType type);
    void scanToken();
    void scanString();
    void scanNumber();
    void scanIdentifier();
};

}
