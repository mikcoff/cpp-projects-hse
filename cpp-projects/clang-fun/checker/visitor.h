#pragma once

#include "../check_names.h"
#include "style_checker.h"
#include "dict.h"

#include <memory>
#include <string_view>
#include <functional>

namespace clang {
class ASTContext;
class SourceManager;
class SourceLocation;
namespace tooling {
class FrontendActionFactory;
}
}  // namespace clang

struct Location {
    std::string file_path;
    unsigned line = 0;
};

struct NameOccurrence {
    Entity entity;
    std::string name;
    Location loc;
    bool is_const_like = false;
    bool is_ok = true;
};

Location MakeLocation(const clang::SourceManager& source_manager, clang::SourceLocation location);

bool IsOwnSourceFile(const clang::SourceManager& source_manager, clang::SourceLocation location);

std::string_view StripTemplateArguments(std::string_view name);

using ReportCallback = std::function<void(const std::string& file, const Statistics& stats)>;

std::unique_ptr<clang::tooling::FrontendActionFactory> CreateFactory(
    const NameStyleChecker& style_checker, const Dictionary* dictionary,
    ReportCallback callback = {});
