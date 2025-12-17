#pragma once

#include <vector>
#include <immintrin.h>

#define STEP(S)                                               \
    do {                                                      \
        int v1 = ptr[pos + S];                                \
        int v2 = ptr[pos + S * 2];                            \
        int v3 = ptr[pos + S * 3];                            \
        int cnt = (v1 < value) + (v2 < value) + (v3 < value); \
        pos += cnt * S;                                       \
    } while (0)

[[clang::always_inline]]
static inline int Tail16(const int* __restrict ptr, int pos, int value) {
    __m256i val_vec = _mm256_set1_epi32(value);
    __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr + pos));
    __m256i v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr + pos + 8));

    __m256i cmp1 = _mm256_cmpgt_epi32(val_vec, v1);
    __m256i cmp2 = _mm256_cmpgt_epi32(val_vec, v2);

    int mask = _mm256_movemask_ps(_mm256_castsi256_ps(cmp1)) |
               (_mm256_movemask_ps(_mm256_castsi256_ps(cmp2)) << 8);

    return pos + __builtin_popcount(mask) - (mask & 1);
}

[[clang::always_inline]]
static inline int Tail8(const int* __restrict ptr, int pos, int value) {
    __m256i val_vec = _mm256_set1_epi32(value);
    __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(ptr + pos));

    __m256i cmp1 = _mm256_cmpgt_epi32(val_vec, v1);

    int mask = _mm256_movemask_ps(_mm256_castsi256_ps(cmp1));

    return pos + __builtin_popcount(mask) - (mask & 1);
}

[[clang::always_inline]]
static inline int FastSearch16(const int* __restrict ptr, int value) {
    int pos = 0;

    __builtin_prefetch(ptr + 32768, 0, 1);
    __builtin_prefetch(ptr + 16384, 0, 1);
    __builtin_prefetch(ptr + 49152, 0, 1);

    STEP(16384);
    STEP(4096);
    STEP(1024);
    STEP(256);
    STEP(64);
    STEP(16);

    return Tail16(ptr, pos, value);
}

[[clang::always_inline]]
static inline int FastSearch17(const int* __restrict ptr, int value) {
    int pos = 0;

    __builtin_prefetch(ptr + 65536, 0, 1);
    __builtin_prefetch(ptr + 32768, 0, 1);
    __builtin_prefetch(ptr + 98304, 0, 1);

    STEP(32768);
    STEP(8192);
    STEP(2048);
    STEP(512);
    STEP(128);
    STEP(32);
    STEP(8);

    return Tail8(ptr, pos, value);
}

[[clang::always_inline]]
static inline int FastSearch18(const int* __restrict ptr, int value) {
    int pos = 0;

    __builtin_prefetch(ptr + 131072, 0, 1);
    __builtin_prefetch(ptr + 65536, 0, 1);
    __builtin_prefetch(ptr + 196608, 0, 1);

    STEP(65536);
    STEP(16384);
    STEP(4096);
    STEP(1024);
    STEP(256);
    STEP(64);
    STEP(16);

    return Tail16(ptr, pos, value);
}

[[clang::always_inline]]
static inline int FastSearch19(const int* __restrict ptr, int value) {
    int pos = 0;

    __builtin_prefetch(ptr + 262144, 0, 1);
    __builtin_prefetch(ptr + 131072, 0, 1);
    __builtin_prefetch(ptr + 393216, 0, 1);

    STEP(131072);
    STEP(32768);
    STEP(8192);
    STEP(2048);
    STEP(512);
    STEP(128);
    STEP(32);
    STEP(8);

    return Tail8(ptr, pos, value);
}

[[clang::always_inline]]
static inline int FastSearch20(const int* __restrict ptr, int value) {
    int pos = 0;

    __builtin_prefetch(ptr + 524288, 0, 1);
    __builtin_prefetch(ptr + 262144, 0, 1);
    __builtin_prefetch(ptr + 786432, 0, 1);

    STEP(262144);
    STEP(65536);
    STEP(16384);
    STEP(4096);
    STEP(1024);
    STEP(256);
    STEP(64);
    STEP(16);

    return Tail16(ptr, pos, value);
}

#undef STEP

[[clang::always_inline]]
static inline bool Search(const std::vector<int>& data, int value) {
    const int n = data.size();
    const int* __restrict ptr = data.data();

    if (__builtin_expect((n & (n - 1)) == 0 && n >= 65536, 1)) [[likely]] {
        int pos = 0;

        switch (n) {
            case 65536:
                pos = FastSearch16(ptr, value);
                break;
            case 131072:
                pos = FastSearch17(ptr, value);
                break;
            case 262144:
                pos = FastSearch18(ptr, value);
                break;
            case 524288:
                pos = FastSearch19(ptr, value);
                break;
            case 1048576:
                pos = FastSearch20(ptr, value);
                break;
            default:
                __builtin_unreachable();
        }

        return ptr[pos] == value || (pos + 1 < n && ptr[pos + 1] == value);
    }

    int l = 0, r = n;
    while (l < r) {
        int m = (l + r) >> 1;
        int v = ptr[m];
        l = (v < value) ? (m + 1) : l;
        r = (v < value) ? r : m;
    }
    return l < n && ptr[l] == value;
}
