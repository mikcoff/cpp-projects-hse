#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <functional>
#include <type_traits>
#include <vector>
#include "error.h"
#include "object.h"

class Function {
public:
    virtual std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) = 0;
    virtual ~Function() = default;
};

// helpers
std::shared_ptr<Cell> GetFunctionArgs(const std::shared_ptr<Cell>& list);

template <typename T>
bool IsExpectedType(const std::shared_ptr<Object>& obj) {
    if constexpr (std::is_same_v<T, int>) {
        return Is<Number>(obj);
    } else if constexpr (std::is_same_v<T, bool>) {
        return (Is<Symbol>(obj) &&
                (As<Symbol>(obj)->GetName() == "#t" || As<Symbol>(obj)->GetName() == "#f"));
    } else {
        return Is<Cell>(obj);
    }
}

int64_t NumberToInt(const std::shared_ptr<Number>& number);

bool SymbolToBool(const std::shared_ptr<Symbol>& symbol);

std::shared_ptr<Object> BoolToSymbol(bool b);

std::vector<std::shared_ptr<Object>> ListToVector(const std::shared_ptr<Cell>& list);

std::shared_ptr<Cell> VectorToList(const std::vector<std::shared_ptr<Object>>& vec);

template <typename bin_func, int64_t init>
class FoldInit : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override {
        std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
        int64_t res = init;
        for (const auto& el : vec) {
            if (!IsExpectedType<int>(el)) {
                throw RuntimeError("invalid arg type");
            }
            res = bin_func()(res, NumberToInt(As<Number>(el)));
        }
        return std::make_shared<Number>(res);
    }
};

template <typename bin_func>
class Fold : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override {
        std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
        if (vec.empty()) {
            throw RuntimeError("empty list in fold function");
        }
        if (!IsExpectedType<int>(vec[0])) {
            throw RuntimeError("invalid arg type");
        }
        int64_t init = NumberToInt(As<Number>(vec[0]));
        for (size_t i = 1; i < vec.size(); ++i) {
            if (!IsExpectedType<int>(vec[i])) {
                throw RuntimeError("invalid arg type");
            }
            init = bin_func()(init, NumberToInt(As<Number>(vec[i])));
        }
        return std::make_shared<Number>(init);
    }
};

template <typename compr>
class CompareFunc : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override {
        std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
        if (vec.empty()) {
            return BoolToSymbol(true);
        }
        bool ordered = true;
        for (size_t i = 0; i < vec.size() - 1; ++i) {
            if (!IsExpectedType<int>(vec[i]) || !IsExpectedType<int>(vec[i + 1])) {
                throw RuntimeError("invalid arg type");
            }
            ordered = ordered &&
                      compr()(NumberToInt(As<Number>(vec[i])), NumberToInt(As<Number>(vec[i + 1])));
        }
        return BoolToSymbol(ordered);
    }
};

struct Maximum {
    int64_t operator()(int64_t f, int64_t s) {
        return std::max(f, s);
    }
};
struct Minimum {
    int64_t operator()(int64_t f, int64_t s) {
        return std::min(f, s);
    }
};

// int
using SumInt = FoldInit<std::plus<int64_t>, 0>;
using ProdInt = FoldInit<std::multiplies<int64_t>, 1>;
using SubInt = Fold<std::minus<int64_t>>;
using DivInt = Fold<std::divides<int64_t>>;
using MaxInt = Fold<Maximum>;
using MinInt = Fold<Minimum>;

using IntEqual = CompareFunc<std::equal_to<int>>;
using IntGreater = CompareFunc<std::greater<int>>;
using IntGreaterOrEq = CompareFunc<std::greater_equal<int>>;
using IntLess = CompareFunc<std::less<int>>;
using IntLessOrEq = CompareFunc<std::less_equal<int>>;

class AbsInt : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

// bool
class BoolAnd : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class BoolOr : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class BoolNot : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

template <typename T>
struct IsType {
    bool operator()(const std::shared_ptr<Object>& obj) {
        return IsExpectedType<T>(obj);
    }
};

template <typename pred>
class BoolPred : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override {
        std::vector<std::shared_ptr<Object>> vec = ListToVector(cell);
        if (vec.size() != 1) {
            throw RuntimeError("invalid number of args");
        }
        return BoolToSymbol(pred()(vec[0]));
    }
};

using IsInt = BoolPred<IsType<int>>;
using IsBool = BoolPred<IsType<bool>>;

class IsPair : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class IsList : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class IsNull : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

// list
class Cons : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class Car : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class Cdr : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class MakeList : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class ListRef : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

class ListTail : public Function {
public:
    std::shared_ptr<Object> Apply(const std::shared_ptr<Cell>& cell) override;
};

// fabric
class FunctionFabric {
public:
    FunctionFabric(const FunctionFabric& other) = delete;
    FunctionFabric(FunctionFabric&& other) = delete;
    FunctionFabric& operator=(const FunctionFabric& other) = delete;
    FunctionFabric& operator=(FunctionFabric&& other) = delete;

    template <typename Func>
    void RegisterFunction(const std::string& func_name) {
        registred_functions[func_name] = []() { return std::make_unique<Func>(); };
    }

    std::unique_ptr<Function> GetFunc(const std::string& func_name) {
        if (!registred_functions.contains(func_name)) {
            throw RuntimeError("unknown function");
        }
        return registred_functions[func_name]();
    }

    static FunctionFabric& Instance() {
        static FunctionFabric instance;
        return instance;
    }

protected:
    FunctionFabric() = default;
    ~FunctionFabric() = default;
    std::unordered_map<std::string, std::function<std::unique_ptr<Function>()>> registred_functions;
};

void RegisterFunctions();