#pragma once

#include <vector>

template <int k>
[[clang::always_inline]]
static inline int FastSearch(const int* ptr, int value) {
    int ind = 0;

    if constexpr (k >= 1) {
        constexpr int kStep = 1 << (k - 1);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 2) {
        constexpr int kStep = 1ULL << (k - 2);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 3) {
        constexpr int kStep = 1ULL << (k - 3);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 4) {
        constexpr int kStep = 1ULL << (k - 4);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 5) {
        constexpr int kStep = 1ULL << (k - 5);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 6) {
        constexpr int kStep = 1ULL << (k - 6);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 7) {
        constexpr int kStep = 1ULL << (k - 7);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 8) {
        constexpr int kStep = 1ULL << (k - 8);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 9) {
        constexpr int kStep = 1ULL << (k - 9);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 10) {
        constexpr int kStep = 1ULL << (k - 10);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 11) {
        constexpr int kStep = 1ULL << (k - 11);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 12) {
        constexpr int kStep = 1ULL << (k - 12);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 13) {
        constexpr int kStep = 1ULL << (k - 13);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 14) {
        constexpr int kStep = 1ULL << (k - 14);
        ind += (ptr[ind + kStep] < value) * kStep;
    }
    if constexpr (k >= 15) {
        constexpr int kStep = 1ULL << (k - 15);
        ind += (ptr[ind + kStep] < value) * kStep;
    }

    return ind;
}

[[clang::always_inline]]
static inline int FastSearch16(const int* __restrict ptr, int value) {
    int pos = 0;

    __builtin_prefetch(ptr + 32768, 0, 1);
    __builtin_prefetch(ptr + 16384, 0, 1);
    __builtin_prefetch(ptr + 49152, 0, 1);

#define STEP(S)                                               \
    do {                                                      \
        int v1 = ptr[pos + S];                                \
        int v2 = ptr[pos + S * 2];                            \
        int v3 = ptr[pos + S * 3];                            \
        int cnt = (v1 < value) + (v2 < value) + (v3 < value); \
        pos += cnt * S;                                       \
    } while (0)

    STEP(16384);
    STEP(4096);
    STEP(1024);
    STEP(256);
    STEP(64);
    STEP(16);
    STEP(4);
    STEP(1);

#undef STEP

    return pos;
}

[[clang::always_inline]]
static inline bool Search(const std::vector<int>& data, int value) {
    const int n = data.size();
    const int* __restrict ptr = data.data();

    if (__builtin_expect((n & (n - 1)) == 0 && n >= 256, 1)) [[likely]] {
        int pos = 0;

        switch (n) {
            case 256:
                pos = FastSearch<8>(ptr, value);
                break;
            case 512:
                pos = FastSearch<9>(ptr, value);
                break;
            case 1024:
                pos = FastSearch<10>(ptr, value);
                break;
            case 2048:
                pos = FastSearch<11>(ptr, value);
                break;
            case 4096:
                pos = FastSearch<12>(ptr, value);
                break;
            case 8192:
                pos = FastSearch<13>(ptr, value);
                break;
            case 16384:
                pos = FastSearch<14>(ptr, value);
                break;
            case 32768:
                pos = FastSearch<15>(ptr, value);
                break;
            case 65536:
                pos = FastSearch16(ptr, value);
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
