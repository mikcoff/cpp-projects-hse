#include "object.h"
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include "error.h"
#include "functions.h"

std::shared_ptr<Object> Object::Evaluate() {
    throw RuntimeError("not implemented");
}
std::shared_ptr<Object> Object::Copy() {
    throw RuntimeError("not implemented");
}
std::string Object::Serialize() {
    throw RuntimeError("not implemented");
}
bool Object::operator==(const std::shared_ptr<Object>&) {
    throw RuntimeError("not implemented");
}

void Object::SetScope(const std::shared_ptr<Object>& scope) {
    current_scope_ = scope;
}

std::shared_ptr<Object> Object::GetScope() {
    return current_scope_;
}

// scope
Scope::Scope(const std::unordered_map<std::string, std::shared_ptr<Object>>& map,
             const std::shared_ptr<Scope>& outer)
    : funcs_in_scope_(map), outer_scope_(outer) {
}

Scope::Scope() : funcs_in_scope_(RegisterFunctions()), outer_scope_(nullptr) {
}

Scope::Scope(const std::shared_ptr<Scope>& outer) : outer_scope_(outer) {
}

void Scope::DefineVariable(const std::string& name, std::shared_ptr<Object> value) {
    funcs_in_scope_[name] = value;
}

std::shared_ptr<Object> Scope::GetVariable(const std::string& name) {
    std::shared_ptr<Scope> cur = std::make_shared<Scope>(*this);
    while (!cur->funcs_in_scope_.contains(name) && cur->outer_scope_) {
        cur = cur->outer_scope_;
    }
    if (cur->funcs_in_scope_.contains(name)) {
        return cur->funcs_in_scope_[name];
    }
    // std::cout << name << std::endl;
    throw NameError("variable/function not initialized");
}

std::shared_ptr<Object> Scope::Copy() {
    return std::make_shared<Scope>(funcs_in_scope_, outer_scope_);
}

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
    if (IsExpectedType<bool>(std::make_shared<Symbol>(*this))) {
        return std::make_shared<Symbol>(*this);
    }
    return As<Scope>(GetScope())->GetVariable(name_);
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

void Cell::SetFirst(const std::shared_ptr<Object>& f) {
    head_ = f;
}
void Cell::SetSecond(const std::shared_ptr<Object>& s) {
    tail_ = s;
}

std::shared_ptr<Object> Cell::Evaluate() {
    std::shared_ptr<Object> func;
    if (Is<Cell>(head_)) {
        if (Is<Symbol>(As<Cell>(head_)->GetFirst()) &&
            As<Symbol>(As<Cell>(head_)->GetFirst())->GetName() == "quote") {
            if (!tail_) {
                return As<Cell>(head_)->GetSecond();
            }
            throw RuntimeError("invalid list");
        }
        DefLambda def{};
        def.SetScope(GetScope());
        func = def.Apply(As<Cell>(head_));
    } else {
        if (!Is<Symbol>(head_)) {
            throw RuntimeError("invalid first arg in list");
        }
        if (As<Symbol>(head_)->GetName() == "quote") {
            return tail_;
        }
        func = As<Scope>(GetScope())->GetVariable(As<Symbol>(head_)->GetName());
    }
    func->SetScope(GetScope());
    if (!Is<Function>(func)) {
        return func;
    }
    if (Is<Cell>(head_) || As<Symbol>(head_)->GetName() == "and" ||
        As<Symbol>(head_)->GetName() == "or" || As<Symbol>(head_)->GetName() == "define" ||
        As<Symbol>(head_)->GetName() == "set!" || As<Symbol>(head_)->GetName() == "if" ||
        As<Symbol>(head_)->GetName() == "set-car!" || As<Symbol>(head_)->GetName() == "set-cdr!") {

        return As<Function>(func)->Apply(As<Cell>(tail_));
    }
    if (As<Symbol>(head_)->GetName() == "lambda") {
        return As<Function>(func)->Apply(std::make_shared<Cell>(*this));
    }
    return As<Function>(func)->Apply(GetFunctionArgs(As<Cell>(tail_)));
}

std::shared_ptr<Object> Cell::Copy() {
    std::shared_ptr<Object> copy_head = nullptr;
    std::shared_ptr<Object> copy_tail = nullptr;
    if (head_) {
        copy_head = head_->Copy();
    }
    if (tail_) {
        copy_tail = tail_->Copy();
    }
    return std::make_shared<Cell>(copy_head, copy_tail);
}

std::string Cell::Serialize() {
    std::string res = "(";
    if (!head_ && !tail_) {
        return "(())";
    }
    if (!head_) {
        return res + tail_->Serialize() + ")";
    }
    if (Is<Cell>(head_)) {
        res += As<Cell>(head_)->SerializeInList();
    } else {
        res += head_->Serialize();
    }
    if (!tail_) {
        return res + ")";
    }
    if (!Is<Cell>(tail_)) {
        res += " . " + tail_->Serialize();
    } else {
        res += " " + As<Cell>(tail_)->SerializeInList();
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
    if (!tail_) {
        return res;
    }
    if (!Is<Cell>(tail_)) {
        res += " . " + tail_->Serialize();
    } else {
        res += " " + As<Cell>(tail_)->SerializeInList();
    }
    return res;
}

void Cell::ShareScope() {
    if (head_) {
        head_->SetScope(GetScope());
        if (Is<Cell>(head_)) {
            As<Cell>(head_)->ShareScope();
        }
    }
    if (tail_) {
        tail_->SetScope(GetScope());
        if (Is<Cell>(tail_)) {
            As<Cell>(tail_)->ShareScope();
        }
    }
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
