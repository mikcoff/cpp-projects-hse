#pragma once

#include <utility>
#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    template <typename Y>
    friend class WeakPtr;

    template <typename Y>
    friend class SharedPtr;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() noexcept : ptr_(nullptr), block_(nullptr) {
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->IncrementWeak();
        }
    }
    WeakPtr(WeakPtr&& other) : ptr_(nullptr), block_(nullptr) {
        other.Swap(*this);
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->IncrementWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (*this == other) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            block_->IncrementWeak();
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (*this == other) {
            return *this;
        }
        Reset();
        other.Swap(*this);
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& other) {
        if (block_ == other.block_) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            block_->IncrementWeak();
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (!block_) {
            return;
        }
        block_->DecrementWeak();
        ptr_ = nullptr;
        block_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->GetCount();
    }
    bool Expired() const {
        return !(UseCount());
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }

    bool operator==(const WeakPtr& other) const {
        return (block_ == other.block_);
    }

private:
    T* ptr_;
    BaseControlBlock* block_;
};
