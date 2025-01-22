#include "object.h"
#include "functions.h"

// number
Number::Number(int v) : value_(v) {
}

int Number::GetValue() const {
    return value_;
}
std::shared_ptr<Object> Number::Evaluate() {
    return std::make_shared<Number>(*this);
}
std::shared_ptr<Object> Number::Copy() {
    return std::make_shared<Number>(value_);
}

std::string Number::Serialize() {
    return std::to_string(value_);
}
bool Number::operator==(const std::shared_ptr<Object>& other) {
    if (!Is<Number>(other)) {
        return false;
    }
    return value_ == As<Number>(other)->GetValue();
}

// symbol
Symbol::Symbol(std::string_view s) : name_(s) {
}

Symbol::Symbol(bool v) {
    v ? name_ = "#t" : name_ = "#f";
}

const std::string& Symbol::GetName() const {
    return name_;
}

std::shared_ptr<Object> Symbol::Evaluate() {
    return std::make_shared<Symbol>(*this);
}

std::shared_ptr<Object> Symbol::Copy() {
    return std::make_shared<Symbol>(name_);
}

std::string Symbol::Serialize() {
    return name_;
}

bool Symbol::operator==(const std::shared_ptr<Object>& other) {
    if (!Is<Symbol>(other)) {
        return false;
    }
    return name_ == As<Symbol>(other)->GetName();
}

// cell
Cell::Cell(const std::shared_ptr<Object>& f, const std::shared_ptr<Object>& s)
    : head_(f), tail_(s) {
}

std::shared_ptr<Object> Cell::GetFirst() const {
    return head_;
}

std::shared_ptr<Object> Cell::GetSecond() const {
    return tail_;
}

std::shared_ptr<Object> Cell::Evaluate() {
    if (!tail_ && Is<Cell>(head_)) {
        return head_->Evaluate();
    }
    if (!Is<Symbol>(head_)) {
        throw RuntimeError("invalid first arg in list");
    }
    if (As<Symbol>(head_)->GetName() == "quote") {
        return tail_;
    }
    FunctionFabric& fabric = FunctionFabric::Instance();
    std::unique_ptr<Function> func = fabric.GetFunc(As<Symbol>(head_)->GetName());
    if (As<Symbol>(head_)->GetName() == "and" || As<Symbol>(head_)->GetName() == "or") {
        return func->Apply(As<Cell>(tail_));
    }
    return func->Apply(GetFunctionArgs(As<Cell>(tail_)));
}

std::shared_ptr<Object> Cell::Copy() {
    return std::make_shared<Cell>(head_, tail_);
}

std::string Cell::Serialize() {
    std::string res = "(";
    if (!head_) {
        return "()";
    }
    if (Is<Cell>(head_)) {
        res += As<Cell>(head_)->SerializeInList();
    } else {
        res += head_->Serialize();
    }
    std::shared_ptr<Object> cur = tail_;
    if (!tail_) {
        return res + ")";
    }
    if (!Is<Cell>(cur)) {
        res += " . " + cur->Serialize();
    } else {
        res += " " + As<Cell>(cur)->SerializeInList();
    }
    res += ")";
    return res;
}

std::string Cell::SerializeInList() {
    std::string res;
    if (!head_) {
        return "()";
    }
    res += head_->Serialize();
    std::shared_ptr<Object> cur = tail_;
    if (!tail_) {
        return res;
    }
    if (!Is<Cell>(cur)) {
        res += " . " + cur->Serialize();
    } else {
        res += " " + As<Cell>(cur)->SerializeInList();
    }
    return res;
}

bool Cell::operator==(const std::shared_ptr<Object>& other) {
    if (!Is<Cell>(other)) {
        return false;
    }
    std::vector<std::shared_ptr<Object>> first = ListToVector(std::make_shared<Cell>(*this));
    std::vector<std::shared_ptr<Object>> second = ListToVector(As<Cell>(other));
    if (first.size() != second.size()) {
        return false;
    }
    for (size_t i = 0; i < first.size(); ++i) {
        if (!(first[i] == second[i])) {
            return false;
        }
    }
    return true;
}
