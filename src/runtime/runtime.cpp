#include "runtime/runtime.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace pin {

namespace {
struct ReturnSignal {
    Value value;
};

double number(const Value& v) {
    if (!std::holds_alternative<double>(v.data))
        throw std::runtime_error("Expected a number.");
    return std::get<double>(v.data);
}

std::string stringArg(const Value& v) {
    return v.toString();
}
}

Value::Value() : data(nullptr) {}
Value::Value(std::nullptr_t) : data(nullptr) {}
Value::Value(bool v) : data(v) {}
Value::Value(double v) : data(v) {}
Value::Value(const std::string& v) : data(v) {}
Value::Value(std::shared_ptr<Array> v) : data(v) {}
Value::Value(std::shared_ptr<Object> v) : data(v) {}
Value::Value(std::shared_ptr<Function> v) : data(v) {}

std::string Value::toString() const {
    if (std::holds_alternative<std::nullptr_t>(data)) return "null";
    if (auto v = std::get_if<bool>(&data)) return *v ? "true" : "false";

    if (auto v = std::get_if<double>(&data)) {
        std::ostringstream out;
        out << std::setprecision(15) << *v;
        return out.str();
    }

    if (auto v = std::get_if<std::string>(&data)) return *v;

    if (auto v = std::get_if<std::shared_ptr<Array>>(&data)) {
        std::string out = "[";
        if (*v) {
            for (size_t i = 0; i < (*v)->size(); ++i) {
                if (i) out += ", ";
                out += (*v)->at(i).toString();
            }
        }
        return out + "]";
    }

    if (auto v = std::get_if<std::shared_ptr<Object>>(&data)) {
        std::string out = "{";
        if (*v) {
            bool first = true;
            for (const auto& item : **v) {
                if (!first) out += ", ";
                first = false;
                out += item.first + ": " + item.second.toString();
            }
        }
        return out + "}";
    }

    return "<function>";
}

bool Value::truthy() const {
    if (std::holds_alternative<std::nullptr_t>(data)) return false;
    if (auto v = std::get_if<bool>(&data)) return *v;
    if (auto v = std::get_if<double>(&data)) return *v != 0.0;
    if (auto v = std::get_if<std::string>(&data)) return !v->empty();
    return true;
}

bool Runtime::isNumber(const Value& v) {
    return std::holds_alternative<double>(v.data);
}

std::string Runtime::decodeString(const std::string& value) {
    if (value.size() < 2) return value;

    std::string out;
    for (size_t i = 1; i + 1 < value.size(); ++i) {
        char c = value[i];

        if (c == '\\' && i + 2 < value.size()) {
            char n = value[++i];
            switch (n) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case '\\': out += '\\'; break;
                case '"': out += '"'; break;
                case '\'': out += '\''; break;
                default: out += n; break;
            }
        } else {
            out += c;
        }
    }
    return out;
}

std::string Runtime::decodeKey(const std::string& value) {
    if (value.size() >= 2 &&
        (value.front() == '"' || value.front() == '\'')) {
        return decodeString(value);
    }
    return value;
}

Value Runtime::getVariable(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    throw std::runtime_error("Undefined variable: " + name);
}

void Runtime::setVariable(const std::string& name, Value value) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            found->second = std::move(value);
            return;
        }
    }
    throw std::runtime_error("Undefined variable: " + name);
}

void Runtime::defineVariable(const std::string& name, Value value) {
    scopes.back()[name] = std::move(value);
}

