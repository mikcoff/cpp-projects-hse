#pragma once

#include "abstract_filter_factory.h"
#include "filter.h"

class Crop : public Filter {
public:
    class Factory : public AbstractFilterFactory {
    public:
        std::unique_ptr<Filter> Construct(const std::vector<std::string>& parameters) override {
            return std::make_unique<Crop>(std::stoul(parameters[0]), std::stoul(parameters[1]));
        }
    };

    explicit Crop(size_t width, size_t height);

    void Apply(Image& image) override;

private:
    size_t width_;
    size_t height_;
};
