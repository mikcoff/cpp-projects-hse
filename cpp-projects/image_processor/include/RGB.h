#pragma once

struct RGB {
    float r_ = 0.0f;
    float g_ = 0.0f;
    float b_ = 0.0f;
    RGB();
    RGB(float r, float g, float b);
    void Set();
};

RGB operator+(const RGB& lhs, const RGB& rhs);
RGB operator-(const RGB& lhs, const RGB& rhs);
RGB operator*(const RGB& lhs, const RGB& rhs);
RGB operator*(const RGB& lhs, float rhs);
RGB operator*(float lhs, const RGB& rhs);
RGB operator/(const RGB& lhs, float rhs);
