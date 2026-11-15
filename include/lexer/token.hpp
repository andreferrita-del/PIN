#pragma once
#include <string>

namespace pin {

enum class TokenType {
    Identifier,
    Number,
    String,

    True,
    False,
    Null,

    Let,
    Function,
    Return,
    If,
    Else,
    While,
    For,
    In,

    Print,

    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    LeftParen,
    RightParen,

    Colon,
    Comma,
    Dot,
    Semicolon,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,

    Equal,
    EqualEqual,
    Bang,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    And,
    Or,

    EndOfFile,
    Invalid
};

struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;
};

std::string token_name(TokenType type);

}
