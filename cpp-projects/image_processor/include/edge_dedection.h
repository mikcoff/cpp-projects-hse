#pragma once

#include "abstract_filter_factory.h"
#include "convolution.h"
#include "greyscale.h"

class EdgeDetection : public Filter {
public:
    class Factory : public AbstractFilterFactory {
    public:
        std::unique_ptr<Filter> Construct(const std::vector<std::string>& parameters) override {
            return std::make_unique<EdgeDetection>(std::stof(parameters[0]));
        }
    };

    explicit EdgeDetection(float threshold);
    void Apply(Image& image) override;

private:
    float threshold_;
    Greyscale greyscale_;
    Convolution edge_detector_;
};
