#include "generate/generate.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "generate/llm.h"
#include "support/logging.h"

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

    // Create parent directories
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

    // Write file
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

// ── symbol context assembly ─────────────────────────────────────────

auto lookup_symbol(const extract::ProjectModel& model, extract::SymbolID id)
    -> std::optional<extract::SymbolInfo> {
    auto it = model.symbols.find(id);
    if(it == model.symbols.end()) return std::nullopt;
    return it->second;
}

auto build_symbol_context(const extract::SymbolInfo& sym,
                          const extract::ProjectModel& model,
                          const extract::FileInfo& file_info) -> SymbolContext {
    SymbolContext ctx;
    ctx.self = sym;

    // Direct callers (called_by) — limit to keep context manageable.
    for(auto& caller_id : sym.called_by) {
        if(ctx.direct_callers.size() >= 5) break;
        if(auto s = lookup_symbol(model, caller_id)) {
            ctx.direct_callers.push_back(*s);
        }
    }

    // Direct callees (calls).
    for(auto& callee_id : sym.calls) {
        if(ctx.direct_callees.size() >= 5) break;
        if(auto s = lookup_symbol(model, callee_id)) {
            ctx.direct_callees.push_back(*s);
        }
    }

    // Siblings: other symbols in the same file.
    for(auto& sib_id : file_info.symbols) {
        if(sib_id == sym.id) continue;
        if(ctx.siblings.size() >= 8) break;
        if(auto s = lookup_symbol(model, sib_id)) {
            ctx.siblings.push_back(*s);
        }
    }

    // Parent class / struct.
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

// ── planner: build PageGraph ────────────────────────────────────────

auto build_page_graph_impl(const config::TaskConfig& config,
                           const extract::ProjectModel& model) -> PageGraph {
    namespace fs = std::filesystem;
    PageGraph graph;
    auto project_root = fs::path(config.project_root);

    // --- Create one File-level page per source file ---
    for(auto& [file_path, file_info] : model.files) {
        if(file_info.symbols.empty()) continue;

        auto rel = fs::relative(fs::path(file_path), project_root).generic_string();
        auto dot = rel.rfind('.');
        std::string out_path = (dot != std::string::npos)
                                   ? rel.substr(0, dot) + ".md"
                                   : rel + ".md";
        auto stem = fs::path(file_path).stem().string();

        StructuredPagePlan plan;
        plan.relative_path = out_path;
        plan.title = stem;
        plan.level = PageLevel::File;

        // Assemble SymbolContext for each symbol in the file.
        for(auto& sym_id : file_info.symbols) {
            auto sym_it = model.symbols.find(sym_id);
            if(sym_it == model.symbols.end()) continue;
            plan.contexts.push_back(
                build_symbol_context(sym_it->second, model, file_info));
        }

        PageNode node;
        node.plan = std::move(plan);
        graph.nodes.emplace(out_path, std::move(node));
    }

    // --- Build inter-page dependency edges from include graph ---
    // If file A includes file B and both have pages, then page(A) depends on
    // page(B) (B's docs should be generated first).
    for(auto& [file_path, file_info] : model.files) {
        auto rel_a = fs::relative(fs::path(file_path), project_root).generic_string();
        auto dot_a = rel_a.rfind('.');
        std::string page_a = (dot_a != std::string::npos)
                                 ? rel_a.substr(0, dot_a) + ".md"
                                 : rel_a + ".md";

        if(graph.nodes.find(page_a) == graph.nodes.end()) continue;

        for(auto& inc_path : file_info.includes) {
            auto rel_path = fs::path(inc_path).lexically_relative(project_root);
            if(rel_path.empty()) continue;
            bool escapes_root = false;
            for(const auto& part : rel_path) {
                if(part == "..") {
                    escapes_root = true;
                    break;
                }
            }
            if(escapes_root) continue;

            auto rel_b = rel_path.generic_string();
            auto dot_b = rel_b.rfind('.');
            std::string page_b = (dot_b != std::string::npos)
                                     ? rel_b.substr(0, dot_b) + ".md"
                                     : rel_b + ".md";

            if(page_b == page_a) continue;
            if(graph.nodes.find(page_b) == graph.nodes.end()) continue;

            graph.nodes[page_a].depends_on.push_back(page_b);
            graph.nodes[page_b].depended_by.push_back(page_a);
            graph.nodes[page_a].plan.linked_pages.push_back(page_b);
        }
    }

    // --- Also add call-graph driven edges between pages ---
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

    // --- Topological sort (Kahn's) for generation order ---
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

    // If cycles exist, append remaining pages in arbitrary order.
    if(graph.generation_order.size() < graph.nodes.size()) {
        for(auto& [path, _] : graph.nodes) {
            if(in_degree[path] > 0) {
                graph.generation_order.push_back(path);
            }
        }
    }

    // De-duplicate linked_pages lists.
    for(auto& [_, node] : graph.nodes) {
        auto& lp = node.plan.linked_pages;
        std::sort(lp.begin(), lp.end());
        lp.erase(std::unique(lp.begin(), lp.end()), lp.end());
    }

    return graph;
}

// ── prompt builder (consumes StructuredPagePlan) ────────────────────

auto build_prompt_for_page(const StructuredPagePlan& plan,
                           const config::TaskConfig& config) -> std::string {
    std::ostringstream ss;

    auto language = config.language.has_value() ? *config.language : std::string("English");

    ss << "Generate Markdown documentation for the following C++ source file.\n\n";
    ss << "## File: `" << plan.relative_path << "`\n\n";

    ss << "### Output language\n";
    ss << "- " << language << "\n\n";

    // Linked pages for cross references
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

        // Callers — "why does this exist?"
        if(!ctx.direct_callers.empty()) {
            ss << "**Called by:**\n";
            for(auto& caller : ctx.direct_callers) {
                ss << "- `" << caller.qualified_name << "`";
                if(!caller.signature.empty()) ss << " — " << caller.signature;
                ss << "\n";
            }
            ss << "\n";
        }

        // Callees — "what does this use?"
        if(!ctx.direct_callees.empty()) {
            ss << "**Calls:**\n";
            for(auto& callee : ctx.direct_callees) {
                ss << "- `" << callee.qualified_name << "`\n";
            }
            ss << "\n";
        }

        // Inheritance
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

        // Members
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
    ss << "1. Write a brief overview of the file's purpose.\n";
    ss << "2. Document each class/struct with its purpose and members.\n";
    ss << "3. Document each function: explain WHY it exists (using caller context), "
          "parameters, and return value.\n";
    ss << "4. Note any inheritance relationships.\n";
    ss << "5. Include cross-references to related pages where appropriate.\n";
    ss << "6. Use proper Markdown headings and code blocks.\n";
    ss << "7. Do NOT wrap the output in a code fence.\n";

    return ss.str();
}

// ── frontmatter ─────────────────────────────────────────────────────

auto render_frontmatter(const StructuredPagePlan& plan,
                        const config::FrontmatterConfig& fm_config) -> std::string {
    std::ostringstream ss;
    ss << "---\n";
    ss << "title: \"" << plan.title << "\"\n";

    for(auto& field : fm_config.fields) {
        ss << field.key << ": " << field.value << "\n";
    }

    ss << "---\n";
    return ss.str();
}

// ── markdown assembly ───────────────────────────────────────────────

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
    return build_page_graph_impl(config, model);
}

