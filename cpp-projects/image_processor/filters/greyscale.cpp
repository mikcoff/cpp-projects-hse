#include "greyscale.h"

const float RED = 0.299;
const float GREEN = 0.587;
const float BLUE = 0.114;

RGB Greyscale::Transform(const RGB& pixel) {
    float res = static_cast<float>(RED * pixel.r_ + GREEN * pixel.g_ + BLUE * pixel.b_);
    return RGB(res, res, res);
}
