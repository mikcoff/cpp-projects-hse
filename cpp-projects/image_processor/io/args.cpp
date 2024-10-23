#include "args.h"
#include <algorithm>
#include <exception>
#include <stdexcept>

Args::Filter::Filter(std::string_view name, std::vector<std::string>&& parameters) {
    name_ = name;
    parameters_ = parameters;
}

const std::string& Args::Filter::GetName() const {
    return name_;
}

const std::vector<std::string>& Args::Filter::GetParameters() const {
    return parameters_;
}

Args::Args() {
    input_file_ = "";
    output_file_ = "";
    filters_ = {};
}

bool Args::IsValidFilterName(std::string_view name) {
    return (name == "-crop" || name == "-gs" || name == "-neg" || name == "-sharp" || name == "-edge" ||
            name == "-blur" || name == "-glass");
}

Args::Args(int argc, const char* argv[]) {
    if (argc < 3) {
        throw std::length_error("No input/output file");
    }
    input_file_ = argv[1];
    output_file_ = argv[2];
    for (int i = 3; i < argc; ++i) {
        if (i > argc - 1) {
            throw std::invalid_argument("Not enough arguments");
        }
        if (!IsValidFilterName(argv[i])) {
            throw std::invalid_argument("Invalid filter name");
        }
        std::string_view name = argv[i];
        std::vector<std::string> parameters = {};
        if (name == "-crop") {
            for (size_t j = 0; j < 2; ++j) {
                ++i;
                if (i > argc - 1) {
                    throw std::invalid_argument("Not enough arguments");
                }
                std::string parameter = argv[i];
                if (parameter.find_first_not_of("0123456789") != parameter.npos) {
                    throw std::invalid_argument("Invalid crop width/height parameter");
                }
                parameters.push_back(parameter);
            }
        }
        if (name == "-edge") {
            ++i;
            if (i > argc - 1) {
                throw std::invalid_argument("Not enough arguments");
            }
            std::string parameter = argv[i];
            if (parameter.find_first_not_of("0123456789.") != parameter.npos ||
                std::count(parameter.begin(), parameter.end(), '.') > 1) {
                throw std::invalid_argument("Invalid edge threshold");
            }
            parameters.push_back(parameter);
        }
        if (name == "-blur") {
            ++i;
            if (i > argc - 1) {
                throw std::invalid_argument("Not enough arguments");
            }
            std::string parameter = argv[i];
            if (parameter.find_first_not_of("0123456789.") != parameter.npos ||
                std::count(parameter.begin(), parameter.end(), '.') > 1) {
                throw std::invalid_argument("Invalid blur sigma");
            }
            parameters.push_back(parameter);
        }
        filters_.push_back(Filter(name, std::move(parameters)));
    }
}

const std::string& Args::GetInputFile() const {
    return input_file_;
}

const std::string& Args::GetOutputFile() const {
    return output_file_;
}

const std::vector<Args::Filter>& Args::GetFilters() const {
    return filters_;
}