void Runtime::installBuiltins() {
    auto native = [](std::function<Value(const std::vector<Value>&)> fn) {
        auto f = std::make_shared<Function>();
        f->native = std::move(fn);
        return Value(f);
    };

    defineVariable("len", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("len expects 1 argument.");
        if (auto s = std::get_if<std::string>(&a[0].data))
            return Value(static_cast<double>(s->size()));
        if (auto p = std::get_if<std::shared_ptr<Array>>(&a[0].data))
            return Value(static_cast<double>((*p)->size()));
        if (auto p = std::get_if<std::shared_ptr<Object>>(&a[0].data))
            return Value(static_cast<double>((*p)->size()));
        throw std::runtime_error("len expects a string, array or object.");
    }));

    defineVariable("str", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("str expects 1 argument.");
        return Value(a[0].toString());
    }));

    defineVariable("number", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("number expects 1 argument.");
        try {
            return Value(std::stod(a[0].toString()));
        } catch (...) {
            throw std::runtime_error("Cannot convert value to number.");
        }
    }));

    defineVariable("sqrt", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("sqrt expects 1 argument.");
        double n = number(a[0]);
        if (n < 0) throw std::runtime_error("sqrt requires a non-negative number.");
        return Value(std::sqrt(n));
    }));

    defineVariable("abs", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("abs expects 1 argument.");
        return Value(std::fabs(number(a[0])));
    }));

    defineVariable("type", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("type expects 1 argument.");
        const auto& d = a[0].data;
        if (std::holds_alternative<std::nullptr_t>(d)) return Value("null");
        if (std::holds_alternative<bool>(d)) return Value("boolean");
        if (std::holds_alternative<double>(d)) return Value("number");
        if (std::holds_alternative<std::string>(d)) return Value("string");
        if (std::holds_alternative<std::shared_ptr<Array>>(d)) return Value("array");
        if (std::holds_alternative<std::shared_ptr<Object>>(d)) return Value("object");
        return Value("function");
    }));

    defineVariable("upper", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("upper expects 1 argument.");
        std::string s = stringArg(a[0]);
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return Value(s);
    }));

    defineVariable("lower", native([](const std::vector<Value>& a) -> Value {
        if (a.size() != 1) throw std::runtime_error("lower expects 1 argument.");
        std::string s = stringArg(a[0]);
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return Value(s);
    }));

    defineVariable("range", native([](const std::vector<Value>& a) -> Value {
        if (a.empty() || a.size() > 3)
            throw std::runtime_error("range expects 1 to 3 arguments.");

        double start = 0, stop = number(a[0]), step = 1;
        if (a.size() >= 2) {
            start = number(a[0]);
            stop = number(a[1]);
        }
        if (a.size() == 3) step = number(a[2]);
        if (step == 0) throw std::runtime_error("range step cannot be zero.");

        auto values = std::make_shared<Array>();
        for (double i = start; step > 0 ? i < stop : i > stop; i += step) {
            if (values->size() >= 100000)
                throw std::runtime_error("range limit reached.");
            values->emplace_back(i);
        }
        return Value(values);
    }));
}

Value Runtime::binary(
    const Value& left, const Token& op, const Value& right
) {
    const std::string& operation = op.text;

    if (operation == "&&") return Value(left.truthy() && right.truthy());
    if (operation == "||") return Value(left.truthy() || right.truthy());

    if (operation == "==" || operation == "!=") {
        bool equal = left.toString() == right.toString() &&
            left.data.index() == right.data.index();
        return Value(operation == "==" ? equal : !equal);
    }

    if (operation == "+" &&
        (std::holds_alternative<std::string>(left.data) ||
         std::holds_alternative<std::string>(right.data))) {
        return Value(left.toString() + right.toString());
    }

    if (!isNumber(left) || !isNumber(right))
        throw std::runtime_error("Operator '" + operation + "' requires numbers.");

    double a = number(left), b = number(right);

    if (operation == "+") return Value(a + b);
    if (operation == "-") return Value(a - b);
    if (operation == "*") return Value(a * b);
    if (operation == "/") {
        if (b == 0) throw std::runtime_error("Division by zero.");
        return Value(a / b);
    }
    if (operation == "%") {
        if (b == 0) throw std::runtime_error("Modulo by zero.");
        return Value(std::fmod(a, b));
    }
    if (operation == ">") return Value(a > b);
    if (operation == ">=") return Value(a >= b);
    if (operation == "<") return Value(a < b);
    if (operation == "<=") return Value(a <= b);

    throw std::runtime_error("Unknown operator: " + operation);
}

Value Runtime::evaluate(const ExprPtr& e) {
    if (!e) return Value(nullptr);

    switch (e->kind) {
        case Expr::Kind::Literal: {
            const std::string& v = e->value;
            if (v == "true") return Value(true);
            if (v == "false") return Value(false);
            if (v == "null") return Value(nullptr);
            if (!v.empty() && (v[0] == '"' || v[0] == '\''))
                return Value(decodeString(v));
            try { return Value(std::stod(v)); }
            catch (...) { throw std::runtime_error("Invalid literal: " + v); }
        }

        case Expr::Kind::Variable:
            return getVariable(e->value);

        case Expr::Kind::Grouping:
            return evaluate(e->children.at(0));

        case Expr::Kind::Unary: {
            Value v = evaluate(e->children.at(0));
            if (e->value == "!") return Value(!v.truthy());
            if (e->value == "-") return Value(-number(v));
            break;
        }

        case Expr::Kind::Binary: {
            Value left = evaluate(e->children.at(0));
            if (e->value == "&&" && !left.truthy()) return Value(false);
            if (e->value == "||" && left.truthy()) return Value(true);
            return binary(left, e->token, evaluate(e->children.at(1)));
        }

        case Expr::Kind::Array: {
            auto a = std::make_shared<Array>();
            for (const auto& child : e->children) a->push_back(evaluate(child));
            return Value(a);
        }

        case Expr::Kind::Object: {
            auto o = std::make_shared<Object>();
            for (size_t i = 0; i < e->children.size(); ++i)
                (*o)[decodeKey(e->keys.at(i))] = evaluate(e->children.at(i));
            return Value(o);
        }

        case Expr::Kind::Property: {
            Value parent = evaluate(e->children.at(0));
            auto object = std::get_if<std::shared_ptr<Object>>(&parent.data);
            if (!object || !*object)
                throw std::runtime_error("Property access requires an object.");
            auto it = (*object)->find(e->value);
            if (it == (*object)->end())
                throw std::runtime_error("Unknown property: " + e->value);
            return it->second;
        }

        case Expr::Kind::Index: {
            Value parent = evaluate(e->children.at(0));
            Value index = evaluate(e->children.at(1));

            if (auto a = std::get_if<std::shared_ptr<Array>>(&parent.data)) {
                double n = number(index);
                if (n < 0 || std::floor(n) != n ||
                    n >= static_cast<double>((*a)->size()))
                    throw std::runtime_error("Array index out of range.");
                return (*a)->at(static_cast<size_t>(n));
            }

            if (auto s = std::get_if<std::string>(&parent.data)) {
                double n = number(index);
                if (n < 0 || std::floor(n) != n ||
                    n >= static_cast<double>(s->size()))
                    throw std::runtime_error("String index out of range.");
                return Value(std::string(1, s->at(static_cast<size_t>(n))));
            }

            if (auto o = std::get_if<std::shared_ptr<Object>>(&parent.data)) {
                auto it = (*o)->find(index.toString());
                if (it == (*o)->end()) throw std::runtime_error("Unknown object key.");
                return it->second;
            }

            throw std::runtime_error("Value cannot be indexed.");
        }

        case Expr::Kind::Call: {
            Value callee = evaluate(e->children.at(0));
            std::vector<Value> args;
            for (size_t i = 1; i < e->children.size(); ++i)
                args.push_back(evaluate(e->children[i]));
            return call(callee, args);
        }
    }

    throw std::runtime_error("Invalid expression.");
}

