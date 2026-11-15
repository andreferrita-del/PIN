#include "parser/parser.hpp"
#include <utility>

namespace pin {

Parser::Parser(const std::vector<Token>& input) : tokens(input) {}

bool Parser::ended() const {
    return current >= tokens.size() ||
        tokens[current].type == TokenType::EndOfFile;
}

const Token& Parser::peek() const {
    static const Token eof{TokenType::EndOfFile, "", 1, 1};
    return current < tokens.size() ? tokens[current] : eof;
}

const Token& Parser::previous() const {
    static const Token eof{TokenType::EndOfFile, "", 1, 1};
    return current > 0 ? tokens[current - 1] : eof;
}

const Token& Parser::advance() {
    if (!ended()) current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    return !ended() && peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

const Token& Parser::consume(
    TokenType type, const std::string& message
) {
    if (check(type)) return advance();
    error(peek(), message);
}

[[noreturn]] void Parser::error(
    const Token& token, const std::string& message
) const {
    throw std::runtime_error(
        "Line " + std::to_string(token.line) +
        ", column " + std::to_string(token.column) +
        ": " + message
    );
}

void Parser::optionalSemicolon() {
    match(TokenType::Semicolon);
}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> result;
    while (!ended()) result.push_back(statement());
    return result;
}

StmtPtr Parser::statement() {
    if (match(TokenType::Function)) return functionDeclaration();
    if (match(TokenType::Let)) return variableDeclaration();
    if (match(TokenType::Print)) return printStatement();
    if (match(TokenType::If)) return ifStatement();
    if (match(TokenType::While)) return whileStatement();
    if (match(TokenType::For)) return forStatement();
    if (match(TokenType::Return)) return returnStatement();
    if (match(TokenType::LeftBrace)) return blockStatement();
    return expressionStatement();
}

StmtPtr Parser::functionDeclaration() {
    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::Function;
    s->token = previous();

    const Token& name = consume(
        TokenType::Identifier, "Expected function name."
    );
    s->name = name.text;

    consume(TokenType::LeftParen, "Expected '(' after function name.");

    if (!check(TokenType::RightParen)) {
        do {
            s->params.push_back(
                consume(TokenType::Identifier, "Expected parameter name.").text
            );
        } while (match(TokenType::Comma));
    }

    consume(TokenType::RightParen, "Expected ')' after parameters.");
    consume(TokenType::LeftBrace, "Expected '{' before function body.");
    s->body = blockStatement()->body;
    return s;
}

StmtPtr Parser::variableDeclaration() {
    const Token& name = consume(
        TokenType::Identifier, "Expected variable name."
    );

    consume(TokenType::Equal, "Expected '=' after variable name.");

    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::Variable;
    s->token = name;
    s->name = name.text;
    s->expression = expression();

    optionalSemicolon();
    return s;
}

StmtPtr Parser::printStatement() {
    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::Print;
    s->token = previous();

    if (match(TokenType::LeftParen)) {
        s->expression = expression();
        consume(TokenType::RightParen, "Expected ')' after print.");
    } else {
        s->expression = expression();
    }

    optionalSemicolon();
    return s;
}

StmtPtr Parser::ifStatement() {
    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::If;
    s->token = previous();

    if (match(TokenType::LeftParen)) {
        s->expression = expression();
        consume(TokenType::RightParen, "Expected ')' after condition.");
    } else {
        s->expression = expression();
    }

    consume(TokenType::LeftBrace, "Expected '{' after condition.");
    s->body = blockStatement()->body;

    if (match(TokenType::Else)) {
        consume(TokenType::LeftBrace, "Expected '{' after else.");
        s->elseBody = blockStatement()->body;
    }

    return s;
}

StmtPtr Parser::whileStatement() {
    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::While;
    s->token = previous();

    if (match(TokenType::LeftParen)) {
        s->expression = expression();
        consume(TokenType::RightParen, "Expected ')' after condition.");
    } else {
        s->expression = expression();
    }

    consume(TokenType::LeftBrace, "Expected '{' after while condition.");
    s->body = blockStatement()->body;
    return s;
}

StmtPtr Parser::forStatement() {
    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::For;
    s->token = previous();

    match(TokenType::LeftParen);

    s->name = consume(
        TokenType::Identifier, "Expected loop variable."
    ).text;

    consume(TokenType::In, "Expected 'in' after loop variable.");
    s->expression = expression();

    match(TokenType::RightParen);
    consume(TokenType::LeftBrace, "Expected '{' before loop body.");

    s->body = blockStatement()->body;
    return s;
}

StmtPtr Parser::blockStatement() {
    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::Block;
    s->token = previous();

    while (!check(TokenType::RightBrace) && !ended()) {
        s->body.push_back(statement());
    }

    consume(TokenType::RightBrace, "Expected '}' after block.");
    return s;
}

StmtPtr Parser::returnStatement() {
    auto s = std::make_shared<Stmt>();
    s->kind = Stmt::Kind::Return;
    s->token = previous();

    if (!check(TokenType::Semicolon) &&
        !check(TokenType::RightBrace) && !ended()) {
        s->expression = expression();
    }

    optionalSemicolon();
    return s;
}

StmtPtr Parser::expressionStatement() {
    auto s = std::make_shared<Stmt>();
    s->token = peek();

    if (check(TokenType::Identifier) &&
        current + 1 < tokens.size() &&
        tokens[current + 1].type == TokenType::Equal) {
        Token name = advance();
        advance();

        s->kind = Stmt::Kind::Assignment;
        s->name = name.text;
        s->token = name;
        s->expression = expression();
    } else {
        s->kind = Stmt::Kind::Expression;
        s->expression = expression();
    }

    optionalSemicolon();
    return s;
}

ExprPtr Parser::binary(ExprPtr left, Token op, ExprPtr right) {
    auto e = std::make_shared<Expr>();
    e->kind = Expr::Kind::Binary;
    e->token = op;
    e->value = op.text;
    e->children = {left, right};
    return e;
}

ExprPtr Parser::expression() { return assignment(); }
ExprPtr Parser::assignment() { return logicalOr(); }

ExprPtr Parser::logicalOr() {
    auto e = logicalAnd();
    while (match(TokenType::Or)) {
        Token op = previous();
        e = binary(e, op, logicalAnd());
    }
    return e;
}

ExprPtr Parser::logicalAnd() {
    auto e = equality();
    while (match(TokenType::And)) {
        Token op = previous();
        e = binary(e, op, equality());
    }
    return e;
}

ExprPtr Parser::equality() {
    auto e = comparison();
    while (match(TokenType::EqualEqual) ||
           match(TokenType::BangEqual)) {
        Token op = previous();
        e = binary(e, op, comparison());
    }
    return e;
}

ExprPtr Parser::comparison() {
    auto e = term();
    while (match(TokenType::Greater) ||
           match(TokenType::GreaterEqual) ||
           match(TokenType::Less) ||
           match(TokenType::LessEqual)) {
        Token op = previous();
        e = binary(e, op, term());
    }
    return e;
}

ExprPtr Parser::term() {
    auto e = factor();
    while (match(TokenType::Plus) || match(TokenType::Minus)) {
        Token op = previous();
        e = binary(e, op, factor());
    }
    return e;
}

ExprPtr Parser::factor() {
    auto e = unary();
    while (match(TokenType::Star) ||
           match(TokenType::Slash) ||
           match(TokenType::Percent)) {
        Token op = previous();
        e = binary(e, op, unary());
    }
    return e;
}

ExprPtr Parser::unary() {
    if (match(TokenType::Bang) || match(TokenType::Minus)) {
        Token op = previous();
        auto e = std::make_shared<Expr>();
        e->kind = Expr::Kind::Unary;
        e->token = op;
        e->value = op.text;
        e->children.push_back(unary());
        return e;
    }
    return postfix();
}

ExprPtr Parser::postfix() {
    auto e = primary();

    while (true) {
        if (match(TokenType::LeftParen)) {
            auto call = std::make_shared<Expr>();
            call->kind = Expr::Kind::Call;
            call->token = previous();
            call->children.push_back(e);

            if (!check(TokenType::RightParen)) {
                do {
                    call->children.push_back(expression());
                } while (match(TokenType::Comma));
            }

            consume(TokenType::RightParen, "Expected ')' after arguments.");
            e = call;
        } else if (match(TokenType::LeftBracket)) {
            auto index = std::make_shared<Expr>();
            index->kind = Expr::Kind::Index;
            index->token = previous();
            index->children = {e, expression()};
            consume(TokenType::RightBracket, "Expected ']'.");
            e = index;
        } else if (match(TokenType::Dot)) {
            Token property = consume(
                TokenType::Identifier, "Expected property name."
            );
            auto access = std::make_shared<Expr>();
            access->kind = Expr::Kind::Property;
            access->token = property;
            access->value = property.text;
            access->children.push_back(e);
            e = access;
        } else {
            break;
        }
    }

    return e;
}

ExprPtr Parser::primary() {
    if (match(TokenType::True) ||
        match(TokenType::False) ||
        match(TokenType::Null) ||
        match(TokenType::Number) ||
        match(TokenType::String)) {
        auto e = std::make_shared<Expr>();
        e->kind = Expr::Kind::Literal;
        e->token = previous();
        e->value = previous().text;
        return e;
    }

    if (match(TokenType::Identifier)) {
        auto e = std::make_shared<Expr>();
        e->kind = Expr::Kind::Variable;
        e->token = previous();
        e->value = previous().text;
        return e;
    }

    if (match(TokenType::LeftParen)) {
        auto e = std::make_shared<Expr>();
        e->kind = Expr::Kind::Grouping;
        e->token = previous();
        e->children.push_back(expression());
        consume(TokenType::RightParen, "Expected ')'.");
        return e;
    }

    if (match(TokenType::LeftBracket)) {
        auto e = std::make_shared<Expr>();
        e->kind = Expr::Kind::Array;
        e->token = previous();

        if (!check(TokenType::RightBracket)) {
            do {
                e->children.push_back(expression());
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightBracket, "Expected ']'.");
        return e;
    }

    if (match(TokenType::LeftBrace)) {
        auto e = std::make_shared<Expr>();
        e->kind = Expr::Kind::Object;
        e->token = previous();

        if (!check(TokenType::RightBrace)) {
            do {
                std::string key;
                if (match(TokenType::Identifier) ||
                    match(TokenType::String)) {
                    key = previous().text;
                } else {
                    error(peek(), "Expected object key.");
                }

                consume(TokenType::Colon, "Expected ':'.");
                e->keys.push_back(key);
                e->children.push_back(expression());
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightBrace, "Expected '}'.");
        return e;
    }

    error(peek(), "Expected expression.");
}

}