auto generate_pages(const config::TaskConfig& config, const extract::ProjectModel& model,
                    std::string_view llm_model)
    -> std::expected<std::vector<GeneratedPage>, GenerateError> {
    auto graph = build_page_graph_impl(config, model);

    if(graph.nodes.empty()) {
        return std::unexpected(GenerateError{
            .message = "no pages to generate (no files with symbols)"});
    }

    logging::info("page graph: {} pages, generation order size {}",
                  graph.nodes.size(), graph.generation_order.size());

    std::vector<GeneratedPage> pages;
    pages.reserve(graph.generation_order.size());

    for(std::size_t i = 0; i < graph.generation_order.size(); ++i) {
        auto& page_path = graph.generation_order[i];
        auto node_it = graph.nodes.find(page_path);
        if(node_it == graph.nodes.end()) continue;

        auto& plan = node_it->second.plan;
        logging::info("generating page {}/{}: {}",
                      i + 1, graph.generation_order.size(), plan.relative_path);

        // Build prompt from StructuredPagePlan — no ProjectModel access needed.
        auto prompt = build_prompt_for_page(plan, config);

        // Call LLM
        auto llm_result = call_llm(llm_model, prompt);
        if(!llm_result.has_value()) {
            logging::warn("LLM call failed for {}: {}", plan.relative_path,
                          llm_result.error().message);
            continue;
        }

        // Render frontmatter
        auto frontmatter = render_frontmatter(plan, config.frontmatter);

        // Assemble page
        auto content = assemble_page(frontmatter, *llm_result);

        auto page = GeneratedPage{
            .relative_path = plan.relative_path,
            .content = std::move(content),
        };

        // Write output eagerly so users can see progress.
        auto write_result = write_page_to_root(page, config.output_root);
        if(!write_result.has_value()) {
            return std::unexpected(write_result.error());
        }

        pages.push_back(std::move(page));
    }

    if(pages.empty()) {
        return std::unexpected(GenerateError{
            .message = "all page generations failed — check LLM configuration"});
    }

    return pages;
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

