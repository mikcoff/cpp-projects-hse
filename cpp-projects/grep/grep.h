#pragma once

#include <algorithm>
#include "utf8.h"
#include <cstddef>
#include <exception>
#include <filesystem>
#include <functional>
#include <optional>
#include <fstream>
#include <stdexcept>
#include <string>

using std::optional;


struct GrepOptions {
    optional<size_t> look_ahead_length;
    size_t max_matches_per_line;

    GrepOptions() {
        max_matches_per_line = 10;
    }

    GrepOptions(size_t look_ahead_length) : GrepOptions() {
        this->look_ahead_length = look_ahead_length;
    }

    GrepOptions(optional<size_t> look_ahead_length, size_t max_matches_per_line) {
        this->look_ahead_length = look_ahead_length;
        this->max_matches_per_line = max_matches_per_line;
    }
};

template <class Visitor>
void Grep(const std::string& path, const std::string& pattern, Visitor visitor,
          const GrepOptions& options) {
    try {
        if (std::filesystem::is_regular_file(path) && !std::filesystem::is_symlink(path)) {
            std::ifstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file " + path);
            }
            std::string line;
            size_t line_number = 1;
            while (std::getline(file, line)) {
                if (utf8::find_invalid(line.begin(), line.end()) != line.end()) {
                    throw std::runtime_error("File " + path + " encoded not in UTF8");
                }
                size_t matches_on_line = 0;
                for (auto it =
                         std::search(line.begin(), line.end(),
                                     std::boyer_moore_searcher(pattern.begin(), pattern.end()));
                     it != line.end();
                     it = std::search(it + 1, line.end(),
                                      std::boyer_moore_searcher(pattern.begin(), pattern.end()))) {
                    ++matches_on_line;
                    if (matches_on_line > options.max_matches_per_line) {
                        break;
                    }
                    if (options.look_ahead_length == std::nullopt) {
                        visitor.OnMatch(path, line_number, utf8::distance(line.begin(), it) + 1,
                                        std::nullopt);
                    } else {
                        auto iter = it + pattern.size();
                        try {
                            for (size_t i = 0; i < *options.look_ahead_length; ++i) {
                                utf8::next(iter, line.end());
                            }
                        } catch (const utf8::not_enough_room&) {
                            iter = line.end();
                        }
                        visitor.OnMatch(path, line_number, utf8::distance(line.begin(), it) + 1,
                                        std::string(it + pattern.size(), iter));
                    }
                }
                ++line_number;
            }
        } else {
            for (const auto& fl : std::filesystem::recursive_directory_iterator(path)) {
                if (std::filesystem::is_regular_file(fl) && !std::filesystem::is_symlink(fl)) {
                    Grep(fl.path(), pattern, visitor, options);
                }
            }
        }
    } catch (const std::exception& e) {
        visitor.OnError(e.what());
    }
}