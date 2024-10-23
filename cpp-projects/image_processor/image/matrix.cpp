#include "matrix.h"

Matrix::Matrix() : width_(0), height_(0), data_({}) {
}

Matrix::Matrix(size_t width, size_t height) : width_(width), height_(height), data_(width, std::vector(height, 0.0f)) {
}

Matrix::Matrix(std::vector<std::vector<float>> data) : width_(data[0].size()), height_(data.size()), data_(data) {
}

float& Matrix::operator()(size_t row, size_t column) {
    return data_[row][column];
}

const float& Matrix::operator()(size_t row, size_t column) const {
    return data_[row][column];
}

size_t Matrix::GetHeight() const {
    return height_;
}

size_t Matrix::GetWidth() const {
    return width_;
}
