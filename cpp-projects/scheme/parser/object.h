#pragma once

#include <memory>
#include <string>
#include <string_view>

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
public:
    Number(int v);

    int GetValue() const;

private:
    int value_;
};

class Symbol : public Object {
public:
    Symbol(std::string_view s);
    const std::string& GetName() const;

private:
    std::string name_;
};

class Cell : public Object {
public:
    Cell(const std::shared_ptr<Object>& f, const std::shared_ptr<Object>& s);
    std::shared_ptr<Object> GetFirst() const;
    std::shared_ptr<Object> GetSecond() const;

private:
    std::shared_ptr<Object> head_;
    std::shared_ptr<Object> tail_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and conversion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return static_cast<bool>(std::dynamic_pointer_cast<T>(obj));
}

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}
