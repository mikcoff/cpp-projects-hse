#pragma once
#include <vector>
#include "RGB.h"
#include <stddef.h>
#include "cstdint"

class Image {
private:
    size_t width_;
    size_t height_;
    int x_pixels_per_m_;
    int y_pixels_per_m_;
    std::vector<std::vector<RGB>> pixels_;

public:
    Image();
    Image(size_t width, size_t height);

    RGB GetRgb(size_t x, size_t y) const;
    void ChangePixel(RGB rgb, size_t x, size_t y);

    size_t Width() const;
    size_t Height() const;

    void SetWidth(const size_t& new_width);
    void SetHeight(const size_t& new_height);
    int GetXPixels() const;
    int GetYPixels() const;

    friend void SaveFile(const char* path, Image& image);

    void Read(const char* path);
};
