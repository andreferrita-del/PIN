#include "lexer/token.hpp"

namespace pin {

std::string token_name(TokenType t) {
    switch (t) {
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::Number: return "NUMBER";
        case TokenType::String: return "STRING";
        case TokenType::True: return "TRUE";
        case TokenType::False: return "FALSE";
        case TokenType::Null: return "NULL";
        case TokenType::Let: return "LET";
        case TokenType::Function: return "FUNCTION";
        case TokenType::Return: return "RETURN";
        case TokenType::If: return "IF";
        case TokenType::Else: return "ELSE";
        case TokenType::While: return "WHILE";
        case TokenType::For: return "FOR";
        case TokenType::In: return "IN";
        case TokenType::Print: return "PRINT";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";
        case TokenType::LeftBracket: return "LEFT_BRACKET";
        case TokenType::RightBracket: return "RIGHT_BRACKET";
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::Colon: return "COLON";
        case TokenType::Comma: return "COMMA";
        case TokenType::Dot: return "DOT";
        case TokenType::Semicolon: return "SEMICOLON";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Percent: return "PERCENT";
        case TokenType::Equal: return "EQUAL";
        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::Bang: return "BANG";
        case TokenType::BangEqual: return "BANG_EQUAL";
        case TokenType::Less: return "LESS";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::Greater: return "GREATER";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";
        case TokenType::And: return "AND";
        case TokenType::Or: return "OR";
        case TokenType::EndOfFile: return "EOF";
        default: return "INVALID";
    }
}

}
