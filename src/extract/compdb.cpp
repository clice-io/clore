#include "extract/compdb.h"

#include <filesystem>

#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "llvm/Support/MemoryBuffer.h"

#include "support/logging.h"

namespace clore::extract {

auto load_compdb(std::string_view path) -> std::expected<CompilationDatabase, CompDbError> {
    namespace fs = std::filesystem;

    auto compdb_path = fs::path(path);
    if(!fs::exists(compdb_path)) {
        return std::unexpected(CompDbError{
            .message = std::format("compile_commands.json not found: {}", path)});
    }

    std::string error_message;
    auto json_db = clang::tooling::JSONCompilationDatabase::loadFromFile(
        std::string(path), error_message, clang::tooling::JSONCommandLineSyntax::AutoDetect);

    if(!json_db) {
        return std::unexpected(CompDbError{
            .message = std::format("failed to load compile_commands.json: {}", error_message)});
    }

    CompilationDatabase db;
    for(auto& cmd : json_db->getAllCompileCommands()) {
        CompileEntry entry;
        entry.file = cmd.Filename;
        entry.directory = cmd.Directory;
        entry.arguments.reserve(cmd.CommandLine.size());
        for(auto& arg : cmd.CommandLine) {
            entry.arguments.push_back(arg);
        }
        db.entries.push_back(std::move(entry));
    }

    logging::info("loaded {} compile commands from {}", db.entries.size(), path);
    return db;
}

auto lookup(const CompilationDatabase& db, std::string_view file)
    -> std::vector<const CompileEntry*> {
    std::vector<const CompileEntry*> results;

    namespace fs = std::filesystem;
    auto target = fs::path(file).lexically_normal();

    for(auto& entry : db.entries) {
        auto entry_path = fs::path(entry.file).lexically_normal();
        if(entry_path == target) {
            results.push_back(&entry);
        }
    }
    return results;
}

}  // namespace clore::extract
