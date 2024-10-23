#include "Image.h"

Image::Image() : width_(0), height_(0) {
}

Image::Image(size_t width, size_t height)
    : width_(width), height_(height), pixels_(height_, std::vector(width_, RGB())) {
}

size_t Image::Width() const {
    return width_;
}

size_t Image::Height() const {
    return height_;
}

void Image::SetWidth(const size_t& new_width) {
    width_ = new_width;
}

void Image::SetHeight(const size_t& new_height) {
    pixels_ =
        std::vector<std::vector<RGB>>(pixels_.begin() + static_cast<uint32_t>(height_ - new_height), pixels_.end());
    height_ = new_height;
}

RGB Image::GetRgb(size_t x, size_t y) const {
    return pixels_[y][x];
}

void Image::ChangePixel(RGB rgb, size_t x, size_t y) {
    pixels_[y][x] = rgb;
}

int Image::GetXPixels() const {
    return x_pixels_per_m_;
}
int Image::GetYPixels() const {
    return y_pixels_per_m_;
}
