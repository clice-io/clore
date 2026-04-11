module;

#include <algorithm>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/DependencyDirectivesScanner.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/Support/Error.h"

export module clore.extract:scan;

import :compdb;
import :tooling;
import clore.support;

export namespace clore::extract {

struct ScanError {
    std::string message;
};

struct IncludeInfo {
    std::string path;
    bool is_angled = false;
};

struct ScanResult {
    std::string module_name;
    bool is_interface_unit = false;
    std::vector<IncludeInfo> includes;
    std::vector<std::string> module_imports;
};

/// Cache mapping normalized file path -> ScanResult.
using ScanCache = std::unordered_map<std::string, ScanResult>;

auto scan_file(const CompileEntry& entry) -> std::expected<ScanResult, ScanError>;

/// Fast module declaration scan using Clang's dependency directives scanner.
/// Populates module_name, is_interface_unit, and module_imports in ScanResult
/// without running the full preprocessor.
auto scan_module_decl(std::string_view file_content, ScanResult& result) -> void;

struct DependencyEdge {
    std::string from;
    std::string to;
};

struct DependencyGraph {
    std::vector<std::string> files;
    std::vector<DependencyEdge> edges;
};

struct DependencyResult {
    DependencyGraph graph;
    ScanCache cache;
};

auto build_dependency_graph(const CompilationDatabase& db)
    -> std::expected<DependencyResult, ScanError>;

auto topological_order(const DependencyGraph& graph)
    -> std::expected<std::vector<std::string>, ScanError>;

}  // namespace clore::extract

// ── implementation ──────────────────────────────────────────────────

namespace clore::extract {

namespace {

auto append_unique_import(ScanResult& result, std::string import_name) -> void {
    if(std::ranges::find(result.module_imports, import_name) != result.module_imports.end()) {
        return;
    }
    result.module_imports.push_back(std::move(import_name));
}

auto normalize_partition_import(std::string_view current_module_name,
                                std::string import_name) -> std::string {
    if(import_name.starts_with(':') && !current_module_name.empty()) {
        auto main_name = current_module_name;
        if(auto colon_pos = current_module_name.find(':'); colon_pos != std::string::npos) {
            main_name = current_module_name.substr(0, colon_pos);
        }

        std::string normalized;
        normalized.reserve(main_name.size() + import_name.size());
        normalized += main_name;
        normalized += import_name;
        return normalized;
    }

    return import_name;
}

class ScanPPCallbacks : public clang::PPCallbacks {
public:
    ScanResult& result;

    explicit ScanPPCallbacks(ScanResult& result) : result(result) {}

    void InclusionDirective(clang::SourceLocation /*hash_loc*/,
                            const clang::Token& /*include_tok*/,
                            llvm::StringRef file_name, bool is_angled,
                            clang::CharSourceRange /*filename_range*/,
                            clang::OptionalFileEntryRef file,
                            llvm::StringRef /*search_path*/,
                            llvm::StringRef /*relative_path*/,
                            const clang::Module* /*suggested_module*/,
                            bool /*module_imported*/,
                            clang::SrcMgr::CharacteristicKind /*file_type*/) override {
        IncludeInfo info;
        if(file) {
            info.path = file->getFileEntry().tryGetRealPathName().str();
            if(info.path.empty()) {
                info.path = file->getName().str();
            }
        } else {
            info.path = file_name.str();
        }
        info.is_angled = is_angled;
        result.includes.push_back(std::move(info));
    }
};

class ScanAction : public clang::PreprocessOnlyAction {
public:
    ScanResult& result;

    explicit ScanAction(ScanResult& result) : result(result) {}

