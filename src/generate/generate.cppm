module;

#include <algorithm>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <queue>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

export module clore.generate;

export import :llm;

import clore.config;
import clore.extract;
import clore.support;

export namespace clore::generate {

// ── page hierarchy ──────────────────────────────────────────────────

enum class PageLevel : std::uint8_t {
    Symbol,
    ClassStruct,
    File,
    Module,      ///< A C++20 module unit.
    Namespace,
    Repository,
};

// ── structured page context ─────────────────────────────────────────

struct SymbolContext {
    extract::SymbolInfo self;
    std::vector<extract::SymbolInfo> direct_callers;
    std::vector<extract::SymbolInfo> direct_callees;
    std::vector<extract::SymbolInfo> siblings;
    std::optional<extract::SymbolInfo> parent_class;
};

struct StructuredPagePlan {
    std::string relative_path;
    std::string title;
    PageLevel level = PageLevel::File;
    std::vector<SymbolContext> contexts;
    std::vector<std::string> linked_pages;
};

// ── page graph ──────────────────────────────────────────────────────

struct PageNode {
    StructuredPagePlan plan;
    std::vector<std::string> depends_on;
    std::vector<std::string> depended_by;
};

struct PageGraph {
    std::unordered_map<std::string, PageNode> nodes;
    std::vector<std::string> generation_order;
};

// ── output ──────────────────────────────────────────────────────────

struct PromptPage {
    std::string relative_path;
    std::string title;
    std::string prompt;
};

struct GeneratedPage {
    std::string relative_path;
    std::string content;
};

struct GenerateError {
    std::string message;
};

// ── public API ──────────────────────────────────────────────────────

auto build_page_graph(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> PageGraph;

auto build_prompts(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<PromptPage>, GenerateError>;

auto generate_pages(const config::TaskConfig& config, const extract::ProjectModel& model,
                    std::string_view llm_model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError>;

auto write_prompts(const std::vector<PromptPage>& prompts, std::string_view output_root)
    -> std::expected<void, GenerateError>;

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError>;

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

// ── helpers ─────────────────────────────────────────────────────────

auto write_page_to_root(const GeneratedPage& page, std::string_view output_root)
    -> std::expected<void, GenerateError> {
    namespace fs = std::filesystem;

    auto root = fs::path(output_root);

    auto rel = fs::path(page.relative_path);
    if(rel.is_absolute()) {
        return std::unexpected(GenerateError{
            .message = std::format("page output path must be relative: {}", page.relative_path)});
    }
    for(const auto& part : rel) {
        if(part == "." || part == "..") {
            return std::unexpected(GenerateError{
                .message = std::format("page output path must not contain '.' or '..': {}",
                                       page.relative_path)});
        }
    }

    auto target = (root / rel).lexically_normal();

    auto parent = target.parent_path();
    if(!fs::exists(parent)) {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if(ec) {
            return std::unexpected(GenerateError{
                .message = std::format("failed to create directory {}: {}",
                                       parent.generic_string(), ec.message())});
        }
    }

    std::ofstream f(target);
    if(!f.is_open()) {
        return std::unexpected(GenerateError{
            .message = std::format("failed to write page: {}", target.generic_string())});
    }
    f << page.content;

    logging::info("  wrote {}", page.relative_path);

    return {};
}

auto symbol_kind_label(extract::SymbolKind kind) -> std::string_view {
    return extract::symbol_kind_name(kind);
}

auto path_relative_to_root(const std::filesystem::path& path,
                          const std::filesystem::path& root)
    -> std::optional<std::string> {
    auto rel = path.lexically_relative(root);
    if(rel.empty()) return std::nullopt;
    for(const auto& part : rel) {
        if(part == "..") return std::nullopt;
    }
    return rel.generic_string();
}

auto output_path_for_source_file(const std::filesystem::path& file_path,
                                 const std::filesystem::path& source_root)
    -> std::optional<std::string> {
    auto rel = path_relative_to_root(file_path, source_root);
    if(!rel.has_value()) return std::nullopt;

    auto dot = rel->rfind('.');
    if(dot != std::string::npos) {
        return rel->substr(0, dot) + ".md";
    }
    return *rel + ".md";
}

// ── module-aware output path ────────────────────────────────────────

/// Convert a module name to a documentation output path.
/// Main modules: "foo.bar" → "foo.bar.md"
/// Partitions:   "foo.bar:baz" → "foo.bar/baz.md"
auto output_path_for_module(const std::string& module_name) -> std::string {
    auto colon_pos = module_name.find(':');
    if(colon_pos != std::string::npos) {
        // Partition: "foo.bar:baz" → "foo.bar/baz.md"
        auto main_name = module_name.substr(0, colon_pos);
        auto partition_name = module_name.substr(colon_pos + 1);
        return main_name + "/" + partition_name + ".md";
    }
    // Main module: "foo.bar" → "foo.bar.md"
    return module_name + ".md";
}

// ── symbol context assembly ─────────────────────────────────────────

auto lookup_symbol(const extract::ProjectModel& model, extract::SymbolID id)
    -> std::optional<extract::SymbolInfo> {
    auto it = model.symbols.find(id);
    if(it == model.symbols.end()) return std::nullopt;
    return it->second;
}

auto build_symbol_context(const extract::SymbolInfo& sym,
                          const extract::ProjectModel& model,
                          const std::vector<extract::SymbolID>& sibling_ids) -> SymbolContext {
    SymbolContext ctx;
    ctx.self = sym;
    std::unordered_set<extract::SymbolID> seen_callers;
    std::unordered_set<extract::SymbolID> seen_callees;
    std::unordered_set<extract::SymbolID> seen_siblings;

    for(auto& caller_id : sym.called_by) {
        if(!seen_callers.insert(caller_id).second) continue;
        if(auto s = lookup_symbol(model, caller_id)) {
            ctx.direct_callers.push_back(*s);
            if(ctx.direct_callers.size() >= 5) break;
        }
    }

    for(auto& callee_id : sym.calls) {
        if(!seen_callees.insert(callee_id).second) continue;
        if(auto s = lookup_symbol(model, callee_id)) {
            ctx.direct_callees.push_back(*s);
            if(ctx.direct_callees.size() >= 5) break;
        }
    }

    for(auto& sib_id : sibling_ids) {
        if(sib_id == sym.id) continue;
        if(!seen_siblings.insert(sib_id).second) continue;
        if(auto s = lookup_symbol(model, sib_id)) {
            ctx.siblings.push_back(*s);
            if(ctx.siblings.size() >= 8) break;
        }
    }

    if(sym.parent.has_value()) {
        if(auto p = lookup_symbol(model, *sym.parent)) {
            if(p->kind == extract::SymbolKind::Class ||
               p->kind == extract::SymbolKind::Struct) {
                ctx.parent_class = *p;
            }
        }
    }

    return ctx;
}

auto build_symbol_context_from_file(const extract::SymbolInfo& sym,
                                    const extract::ProjectModel& model,
                                    const extract::FileInfo& file_info) -> SymbolContext {
    return build_symbol_context(sym, model, file_info.symbols);
}

// ── file-based page graph (traditional) ─────────────────────────────

auto build_file_page_graph(const config::TaskConfig& config,
                           const extract::ProjectModel& model) -> PageGraph {
    namespace fs = std::filesystem;
    PageGraph graph;
    auto source_root = fs::path(config.project_root).lexically_normal();

    for(auto& [file_path, file_info] : model.files) {
        if(file_info.symbols.empty()) continue;

        auto out_path = output_path_for_source_file(fs::path(file_path), source_root);
        if(!out_path.has_value()) continue;
        auto stem = fs::path(file_path).stem().string();

        auto [node_it, inserted] = graph.nodes.emplace(*out_path, PageNode{});
        auto& node = node_it->second;
        if(inserted) {
            node.plan.relative_path = *out_path;
            node.plan.title = stem;
            node.plan.level = PageLevel::File;
        }

        for(auto& sym_id : file_info.symbols) {
            auto sym_it = model.symbols.find(sym_id);
            if(sym_it == model.symbols.end()) continue;
            auto exists = std::ranges::any_of(node.plan.contexts, [&](const SymbolContext& ctx) {
                return ctx.self.id == sym_id;
            });
            if(exists) continue;
            node.plan.contexts.push_back(build_symbol_context_from_file(sym_it->second, model, file_info));
        }
    }

    // Include graph edges
    for(auto& [file_path, file_info] : model.files) {
        auto page_a = output_path_for_source_file(fs::path(file_path), source_root);
        if(!page_a.has_value()) continue;
        if(graph.nodes.find(*page_a) == graph.nodes.end()) continue;

        for(auto& inc_path : file_info.includes) {
            auto page_b = output_path_for_source_file(fs::path(inc_path), source_root);
            if(!page_b.has_value()) continue;
            if(*page_b == *page_a) continue;
            if(graph.nodes.find(*page_b) == graph.nodes.end()) continue;

            graph.nodes[*page_a].depends_on.push_back(*page_b);
            graph.nodes[*page_b].depended_by.push_back(*page_a);
            graph.nodes[*page_a].plan.linked_pages.push_back(*page_b);
        }
    }

    // Call graph edges
    std::unordered_map<extract::SymbolID, std::string> sym_to_page;
    for(auto& [page_path, node] : graph.nodes) {
        for(auto& ctx : node.plan.contexts) {
            sym_to_page[ctx.self.id] = page_path;
        }
    }
    for(auto& [page_path, node] : graph.nodes) {
        std::unordered_set<std::string> already;
        for(auto& dep : node.depends_on) already.insert(dep);

        for(auto& ctx : node.plan.contexts) {
            for(auto& callee_id : ctx.self.calls) {
                auto it = sym_to_page.find(callee_id);
                if(it == sym_to_page.end()) continue;
                if(it->second == page_path) continue;
                if(!already.insert(it->second).second) continue;
                node.depends_on.push_back(it->second);
                graph.nodes[it->second].depended_by.push_back(page_path);
                node.plan.linked_pages.push_back(it->second);
            }
        }
    }

    return graph;
}

// ── module-based page graph ─────────────────────────────────────────

auto build_module_page_graph(const config::TaskConfig& config,
                             const extract::ProjectModel& model) -> PageGraph {
    PageGraph graph;

    // One page per module unit, path based on module name hierarchy
    for(auto& [mod_name, mod_unit] : model.modules) {
        if(!mod_unit.is_interface) continue;

        auto out_path = output_path_for_module(mod_name);
        auto& node = graph.nodes[out_path];
        node.plan.relative_path = out_path;
        node.plan.level = PageLevel::Module;

        // Title: module name (without the partition colon for display)
        auto colon_pos = mod_name.find(':');
        if(colon_pos != std::string::npos) {
            node.plan.title = mod_name.substr(colon_pos + 1);
        } else {
            node.plan.title = mod_name;
        }

        // Collect symbols for this module unit
        for(auto& sym_id : mod_unit.symbols) {
            auto sym_it = model.symbols.find(sym_id);
            if(sym_it == model.symbols.end()) continue;
            auto exists = std::ranges::any_of(node.plan.contexts, [&](const SymbolContext& ctx) {
                return ctx.self.id == sym_id;
            });
            if(exists) continue;
            node.plan.contexts.push_back(build_symbol_context(sym_it->second, model, mod_unit.symbols));
        }
    }

    // Build dependency edges from module imports
    // Map module name → output path
    std::unordered_map<std::string, std::string> mod_to_page;
    for(auto& [mod_name, mod_unit] : model.modules) {
        if(!mod_unit.is_interface) continue;
        mod_to_page[mod_name] = output_path_for_module(mod_name);
    }

    for(auto& [mod_name, mod_unit] : model.modules) {
        if(!mod_unit.is_interface) continue;
        auto page_a = output_path_for_module(mod_name);
        auto node_a_it = graph.nodes.find(page_a);
        if(node_a_it == graph.nodes.end()) continue;

        for(auto& import_name : mod_unit.imports) {
            auto it = mod_to_page.find(import_name);
            if(it == mod_to_page.end()) continue;
            if(it->second == page_a) continue;

            node_a_it->second.depends_on.push_back(it->second);
            graph.nodes[it->second].depended_by.push_back(page_a);
            node_a_it->second.plan.linked_pages.push_back(it->second);
        }

        // Partition imports from main module: if this is a partition, depend on the main module
        if(auto colon_pos = mod_name.find(':'); colon_pos != std::string::npos) {
            auto main_name = mod_name.substr(0, colon_pos);
            auto main_it = mod_to_page.find(main_name);
            if(main_it != mod_to_page.end() && main_it->second != page_a) {
                node_a_it->second.plan.linked_pages.push_back(main_it->second);
            }
        }
    }

    // Call graph edges between module pages
    std::unordered_map<extract::SymbolID, std::string> sym_to_page;
    for(auto& [page_path, node] : graph.nodes) {
        for(auto& ctx : node.plan.contexts) {
            sym_to_page[ctx.self.id] = page_path;
        }
    }
    for(auto& [page_path, node] : graph.nodes) {
        std::unordered_set<std::string> already;
        for(auto& dep : node.depends_on) already.insert(dep);

        for(auto& ctx : node.plan.contexts) {
            for(auto& callee_id : ctx.self.calls) {
                auto it = sym_to_page.find(callee_id);
                if(it == sym_to_page.end()) continue;
                if(it->second == page_path) continue;
                if(!already.insert(it->second).second) continue;
                node.depends_on.push_back(it->second);
                graph.nodes[it->second].depended_by.push_back(page_path);
                node.plan.linked_pages.push_back(it->second);
            }
        }
    }

    return graph;
}

// ── topological sort for page graph ─────────────────────────────────

auto sort_page_graph(PageGraph& graph) -> void {
    std::unordered_map<std::string, int> in_degree;
    for(auto& [path, _] : graph.nodes) in_degree[path] = 0;
    for(auto& [path, node] : graph.nodes) {
        in_degree[path] += static_cast<int>(node.depends_on.size());
    }

    std::queue<std::string> q;
    for(auto& [path, degree] : in_degree) {
        if(degree == 0) q.push(path);
    }

    while(!q.empty()) {
        auto current = q.front();
        q.pop();
        graph.generation_order.push_back(current);

        auto node_it = graph.nodes.find(current);
        if(node_it == graph.nodes.end()) continue;
        for(auto& dependent : node_it->second.depended_by) {
            if(--in_degree[dependent] == 0) {
                q.push(dependent);
            }
        }
    }

    if(graph.generation_order.size() < graph.nodes.size()) {
        for(auto& [path, _] : graph.nodes) {
            if(in_degree[path] > 0) {
                graph.generation_order.push_back(path);
            }
        }
    }

    for(auto& [_, node] : graph.nodes) {
        auto& lp = node.plan.linked_pages;
        std::sort(lp.begin(), lp.end());
        lp.erase(std::unique(lp.begin(), lp.end()), lp.end());
    }
}

// ── prompt builder ──────────────────────────────────────────────────

auto build_prompt_for_page(const StructuredPagePlan& plan,
                           const config::TaskConfig& config) -> std::string {
    std::ostringstream ss;

    auto language = config.language.has_value() ? *config.language : std::string("English");

    if(plan.level == PageLevel::Module) {
        ss << "Generate Markdown documentation for the following C++20 module.\n\n";
        ss << "## Module: `" << plan.title << "`\n\n";
    } else {
        ss << "Generate Markdown documentation for the following C++ source file.\n\n";
        ss << "## File: `" << plan.relative_path << "`\n\n";
    }

    ss << "### Output language\n";
    ss << "- " << language << "\n\n";

    if(!plan.linked_pages.empty()) {
        ss << "### Related pages\n";
        for(auto& lp : plan.linked_pages) {
            ss << "- [" << lp << "](" << lp << ")\n";
        }
        ss << "\n";
    }

    ss << "### Symbols\n\n";

    for(auto& ctx : plan.contexts) {
        auto& sym = ctx.self;
        ss << "#### " << symbol_kind_label(sym.kind) << ": `"
           << sym.qualified_name << "`\n\n";

        if(!sym.signature.empty()) {
            ss << "**Signature:**\n```cpp\n" << sym.signature << "\n```\n\n";
        }

        if(!sym.doc_comment.empty()) {
            ss << "**Existing doc comment:**\n" << sym.doc_comment << "\n\n";
        }

        if(!sym.source_snippet.empty()) {
            ss << "**Source:**\n```cpp\n" << sym.source_snippet << "\n```\n\n";
        }

        if(!ctx.direct_callers.empty()) {
            ss << "**Called by:**\n";
            for(auto& caller : ctx.direct_callers) {
                ss << "- `" << caller.qualified_name << "`";
                if(!caller.signature.empty()) ss << " — " << caller.signature;
                ss << "\n";
            }
            ss << "\n";
        }

        if(!ctx.direct_callees.empty()) {
            ss << "**Calls:**\n";
            for(auto& callee : ctx.direct_callees) {
                ss << "- `" << callee.qualified_name << "`\n";
            }
            ss << "\n";
        }

        if(!sym.bases.empty()) {
            ss << "**Bases:** ";
            bool first = true;
            for(auto& sib : ctx.siblings) {
                for(auto& base_id : sym.bases) {
                    if(sib.id == base_id) {
                        if(!first) ss << ", ";
                        first = false;
                        ss << "`" << sib.qualified_name << "`";
                    }
                }
            }
            if(ctx.parent_class.has_value() && first) {
                ss << "`" << ctx.parent_class->qualified_name << "`";
            }
            ss << "\n\n";
        }

        if(!sym.children.empty()) {
            ss << "**Members:**\n";
            for(auto& child_id : sym.children) {
                for(auto& sib : ctx.siblings) {
                    if(sib.id == child_id) {
                        ss << "- " << symbol_kind_label(sib.kind)
                           << " `" << sib.name << "`";
                        if(!sib.signature.empty()) ss << ": " << sib.signature;
                        ss << "\n";
                        break;
                    }
                }
            }
            ss << "\n";
        }
    }

    ss << "\n### Instructions\n\n";
    if(plan.level == PageLevel::Module) {
        ss << "1. Write a brief overview of this module's purpose and public interface.\n";
        ss << "2. Document each exported class/struct with its purpose and members.\n";
        ss << "3. Document each exported function: explain WHY it exists, parameters, and return value.\n";
        ss << "4. Note module dependencies and partition relationships.\n";
    } else {
        ss << "1. Write a brief overview of the file's purpose.\n";
        ss << "2. Document each class/struct with its purpose and members.\n";
        ss << "3. Document each function: explain WHY it exists (using caller context), "
              "parameters, and return value.\n";
        ss << "4. Note any inheritance relationships.\n";
    }
    ss << "5. Include cross-references to related pages where appropriate.\n";
    ss << "6. Use proper Markdown headings and code blocks.\n";
    ss << "7. Do NOT wrap the output in a code fence.\n";

    return ss.str();
}

auto render_frontmatter(std::string_view title,
                        const config::FrontmatterConfig& fm_config) -> std::string {
    std::ostringstream ss;
    ss << "---\n";
    ss << "title: \"" << title << "\"\n";

    for(auto& field : fm_config.fields) {
        ss << field.key << ": " << field.value << "\n";
    }

    ss << "---\n";
    return ss.str();
}

auto assemble_page(std::string_view frontmatter,
                   std::string_view llm_body) -> std::string {
    std::string content;
    content.reserve(frontmatter.size() + llm_body.size() + 64);
    content += frontmatter;
    content += "\n";
    content += llm_body;
    content += "\n";
    return content;
}

}  // namespace

// ── public API ──────────────────────────────────────────────────────

auto build_page_graph(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> PageGraph {
    PageGraph graph;

    if(model.uses_modules) {
        logging::info("building module-based page graph ({} modules)", model.modules.size());
        graph = build_module_page_graph(config, model);
    } else {
        graph = build_file_page_graph(config, model);
    }

    sort_page_graph(graph);
    return graph;
}

auto build_prompts(const config::TaskConfig& config, const extract::ProjectModel& model)
    -> std::expected<std::vector<PromptPage>, GenerateError> {
    auto graph = build_page_graph(config, model);

    if(graph.nodes.empty()) {
        return std::unexpected(GenerateError{
            .message = "no pages to generate (no files with symbols)"});
    }

    logging::info("page graph: {} pages, generation order size {}",
                  graph.nodes.size(), graph.generation_order.size());

    std::vector<PromptPage> prompts;
    prompts.reserve(graph.generation_order.size());

    for(auto& page_path : graph.generation_order) {
        auto node_it = graph.nodes.find(page_path);
        if(node_it == graph.nodes.end()) continue;

        auto& plan = node_it->second.plan;
        prompts.push_back(PromptPage{
            .relative_path = plan.relative_path,
            .title = plan.title,
            .prompt = build_prompt_for_page(plan, config),
        });
    }

    if(prompts.empty()) {
        return std::unexpected(GenerateError{
            .message = "page graph did not yield any promptable pages"});
    }

    return prompts;
}

auto generate_pages(const config::TaskConfig& config, const extract::ProjectModel& model,
                    std::string_view llm_model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError> {
    auto prompts_result = build_prompts(config, model);
    if(!prompts_result.has_value()) {
        return std::unexpected(std::move(prompts_result.error()));
    }

    auto& prompts = *prompts_result;

    std::vector<GeneratedPage> pages;
    pages.reserve(prompts.size());

    for(std::size_t i = 0; i < prompts.size(); ++i) {
        auto& prompt = prompts[i];
        logging::info("generating page {}/{}: {}",
                      i + 1, prompts.size(), prompt.relative_path);

        auto llm_result = call_llm(llm_model, prompt.prompt);
        if(!llm_result.has_value()) {
            logging::warn("LLM call failed for {}: {}", prompt.relative_path,
                          llm_result.error().message);
            continue;
        }

        auto frontmatter = render_frontmatter(prompt.title, config.frontmatter);
        auto content = assemble_page(frontmatter, *llm_result);

        auto page = GeneratedPage{
            .relative_path = prompt.relative_path,
            .content = std::move(content),
        };

        pages.push_back(std::move(page));
    }

    if(pages.empty()) {
        return std::unexpected(GenerateError{
            .message = "all page generations failed — check LLM configuration"});
    }

    return pages;
}

auto write_prompts(const std::vector<PromptPage>& prompts, std::string_view output_root)
    -> std::expected<void, GenerateError> {
    if(output_root.empty()) {
        return std::unexpected(GenerateError{
            .message = "output root must not be empty for dry-run"});
    }
    if(prompts.empty()) {
        return std::unexpected(GenerateError{
            .message = "no prompts available for dry-run output"});
    }

    for(auto& prompt : prompts) {
        auto page = GeneratedPage{
            .relative_path = prompt.relative_path,
            .content = prompt.prompt,
        };
        if(auto result = write_page_to_root(page, output_root); !result.has_value()) {
            return result;
        }
    }

    return {};
}

auto write_pages(const std::vector<GeneratedPage>& pages, std::string_view output_root)
    -> std::expected<void, GenerateError> {
    for(auto& page : pages) {
        if(auto r = write_page_to_root(page, output_root); !r.has_value()) {
            return r;
        }
    }

    return {};
}

}  // namespace clore::generate
