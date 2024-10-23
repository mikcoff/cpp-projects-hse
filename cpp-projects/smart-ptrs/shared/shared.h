#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <utility>

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    template <typename Y>
    friend class SharedPtr;
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() noexcept : ptr_(nullptr), block_(nullptr) {
    }
    SharedPtr(std::nullptr_t) noexcept : ptr_(nullptr), block_(nullptr) {
    }
    explicit SharedPtr(T* ptr) : ptr_(ptr), block_(new PointingControlBlock<T>(ptr)) {
    }

    template <typename Y>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), block_(new PointingControlBlock<Y>(ptr)) {
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->Increment();
        }
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) : ptr_(std::move(other.ptr_)), block_(std::move(other.block_)) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            block_->Increment();
        }
    }
    SharedPtr(SharedPtr&& other) : ptr_(nullptr), block_(nullptr) {
        other.Swap(*this);
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : ptr_(ptr), block_(other.block_) {
        if (block_) {
            block_->Increment();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (*this == other) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            block_->Increment();
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (*this == other) {
            return *this;
        }
        Reset();
        other.Swap(*this);
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_) {
            block_->Increment();
        }
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        if (*this == other) {
            return *this;
        }
        Reset();
        ptr_ = std::move(other.ptr_);
        block_ = std::move(other.block_);
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (!block_) {
            return;
        }
        block_->Decrement();
        block_ = nullptr;
        ptr_ = nullptr;
    }

    template <typename Y>
    void Reset(Y* ptr) {
        if (block_) {
            block_->Decrement();
        }
        ptr_ = ptr;
        block_ = new PointingControlBlock<Y>(ptr);
    }

    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (block_) {
            return block_->GetCount();
        }
        return 0;
    }
    explicit operator bool() const {
        return static_cast<bool>(ptr_);
    }

    SharedPtr<T> ConstructWithBlock(T* ptr, BaseControlBlock* block) {
        SharedPtr<T> p{};
        p.ptr_ = ptr;
        p.block_ = block;
        if (block) {
            block->Increment();
        }
        return p;
    }

private:
    T* ptr_;
    BaseControlBlock* block_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return (left.Get() == right.Get());
}

template <typename T, typename U>
inline bool operator!=(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return !(left == right);
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    EmplacingControlBlock<T>* block = new EmplacingControlBlock<T>(std::forward<Args>(args)...);
    return SharedPtr<T>().ConstructWithBlock(block->Get(), block);
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
