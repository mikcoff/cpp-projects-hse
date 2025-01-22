#include "functions.h"

// helpers
std::shared_ptr<Cell> GetFunctionArgs(const std::shared_ptr<Cell>& list) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(list);
    for (size_t i = 0; i < vec.size(); ++i) {
        if (Is<Cell>(vec[i])) {
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
        if (Is<Cell>(el)) {
            el = el->Evaluate();
        }
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
        throw RuntimeError("invalid args");
    }
    return BoolToSymbol(ListToVector(As<Cell>(As<Cell>(vec[0])->GetFirst())).size() == 2);
}

std::shared_ptr<Object> IsList::Apply(const std::shared_ptr<Cell>& cell) {
    std::shared_ptr<Object> cur = As<Cell>(cell->GetFirst())->GetFirst();
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
    return BoolToSymbol(!As<Cell>(cell->GetFirst())->GetFirst());
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
    if (!As<Cell>(vec[0])->GetFirst()) {
        throw RuntimeError("empty list");
    }
    vec = ListToVector(As<Cell>(As<Cell>(vec[0])->GetFirst()));
    return vec[0];
}

std::shared_ptr<Object> Cdr::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 1 || !Is<Cell>(vec[0])) {
        throw RuntimeError("invalid args");
    }
    if (!As<Cell>(vec[0])->GetFirst()) {
        throw RuntimeError("empty list");
    }
    std::shared_ptr<Cell> list = As<Cell>(As<Cell>(vec[0])->GetFirst());
    return list->GetSecond();
}

std::shared_ptr<Object> MakeList::Apply(const std::shared_ptr<Cell>& cell) {
    return cell;
}

std::shared_ptr<Object> ListRef::Apply(const std::shared_ptr<Cell>& cell) {
    std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
    if (vec.size() != 2 || !Is<Cell>(vec[0])) {
        throw RuntimeError("invalid args");
    }
    if (!As<Cell>(vec[0])->GetFirst()) {
        throw RuntimeError("empty list");
    }
    std::shared_ptr<Object> ref = vec[1];
    vec = ListToVector(As<Cell>(As<Cell>(vec[0])->GetFirst()));
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
    if (!As<Cell>(vec[0])->GetFirst()) {
        throw RuntimeError("empty list");
    }
    std::shared_ptr<Object> ref = vec[1];
    vec = ListToVector(As<Cell>(As<Cell>(vec[0])->GetFirst()));
    for (size_t i = 0; i < vec.size(); ++i) {
        if (*ref == vec[i]) {
            return VectorToList(
                std::vector<std::shared_ptr<Object>>(vec.begin() + i + 1, vec.end()));
        }
    }
    throw RuntimeError("element not in the list");
}

// register
void RegisterFunctions() {
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

    // list
    fabric.RegisterFunction<Cons>("cons");
    fabric.RegisterFunction<Car>("car");
    fabric.RegisterFunction<Cdr>("cdr");
    fabric.RegisterFunction<MakeList>("list");
    fabric.RegisterFunction<ListRef>("list-ref");
    fabric.RegisterFunction<ListTail>("list-tail");
}