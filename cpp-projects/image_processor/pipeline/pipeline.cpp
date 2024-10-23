#include "pipeline.h"

void Pipeline::AddFilter(std::unique_ptr<Filter>&& filter) {
    filters_.push_back(std::move(filter));
}

void Pipeline::Process(Image& image) {
    for (const auto& filter : filters_) {
        filter->Apply(image);
    }
}

std::unique_ptr<Filter> Pipeline::Factory::CreateFilter(std::string name, const std::vector<std::string>& parameters) {
    return factories_.find(name)->second(parameters);
}
