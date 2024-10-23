#pragma once

#include "abstract_filter_factory.h"
#include "convolution.h"

class Sharpening : public Convolution {
public:
    class Factory : public AbstractFilterFactory {
    public:
        std::unique_ptr<Filter> Construct(const std::vector<std::string>&) override {
            return std::make_unique<Sharpening>();
        }
    };

    Sharpening();
};
