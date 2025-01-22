#include "functions.h"
#include <cstddef>
#include <memory>
#include "error.h"
#include "object.h"
#include <iostream>
#include <ostream>

// helpers
std::shared_ptr<Cell> GetFunctionArgs(const std::shared_ptr<Cell>& list) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(list);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i]) {
            vec[i] = vec[i]->Evaluate();
        }
    }
    return VectorToList(vec);
}

int64_t NumberToInt(const std::shared_ptr<Number>& number) {
    return number->GetValue();
}

bool SymbolToBool(const std::shared_ptr<Symbol>& symbol) {
    return (symbol->GetName() == "#f")
               ? false
               : (symbol->GetName() == "#t" ? true : throw RuntimeError("invalid arg"));
}

std::shared_ptr<Object> BoolToSymbol(bool b) {
    return std::make_shared<Symbol>(b);
}

std::vector<std::shared_ptr<Object>> ListToVector(const std::shared_ptr<Cell>& list) {
    std::vector<std::shared_ptr<Object>> vec;
    if (!list) {
        return vec;
    }
    vec.push_back(list->GetFirst());
    std::shared_ptr<Object> cur = list->GetSecond();
    while (cur) {
        if (!Is<Cell>(cur)) {
            vec.push_back(cur);
            return vec;
        }
        vec.push_back(As<Cell>(cur)->GetFirst());
        cur = As<Cell>(cur)->GetSecond();
    }
    return vec;
}

std::shared_ptr<Cell> VectorToList(const std::vector<std::shared_ptr<Object>>& vec) {
    if (vec.empty()) {
        return nullptr;
    }
    return std::make_shared<Cell>(
        vec[0], VectorToList(std::vector<std::shared_ptr<Object>>(vec.begin() + 1, vec.end())));
}

// int
std::shared_ptr<Object> AbsInt::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 1 || !IsExpectedType<int>(vec[0])) {
        throw RuntimeError("invalid args");
    }
    return std::make_shared<Number>(std::abs(NumberToInt(As<Number>(vec[0]))));
}

// bool
std::shared_ptr<Object> BoolAnd::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.empty()) {
        return BoolToSymbol(true);
    }
    for (auto& el : vec) {
        el = el->Evaluate();
        if (IsExpectedType<bool>(el) && !SymbolToBool(As<Symbol>(el))) {
            return BoolToSymbol(false);
        }
    }
    return vec[vec.size() - 1];
}

std::shared_ptr<Object> BoolOr::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.empty()) {
        return BoolToSymbol(false);
    }
    for (auto& el : vec) {
        if (Is<Cell>(el)) {
            el = el->Evaluate();
        }
        if (IsExpectedType<bool>(el) && SymbolToBool(As<Symbol>(el))) {
            return BoolToSymbol(true);
        }
    }
    return vec[vec.size() - 1];
}

std::shared_ptr<Object> BoolNot::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 1) {
        throw RuntimeError("invalid number of args");
    }
    if (IsExpectedType<bool>(vec[0]) && !SymbolToBool(As<Symbol>(vec[0]))) {
        return BoolToSymbol(true);
    }
    return BoolToSymbol(false);
}

std::shared_ptr<Object> IsPair::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 1 || !Is<Cell>(vec[0])) {
        return BoolToSymbol(false);
    }
    vec = ListToVector(As<Cell>(vec[0]));
    return BoolToSymbol(vec.size() == 2);
}

std::shared_ptr<Object> IsList::Apply(const std::shared_ptr<Cell>& cell) {
    std::shared_ptr<Object> cur = cell->GetFirst();
    if (!cur) {
        return BoolToSymbol(true);
    }
    while (cur) {
        if (!Is<Cell>(cur)) {
            return BoolToSymbol(false);
        }
        cur = As<Cell>(cur)->GetSecond();
    }
    return BoolToSymbol(true);
}

std::shared_ptr<Object> IsNull::Apply(const std::shared_ptr<Cell>& cell) {
    return BoolToSymbol(!As<Cell>(cell->GetFirst()));
}

std::shared_ptr<Object> IsSymbol::Apply(const std::shared_ptr<Cell>& cell) {
    if (cell->GetSecond()) {
        throw RuntimeError("invalid number of args");
    }
    return BoolToSymbol(Is<Symbol>(cell->GetFirst()));
}

// list
std::shared_ptr<Object> Cons::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2) {
        throw RuntimeError("too many args in pair");
    }
    return std::make_shared<Cell>(vec[0], vec[1]);
}

std::shared_ptr<Object> Car::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 1 || !Is<Cell>(vec[0])) {
        throw RuntimeError("invalid args");
    }
    return As<Cell>(vec[0])->GetFirst();
}

