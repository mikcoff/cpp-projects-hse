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
    BaseControlBlock() : strong_counter_(0), weak_counter_(0) {
    }

    void Increment() {
        ++strong_counter_;
    }
    void IncrementWeak() {
        ++weak_counter_;
    }
    void Decrement() {
        --strong_counter_;
        if (!strong_counter_) {
            OnZeroStrongCounter();
        }
    }
    void DecrementWeak() {
        --weak_counter_;
        if (!weak_counter_ && !strong_counter_) {
            delete this;
        }
    }
    int GetCount() {
        return strong_counter_;
    }
    int GetWeakCount() {
        return weak_counter_;
    }
    virtual void OnZeroStrongCounter() = 0;
    virtual ~BaseControlBlock() = default;

private:
    int strong_counter_;
    int weak_counter_;
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
        if (!GetWeakCount()) {
            delete this;
        }
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
        if (!GetWeakCount()) {
            delete this;
        }
    }

    T* Get() {
        return reinterpret_cast<T*>(&buffer_);
    }

private:
    alignas(T) std::byte buffer_[sizeof(T)];
};