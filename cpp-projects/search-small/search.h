#pragma once

#include <immintrin.h>
#include <vector>

[[clang::always_inline]]
inline bool Search(const std::vector<int>& data, int value) {
    int size = static_cast<int>(data.size());
    int i = 0;

    __m256i value_vec = _mm256_set1_epi32(value);

    for (; i + 7 < size; i += 8) {
        __m256i vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&data[i]));
        __m256i comp = _mm256_cmpeq_epi32(vec, value_vec);
        int mask = _mm256_movemask_ps(_mm256_castsi256_ps(comp));
        if (mask) {
            return true;
        }
    }

    int rest = size - i;

    if (rest >= 4) {
        __m128i vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&data[i]));
        __m128i value_vec_128 = _mm_set1_epi32(value);
        __m128i comp = _mm_cmpeq_epi32(vec, value_vec_128);
        int mask = _mm_movemask_ps(_mm_castsi128_ps(comp));
        if (mask) {
            return true;
        }
        i += 4;
        rest -= 4;
    }

    int found = 0;
    found |= (rest > 0) ? (data[i] == value) : 0;
    found |= (rest > 1) ? (data[i + 1] == value) : 0;
    found |= (rest > 2) ? (data[i + 2] == value) : 0;

    return found;
}
