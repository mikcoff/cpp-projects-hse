#include "sharpening.h"

const float FIVE = 5.0f;

const Matrix SHARPENING_MATRIX({{0.0f, -1.0f, 0.0f}, {-1.0f, FIVE, -1.0f}, {0.0f, -1.0f, 0.0f}});
Sharpening::Sharpening() : Convolution({SHARPENING_MATRIX}) {
}
