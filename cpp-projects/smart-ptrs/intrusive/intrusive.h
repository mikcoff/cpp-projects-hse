#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        ++count_;
        return count_;
    }
    size_t DecRef() {
        --count_;
        return count_;
    }
    size_t RefCount() const {
        return count_;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    RefCounted() = default;
    RefCounted(const RefCounted&) {
    }
    RefCounted& operator=(const RefCounted&) {
        return *this;
    }
    // Increase reference counter.
    void IncRef() {
        this->counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (!(this->counter_.DecRef())) {
            Deleter::Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() : ptr_(nullptr) {
    }
    IntrusivePtr(std::nullptr_t) : ptr_(nullptr) {
    }
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : ptr_(std::move(other.ptr_)) {
    }

    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    IntrusivePtr(IntrusivePtr&& other) : ptr_(nullptr) {
        other.Swap(*this);
    }

    // `operator=`-s
    template <typename Y>
    IntrusivePtr& operator=(const IntrusivePtr<Y>& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
        return *this;
    }

    template <typename Y>
    IntrusivePtr& operator=(IntrusivePtr<Y>&& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = std::move(other.ptr_);
        if (ptr_) {
            ptr_->IncRef();
        }
        return *this;
    }

    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = other.ptr_;
        if (ptr_) {
            ptr_->IncRef();
        }
        return *this;
    }
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        if (ptr_) {
            ptr_->DecRef();
        }
        other.Swap(*this);
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        Reset();
    }

    // Modifiers
    void Reset() {
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        if (ptr_) {
            ptr_->DecRef();
        }
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    // Observers
    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (ptr_) {
            return ptr_->RefCount();
        }
        return 0;
    }
    explicit operator bool() const {
        return static_cast<bool>(ptr_);
    }

private:
    T* ptr_;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    return IntrusivePtr<T>(new T(std::forward<Args>(args)...));
}