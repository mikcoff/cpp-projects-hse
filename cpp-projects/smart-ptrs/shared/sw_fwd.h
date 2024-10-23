#pragma once

#include <exception>
#include <cstddef>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class BaseControlBlock {
public:
    BaseControlBlock() : counter_(0) {
    }

    void Increment() {
        ++counter_;
    }
    void Decrement() {
        --counter_;
        if (!counter_) {
            OnZeroStrongCounter();
        }
    }
    int GetCount() {
        return counter_;
    }
    virtual void OnZeroStrongCounter() = 0;
    virtual ~BaseControlBlock() = default;

private:
    int counter_;
};

template <typename T>
class PointingControlBlock : public BaseControlBlock {
public:
    PointingControlBlock(T* ptr) : ptr_(ptr) {
        if (ptr) {
            BaseControlBlock::Increment();
        }
    }

    void OnZeroStrongCounter() {
        delete ptr_;
        delete this;
    }

    T* Get() {
        return ptr_;
    }

private:
    T* ptr_;
};

template <typename T>
class EmplacingControlBlock : public BaseControlBlock {
public:
    template <typename... Args>
    EmplacingControlBlock(Args&&... args) {
        new (&buffer_) T(std::forward<Args>(args)...);
    }

    void OnZeroStrongCounter() {
        Get()->~T();
        delete this;
    }

    T* Get() {
        return reinterpret_cast<T*>(&buffer_);
    }

private:
    alignas(T) std::byte buffer_[sizeof(T)];
};