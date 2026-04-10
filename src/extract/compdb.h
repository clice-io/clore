#pragma once

#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace clore::extract {

struct CompileEntry {
    std::string file;
    std::string directory;
    std::vector<std::string> arguments;
};

struct CompilationDatabase {
    std::vector<CompileEntry> entries;
};

struct CompDbError {
    std::string message;
};

auto load_compdb(std::string_view path) -> std::expected<CompilationDatabase, CompDbError>;

auto lookup(const CompilationDatabase& db, std::string_view file)
    -> std::vector<const CompileEntry*>;

}  // namespace clore::extract
