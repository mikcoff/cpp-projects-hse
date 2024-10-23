#include "RGB.h"

RGB::RGB() : r_(0), g_(0), b_(0) {
}

RGB::RGB(float r, float g, float b) : r_(r), g_(g), b_(b) {
}

void RGB::Set() {
    if (r_ < 0.0f) {
        r_ = 0.0f;
    }
    if (r_ > 1.0f) {
        r_ = 1.0f;
    }
    if (g_ < 0.0f) {
        g_ = 0.0f;
    }
    if (g_ > 1.0f) {
        g_ = 1.0f;
    }
    if (b_ < 0.0f) {
        b_ = 0.0f;
    }
    if (b_ > 1.0f) {
        b_ = 1.0f;
    }
}

RGB operator+(const RGB& lhs, const RGB& rhs) {
    return RGB(lhs.r_ + rhs.r_, lhs.g_ + rhs.g_, lhs.b_ + rhs.b_);
}

RGB operator-(const RGB& lhs, const RGB& rhs) {
    return RGB(lhs.r_ - rhs.r_, lhs.g_ - rhs.g_, lhs.b_ - rhs.b_);
}

RGB operator*(const RGB& lhs, const RGB& rhs) {
    return RGB(lhs.r_ * rhs.r_, lhs.g_ * rhs.g_, lhs.b_ * rhs.b_);
}

RGB operator*(const RGB& lhs, float rhs) {
    return RGB(rhs * lhs.r_, rhs * lhs.g_, rhs * lhs.b_);
}

RGB operator*(float lhs, const RGB& rhs) {
    return rhs * lhs;
}

RGB operator/(const RGB& lhs, float rhs) {
    return lhs * (1.0f / rhs);
}
