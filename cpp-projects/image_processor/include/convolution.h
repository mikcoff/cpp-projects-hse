#pragma once

#include "filter.h"
#include "matrix.h"

class Convolution : public Filter {
public:
    explicit Convolution(std::vector<Matrix> matrixes);
    void Apply(Image& image) override;

private:
    std::vector<Matrix> matrixes_;
};
