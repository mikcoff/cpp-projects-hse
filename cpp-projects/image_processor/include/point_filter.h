#pragma once

#include "filter.h"

class PointFilter : public Filter {
public:
    void Apply(Image& image) override;
    virtual RGB Transform(const RGB& pixel) = 0;
};
