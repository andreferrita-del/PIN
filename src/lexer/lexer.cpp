#include "lexer/lexer.hpp"
#include <cctype>

namespace pin {

Lexer::Lexer(const std::string& input) : source(input) {}

bool Lexer::ended() const {
    return current >= source.size();
}

char Lexer::peek() const {
    return ended() ? '\0' : source[current];
}

char Lexer::peekNext() const {
    return current + 1 >= source.size() ? '\0' : source[current + 1];
}

char Lexer::advance() {
    char c = source[current++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (ended() || peek() != expected) return false;
    advance();
    return true;
}

void Lexer::add(TokenType type) {
    tokens.push_back({
        type,
        source.substr(start, current - start),
        line,
        startColumn
    });
}

void Lexer::scanString() {
    char quote = source[start];

    while (!ended() && peek() != quote) {
        if (peek() == '\n') break;

        if (peek() == '\\' && peekNext() != '\0') {
            advance();
        }
        advance();
    }

    if (ended() || peek() != quote) {
        add(TokenType::Invalid);
        return;
    }

    advance();
    add(TokenType::String);
}

void Lexer::scanNumber() {
    while (std::isdigit(
        static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' &&
        std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance();

        while (std::isdigit(
            static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    add(TokenType::Number);
}

void Lexer::scanIdentifier() {
    while (std::isalnum(
        static_cast<unsigned char>(peek())) ||
        peek() == '_') {
        advance();
    }

    std::string word = source.substr(start, current - start);

    static const std::pair<const char*, TokenType> keywords[] = {
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"null", TokenType::Null},
        {"let", TokenType::Let},
        {"function", TokenType::Function},
        {"return", TokenType::Return},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"while", TokenType::While},
        {"for", TokenType::For},
        {"in", TokenType::In},
        {"print", TokenType::Print}
    };

    for (const auto& item : keywords) {
        if (word == item.first) {
            add(item.second);
            return;
        }
    }

    add(TokenType::Identifier);
}

void Lexer::scanToken() {
    char c = advance();

    switch (c) {
        case ' ':
        case '\r':
        case '\t':
        case '\n':
            return;

        case '#':
            while (!ended() && peek() != '\n') advance();
            return;

        case '"':
        case '\'':
            scanString();
            return;

        case '{': add(TokenType::LeftBrace); return;
        case '}': add(TokenType::RightBrace); return;
        case '[': add(TokenType::LeftBracket); return;
        case ']': add(TokenType::RightBracket); return;
        case '(': add(TokenType::LeftParen); return;
        case ')': add(TokenType::RightParen); return;
        case ':': add(TokenType::Colon); return;
        case ',': add(TokenType::Comma); return;
        case '.': add(TokenType::Dot); return;
        case ';': add(TokenType::Semicolon); return;

        case '+': add(TokenType::Plus); return;
        case '-': add(TokenType::Minus); return;
        case '*': add(TokenType::Star); return;
        case '/': add(TokenType::Slash); return;
        case '%': add(TokenType::Percent); return;

        case '=':
            add(match('=') ? TokenType::EqualEqual : TokenType::Equal);
            return;

        case '!':
            add(match('=') ? TokenType::BangEqual : TokenType::Bang);
            return;

        case '<':
            add(match('=') ? TokenType::LessEqual : TokenType::Less);
            return;

        case '>':
            add(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
            return;

        case '&':
            if (match('&')) add(TokenType::And);
            else add(TokenType::Invalid);
            return;

        case '|':
            if (match('|')) add(TokenType::Or);
            else add(TokenType::Invalid);
            return;

        default:
            if (std::isdigit(static_cast<unsigned char>(c))) {
                scanNumber();
            } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                scanIdentifier();
            } else {
                add(TokenType::Invalid);
            }
    }
}

std::vector<Token> Lexer::scan() {
    while (!ended()) {
        start = current;
        startColumn = column;
        scanToken();
    }

    tokens.push_back({
        TokenType::EndOfFile,
        "",
        line,
        column
    });

    return tokens;
}

}
