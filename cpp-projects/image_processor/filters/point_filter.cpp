#include "point_filter.h"

void PointFilter::Apply(Image& image) {
    for (size_t i = 0; i < image.Width(); ++i) {
        for (size_t j = 0; j < image.Height(); ++j) {
            image.ChangePixel(Transform(image.GetRgb(i, j)), i, j);
        }
    }
}
