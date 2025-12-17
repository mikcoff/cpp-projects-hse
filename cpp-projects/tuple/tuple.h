#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <compare>

template <typename T>
concept CopyListInitializable = requires { T{}; };

template <size_t N, typename T>
struct TupleElement {
    T value;
};

template <size_t N, typename... Args>
struct ElementT;

template <typename T, typename... Tail>
struct ElementT<0, T, Tail...> {
    using Type = T;
};

template <size_t N, typename T, typename... Tail>
struct ElementT<N, T, Tail...> {
    using Type = typename ElementT<N - 1, Tail...>::Type;
};

template <size_t N, typename... Args>
class TupleBase {};

template <size_t N>
class TupleBase<N> {};

template <size_t N, typename T, typename... Tail>
class TupleBase<N, T, Tail...> : public TupleElement<N, T>, public TupleBase<N + 1, Tail...> {
public:
    constexpr TupleBase()
        requires(std::is_default_constructible_v<T>)
        : TupleElement<N, T>{}, TupleBase<N + 1, Tail...>() {

          };

    constexpr TupleBase()
        requires(!std::is_default_constructible_v<T>)
        : TupleBase<N + 1, Tail...>() {
    }

    template <typename U, typename... Us>
    constexpr TupleBase(U&& t, Us&&... tail)
        : TupleElement<N, T>(std::forward<U>(t)),
          TupleBase<N + 1, Tail...>(std::forward<Us>(tail)...) {
    }
};

template <typename... Args>
class Tuple : public TupleBase<0, Args...> {
private:
    using BaseT = TupleBase<0, Args...>;

    template <typename TupleT, std::size_t... Js>
    constexpr Tuple(const TupleT& other, std::index_sequence<Js...>) : BaseT(Get<Js>(other)...) {
    }

    template <typename TupleT, std::size_t... Js>
    constexpr Tuple(TupleT&& other, std::index_sequence<Js...>)
        : BaseT(Get<Js>(std::forward<TupleT>(other))...) {
    }

    template <typename TupleT, std::size_t... Is>
    void CopyAssign(const TupleT& other, std::index_sequence<Is...>) {
        ((Get<Is>(*this) = Get<Is>(other)), ...);
    }

    template <typename TupleT, std::size_t... Is>
    void MoveAssign(TupleT&& other, std::index_sequence<Is...>) {
        ((Get<Is>(*this) = Get<Is>(std::forward<TupleT>(other))), ...);
    }

public:
    constexpr static size_t kN = sizeof...(Args);
    using Is = std::make_index_sequence<kN>;

    constexpr explicit(!(CopyListInitializable<Args> && ...)) Tuple()  // (1)
        requires(std::is_default_constructible_v<Args> && ...)
    = default;

    explicit(!(std::is_convertible_v<const Args&, Args> && ...)) Tuple(const Args&... args)  // (2)
        requires(std::is_copy_constructible_v<Args> && ...)
        : BaseT(args...) {
    }

    template <typename... UTypes>
    explicit(!(std::is_convertible_v<UTypes&&, Args> && ...)) Tuple(UTypes&&... args)  // (3)
        requires((sizeof...(UTypes) == kN) && (std::is_constructible_v<Args, UTypes> && ...))
        : BaseT(std::forward<UTypes>(args)...) {
    }

    template <typename... UTypes>
    explicit(!((std::is_convertible_v<UTypes, Args> && ...)))
        Tuple(const Tuple<UTypes...>& other)  // (5)
        requires((sizeof...(UTypes) == kN) && (std::is_constructible_v<Args, UTypes> && ...) &&
                 ((kN != 1) || (!std::is_convertible_v<decltype(other), Args...> &&
                                !std::is_constructible_v<Args..., decltype(other)> &&
                                !std::is_same_v<Args..., UTypes...>)))
        : Tuple(other, Is{}) {
    }

    template <typename... UTypes>
    explicit(!((std::is_convertible_v<UTypes, Args> && ...)))
        Tuple(Tuple<UTypes...>&& other)  // (6)
        requires((sizeof...(UTypes) == kN) && (std::is_constructible_v<Args, UTypes> && ...) &&
                 ((kN != 1) || (!std::is_convertible_v<decltype(other), Args...> &&
                                !std::is_constructible_v<Args..., decltype(other)> &&
                                !std::is_same_v<Args..., UTypes...>)))
        : Tuple(std::move(other), Is{}) {
    }

    template <typename U1, typename U2>
    Tuple(const std::pair<U1, U2>& p)  // (9)
        requires(kN == 2 &&
                 std::is_constructible_v<typename ElementT<0, Args...>::Type, const U1&> &&
                 std::is_constructible_v<typename ElementT<1, Args...>::Type, const U2&>)
        : BaseT(p.first, p.second) {
    }

