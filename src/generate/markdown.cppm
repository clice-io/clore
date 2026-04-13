module;

#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

export module generate:markdown;

export namespace clore::generate {

enum class SemanticKind : std::uint8_t {
    Index,
    Namespace,
    Module,
    Type,
    Function,
    Variable,
    File,
    Workflow,
    Section,
};

struct Frontmatter {
    std::string title;
    std::string description;
    std::string layout = "doc";
    std::string page_template = "doc";
};

struct TextFragment {
    std::string text;
};

struct CodeFragment {
    std::string code;
};

struct LinkFragment {
    std::string label;
    std::string target;
    bool code_style = false;
};

using InlineFragment = std::variant<TextFragment, CodeFragment, LinkFragment>;

struct Paragraph {
    std::vector<InlineFragment> fragments;
};

struct ListItem {
    std::vector<InlineFragment> fragments;
};

struct BulletList {
    std::vector<ListItem> items;
};

struct CodeFence {
    std::string language;
    std::string code;
};

struct MermaidDiagram {
    std::string code;
};

struct RawMarkdown {
    std::string markdown;
};

struct SemanticSection;
using SemanticSectionPtr = std::shared_ptr<SemanticSection>;

struct MarkdownNode {
    std::variant<Paragraph, BulletList, CodeFence, MermaidDiagram, RawMarkdown,
                 SemanticSectionPtr>
        value;
};

struct SemanticSection {
    SemanticKind kind = SemanticKind::Section;
    std::string subject_key;
    std::string heading;
    std::uint8_t level = 2;
    bool omit_if_empty = true;
    std::vector<MarkdownNode> children;
};

struct MarkdownDocument {
    std::optional<Frontmatter> frontmatter;
    std::vector<MarkdownNode> children;
};

auto make_text(std::string text) -> InlineFragment;

auto make_code(std::string code) -> InlineFragment;

auto make_link(std::string label, std::string target, bool code_style = false)
    -> InlineFragment;

auto make_paragraph(std::string text) -> MarkdownNode;

auto make_raw_markdown(std::string markdown) -> MarkdownNode;

auto make_code_fence(std::string language, std::string code) -> MarkdownNode;

auto make_mermaid(std::string code) -> MarkdownNode;

auto make_section(SemanticKind kind, std::string subject_key, std::string heading,
                  std::uint8_t level, bool omit_if_empty = true)
    -> SemanticSectionPtr;

auto render_markdown(const MarkdownDocument& document) -> std::string;

}  // namespace clore::generate

