#pragma once

#include <stddef.h>
#include <vector>

class Matrix {
public:
    Matrix();
    Matrix(size_t width, size_t height);
    explicit Matrix(std::vector<std::vector<float>> data);

    float& operator()(size_t row, size_t column);
    const float& operator()(size_t row, size_t column) const;

    size_t GetWidth() const;
    size_t GetHeight() const;

private:
    size_t width_;
    size_t height_;
    std::vector<std::vector<float>> data_;
};
