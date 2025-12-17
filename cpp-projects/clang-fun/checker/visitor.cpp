#include "visitor.h"

#include <algorithm>
#include <set>
#include <tuple>
#include <utility>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Path.h"

using clang::ast_matchers::MatchFinder;

static void NormalizeStatistics(Statistics& stats) {
    {
        using KeyT = std::tuple<Entity, std::string, std::string, size_t>;
        std::set<KeyT> seen;

        std::unordered_map<std::string, size_t> file_order;
        size_t next_file_index = 0;
        file_order.reserve(8);

        for (const auto& bad : stats.bad_names) {
            auto [it, inserted] = file_order.emplace(bad.file, next_file_index);
            if (inserted) {
                ++next_file_index;
            }
        }

        std::vector<BadName> unique;
        unique.reserve(stats.bad_names.size());
        for (const auto& bad : stats.bad_names) {
            KeyT k{bad.entity, bad.name, bad.file, bad.line};
            if (seen.insert(std::move(k)).second) {
                unique.push_back(bad);
            }
        }

        std::stable_sort(unique.begin(), unique.end(), [&](const BadName& lhs, const BadName& rhs) {
            const size_t li = file_order.at(lhs.file);
            const size_t ri = file_order.at(rhs.file);
            return std::tie(li, lhs.line) < std::tie(ri, rhs.line);
        });

        stats.bad_names = std::move(unique);
    }

    {
        using KeyT = std::tuple<std::string, std::string, std::string, std::string, size_t>;
        std::set<KeyT> seen;

        std::unordered_map<std::string, size_t> file_order;
        size_t next_file_index = 0;
        file_order.reserve(8);

        for (const auto& mist : stats.mistakes) {
            auto [it, inserted] = file_order.emplace(mist.file, next_file_index);
            if (inserted) {
                ++next_file_index;
            }
        }

        std::vector<Mistake> unique;
        unique.reserve(stats.mistakes.size());
        for (const auto& mist : stats.mistakes) {
            KeyT k{mist.name, mist.wrong_word, mist.ok_word, mist.file, mist.line};
            if (seen.insert(std::move(k)).second) {
                unique.push_back(mist);
            }
        }

        std::stable_sort(unique.begin(), unique.end(), [&](const Mistake& lhs, const Mistake& rhs) {
            const size_t li = file_order.at(lhs.file);
            const size_t ri = file_order.at(rhs.file);
            return std::tie(li, lhs.line) < std::tie(ri, rhs.line);
        });

        stats.mistakes = std::move(unique);
    }
}

Location MakeLocation(const clang::SourceManager& source_manager, clang::SourceLocation location) {
    clang::SourceLocation loc = source_manager.getExpansionLoc(location);
    Location result;
    const auto full_path = source_manager.getFilename(loc);
    result.file_path = llvm::sys::path::filename(full_path).str();
    result.line = source_manager.getSpellingLineNumber(loc);
    return result;
}

bool IsOwnSourceFile(const clang::SourceManager& source_manager, clang::SourceLocation location) {
    const clang::SourceLocation loc = source_manager.getSpellingLoc(location);
    const auto filename = source_manager.getFilename(loc);
    return filename.ends_with("check_names.cpp");
}

std::string_view StripTemplateArguments(std::string_view name) {
    const size_t angle_pos = name.find('<');
    if (angle_pos != std::string_view::npos) {
        return name.substr(0, angle_pos);
    }
    return name;
}

class NameMatchCallback : public MatchFinder::MatchCallback {
public:
    NameMatchCallback(Statistics& report, const NameStyleChecker& style_checker,
                      const Dictionary* dictionary)
        : report_(report), style_checker_(style_checker), dictionary_(dictionary) {
    }

