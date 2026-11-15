#pragma once
#include "lexer/token.hpp"
#include <memory>
#include <string>
#include <vector>

namespace pin {

struct Expr {
    enum class Kind {
        Literal, Variable, Unary, Binary, Grouping,
        Array, Object, Property, Call, Index
    };

    Kind kind;
    Token token;
    std::string value;
    std::vector<std::shared_ptr<Expr>> children;
    std::vector<std::string> keys;
};

struct Stmt {
    enum class Kind {
        Variable, Assignment, Print, Expression,
        Block, If, While, Return, Function, For
    };

    Kind kind;
    Token token;
    std::string name;
    std::shared_ptr<Expr> expression;
    std::vector<std::shared_ptr<Stmt>> body;
    std::vector<std::shared_ptr<Stmt>> elseBody;
    std::vector<std::string> params;
};

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

}