    void ExecuteAction() override {
        auto& pp = getCompilerInstance().getPreprocessor();
        pp.addPPCallbacks(std::make_unique<ScanPPCallbacks>(result));
        clang::PreprocessOnlyAction::ExecuteAction();
    }
};

}  // namespace

auto scan_module_decl(std::string_view file_content, ScanResult& result) -> void {
    // Use Clang's dependency directives scanner for fast module detection.
    llvm::SmallVector<clang::dependency_directives_scan::Token, 64> tokens;
    llvm::SmallVector<clang::dependency_directives_scan::Directive, 16> directives;

    if(clang::scanSourceForDependencyDirectives(file_content, tokens, directives)) {
        // Scanner failed; fall back to no module detection.
        return;
    }

    namespace dds = clang::dependency_directives_scan;

    for(auto& dir : directives) {
        if(dir.Kind == dds::cxx_export_module_decl || dir.Kind == dds::cxx_module_decl) {
            // Collect module name from tokens: identifiers + '.' + ':'
            std::string module_name;

            // Skip 'export' and 'module' keywords
            bool past_module_keyword = false;
            for(auto& tok : dir.Tokens) {
                auto tok_text = file_content.substr(tok.Offset, tok.Length);

                if(!past_module_keyword) {
                    if(tok_text == "module") {
                        past_module_keyword = true;
                    }
                    continue;
                }

                // Stop at semicolon or end
                if(tok_text == ";") break;

                module_name += tok_text;
            }

            if(!module_name.empty()) {
                result.module_name = std::move(module_name);
                result.is_interface_unit = (dir.Kind == dds::cxx_export_module_decl);
            }
        } else if(dir.Kind == dds::cxx_import_decl) {
            // Collect import name
            std::string import_name;

            bool past_import_keyword = false;
            for(auto& tok : dir.Tokens) {
                auto tok_text = file_content.substr(tok.Offset, tok.Length);

                if(!past_import_keyword) {
                    if(tok_text == "import") {
                        past_import_keyword = true;
                    }
                    continue;
                }

                if(tok_text == ";") break;

                import_name += tok_text;
            }

            if(!import_name.empty()) {
                append_unique_import(result,
                                     normalize_partition_import(result.module_name,
                                                                std::move(import_name)));
            }
        }
    }
}

auto scan_file(const CompileEntry& entry) -> std::expected<ScanResult, ScanError> {
    if(entry.arguments.empty()) {
        return std::unexpected(ScanError{
            .message = std::format("empty argument list for file: {}", entry.file)});
    }

    ScanResult result;

    // Fast module declaration scan from file content
    {
        namespace fs = std::filesystem;
        auto file_path = fs::path(entry.file);
        if(file_path.is_relative()) {
            file_path = fs::path(entry.directory) / file_path;
        }
        std::ifstream ifs(file_path);
        if(ifs.is_open()) {
            std::string content((std::istreambuf_iterator<char>(ifs)),
                                 std::istreambuf_iterator<char>());
            scan_module_decl(content, result);
        }
    }

    auto instance = create_compiler_instance(entry);
    if(!instance) {
        return std::unexpected(ScanError{
            .message = std::format("failed to create compiler instance for file: {}",
                                   entry.file)});
    }

    auto action = std::make_unique<ScanAction>(result);
    if(!action->BeginSourceFile(*instance, instance->getFrontendOpts().Inputs[0])) {
        return std::unexpected(ScanError{
            .message = std::format("failed to begin preprocessing for file: {}", entry.file)});
    }

    if(auto error = action->Execute()) {
        llvm::consumeError(std::move(error));
        action->EndSourceFile();
        return std::unexpected(ScanError{
            .message = std::format("preprocessor scan failed for file: {}", entry.file)});
    }

    action->EndSourceFile();
    return result;
}

auto build_dependency_graph(const CompilationDatabase& db)
    -> std::expected<DependencyResult, ScanError> {
    DependencyResult result;
    auto& graph = result.graph;
    auto& cache = result.cache;
    std::unordered_set<std::string> entry_files;
    std::unordered_set<std::string> file_set;

    for(auto& entry : db.entries) {
        namespace fs = std::filesystem;
        auto normalized = fs::path(entry.file).lexically_normal().string();
        entry_files.insert(normalized);
        if(file_set.insert(normalized).second) graph.files.push_back(normalized);
    }

    for(auto& entry : db.entries) {
        namespace fs = std::filesystem;
        auto normalized = fs::path(entry.file).lexically_normal().string();

        auto scan_result = scan_file(entry);
        if(!scan_result.has_value()) {
            return std::unexpected(std::move(scan_result.error()));
        }

        for(auto& inc : scan_result->includes) {
            auto inc_normalized = fs::path(inc.path).lexically_normal().string();
            if(entry_files.contains(inc_normalized)) {
                graph.edges.push_back(DependencyEdge{
                    .from = normalized,
                    .to = inc_normalized,
                });
            }
        }

        cache.emplace(normalized, std::move(*scan_result));
    }

    return result;
}

auto topological_order(const DependencyGraph& graph)
    -> std::expected<std::vector<std::string>, ScanError> {
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, int> in_degree;

    for(auto& f : graph.files) {
        in_degree[f] = 0;
    }

    for(auto& edge : graph.edges) {
        adj[edge.to].push_back(edge.from);
        in_degree[edge.from]++;
    }

    std::queue<std::string> q;
    for(auto& [file, degree] : in_degree) {
        if(degree == 0) {
            q.push(file);
        }
    }

    std::vector<std::string> order;
    order.reserve(graph.files.size());

    while(!q.empty()) {
        auto current = q.front();
        q.pop();
        order.push_back(current);

        if(auto it = adj.find(current); it != adj.end()) {
            for(auto& next : it->second) {
                if(--in_degree[next] == 0) {
                    q.push(next);
                }
            }
        }
    }

    if(order.size() < graph.files.size()) {
        return std::unexpected(ScanError{
            .message = "dependency cycle detected in project include graph"});
    }

    return order;
}

}  // namespace clore::extract
