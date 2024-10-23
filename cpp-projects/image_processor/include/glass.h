#pragma once

#include "abstract_filter_factory.h"
#include <cmath>

class Glass : public Filter {
public:
    class Factory : public AbstractFilterFactory {
    public:
        std::unique_ptr<Filter> Construct(const std::vector<std::string>&) override {
            return std::make_unique<Glass>();
        }
    };

    void Apply(Image& image) override;
};