    template <typename U1, typename U2>
    Tuple(std::pair<U1, U2>&& p)  // (10)
        requires(kN == 2 && std::is_constructible_v<typename ElementT<0, Args...>::Type, U1 &&> &&
                 std::is_constructible_v<typename ElementT<1, Args...>::Type, U2 &&>)
        : BaseT(std::forward<U1>(p.first), std::forward<U2>(p.second)) {
    }

    Tuple(const Tuple& other)
        requires((std::is_copy_constructible_v<Args> && ...))
    = default;

    Tuple(const Tuple&)
        requires(!(std::is_copy_constructible_v<Args> && ...))
    = delete;

    Tuple(Tuple&& other)
        requires((std::is_move_constructible_v<Args> && ...))
    = default;

    Tuple& operator=(const Tuple& other)
        requires((std::is_copy_assignable_v<Args> && ...))
    {
        CopyAssign(other, Is{});
        return *this;
    }

    Tuple& operator=(const Tuple& other)
        requires(!(std::is_copy_assignable_v<Args> && ...))
    = delete;

    Tuple& operator=(Tuple&& other)
        requires(std::is_move_assignable_v<Args> && ...)
    {
        MoveAssign(std::move(other), Is{});
        return *this;
    }

    template <typename... UTypes>
    Tuple& operator=(const Tuple<UTypes...>& other)
        requires((sizeof...(UTypes) == kN) && (std::is_assignable_v<Args&, const UTypes&> && ...))
    {
        CopyAssign(other, Is{});
        return *this;
    }

    template <typename... UTypes>
    Tuple& operator=(Tuple<UTypes...>&& other)
        requires((sizeof...(UTypes) == kN) && (std::is_assignable_v<Args&, UTypes> && ...))
    {
        MoveAssign(std::move(other), Is{});
        return *this;
    }

    template <typename U1, typename U2>
    Tuple& operator=(const std::pair<U1, U2>& p)
        requires(kN == 2 && std::is_assignable_v<typename ElementT<0, Args...>::Type&, const U1&> &&
                 std::is_assignable_v<typename ElementT<1, Args...>::Type&, const U2&>)
    {
        Get<0>(*this) = p.first;
        Get<1>(*this) = p.second;
        return *this;
    }

    template <typename U1, typename U2>
    Tuple& operator=(std::pair<U1, U2>&& p)
        requires(kN == 2 && std::is_assignable_v<typename ElementT<0, Args...>::Type&, U1> &&
                 std::is_assignable_v<typename ElementT<1, Args...>::Type&, U2>)
    {
        Get<0>(*this) = std::forward<U1>(p.first);
        Get<1>(*this) = std::forward<U2>(p.second);
        return *this;
    }
};

template <typename T1, typename T2>
Tuple(const std::pair<T1, T2>&) -> Tuple<T1, T2>;

template <typename T1, typename T2>
Tuple(std::pair<T1, T2>&&) -> Tuple<T1, T2>;

template <size_t I, typename... Args>
constexpr typename ElementT<I, Args...>::Type& Get(Tuple<Args...>& tuple) noexcept {
    return static_cast<typename ElementT<I, Args...>::Type&>(
        static_cast<TupleElement<I, typename ElementT<I, Args...>::Type>&>(tuple).value);
}

template <size_t I, typename... Args>
constexpr const typename ElementT<I, Args...>::Type& Get(const Tuple<Args...>& tuple) noexcept {
    return static_cast<const typename ElementT<I, Args...>::Type&>(
        static_cast<const TupleElement<I, typename ElementT<I, Args...>::Type>&>(tuple).value);
}

template <size_t I, typename... Args>
constexpr typename ElementT<I, Args...>::Type&& Get(Tuple<Args...>&& tuple) noexcept {
    return static_cast<typename ElementT<I, Args...>::Type&&>(
        static_cast<TupleElement<I, typename ElementT<I, Args...>::Type>&&>(tuple).value);
}

template <size_t I, typename... Args>
constexpr const typename ElementT<I, Args...>::Type&& Get(const Tuple<Args...>&& tuple) noexcept {
    return static_cast<const typename ElementT<I, Args...>::Type&&>(
        static_cast<const TupleElement<I, typename ElementT<I, Args...>::Type>&&>(tuple).value);
}

template <typename T, typename... Args>
struct NumT;

template <typename T>
struct NumT<T> {
    static constexpr int kIND = -1;
    static constexpr int kCOUNT = 0;
};

