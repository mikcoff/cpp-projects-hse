#pragma once

#include "abstract_filter_factory.h"
#include "point_filter.h"

class Negative : public PointFilter {
public:
    class Factory : public AbstractFilterFactory {
    public:
        std::unique_ptr<Filter> Construct(const std::vector<std::string>&) override {
            return std::make_unique<Negative>();
        }
    };

    RGB Transform(const RGB& pixel) override;
};