    void run(const MatchFinder::MatchResult& result) override {
        const auto* source_manager = result.SourceManager;
        if (!source_manager) {
            return;
        }

        if (const auto* binding = result.Nodes.getNodeAs<clang::BindingDecl>("binding")) {
            HandleBinding(binding, *source_manager);
        }

        if (const auto* var = result.Nodes.getNodeAs<clang::VarDecl>("var")) {
            HandleVariable(var, *source_manager);
        }

        if (const auto* field = result.Nodes.getNodeAs<clang::FieldDecl>("field")) {
            HandleField(field, *source_manager);
        }

        if (const auto* func = result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
            HandleFunction(func, *source_manager);
        }

        if (const auto* rec = result.Nodes.getNodeAs<clang::CXXRecordDecl>("type")) {
            HandleType(rec, *source_manager);
        }

        if (const auto* en = result.Nodes.getNodeAs<clang::EnumDecl>("enum")) {
            HandleEnum(en, *source_manager);
        }

        if (const auto* typedef_decl = result.Nodes.getNodeAs<clang::TypedefDecl>("typedef")) {
            HandleTypedef(typedef_decl, *source_manager);
        }

        if (const auto* alias_decl = result.Nodes.getNodeAs<clang::TypeAliasDecl>("typeAlias")) {
            HandleTypeAlias(alias_decl, *source_manager);
        }
    }

private:
    template <typename DeclT>
    bool ShouldSkipNamedDecl(const DeclT* decl, const clang::SourceManager& source_manager,
                             clang::SourceLocation loc) const {
        if (!decl || decl->isImplicit() || decl->getNameAsString().empty()) {
            return true;
        }
        if (IsOwnSourceFile(source_manager, loc)) {
            return true;
        }

        const clang::SourceLocation expansion_loc = source_manager.getExpansionLoc(loc);
        if (source_manager.isInSystemHeader(expansion_loc) ||
            source_manager.isInExternCSystemHeader(expansion_loc)) {
            return true;
        }

        if (loc.isMacroID()) {
            const clang::SourceLocation spelling_loc = source_manager.getSpellingLoc(loc);
            if (source_manager.isInSystemHeader(spelling_loc) ||
                source_manager.isInExternCSystemHeader(spelling_loc)) {
                return true;
            }
        }

        const auto filename = source_manager.getFilename(expansion_loc);
        if (filename.empty()) {
            return true;
        }
        return false;
    }

    static std::pair<Entity, Entity> AdjustForStructOrUnion(Entity entity, Entity style_entity,
                                                            const clang::RecordDecl* parent) {
        if (parent && (parent->isStruct() || parent->isUnion())) {
            entity = Entity::kVariable;
            style_entity = Entity::kVariable;
        }
        return {entity, style_entity};
    }

    void HandleBinding(const clang::BindingDecl* decl, const clang::SourceManager& source_manager) {
        const clang::SourceLocation loc = decl->getLocation();
        if (ShouldSkipNamedDecl(decl, source_manager, loc)) {
            return;
        }

        const bool is_const_like = decl->getType().isConstQualified();

        RecordName(Entity::kVariable, decl->getNameAsString(), is_const_like, source_manager, loc,
                   Entity::kVariable);
    }

    void HandleVariable(const clang::VarDecl* decl, const clang::SourceManager& source_manager) {
        const clang::SourceLocation loc = decl->getLocation();
        if (ShouldSkipNamedDecl(decl, source_manager, loc)) {
            return;
        }

        const bool is_const_like = decl->getType().isConstQualified() || decl->isConstexpr();

        const std::string var_name = decl->getNameAsString();

        auto entity = Entity::kVariable;
        auto style_entity = Entity::kVariable;

        if (decl->isStaticDataMember()) {
            entity = Entity::kField;
            style_entity = Entity::kField;

            if (const auto* parent_rec =
                    llvm::dyn_cast<clang::RecordDecl>(decl->getDeclContext())) {
                std::tie(entity, style_entity) =
                    AdjustForStructOrUnion(entity, style_entity, parent_rec);
            }
        }

        RecordName(entity, var_name, is_const_like, source_manager, loc, style_entity);
    }

