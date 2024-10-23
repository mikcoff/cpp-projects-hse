#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <utility>

template <typename T>
class DefaultDeleter {
public:
    DefaultDeleter() = default;

    template <typename U>
    DefaultDeleter(DefaultDeleter<U>&&) : DefaultDeleter<T>() {
    }
    void operator()(T* ptr) const {
        static_assert(!std::is_void_v<T>);
        static_assert(sizeof(T) > 0);
        delete ptr;
    }
};

template <typename T>
class DefaultDeleter<T[]> {
public:
    DefaultDeleter() = default;

    template <typename U>
    DefaultDeleter(DefaultDeleter<U[]>&&) : DefaultDeleter<T[]>() {
    }

    void operator()(T* ptr) const {
        static_assert(!std::is_void_v<T>);
        static_assert(sizeof(T) > 0);
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_pair_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, const Deleter& deleter) : ptr_pair_(ptr, deleter) {
    }
    UniquePtr(T* ptr, Deleter&& deleter) : ptr_pair_(ptr, std::move(deleter)) {
    }

    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr(UniquePtr&& other) noexcept : ptr_pair_(nullptr, Deleter()) {
        other.Swap(*this);
    }

    template <typename U, typename UDeleter>
    UniquePtr(UniquePtr<U, UDeleter>&& other)
        : ptr_pair_(other.Release(), std::move(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (*this != other) {
            Reset();
        }
        other.Swap(*this);
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        ptr_pair_.GetFirst() = nullptr;
        return *this;
    }
    template <typename U, typename UDeleter>
    UniquePtr& operator=(UniquePtr<U, UDeleter>&& other) {
        Reset();
        ptr_pair_ = CompressedPair<T*, Deleter>(other.Release(), std::move(other.GetDeleter()));
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() noexcept {
        T* temp = Get();
        ptr_pair_.GetFirst() = nullptr;
        return temp;
    }
    void Reset(T* ptr = nullptr) {
        T* temp = Get();
        ptr_pair_.GetFirst() = ptr;
        if (temp) {
            GetDeleter()(temp);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(ptr_pair_, other.ptr_pair_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_pair_.GetSecond();
    }
    explicit operator bool() const {
        return ptr_pair_.GetFirst();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_pair_.GetFirst();
    }
    T* operator->() const {
        return ptr_pair_.GetFirst();
    }

    bool operator==(const UniquePtr& other) const {
        return Get() == other.Get();
    }
    bool operator!=(const UniquePtr& other) const {
        return !(*this == other);
    }

private:
    CompressedPair<T*, Deleter> ptr_pair_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : ptr_pair_(ptr, Deleter()) {
    }
    UniquePtr(T* ptr, Deleter deleter) : ptr_pair_(ptr, deleter) {
    }
    UniquePtr(T* ptr, Deleter&& deleter) : ptr_pair_(ptr, std::move(deleter)) {
    }

    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr(UniquePtr&& other) noexcept : ptr_pair_(nullptr, Deleter()) {
        other.Swap(*this);
    }

    template <typename U, typename UDeleter>
    UniquePtr(UniquePtr<U, UDeleter>&& other)
        : ptr_pair_(other.Release(), std::move(other.GetDeleter())) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (*this != other) {
            Reset();
        }
        other.Swap(*this);
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        ptr_pair_.GetFirst() = nullptr;
        return *this;
    }
    template <typename U, typename UDeleter>
    UniquePtr& operator=(UniquePtr<U, UDeleter>&& other) {
        Reset();
        ptr_pair_ = CompressedPair<T*, Deleter>(other.Release(), std::move(other.GetDeleter()));

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* temp = Get();
        ptr_pair_.GetFirst() = nullptr;
        return temp;
    }
    void Reset(T* ptr = nullptr) {
        T* temp = Get();
        ptr_pair_.GetFirst() = ptr;
        if (temp) {
            GetDeleter()(temp);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(ptr_pair_, other.ptr_pair_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_pair_.GetSecond();
    }
    explicit operator bool() const {
        return ptr_pair_.GetFirst();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator[](size_t ind) {
        return Get()[ind];
    }

private:
    CompressedPair<T*, Deleter> ptr_pair_;
};
