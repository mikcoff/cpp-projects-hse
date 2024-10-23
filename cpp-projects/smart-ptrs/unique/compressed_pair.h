#pragma once
#include <type_traits>
#include <utility>

template <typename F, bool is_empty = std::is_empty_v<F> && !std::is_final_v<F>>
class First;

template <typename F>
class First<F, true> : private F {
public:
    First() = default;
    First(const F& first) : F(first) {
    }
    First(F&& first) : F(std::move(first)) {
    }

    F& Get() {
        return *this;
    }
    const F& Get() const {
        return *this;
    }
};

template <typename F>
class First<F, false> {
public:
    First() : first_(){};
    First(const F& first) : first_(first) {
    }
    First(F&& first) : first_(std::move(first)) {
    }

    F& Get() {
        return first_;
    }
    const F& Get() const {
        return first_;
    }

private:
    F first_;
};

template <typename S, bool is_empty = std::is_empty_v<S> && !std::is_final_v<S>>
class Second;

template <typename S>
class Second<S, true> : private S {
public:
    Second() = default;
    Second(const S& second) : S(second) {
    }
    Second(S&& second) : S(std::move(second)) {
    }

    S& Get() {
        return *this;
    }
    const S& Get() const {
        return *this;
    }
};

template <typename S>
class Second<S, false> {
public:
    Second() : second_(){};
    Second(const S& second) : second_(second) {
    }
    Second(S&& second) : second_(std::move(second)) {
    }

    S& Get() {
        return second_;
    }
    const S& Get() const {
        return second_;
    }

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair : private First<F>, private Second<S> {
public:
    CompressedPair() = default;
    CompressedPair(const F& first, const S& second) : First<F>(first), Second<S>(second) {
    }
    CompressedPair(const F& first, S&& second) : First<F>(first), Second<S>(std::move(second)) {
    }
    CompressedPair(F&& first, const S& second) : First<F>(std::move(first)), Second<S>(second) {
    }
    CompressedPair(F&& first, S&& second)
        : First<F>(std::move(first)), Second<S>(std::move(second)) {
    }

    F& GetFirst() {
        return First<F>::Get();
    }
    const F& GetFirst() const {
        return First<F>::Get();
    }

    S& GetSecond() {
        return Second<S>::Get();
    }
    const S& GetSecond() const {
        return Second<S>::Get();
    }
};