    void HandleField(const clang::FieldDecl* decl, const clang::SourceManager& source_manager) {
        const clang::SourceLocation loc = decl->getLocation();
        if (ShouldSkipNamedDecl(decl, source_manager, loc)) {
            return;
        }

        const clang::RecordDecl* parent = decl->getParent();
        if (!parent) {
            return;
        }

        const bool is_const_like = decl->getType().isConstQualified();

        auto entity = Entity::kField;
        auto style_entity = Entity::kField;

        std::tie(entity, style_entity) = AdjustForStructOrUnion(entity, style_entity, parent);

        RecordName(entity, decl->getNameAsString(), is_const_like, source_manager, loc,
                   style_entity);
    }

    void HandleFunction(const clang::FunctionDecl* decl,
                        const clang::SourceManager& source_manager) {
        if (!decl || decl->isImplicit()) {
            return;
        }
        if (decl->isMain()) {
            return;
        }
        if (decl->isOverloadedOperator()) {
            return;
        }

        const clang::SourceLocation loc = source_manager.getSpellingLoc(decl->getLocation());

        if (IsOwnSourceFile(source_manager, loc)) {
            return;
        }
        const clang::SourceLocation expansion_loc = source_manager.getExpansionLoc(loc);
        if (source_manager.isInSystemHeader(expansion_loc) ||
            source_manager.isInExternCSystemHeader(expansion_loc)) {
            return;
        }
        if (loc.isMacroID()) {
            const clang::SourceLocation spelling_loc = source_manager.getSpellingLoc(loc);
            if (source_manager.isInSystemHeader(spelling_loc) ||
                source_manager.isInExternCSystemHeader(spelling_loc)) {
                return;
            }
        }
        if (source_manager.getFilename(expansion_loc).empty()) {
            return;
        }

        if (const auto* dtor = llvm::dyn_cast<clang::CXXDestructorDecl>(decl)) {
            const auto* record = dtor->getParent();
            if (!record || !record->getIdentifier()) {
                return;
            }

            const std::string type_name = record->getNameAsString();
            if (type_name.empty()) {
                return;
            }

            NameOccurrence occurrence;
            occurrence.entity = style_checker_.EffectiveEntityForStyle(Entity::kFunction, false);
            occurrence.name = ("~" + type_name);
            occurrence.loc = MakeLocation(source_manager, loc);
            occurrence.is_const_like = false;

            occurrence.is_ok = style_checker_.IsNameOk(Entity::kType, type_name, false);

            if (!occurrence.is_ok) {
                report_.bad_names.push_back(BadName{occurrence.loc.file_path, occurrence.name,
                                                    occurrence.entity, occurrence.loc.line});
            }
            RecordTypoEvents(type_name, occurrence);
            return;
        }

        std::string func_name;

        if (const auto* ctor = llvm::dyn_cast<clang::CXXConstructorDecl>(decl)) {
            const auto* record = ctor->getParent();
            if (!record || !record->getIdentifier()) {
                return;
            }
            func_name = record->getNameAsString();
        } else {
            if (!decl->getIdentifier()) {
                return;
            }
            func_name = decl->getNameAsString();
        }

        if (func_name.empty()) {
            return;
        }

        RecordName(Entity::kFunction, func_name, false, source_manager, loc, Entity::kFunction);
    }

    void HandleType(const clang::CXXRecordDecl* decl, const clang::SourceManager& source_manager) {
        const clang::SourceLocation loc = decl->getLocation();
        if (ShouldSkipNamedDecl(decl, source_manager, loc)) {
            return;
        }

        RecordName(Entity::kType, decl->getNameAsString(), false, source_manager, loc,
                   Entity::kType);
    }

    void HandleEnum(const clang::EnumDecl* decl, const clang::SourceManager& source_manager) {
        if (!decl || decl->isImplicit() || !decl->getIdentifier()) {
            return;
        }
        if (!decl->isThisDeclarationADefinition()) {
            return;
        }

        const clang::SourceLocation loc = decl->getLocation();
        if (IsOwnSourceFile(source_manager, loc)) {
            return;
        }
        const clang::SourceLocation expansion_loc = source_manager.getExpansionLoc(loc);
        if (source_manager.isInSystemHeader(expansion_loc) ||
            source_manager.isInExternCSystemHeader(expansion_loc)) {
            return;
        }
        if (loc.isMacroID()) {
            const clang::SourceLocation spelling_loc = source_manager.getSpellingLoc(loc);
            if (source_manager.isInSystemHeader(spelling_loc) ||
                source_manager.isInExternCSystemHeader(spelling_loc)) {
                return;
            }
        }
        if (source_manager.getFilename(expansion_loc).empty()) {
            return;
        }

        RecordName(Entity::kType, decl->getNameAsString(), false, source_manager, loc,
                   Entity::kType);
    }

