#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <cstddef>
#include <unordered_map>
#include <vector>
#include "error.h"

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::shared_ptr<Object> Evaluate();
    virtual std::shared_ptr<Object> Copy();
    virtual std::string Serialize();
    void SetScope(const std::shared_ptr<Object>& scope);
    std::shared_ptr<Object> GetScope();
    virtual bool operator==(const std::shared_ptr<Object>& other);
    virtual ~Object() = default;

private:
    std::shared_ptr<Object> current_scope_;
};

class Scope : public Object {
public:
    Scope(const std::unordered_map<std::string, std::shared_ptr<Object>>& map,
          const std::shared_ptr<Scope>& outer);
    Scope(const std::shared_ptr<Scope>& outer);
    Scope();
    void DefineVariable(const std::string& name, std::shared_ptr<Object> value);
    std::shared_ptr<Scope> GetOuter();
    std::shared_ptr<Object> GetVariable(const std::string& name);
    std::shared_ptr<Object> Copy() override;

private:
    std::unordered_map<std::string, std::shared_ptr<Object>> funcs_in_scope_;
    std::shared_ptr<Scope> outer_scope_;
};

class Number : public Object {
public:
    explicit Number(int v);
    int GetValue() const;
    std::shared_ptr<Object> Evaluate() override;
    std::shared_ptr<Object> Copy() override;
    std::string Serialize() override;
    bool operator==(const std::shared_ptr<Object>& other) override;

private:
    int value_;
};

class Symbol : public Object {
public:
    explicit Symbol(std::string_view s);
    explicit Symbol(bool v);
    const std::string& GetName() const;
    std::shared_ptr<Object> Evaluate() override;
    std::shared_ptr<Object> Copy() override;
    std::string Serialize() override;
    bool operator==(const std::shared_ptr<Object>& other) override;

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell(const std::shared_ptr<Object>& f, const std::shared_ptr<Object>& s);
    std::shared_ptr<Object> GetFirst() const;
    std::shared_ptr<Object> GetSecond() const;
    void SetFirst(const std::shared_ptr<Object>& f);
    void SetSecond(const std::shared_ptr<Object>& s);
    std::shared_ptr<Object> Evaluate() override;
    std::shared_ptr<Object> Copy() override;
    std::string Serialize() override;
    std::string SerializeInList();
    void ShareScope();
    bool operator==(const std::shared_ptr<Object>& other) override;

private:
    std::shared_ptr<Object> head_;
    std::shared_ptr<Object> tail_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj) != nullptr;
}

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}
