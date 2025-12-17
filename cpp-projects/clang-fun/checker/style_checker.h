#pragma once

#include "../check_names.h"

#include <string>
#include <string_view>
#include <vector>

class NameStyleChecker {
public:
    NameStyleChecker() = default;

    static Entity EffectiveEntityForStyle(Entity entity, bool is_const_like);
    bool IsNameOk(Entity entity, std::string_view id, bool is_const_like) const;
    std::vector<std::string> SplitIntoWords(std::string_view name) const;

private:
    bool IsBaseStyleOk(Entity entity, std::string_view id) const;
    static bool HasDigit(std::string_view name);
    static bool UppercaseChunksValidForNonVariable(std::string_view name);
    static bool IsSnakeCase(std::string_view name);
    static bool IsPascalCase(std::string_view name);
    static bool IsLowerCamelCase(std::string_view name);
    static bool IsValidVariableName(std::string_view name);
    static bool IsValidFieldName(std::string_view name);
    static bool IsValidTypeName(std::string_view name);
    static bool IsValidConstName(std::string_view name);
    static bool IsValidFunctionName(std::string_view name);
    void SplitSegmentByCase(std::string_view segment, std::vector<std::string>& words) const;
};