std::shared_ptr<Object> Cdr::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 1 || !Is<Cell>(vec[0])) {
        throw RuntimeError("invalid args");
    }
    return As<Cell>(vec[0])->GetSecond();
}

std::shared_ptr<Object> MakeList::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() == 1) {
        return std::make_shared<Cell>(cell, nullptr);
    }
    return cell;
}

std::shared_ptr<Object> ListRef::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2 || !Is<Cell>(vec[0])) {
        throw RuntimeError("invalid args");
    }
    std::shared_ptr<Object> ref = vec[1];
    vec = ListToVector(As<Cell>(vec[0]));
    for (size_t i = 0; i < vec.size() - 1; ++i) {
        if (*ref == vec[i]) {
            return vec[i + 1];
        }
    }
    throw RuntimeError("element not in the list");
}

std::shared_ptr<Object> ListTail::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2 || !Is<Cell>(vec[0])) {
        throw RuntimeError("invalid args");
    }
    std::shared_ptr<Object> ref = vec[1];
    vec = ListToVector(As<Cell>(vec[0]));
    for (size_t i = 0; i < vec.size(); ++i) {
        if (*ref == vec[i]) {
            return VectorToList(
                std::vector<std::shared_ptr<Object>>(vec.begin() + i + 1, vec.end()));
        }
    }
    throw RuntimeError("element not in the list");
}

// control-flow
std::shared_ptr<Object> IfFunc::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2 && vec.size() != 3) {
        throw SyntaxError("invalid number of args in if function");
    }
    vec[0] = vec[0]->Evaluate();
    if (!IsExpectedType<bool>(vec[0])) {
        throw SyntaxError("first arg should be bool");
    }
    if (SymbolToBool(As<Symbol>(vec[0]))) {
        return vec[1]->Evaluate();
    }
    if (vec.size() == 3) {
        return vec[2]->Evaluate();
    }
    return nullptr;
}

std::shared_ptr<Object> Def::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() < 2) {
        throw SyntaxError("not enough args");
    }
    if (Is<Cell>(vec[0])) {
        DefLambda def{};
        def.SetScope(GetScope());
        def.Apply(cell);
        return std::make_shared<Number>(0);
    }
    if (vec.size() != 2) {
        throw SyntaxError("invalid number of args");
    }
    if (!Is<Symbol>(vec[0])) {
        throw SyntaxError("invalid name for variable");
    }
    As<Scope>(GetScope())->DefineVariable(As<Symbol>(vec[0])->GetName(), vec[1]->Evaluate());
    return std::make_shared<Number>(0);
}

std::shared_ptr<Object> Set::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2) {
        throw SyntaxError("invalid number of args");
    }
    if (!Is<Symbol>(vec[0])) {
        throw SyntaxError("invalid name for variable");
    }
    if (As<Scope>(GetScope())->GetVariable(As<Symbol>(vec[0])->GetName())) {
        As<Scope>(GetScope())->DefineVariable(As<Symbol>(vec[0])->GetName(), vec[1]->Evaluate());
    }
    return std::make_shared<Number>(0);
}

std::shared_ptr<Object> SetCar::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2) {
        throw SyntaxError("invalid number of args");
    }
    if (!As<Cell>(vec[0]->Evaluate())) {
        throw RuntimeError("variable is not pair");
    }
    std::shared_ptr<Object> updated = vec[1]->Evaluate();
    std::shared_ptr<Cell> current = As<Cell>(vec[0]->Evaluate());
    current->SetFirst(updated);
    return std::make_shared<Number>(0);
}

std::shared_ptr<Object> SetCdr::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2) {
        throw SyntaxError("invalid number of args");
    }
    if (!As<Cell>(vec[0]->Evaluate())) {
        throw RuntimeError("variable is not pair");
    }
    std::shared_ptr<Object> updated = vec[1]->Evaluate();
    std::shared_ptr<Cell> current = As<Cell>(vec[0]->Evaluate());
    current->SetSecond(updated);
    return std::make_shared<Number>(0);
}

