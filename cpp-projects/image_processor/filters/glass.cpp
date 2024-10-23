#include "glass.h"

std::pair<float, float> Random(int ix, int iy) {
    const unsigned i = sizeof(unsigned) * sizeof(unsigned) * 2;
    const unsigned j = sizeof(unsigned) * sizeof(unsigned);
    unsigned a = ix;
    unsigned b = iy;

    a *= 3284157443;  // NOLINT

    b ^= a << j | a >> (i - j);
    b *= 1911520717;  // NOLINT

    a ^= b << j | b >> (i - j);
    a *= 2048419325;  // NOLINT
    float rand = static_cast<float>(a * (M_PI / ~(~0u >> 1)));
    return std::pair<float, float>(std::sin(rand), std::cos(rand));
}

float Gradient(int ix, int iy, float x, float y) {
    std::pair<float, float> grad = Random(ix, iy);

    float dx = x - static_cast<float>(ix);
    float dy = y - static_cast<float>(iy);

    return dx * grad.first + dy * grad.second;
}

float Interpolate(float x, float y, float w) {
    return (y - x) * (3.0f - 2.0f * w) * w * w + x;  // NOLINT
}

float Perlin(float x, float y) {
    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float sx = x - static_cast<float>(x0);
    float sy = y - static_cast<float>(y0);

    float n0 = Gradient(x0, y0, x, y);
    float n1 = Gradient(x1, y0, x, y);
    float ix0 = Interpolate(n0, n1, sx);

    n0 = Gradient(x0, y1, x, y);
    n1 = Gradient(x1, y1, x, y);
    float ix1 = Interpolate(n0, n1, sx);

    float value = Interpolate(ix0, ix1, sy);
    return value;
}

void Glass::Apply(Image& image) {
    Image noise(image.Width() + 1, image.Height() + 1);
    Image res(image.Width(), image.Height());

    for (size_t i = 0; i < image.Width() + 1; ++i) {
        for (size_t j = 0; j < image.Height() + 1; ++j) {
            float val = 0;
            float freq = 1;
            float amp = 1;
            for (size_t k = 0; k < 5; ++k) {                          // NOLINT
                val += Perlin(i * freq / 400, j * freq / 400) * amp;  // NOLINT

                freq *= 2;
                amp /= 2;
            }
            val *= 1.2f;  // NOLINT

            if (val < -1.0f) {
                val = -1.0f;
            }
            if (val > 1.0f) {
                val = 1.0f;
            }
            val = (val + 1.0f) / 2;
            noise.ChangePixel(RGB(val, val, val), i, j);
        }
    }

    for (size_t i = 0; i < image.Width(); ++i) {
        for (size_t j = 0; j < image.Height(); ++j) {
            float p0 = noise.GetRgb(i, j).r_;
            float p1 = noise.GetRgb(i + 1, j).r_;
            float p2 = noise.GetRgb(i, j + 1).r_;
            int dx = static_cast<int>(floorf(p1 - p0) * 20.0f + 5.0f);  // NOLINT
            int dy = static_cast<int>(floorf(p2 - p0) * 20.0f + 5.0f);  // NOLINT
            size_t ind_x = std::min(image.Width() - 1, std::max(static_cast<size_t>(static_cast<int>(i) + dx), 0UL));
            size_t ind_y = std::min(image.Height() - 1, std::max(static_cast<size_t>(static_cast<int>(j) + dy), 0UL));
            res.ChangePixel(image.GetRgb(ind_x, ind_y), i, j);
        }
    }
    image = res;
}
 