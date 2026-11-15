#pragma once

#include "parser/ast.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace pin {

struct Value;
struct Function;

using Array = std::vector<Value>;
using Object = std::unordered_map<std::string, Value>;

struct Value {
    using Data = std::variant<
        std::nullptr_t, bool, double, std::string,
        std::shared_ptr<Array>, std::shared_ptr<Object>,
        std::shared_ptr<Function>
    >;

    Data data = nullptr;

    Value();
    Value(std::nullptr_t);
    Value(bool value);
    Value(double value);
    Value(const std::string& value);
    Value(std::shared_ptr<Array> value);
    Value(std::shared_ptr<Object> value);
    Value(std::shared_ptr<Function> value);

    std::string toString() const;
    bool truthy() const;
};

struct Function {
    std::vector<std::string> params;
    std::vector<StmtPtr> body;
    std::function<Value(const std::vector<Value>&)> native;
};

class Runtime {
public:
    void execute(const std::vector<StmtPtr>& program);
    void executeInteractive(const std::vector<StmtPtr>& program);

private:
    using Scope = std::unordered_map<std::string, Value>;
    std::vector<Scope> scopes;
    bool initialized = false;

    Value evaluate(const ExprPtr& expression);
    void executeStatement(const StmtPtr& statement);
    void executeBlock(const std::vector<StmtPtr>& body);

    Value call(const Value& callee, const std::vector<Value>& args);
    Value binary(const Value& left, const Token& op, const Value& right);

    Value getVariable(const std::string& name);
    void setVariable(const std::string& name, Value value);
    void defineVariable(const std::string& name, Value value);

    void installBuiltins();

    static bool isNumber(const Value& value);
    static std::string decodeString(const std::string& value);
    static std::string decodeKey(const std::string& value);
};

}
