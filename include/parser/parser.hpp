#pragma once
#include "parser/ast.hpp"
#include <stdexcept>
#include <vector>

namespace pin {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    std::vector<StmtPtr> parse();

private:
    const std::vector<Token>& tokens;
    size_t current = 0;

    bool ended() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& consume(TokenType type, const std::string& message);

    StmtPtr statement();
    StmtPtr variableDeclaration();
    StmtPtr functionDeclaration();
    StmtPtr printStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr forStatement();
    StmtPtr blockStatement();
    StmtPtr returnStatement();
    StmtPtr expressionStatement();

    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr postfix();
    ExprPtr primary();

    ExprPtr binary(ExprPtr left, Token op, ExprPtr right);
    void optionalSemicolon();

    [[noreturn]] void error(
        const Token& token,
        const std::string& message
    ) const;
};

}
