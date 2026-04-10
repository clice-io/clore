#include "extract/scan.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/Support/Error.h"

#include "support/logging.h"
#include "extract/tooling.h"

namespace clore::extract {

namespace {

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

auto scan_file(const CompileEntry& entry) -> std::expected<ScanResult, ScanError> {
    if(entry.arguments.empty()) {
        return std::unexpected(ScanError{
            .message = std::format("empty argument list for file: {}", entry.file)});
    }

    ScanResult result;
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

    // Only track compilation-database entries as nodes in the dependency graph.
    // Includes are treated as edges but do not become standalone nodes, which
    // avoids pulling in dependency/system headers (and potential cycles) that
    // are outside the project boundary.
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

        // Cache the scan result so callers don't need to re-scan.
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

    // A cycle means some files were never enqueued.  Fail fast instead of
    // silently processing files in an arbitrary order.
    if(order.size() < graph.files.size()) {
        return std::unexpected(ScanError{
            .message = "dependency cycle detected in project include graph"});
    }

    return order;
}

}  // namespace clore::extract