    void HandleTypedef(const clang::TypedefDecl* decl, const clang::SourceManager& source_manager) {
        const clang::SourceLocation loc = decl->getLocation();
        if (ShouldSkipNamedDecl(decl, source_manager, loc)) {
            return;
        }

        RecordName(Entity::kType, decl->getNameAsString(), false, source_manager, loc,
                   Entity::kType);
    }

    void HandleTypeAlias(const clang::TypeAliasDecl* decl,
                         const clang::SourceManager& source_manager) {
        const clang::SourceLocation loc = decl->getLocation();
        if (ShouldSkipNamedDecl(decl, source_manager, loc)) {
            return;
        }

        RecordName(Entity::kType, decl->getNameAsString(), false, source_manager, loc,
                   Entity::kType);
    }

    NameOccurrence BuildNameOccurrence(const Entity entity, const std::string_view clean_name,
                                       const bool is_const_like,
                                       const clang::SourceManager& source_manager,
                                       clang::SourceLocation location,
                                       const Entity style_entity) const {
        NameOccurrence occurrence;

        occurrence.entity = style_checker_.EffectiveEntityForStyle(entity, is_const_like);
        occurrence.name.assign(clean_name.begin(), clean_name.end());
        occurrence.loc = MakeLocation(source_manager, location);
        occurrence.is_const_like = is_const_like;

        const Entity style_entity_effective =
            style_checker_.EffectiveEntityForStyle(style_entity, is_const_like);

        occurrence.is_ok =
            style_checker_.IsNameOk(style_entity_effective, clean_name, is_const_like);

        return occurrence;
    }

    void RecordTypoEvents(std::string_view clean_name, const NameOccurrence& occurrence) {
        if (!dictionary_ || !dictionary_->IsLoaded()) {
            return;
        }

        auto words = style_checker_.SplitIntoWords(clean_name);

        const auto is_all_upper_ascii = [](const std::string_view s) {
            if (s.empty()) {
                return false;
            }
            for (const unsigned char ch : s) {
                if (ch < 'A' || ch > 'Z') {
                    return false;
                }
            }
            return true;
        };

        const auto has_digit = [](const std::string_view s) {
            for (const unsigned char ch : s) {
                if (ch >= '0' && ch <= '9') {
                    return true;
                }
            }
            return false;
        };

        for (size_t i = 0; i < words.size(); ++i) {
            const auto& word_str = words[i];
            const std::string_view word = word_str;

            if (word.size() <= 3) {
                continue;
            }

            if (has_digit(word)) {
                continue;
            }

            if (i > 0) {
                const std::string_view prev = words[i - 1];
                if (prev.size() == 2 && is_all_upper_ascii(prev)) {
                    continue;
                }
            }

            int dist = -1;
            const std::string* closest = dictionary_->FindClosest(std::string(word), dist);
            if (!closest) {
                continue;
            }

            if (dist > 0 && dist < 4) {
                report_.mistakes.push_back(Mistake{occurrence.loc.file_path, occurrence.name,
                                                   std::string(word), *closest,
                                                   occurrence.loc.line});
            }
        }
    }

    void RecordName(const Entity entity, const std::string_view name, const bool is_const_like,
                    const clang::SourceManager& source_manager, clang::SourceLocation location,
                    const Entity style_entity) {
        const std::string_view clean_view = StripTemplateArguments(name);

        const NameOccurrence occurrence = BuildNameOccurrence(
            entity, clean_view, is_const_like, source_manager, location, style_entity);

        if (!occurrence.is_ok) {
            report_.bad_names.push_back(BadName{occurrence.loc.file_path, occurrence.name,
                                                occurrence.entity, occurrence.loc.line});
        }
        RecordTypoEvents(clean_view, occurrence);
    }