namespace clore::generate {

namespace {

auto ensure_double_newline(std::string text) -> std::string {
    while(text.ends_with('\n')) {
        text.pop_back();
    }
    text += "\n\n";
    return text;
}

auto yaml_quote(std::string_view value) -> std::string {
    std::string out;
    out.reserve(value.size() + 2);
    out.push_back('\'');
    for(auto ch : value) {
        if(ch == '\'') {
            out += "''";
            continue;
        }
        if(ch == '\r' || ch == '\n') {
            out.push_back(' ');
            continue;
        }
        out.push_back(ch);
    }
    out.push_back('\'');
    return out;
}

auto render_inline(const InlineFragment& fragment) -> std::string {
    return std::visit(
        [](const auto& value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            if constexpr(std::is_same_v<T, TextFragment>) {
                return value.text;
            } else if constexpr(std::is_same_v<T, CodeFragment>) {
                return "`" + value.code + "`";
            } else {
                auto label = value.code_style ? "`" + value.label + "`" : value.label;
                return "[" + label + "](" + value.target + ")";
            }
        },
        fragment);
}

auto render_inlines(const std::vector<InlineFragment>& fragments) -> std::string {
    std::string rendered;
    for(const auto& fragment : fragments) {
        rendered += render_inline(fragment);
    }
    return rendered;
}

auto render_node(const MarkdownNode& node) -> std::string;

auto render_section(const SemanticSection& section) -> std::string {
    std::string children;
    for(const auto& child : section.children) {
        children += render_node(child);
    }

    if(children.empty() && section.omit_if_empty) {
        return {};
    }

    auto heading_prefix = std::string(section.level, '#');
    std::string rendered = std::format("{} {}\n\n", heading_prefix, section.heading);
    rendered += children;
    return rendered;
}

auto render_node(const MarkdownNode& node) -> std::string {
    return std::visit(
        [](const auto& value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            if constexpr(std::is_same_v<T, Paragraph>) {
                if(value.fragments.empty()) {
                    return {};
                }
                return ensure_double_newline(render_inlines(value.fragments));
            } else if constexpr(std::is_same_v<T, BulletList>) {
                if(value.items.empty()) {
                    return {};
                }
                std::string rendered;
                for(const auto& item : value.items) {
                    rendered += "- ";
                    rendered += render_inlines(item.fragments);
                    rendered += "\n";
                }
                rendered += "\n";
                return rendered;
            } else if constexpr(std::is_same_v<T, CodeFence>) {
                if(value.code.empty()) {
                    return {};
                }
                std::string rendered = "```";
                rendered += value.language;
                rendered += "\n";
                rendered += value.code;
                if(!rendered.ends_with('\n')) {
                    rendered += "\n";
                }
                rendered += "```\n\n";
                return rendered;
            } else if constexpr(std::is_same_v<T, MermaidDiagram>) {
                if(value.code.empty()) {
                    return {};
                }
                std::string rendered = "```mermaid\n";
                rendered += value.code;
                if(!rendered.ends_with('\n')) {
                    rendered += "\n";
                }
                rendered += "```\n\n";
                return rendered;
            } else if constexpr(std::is_same_v<T, RawMarkdown>) {
                if(value.markdown.empty()) {
                    return {};
                }
                return ensure_double_newline(value.markdown);
            } else {
                return value != nullptr ? render_section(*value) : std::string{};
            }
        },
        node.value);
}

}  // namespace

auto make_text(std::string text) -> InlineFragment {
    return TextFragment{.text = std::move(text)};
}

auto make_code(std::string code) -> InlineFragment {
    return CodeFragment{.code = std::move(code)};
}

auto make_link(std::string label, std::string target, bool code_style) -> InlineFragment {
    return LinkFragment{
        .label = std::move(label),
        .target = std::move(target),
        .code_style = code_style,
    };
}

auto make_paragraph(std::string text) -> MarkdownNode {
    return MarkdownNode{Paragraph{{make_text(std::move(text))}}};
}

auto make_raw_markdown(std::string markdown) -> MarkdownNode {
    return MarkdownNode{RawMarkdown{.markdown = std::move(markdown)}};
}

auto make_code_fence(std::string language, std::string code) -> MarkdownNode {
    return MarkdownNode{CodeFence{
        .language = std::move(language),
        .code = std::move(code),
    }};
}

auto make_mermaid(std::string code) -> MarkdownNode {
    return MarkdownNode{MermaidDiagram{.code = std::move(code)}};
}

auto make_section(SemanticKind kind, std::string subject_key, std::string heading,
                  std::uint8_t level, bool omit_if_empty) -> SemanticSectionPtr {
    return std::make_shared<SemanticSection>(SemanticSection{
        .kind = kind,
        .subject_key = std::move(subject_key),
        .heading = std::move(heading),
        .level = level,
        .omit_if_empty = omit_if_empty,
    });
}

auto render_markdown(const MarkdownDocument& document) -> std::string {
    std::string rendered;
    if(document.frontmatter.has_value()) {
        auto& fm = *document.frontmatter;
        rendered += "---\n";
        rendered += "title: " + yaml_quote(fm.title) + "\n";
        rendered += "description: " + yaml_quote(fm.description) + "\n";
        rendered += "layout: " + fm.layout + "\n";
        rendered += "template: " + fm.page_template + "\n";
        rendered += "---\n\n";
    }

    for(const auto& child : document.children) {
        rendered += render_node(child);
    }

    return rendered;
}

}  // namespace clore::generate