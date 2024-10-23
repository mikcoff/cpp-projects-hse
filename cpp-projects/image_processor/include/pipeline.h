#pragma once

#include "abstract_filter_factory.h"
#include <functional>
#include <unordered_map>
#include <string_view>

class Pipeline {
public:
    class Factory {
    private:
        std::unordered_map<std::string, std::function<std::unique_ptr<Filter>(const std::vector<std::string>&)>>
            factories_;

    public:
        template <FilterFactory F>
        void RegisterFilterFactory(std::string_view filter_name) {
            factories_.emplace(filter_name, [factory = F{}](const std::vector<std::string>& parameters) mutable {
                return factory(parameters);
            });
        }

        std::unique_ptr<Filter> CreateFilter(std::string name, const std::vector<std::string>& parameters);
    };

    void AddFilter(std::unique_ptr<Filter>&& filter);
    void Process(Image& image);

private:
    std::vector<std::unique_ptr<Filter>> filters_;
};
