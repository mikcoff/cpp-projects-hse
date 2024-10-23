#include "crop.h"

Crop::Crop(size_t width, size_t height) : width_(width), height_(height){};

void Crop::Apply(Image& image) {
    image.SetWidth(std::min(width_, image.Width()));
    image.SetHeight(std::min(height_, image.Height()));
}
