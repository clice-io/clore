#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "extract/compdb.h"

namespace clore::extract {

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

/// Cache mapping normalized file path -> ScanResult from the dependency graph
/// build phase, so that callers can reuse scan results without re-running the
/// preprocessor.
using ScanCache = std::unordered_map<std::string, ScanResult>;

/// Strip the leading compiler-executable element (args[0]) from a raw argument
/// list. Both scan_file and extract_symbols use this; defining it here avoids
/// duplication across translation units.
inline auto strip_compiler_path(const std::vector<std::string>& args) -> std::vector<std::string> {
    if(args.size() <= 1) {
        return {};
    }
    return std::vector<std::string>(args.begin() + 1, args.end());
}

inline auto normalize_argument_path(std::string_view path, std::string_view directory)
    -> std::filesystem::path {
    auto normalized = std::filesystem::path(path);
    if(normalized.is_relative()) {
        normalized = std::filesystem::path(directory) / normalized;
    }
    return normalized.lexically_normal();
}

inline auto sanitize_driver_arguments(const CompileEntry& entry) -> std::vector<std::string> {
    auto adjusted = entry.arguments;
    auto source_path = normalize_argument_path(entry.file, entry.directory);

    std::erase_if(adjusted, [&](const std::string& arg) {
        if(arg.empty() || arg.starts_with('-')) {
            return false;
        }
        return normalize_argument_path(arg, entry.directory) == source_path;
    });

    return adjusted;
}

inline auto sanitize_tool_arguments(const CompileEntry& entry) -> std::vector<std::string> {
    return strip_compiler_path(sanitize_driver_arguments(entry));
}

/// Returns an error if the entry's argument list is empty or the tool fails.
auto scan_file(const CompileEntry& entry) -> std::expected<ScanResult, ScanError>;

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

/// Build the include dependency graph from the compilation database.  Also
/// populates a ScanCache so callers can look up per-file scan results without
/// re-running the preprocessor.
auto build_dependency_graph(const CompilationDatabase& db)
    -> std::expected<DependencyResult, ScanError>;

/// Returns an error if a dependency cycle is detected.
auto topological_order(const DependencyGraph& graph)
    -> std::expected<std::vector<std::string>, ScanError>;

}  // namespace clore::extract
