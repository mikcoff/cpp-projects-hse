#include "style_checker.h"

#include <algorithm>

constexpr bool IsAsciiLower(const char c) noexcept {
    return c >= 'a' && c <= 'z';
}

constexpr bool IsAsciiUpper(const char c) noexcept {
    return c >= 'A' && c <= 'Z';
}

constexpr bool IsAsciiDigit(const char c) noexcept {
    return c >= '0' && c <= '9';
}

Entity NameStyleChecker::EffectiveEntityForStyle(const Entity entity, const bool is_const_like) {
    if (!is_const_like) {
        return entity;
    }
    if (entity == Entity::kVariable || entity == Entity::kField) {
        return Entity::kConst;
    }
    return entity;
}

bool NameStyleChecker::IsNameOk(const Entity entity, const std::string_view id,
                                const bool is_const_like) const {
    if (id.empty() || HasDigit(id)) {
        return false;
    }

    const Entity effective_entity = EffectiveEntityForStyle(entity, is_const_like);
    if (!IsBaseStyleOk(effective_entity, id)) {
        return false;
    }

    if (effective_entity == Entity::kVariable || effective_entity == Entity::kField) {
        return true;
    }

    return UppercaseChunksValidForNonVariable(id);
}

std::vector<std::string> NameStyleChecker::SplitIntoWords(std::string_view name) const {
    std::vector<std::string> words;
    words.reserve(4);

    const size_t n = name.size();
    size_t pos = 0;

    while (pos < n) {
        const size_t next = name.find('_', pos);
        const size_t end = next == std::string_view::npos ? n : next;

        std::string_view segment = name.substr(pos, end - pos);
        if (!segment.empty()) {
            SplitSegmentByCase(segment, words);
        }

        pos = end + 1;
    }

    return words;
}

bool NameStyleChecker::IsBaseStyleOk(const Entity entity, const std::string_view id) const {
    switch (entity) {
        case Entity::kVariable:
            return IsValidVariableName(id);
        case Entity::kField:
            return IsValidFieldName(id);
        case Entity::kType:
            return IsValidTypeName(id);
        case Entity::kConst:
            return IsValidConstName(id);
        case Entity::kFunction:
            return IsValidFunctionName(id);
    }
    return false;
}

bool NameStyleChecker::HasDigit(const std::string_view name) {
    return std::any_of(name.begin(), name.end(), IsAsciiDigit);
}

bool NameStyleChecker::UppercaseChunksValidForNonVariable(const std::string_view name) {
    const bool has_upper = std::any_of(name.begin(), name.end(), IsAsciiUpper);
    const bool has_lower = std::any_of(name.begin(), name.end(), IsAsciiLower);

    if (has_upper && !has_lower) {
        return false;
    }

    const size_t n = name.size();
    size_t i = 0;

    while (i < n) {
        if (!IsAsciiUpper(name[i])) {
            ++i;
            continue;
        }

        const size_t start = i;
        while (i < n && IsAsciiUpper(name[i])) {
            ++i;
        }
        const size_t end = i;
        const size_t run_len = end - start;
        if (run_len == 0) {
            continue;
        }

        const bool has_lower_after = (end < n) && IsAsciiLower(name[end]);

        const size_t chunk_len = has_lower_after ? (run_len - 1) : run_len;

        if (chunk_len > 0 && chunk_len < 3) {
            return false;
        }
    }

    return true;
}

bool NameStyleChecker::IsSnakeCase(const std::string_view name) {
    if (name.empty()) {
        return false;
    }

    if (name.front() == '_') {
        return false;
    }

    bool has_letter = false;
    bool prev_underscore = false;

    for (const auto& c : name) {
        if (c == '_') {
            if (prev_underscore) {
                return false;
            }
            prev_underscore = true;
            continue;
        }

        if (!IsAsciiLower(c)) {
            return false;
        }

        has_letter = true;
        prev_underscore = false;
    }

    if (prev_underscore) {
        return false;
    }

    return has_letter;
}

bool NameStyleChecker::IsPascalCase(const std::string_view name) {
    if (name.empty() || !IsAsciiUpper(name.front())) {
        return false;
    }
    for (size_t i = 1; i < name.size(); ++i) {
        const char c = name[i];
        if (!IsAsciiLower(c) && !IsAsciiUpper(c)) {
            return false;
        }
    }
    return true;
}

bool NameStyleChecker::IsLowerCamelCase(const std::string_view name) {
    if (name.empty()) {
        return false;
    }
    if (!IsAsciiLower(name.front())) {
        return false;
    }
    for (const auto& c : name) {
        if (c == '_') {
            return false;
        }
        if (!IsAsciiLower(c) && !IsAsciiUpper(c)) {
            return false;
        }
    }
    return true;
}

bool NameStyleChecker::IsValidVariableName(const std::string_view name) {
    return IsSnakeCase(name);
}

bool NameStyleChecker::IsValidFieldName(const std::string_view name) {
    if (name.size() < 2) {
        return false;
    }
    if (name.back() != '_') {
        return false;
    }
    const std::string_view base = name.substr(0, name.size() - 1);
    return IsSnakeCase(base);
}

bool NameStyleChecker::IsValidTypeName(const std::string_view name) {
    if (!IsPascalCase(name)) {
        return false;
    }
    for (char c : name) {
        if (c == '_') {
            return false;
        }
    }
    return true;
}

bool NameStyleChecker::IsValidConstName(const std::string_view name) {
    if (name.size() < 2) {
        return false;
    }
    if (name[0] != 'k') {
        return false;
    }
    if (!IsAsciiUpper(name[1])) {
        return false;
    }

    bool has_letter = false;
    bool prev_underscore = false;

    for (size_t i = 1; i < name.size(); ++i) {
        const char c = name[i];
        if (c == '_') {
            if (prev_underscore) {
                return false;
            }
            prev_underscore = true;
            continue;
        }
        if (!IsAsciiLower(c) && !IsAsciiUpper(c)) {
            return false;
        }
        has_letter = true;
        prev_underscore = false;
    }

    if (prev_underscore) {
        return false;
    }

    return has_letter;
}

bool NameStyleChecker::IsValidFunctionName(const std::string_view name) {
    return IsPascalCase(name);
}

void NameStyleChecker::SplitSegmentByCase(const std::string_view segment,
                                          std::vector<std::string>& words) const {
    const size_t seg_len = segment.size();
    if (seg_len == 0) {
        return;
    }

    size_t word_start = 0;
    for (size_t i = 1; i < seg_len; ++i) {
        const char prev = segment[i - 1];
        const char cur = segment[i];
        const char next_char = (i + 1 < seg_len) ? segment[i + 1] : '\0';

        const bool prev_lower = IsAsciiLower(prev);
        const bool cur_upper = IsAsciiUpper(cur);
        const bool prev_upper = IsAsciiUpper(prev);
        const bool next_lower = IsAsciiLower(next_char);

        if (prev_lower && cur_upper) {
            words.emplace_back(segment.substr(word_start, i - word_start));
            word_start = i;
            continue;
        }

        if (prev_upper && cur_upper && next_lower) {
            words.emplace_back(segment.substr(word_start, i - word_start));
            word_start = i;
        }
    }

    if (word_start < seg_len) {
        words.emplace_back(segment.substr(word_start, seg_len - word_start));
    }
}
