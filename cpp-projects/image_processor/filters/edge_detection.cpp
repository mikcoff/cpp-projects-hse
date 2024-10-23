#include "edge_dedection.h"

const float FOUR = 4.0f;
const Matrix EDGE_DETECTION_MATRIX({{0.0f, -1.0f, 0.0f}, {-1.0f, FOUR, -1.0f}, {0.0f, -1.0f, 0.0f}});
EdgeDetection::EdgeDetection(float threshold) : threshold_(threshold), edge_detector_({EDGE_DETECTION_MATRIX}) {
}

void EdgeDetection::Apply(Image& image) {
    greyscale_.Apply(image);
    edge_detector_.Apply(image);
    for (size_t i = 0; i < image.Width(); ++i) {
        for (size_t j = 0; j < image.Height(); ++j) {
            if (image.GetRgb(i, j).r_ > threshold_) {
                image.ChangePixel({1.0f, 1.0f, 1.0f}, i, j);
            } else {
                image.ChangePixel({0.0f, 0.0f, 0.0f}, i, j);
            }
        }
    }
}