template <typename T, typename Head, typename... Tail>
struct NumT<T, Head, Tail...> {
    static constexpr int kNEXT = NumT<T, Tail...>::kIND;
    static constexpr int kNEXTCOUNT = NumT<T, Tail...>::kCOUNT;

    static constexpr int kIND = std::is_same_v<T, Head> ? 0 : (kNEXT == -1 ? -1 : 1 + kNEXT);
    static constexpr int kCOUNT = std::is_same_v<T, Head> ? 1 + kNEXTCOUNT : kNEXTCOUNT;
};

template <typename T, typename... Args>
constexpr decltype(auto) Get(Tuple<Args...>& tuple) noexcept {
    constexpr int kCountT = NumT<T, Args...>::kCOUNT;
    static_assert(kCountT == 1, "No such type in tuple or there are more than 2 of them");
    return Get<NumT<T, Args...>::kIND>(tuple);
}

template <typename T, typename... Args>
constexpr decltype(auto) Get(const Tuple<Args...>& tuple) noexcept {
    constexpr int kCountT = NumT<T, Args...>::kCOUNT;
    static_assert(kCountT == 1, "No such type in tuple or there are more than 2 of them");
    return Get<NumT<T, Args...>::kIND>(tuple);
}

template <typename T, typename... Args>
constexpr decltype(auto) Get(Tuple<Args...>&& tuple) noexcept {
    constexpr int kCountT = NumT<T, Args...>::kCOUNT;
    static_assert(kCountT == 1, "No such type in tuple or there are more than 2 of them");
    return Get<NumT<T, Args...>::kIND>(std::move(tuple));
}

template <typename T, typename... Args>
constexpr decltype(auto) Get(const Tuple<Args...>&& tuple) noexcept {
    constexpr int kCountT = NumT<T, Args...>::kCOUNT;
    static_assert(kCountT == 1, "No such type in tuple or there are more than 2 of them");
    return Get<NumT<T, Args...>::kIND>(std::move(tuple));
}

template <typename... Args>
constexpr Tuple<std::decay_t<Args>...> MakeTuple(Args&&... args) {
    return Tuple<std::decay_t<Args>...>(std::forward<Args>(args)...);
}

template <typename... Args>
constexpr Tuple<Args&...> Tie(Args&... args) noexcept {
    return Tuple<Args&...>(args...);
}

template <typename... Args>
constexpr Tuple<Args&&...> ForwardAsTuple(Args&&... args) noexcept {
    return Tuple<Args&&...>(std::forward<Args>(args)...);
}

template <std::size_t I, typename FirstTuple, typename... RestTuples>
constexpr decltype(auto) GetAt(FirstTuple&& first, RestTuples&&... rest) {
    if constexpr (I < std::remove_reference_t<FirstTuple>::kN) {
        return Get<I>(std::forward<FirstTuple>(first));
    } else {
        return GetAt<I - std::remove_reference_t<FirstTuple>::kN>(
            std::forward<RestTuples>(rest)...);
    }
}

template <std::size_t... Is, typename... Tuples>
constexpr auto TupleCatImpl(std::index_sequence<Is...>, Tuples&&... tuples) {
    return Tuple(GetAt<Is>(std::forward<Tuples>(tuples)...)...);
}

template <typename... Tuples>
constexpr auto TupleCat(Tuples&&... tuples) {
    return TupleCatImpl(std::make_index_sequence<(std::remove_reference_t<Tuples>::kN + ... + 0)>{},
                        std::forward<Tuples>(tuples)...);
}

template <std::size_t... Is, typename... Args1, typename... Args2>
constexpr auto Compare(const Tuple<Args1...>& lhs, const Tuple<Args2...>& rhs,
                       std::index_sequence<Is...>) {
    auto result = std::strong_ordering::equal;
    ((result = (result == std::strong_ordering::equal) ? Get<Is>(lhs) <=> Get<Is>(rhs) : result),
     ...);
    return result;
}

template <typename... Args1, typename... Args2>
constexpr auto operator<=>(const Tuple<Args1...>& lhs, const Tuple<Args2...>& rhs)
    requires(sizeof...(Args1) == sizeof...(Args2) &&
             (std::three_way_comparable_with<Args1, Args2> && ...))
{
    return Compare(lhs, rhs, Tuple<Args1...>::Is());
}

template <typename... Args1, typename... Args2>
constexpr bool operator==(const Tuple<Args1...>& lhs, const Tuple<Args2...>& rhs)
    requires(sizeof...(Args1) == sizeof...(Args2) &&
             (std::equality_comparable_with<Args1, Args2> && ...))
{
    return Compare(lhs, rhs, Tuple<Args1...>::Is()) == 0;
}
