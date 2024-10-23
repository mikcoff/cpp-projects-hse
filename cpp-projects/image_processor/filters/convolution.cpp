#include "convolution.h"

Convolution::Convolution(std::vector<Matrix> matrixes) : matrixes_(matrixes) {
}

void Convolution::Apply(Image& image) {
    for (const auto& matrix : matrixes_) {
        Image res(image.Width(), image.Height());
        int width = static_cast<int>(matrix.GetWidth()) / 2;
        int height = static_cast<int>(matrix.GetHeight()) / 2;
        for (size_t i = 0; i < image.Width(); ++i) {
            for (size_t j = 0; j < image.Height(); ++j) {
                RGB curr(0, 0, 0);
                for (int x = -width; x < width + 1; ++x) {
                    for (int y = -height; y < height + 1; ++y) {
                        size_t ind_x = 0;
                        size_t ind_y = 0;
                        if (static_cast<int>(i) + x > 0) {
                            if (static_cast<int>(i) + x > static_cast<int>(image.Width() - 1)) {
                                ind_x = image.Width() - 1;
                            } else {
                                ind_x = static_cast<size_t>(static_cast<int>(i) + x);
                            }
                        }
                        if (static_cast<int>(j) + y > 0) {
                            if (static_cast<int>(j) + y > static_cast<int>(image.Height() - 1)) {
                                ind_y = image.Height() - 1;
                            } else {
                                ind_y = static_cast<size_t>(static_cast<int>(j) + y);
                            }
                        }
                        curr = curr + matrix(width + x, height + y) * image.GetRgb(ind_x, ind_y);
                    }
                }
                curr.Set();
                res.ChangePixel(curr, i, j);
            }
        }
        image = res;
    }
}