    Statistics& report_;
    const NameStyleChecker& style_checker_;
    const Dictionary* dictionary_;
};

class NamesAstConsumer : public clang::ASTConsumer {
public:
    NamesAstConsumer(Statistics& report, const NameStyleChecker& style_checker,
                     const Dictionary* dictionary)
        : report_(report),
          style_checker_(style_checker),
          match_finder_(),
          callback_(report_, style_checker_, dictionary) {
        RegisterMatchers();
    }

    void HandleTranslationUnit(clang::ASTContext& context) override {
        match_finder_.matchAST(context);
    }

private:
    void RegisterMatchers() {
        using clang::ast_matchers::bindingDecl;
        using clang::ast_matchers::cxxRecordDecl;
        using clang::ast_matchers::enumDecl;
        using clang::ast_matchers::fieldDecl;
        using clang::ast_matchers::functionDecl;
        using clang::ast_matchers::isDefinition;
        using clang::ast_matchers::isExpansionInSystemHeader;
        using clang::ast_matchers::typeAliasDecl;
        using clang::ast_matchers::typedefDecl;
        using clang::ast_matchers::unless;
        using clang::ast_matchers::varDecl;

        const auto in_user_code = unless(isExpansionInSystemHeader());

        match_finder_.addMatcher(bindingDecl(in_user_code).bind("binding"), &callback_);

        match_finder_.addMatcher(varDecl(in_user_code).bind("var"), &callback_);

        match_finder_.addMatcher(fieldDecl(in_user_code).bind("field"), &callback_);

        match_finder_.addMatcher(functionDecl(in_user_code).bind("func"), &callback_);

        match_finder_.addMatcher(cxxRecordDecl(in_user_code).bind("type"), &callback_);

        match_finder_.addMatcher(enumDecl(in_user_code, isDefinition()).bind("enum"), &callback_);

        match_finder_.addMatcher(typedefDecl(in_user_code).bind("typedef"), &callback_);

        match_finder_.addMatcher(typeAliasDecl(in_user_code).bind("typeAlias"), &callback_);
    }

    Statistics& report_;
    const NameStyleChecker& style_checker_;
    MatchFinder match_finder_;
    NameMatchCallback callback_;
};

class NamesAction : public clang::ASTFrontendAction {
public:
    NamesAction(NameStyleChecker style_checker, const Dictionary* dictionary,
                ReportCallback callback)
        : style_checker_(std::move(style_checker)),
          dictionary_(dictionary),
          callback_(std::move(callback)) {
    }

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance&,
                                                          llvm::StringRef file) override {
        file_path_ = file.str();
        stats_.bad_names.clear();
        stats_.mistakes.clear();
        return std::make_unique<NamesAstConsumer>(stats_, style_checker_, dictionary_);
    }

    void EndSourceFileAction() override {
        const std::string filename = file_path_.empty() ? getCurrentFile().str() : file_path_;

        const std::string normalized_filename = llvm::sys::path::filename(filename).str();

        NormalizeStatistics(stats_);

        if (callback_) {
            callback_(normalized_filename, stats_);
        }
    }

private:
    NameStyleChecker style_checker_;
    const Dictionary* dictionary_;
    ReportCallback callback_;
    std::string file_path_;
    Statistics stats_;
};

class NamesActionFactory : public clang::tooling::FrontendActionFactory {
public:
    NamesActionFactory(NameStyleChecker style_checker, const Dictionary* dictionary,
                       ReportCallback callback)
        : style_checker_(std::move(style_checker)),
          dictionary_(dictionary),
          callback_(std::move(callback)) {
    }

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<NamesAction>(style_checker_, dictionary_, callback_);
    }

private:
    NameStyleChecker style_checker_;
    const Dictionary* dictionary_;
    ReportCallback callback_;
};

std::unique_ptr<clang::tooling::FrontendActionFactory> CreateFactory(
    const NameStyleChecker& style_checker, const Dictionary* dictionary, ReportCallback callback) {
    return std::make_unique<NamesActionFactory>(style_checker, dictionary, std::move(callback));
}
