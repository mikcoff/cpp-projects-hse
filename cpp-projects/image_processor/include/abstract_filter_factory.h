
#pragma once

#include "filter.h"

class AbstractFilterFactory {
public:
    AbstractFilterFactory() = default;
    virtual ~AbstractFilterFactory() = default;

    std::unique_ptr<Filter> operator()(const std::vector<std::string>& parameters) {
        return Construct(parameters);
    }

    virtual std::unique_ptr<Filter> Construct(const std::vector<std::string>& parametrs) = 0;
};