#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <cstddef>
#include <vector>
#include "error.h"

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual std::shared_ptr<Object> Evaluate() = 0;
    virtual std::shared_ptr<Object> Copy() = 0;
    virtual std::string Serialize() = 0;
    virtual bool operator==(const std::shared_ptr<Object>& other) = 0;
    virtual ~Object() = default;
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
    std::shared_ptr<Object> Evaluate() override;
    std::shared_ptr<Object> Copy() override;
    std::string Serialize() override;
    std::string SerializeInList();
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