std::shared_ptr<Object> DefLambda::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() < 2) {
        throw SyntaxError("not enough args in lambda");
    }
    std::vector<std::shared_ptr<Object>> args;
    std::shared_ptr<Symbol> name;
    std::vector<std::shared_ptr<Object>> body;
    if (Is<Cell>(vec[0])) {
        std::vector<std::shared_ptr<Object>> lambda_name = ListToVector(As<Cell>(vec[0]));
        if (!Is<Symbol>(lambda_name[0])) {
            throw SyntaxError("invalid lambda name");
        }
        name = As<Symbol>(lambda_name[0]);
        if (lambda_name.size() > 1) {
            for (size_t i = 1; i < lambda_name.size(); ++i) {
                args.push_back(lambda_name[i]);
            }
        }
        if (vec.size() < 2) {
            throw SyntaxError("empty lambda body");
        }
        for (size_t i = 1; i < vec.size(); ++i) {
            body.push_back(vec[i]);
        }
    } else {
        if (!Is<Symbol>(vec[0])) {
            throw SyntaxError("invalid lambda name");
        }
        name = As<Symbol>(vec[0]);
        args = ListToVector(As<Cell>(vec[1]));
        if (vec.size() < 3) {
            throw SyntaxError("empty lambda body");
        }
        for (size_t i = 2; i < vec.size(); ++i) {
            body.push_back(vec[i]);
        }
    }
    auto scope = GetScope();
    auto res = std::make_shared<LambdaFunc>(args, body);
    res->SetScope(scope);
    if (name->GetName() == "lambda") {
        res->SetScope(GetScope());
        return res;
    }
    if (!Is<Scope>(GetScope())) {
        throw RuntimeError("error");
    }
    As<Scope>(GetScope())->DefineVariable(name->GetName(), res);
    return std::make_shared<Number>(0);
}

LambdaFunc::LambdaFunc(const std::vector<std::shared_ptr<Object>>& args,
                       const std::vector<std::shared_ptr<Object>>& body)
    : args_(args), body_(body) {
}

std::shared_ptr<Object> LambdaFunc::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != args_.size()) {
        throw RuntimeError("invalid number of arguments for lambda");
    }

    std::shared_ptr<Scope> cur_scope = As<Scope>(GetScope());
    std::shared_ptr<Scope> new_scope_ptr = std::make_shared<Scope>(cur_scope);
    
    for (size_t i = 0; i < args_.size(); ++i) {
        if (!vec[i]) {
            throw RuntimeError("bad args");
        }
        if (!Is<Symbol>(args_[i])) {
            throw RuntimeError("bad args");
        }
        new_scope_ptr->DefineVariable(As<Symbol>(args_[i])->GetName(), vec[i]->Evaluate());
    }

    std::vector<std::shared_ptr<Object>> local_body(body_.size());
    for (size_t i = 0; i < body_.size(); ++i) {
        if (!body_[i]) {
            continue;
        }
        local_body[i] = body_[i]->Copy();
        local_body[i]->SetScope(new_scope_ptr);
        if (Is<Cell>(local_body[i])) {
            As<Cell>(local_body[i])->ShareScope();
        }
    }

    std::shared_ptr<Object> result = nullptr;
    for (const auto& expr : local_body) {
        if (!expr) {
            continue;
        }
        // std::cout << expr->Serialize() << std::endl;
        result = expr->Evaluate();
        // std::cout << result->Serialize() << std::endl;
    }

    return result;
}

// register
std::unordered_map<std::string, std::shared_ptr<Object>> RegisterFunctions() {
    FunctionFabric& fabric = FunctionFabric::Instance();

    // int
    fabric.RegisterFunction<SumInt>("+");
    fabric.RegisterFunction<SubInt>("-");
    fabric.RegisterFunction<ProdInt>("*");
    fabric.RegisterFunction<DivInt>("/");
    fabric.RegisterFunction<MaxInt>("max");
    fabric.RegisterFunction<MinInt>("min");
    fabric.RegisterFunction<IntEqual>("=");
    fabric.RegisterFunction<IntGreater>(">");
    fabric.RegisterFunction<IntGreaterOrEq>(">=");
    fabric.RegisterFunction<IntLess>("<");
    fabric.RegisterFunction<IntLessOrEq>("<=");
    fabric.RegisterFunction<AbsInt>("abs");

    // bool
    fabric.RegisterFunction<BoolAnd>("and");
    fabric.RegisterFunction<BoolOr>("or");
    fabric.RegisterFunction<BoolNot>("not");
    fabric.RegisterFunction<IsNull>("null?");
    fabric.RegisterFunction<IsBool>("boolean?");
    fabric.RegisterFunction<IsInt>("number?");
    fabric.RegisterFunction<IsPair>("pair?");
    fabric.RegisterFunction<IsList>("list?");
    fabric.RegisterFunction<IsSymbol>("symbol?");

    // list
    fabric.RegisterFunction<Cons>("cons");
    fabric.RegisterFunction<Car>("car");
    fabric.RegisterFunction<Cdr>("cdr");
    fabric.RegisterFunction<MakeList>("list");
    fabric.RegisterFunction<ListRef>("list-ref");
    fabric.RegisterFunction<ListTail>("list-tail");

    // control-flow
    fabric.RegisterFunction<IfFunc>("if");
    fabric.RegisterFunction<Def>("define");
    fabric.RegisterFunction<Set>("set!");
    fabric.RegisterFunction<SetCar>("set-car!");
    fabric.RegisterFunction<SetCdr>("set-cdr!");
    fabric.RegisterFunction<DefLambda>("lambda");

    return fabric.GetRegisteredFunctions();
}