Value Runtime::call(const Value& callee, const std::vector<Value>& args) {
    auto p = std::get_if<std::shared_ptr<Function>>(&callee.data);
    if (!p || !*p) throw std::runtime_error("Value is not callable.");

    auto function = *p;

    if (function->native) return function->native(args);

    if (args.size() != function->params.size())
        throw std::runtime_error("Incorrect number of function arguments.");

    scopes.emplace_back();
    for (size_t i = 0; i < args.size(); ++i)
        defineVariable(function->params[i], args[i]);

    Value result(nullptr);
    try {
        executeBlock(function->body);
    } catch (const ReturnSignal& signal) {
        result = signal.value;
    }

    scopes.pop_back();
    return result;
}

void Runtime::executeBlock(const std::vector<StmtPtr>& body) {
    for (const auto& item : body) executeStatement(item);
}

void Runtime::executeStatement(const StmtPtr& s) {
    if (!s) return;

    switch (s->kind) {
        case Stmt::Kind::Variable:
            defineVariable(s->name, evaluate(s->expression));
            return;

        case Stmt::Kind::Assignment:
            setVariable(s->name, evaluate(s->expression));
            return;

        case Stmt::Kind::Print:
            std::cout << evaluate(s->expression).toString() << '\n';
            return;

        case Stmt::Kind::Expression:
            evaluate(s->expression);
            return;

        case Stmt::Kind::Block:
            scopes.emplace_back();
            executeBlock(s->body);
            scopes.pop_back();
            return;

        case Stmt::Kind::If:
            scopes.emplace_back();
            executeBlock(evaluate(s->expression).truthy() ? s->body : s->elseBody);
            scopes.pop_back();
            return;

        case Stmt::Kind::While: {
            size_t count = 0;
            while (evaluate(s->expression).truthy()) {
                if (++count > 1000000)
                    throw std::runtime_error("Loop limit reached.");
                scopes.emplace_back();
                executeBlock(s->body);
                scopes.pop_back();
            }
            return;
        }

        case Stmt::Kind::For: {
            Value iterable = evaluate(s->expression);
            std::vector<Value> values;

            if (auto a = std::get_if<std::shared_ptr<Array>>(&iterable.data)) {
                values = **a;
            } else if (auto str = std::get_if<std::string>(&iterable.data)) {
                for (char c : *str) values.emplace_back(std::string(1, c));
            } else {
                throw std::runtime_error("for/in requires an array or string.");
            }

            if (values.size() > 1000000)
                throw std::runtime_error("Loop limit reached.");

            for (const auto& value : values) {
                scopes.emplace_back();
                defineVariable(s->name, value);
                executeBlock(s->body);
                scopes.pop_back();
            }
            return;
        }

        case Stmt::Kind::Function: {
            auto f = std::make_shared<Function>();
            f->params = s->params;
            f->body = s->body;
            defineVariable(s->name, Value(f));
            return;
        }

        case Stmt::Kind::Return:
            throw ReturnSignal{evaluate(s->expression)};

    }
}

void Runtime::execute(const std::vector<StmtPtr>& program) {
    scopes.clear();
    scopes.emplace_back();
    installBuiltins();
    initialized = true;
    executeBlock(program);
}

void Runtime::executeInteractive(
    const std::vector<StmtPtr>& program
) {
    if (!initialized) {
        scopes.clear();
        scopes.emplace_back();
        installBuiltins();
        initialized = true;
    }

    executeBlock(program);
}

}
