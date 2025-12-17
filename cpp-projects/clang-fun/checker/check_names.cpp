#include "dict.h"
#include "style_checker.h"
#include "visitor.h"

#include <unordered_map>

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

std::unordered_map<std::string, Statistics> CheckNames(int argc, const char* argv[]) {
    std::unordered_map<std::string, Statistics> results;

    llvm::cl::OptionCategory options_category("check-names options");

    llvm::cl::opt<std::string> dict_path("dict", llvm::cl::desc("Path to dictionary file"),
                                         llvm::cl::value_desc("filename"), llvm::cl::init(""),
                                         llvm::cl::cat(options_category));

    auto parser = clang::tooling::CommonOptionsParser::create(argc, argv, options_category);
    if (!parser) {
        llvm::consumeError(parser.takeError());
        return results;
    }

    auto& options_parser = parser.get();
    clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                   options_parser.getSourcePathList());

    constexpr NameStyleChecker kStyleChecker;
    Dictionary dictionary;

    auto callback = [&](const std::string& file, const Statistics& stats) {
        results[file] = stats;
    };

    auto factory = CreateFactory(kStyleChecker, &dictionary, callback);
    tool.run(factory.get());

    return results;
}
