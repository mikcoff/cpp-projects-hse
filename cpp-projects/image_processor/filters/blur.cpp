#include "blur.h"
#include "cmath"

Matrix Blur::CreateHKernel(float sigma) {
    size_t size = static_cast<size_t>(3 * sigma);
    if (size % 2 == 0) {
        --size;
    }
    Matrix h_kernel(size, 1);
    float sum = 0.0f;
    int mid = static_cast<int>(size) / 2;
    for (int i = -mid; i < mid + 1; ++i) {
        float x = static_cast<float>(i);
        float curr = 1.0f / static_cast<float>((sqrt(2 * M_PI) * sigma) * exp(-(x * x) / (2 * sigma * sigma)));
        sum += curr;
        h_kernel(static_cast<size_t>(mid + i), 0) = curr;
    }
    for (size_t i = 0; i < size; ++i) {
        h_kernel(i, 0) = h_kernel(i, 0) / sum;
    }
    return h_kernel;
}

Matrix Blur::CreateVKernel(float sigma) {
    size_t size = static_cast<size_t>(3 * sigma);
    if (size % 2 == 0) {
        --size;
    }
    Matrix v_kernel(1, size);
    float sum = 0.0f;
    int mid = static_cast<int>(size) / 2;
    for (int i = -mid; i < mid + 1; ++i) {
        float x = static_cast<float>(i);
        float curr = 1.0f / static_cast<float>((sqrt(2 * M_PI) * sigma) * exp(-(x * x) / (2 * sigma * sigma)));
        sum += curr;
        v_kernel(0, static_cast<size_t>(mid + i)) = curr;
    }
    for (size_t i = 0; i < size; ++i) {
        v_kernel(0, i) = v_kernel(0, i) / sum;
    }
    return v_kernel;
}

Blur::Blur(float sigma) : Convolution({CreateHKernel(sigma), CreateVKernel(sigma)}) {
}
