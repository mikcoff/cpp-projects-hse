#pragma once

#include "abstract_filter_factory.h"
#include "convolution.h"

class Blur : public Convolution {
public:
    class Factory : public AbstractFilterFactory {
    public:
        std::unique_ptr<Filter> Construct(const std::vector<std::string>& parameters) override {
            return std::make_unique<Blur>(std::stof(parameters[0]));
        }
    };

    explicit Blur(float sigma);

private:
    Matrix CreateHKernel(float sigma);
    Matrix CreateVKernel(float sigma);
};
