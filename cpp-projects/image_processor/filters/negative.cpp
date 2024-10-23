#include "negative.h"

RGB Negative::Transform(const RGB& pixel) {
    return RGB(1.0f - pixel.r_, 1.0f - pixel.g_, 1.0f - pixel.b_);
}
