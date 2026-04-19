#pragma once

using namespace clore;
using namespace clore::generate;

namespace {

namespace fs = std::filesystem;

struct ScopedTempDir {
    fs::path path;

    explicit ScopedTempDir(std::string_view name) {
        auto ticks = std::chrono::steady_clock::now().time_since_epoch().count();
        path = fs::temp_directory_path() / std::format("clore_{}_{}", name, ticks);
        fs::create_directories(path);
    }

    ~ScopedTempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

[[maybe_unused]] auto read_text_file(const fs::path& path) -> std::string {
    std::ifstream f(path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

[[maybe_unused]] auto make_config(const fs::path& project_root) -> config::TaskConfig {
    config::TaskConfig config;
    config.project_root = project_root.generic_string();
    config.output_root = (project_root / "docs").generic_string();
    config.workspace_root = project_root.generic_string();

    config.llm.system_prompt = "You are a writer.";
    config.llm.retry_count = 3;
    config.llm.retry_initial_backoff_ms = 250;

    return config;
}

auto make_function_symbol(std::uint64_t id,
                          std::string_view name,
                          std::string_view qualified_name,
                          std::string_view signature,
                          const std::string& file,
                          std::string_view ns_name = {}) -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Function;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.enclosing_namespace = std::string(ns_name);
    symbol.signature = std::string(signature);
    symbol.source_snippet = std::format("{} {{ return 0; }}", signature);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    symbol.definition_location = extract::SourceLocation{
        .file = file,
        .line = 3,
        .column = 1,
    };
    return symbol;
}

[[maybe_unused]] auto make_type_symbol(std::uint64_t id,
                                       std::string_view name,
                                       std::string_view qualified_name,
                                       const std::string& file,
                                       std::string_view doc_comment,
                                       std::string_view ns_name = {}) -> extract::SymbolInfo {
    extract::SymbolInfo symbol;
    symbol.id = extract::SymbolID{.hash = id};
    symbol.kind = extract::SymbolKind::Struct;
    symbol.name = std::string(name);
    symbol.qualified_name = std::string(qualified_name);
    symbol.enclosing_namespace = std::string(ns_name);
    symbol.signature = std::format("struct {}", name);
    symbol.doc_comment = std::string(doc_comment);
    symbol.source_snippet = std::format("struct {} {{}};", name);
    symbol.declaration_location = extract::SourceLocation{
        .file = file,
        .line = 1,
        .column = 1,
    };
    return symbol;
}

auto add_symbol(extract::ProjectModel& model, extract::SymbolInfo symbol) -> void {
    auto file = symbol.declaration_location.file;
    auto symbol_id = symbol.id;
    auto qualified_name = symbol.qualified_name;

    auto& file_info = model.files[file];
    file_info.path = file;
    file_info.symbols.push_back(symbol_id);
    if(std::ranges::find(model.file_order, file) == model.file_order.end()) {
        model.file_order.push_back(file);
    }

    model.symbol_ids_by_qualified_name[qualified_name].push_back(symbol_id);
    model.symbols.emplace(symbol_id, std::move(symbol));
}

auto add_namespace(extract::ProjectModel& model,
                   std::string name,
                   std::vector<extract::SymbolID> symbols = {},
                   std::vector<std::string> children = {}) -> void {
    auto key = name;
    model.namespaces[key] = extract::NamespaceInfo{
        .name = std::move(name),
        .symbols = std::move(symbols),
        .children = std::move(children),
    };
}

[[maybe_unused]] auto add_module(extract::ProjectModel& model, extract::ModuleUnit module) -> void {
    model.uses_modules = true;
    model.module_name_to_sources[module.name].push_back(module.source_file);
    model.modules.emplace(module.source_file, std::move(module));
}

[[maybe_unused]] auto make_basic_model(const fs::path& project_root) -> extract::ProjectModel {
    extract::ProjectModel model;
    auto file_path = (project_root / "src" / "math.cpp").generic_string();

    auto symbol =
        make_function_symbol(1, "add", "math::add", "int add(int lhs, int rhs)", file_path, "math");
    auto symbol_id = symbol.id;
    add_symbol(model, std::move(symbol));
    add_namespace(model, "math", {symbol_id});

    return model;
}

[[maybe_unused]] auto make_prompt_outputs(std::initializer_list<PromptRequest> requests,
                                          std::string_view text)
    -> std::unordered_map<std::string, std::string> {
    std::unordered_map<std::string, std::string> outputs;
    for(const auto& request: requests) {
        outputs[prompt_request_key(request)] = std::string(text);
    }
    return outputs;
}

}  // namespace
