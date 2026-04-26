export module generate:markdown;

import std;

export namespace clore::generate {

enum class SemanticKind : std::uint8_t {
    Index,
    Namespace,
    Module,
    Type,
    Function,
    Variable,
    File,
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

struct BlockQuote {
    std::vector<InlineFragment> fragments;
};

struct RawMarkdown {
    std::string markdown;
};

struct SemanticSection;
using SemanticSectionPtr = std::unique_ptr<SemanticSection>;

struct MarkdownNode {
    std::variant<Paragraph,
                 BulletList,
                 CodeFence,
                 MermaidDiagram,
                 BlockQuote,
                 RawMarkdown,
                 SemanticSectionPtr>
        value;
};

struct SemanticSection {
    SemanticKind kind = SemanticKind::Section;
    std::string subject_key;
    std::string heading;
    std::uint8_t level = 2;
    bool omit_if_empty = true;
    bool code_style_heading = false;
    std::vector<MarkdownNode> children;
};

struct MarkdownDocument {
    std::optional<Frontmatter> frontmatter;
    std::vector<MarkdownNode> children;
};

auto make_text(std::string text) -> InlineFragment;

auto make_code(std::string code) -> InlineFragment;

auto make_link(std::string label, std::string target, bool code_style = false) -> InlineFragment;

auto make_paragraph(std::string text) -> MarkdownNode;

auto make_raw_markdown(std::string markdown) -> MarkdownNode;

auto make_code_fence(std::string language, std::string code) -> MarkdownNode;

auto make_mermaid(std::string code) -> MarkdownNode;

auto make_blockquote(std::string text) -> MarkdownNode;

auto make_section(SemanticKind kind,
                  std::string subject_key,
                  std::string heading,
                  std::uint8_t level,
                  bool omit_if_empty = true,
                  bool code_style_heading = false) -> SemanticSectionPtr;

auto render_markdown(const MarkdownDocument& document) -> std::string;

auto code_spanned_fragments(std::string_view text) -> std::vector<InlineFragment>;

auto code_spanned_markdown(std::string_view markdown) -> std::string;

}  // namespace clore::generate

namespace clore::generate {

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
    return MarkdownNode{
        CodeFence{
                  .language = std::move(language),
                  .code = std::move(code),
                  }
    };
}

auto make_mermaid(std::string code) -> MarkdownNode {
    return MarkdownNode{MermaidDiagram{.code = std::move(code)}};
}

auto make_blockquote(std::string text) -> MarkdownNode {
    return MarkdownNode{BlockQuote{{make_text(std::move(text))}}};
}

auto make_section(SemanticKind kind,
                  std::string subject_key,
                  std::string heading,
                  std::uint8_t level,
                  bool omit_if_empty,
                  bool code_style_heading) -> SemanticSectionPtr {
    return std::make_unique<SemanticSection>(SemanticSection{
        .kind = kind,
        .subject_key = std::move(subject_key),
        .heading = std::move(heading),
        .level = level,
        .omit_if_empty = omit_if_empty,
        .code_style_heading = code_style_heading,
        .children = {},
    });
}

namespace {

auto ensure_double_newline(std::string text) -> std::string {
    while(text.ends_with('\n')) {
        text.pop_back();
    }
    text += "\n\n";
    return text;
}

auto prefix_blockquote_lines(std::string_view text) -> std::string {
    std::string prefixed;
    prefixed.reserve(text.size() + 2);

    std::size_t line_start = 0;
    while(true) {
        auto line_end = text.find('\n', line_start);
        prefixed += "> ";
        if(line_end == std::string_view::npos) {
            prefixed.append(text.substr(line_start));
            break;
        }

        prefixed.append(text.substr(line_start, line_end - line_start));
        prefixed.push_back('\n');
        line_start = line_end + 1;
    }

    return prefixed;
}

auto yaml_quote(std::string_view value) -> std::string {
    std::string out;
    out.reserve(value.size() + 2);
    out.push_back('\'');
    for(auto ch: value) {
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

auto render_code_span(std::string_view code) -> std::string {
    auto fence_size = std::size_t{1};
    auto run_size = std::size_t{0};
    for(auto ch: code) {
        if(ch == '`') {
            ++run_size;
            fence_size = std::max(fence_size, run_size + 1);
            continue;
        }
        run_size = 0;
    }

    auto fence = std::string(fence_size, '`');
    auto needs_padding = code.starts_with('`') || code.ends_with('`');
    std::string rendered;
    rendered.reserve(fence_size * 2 + code.size() + (needs_padding ? 2 : 0));
    rendered += fence;
    if(needs_padding) {
        rendered.push_back(' ');
    }
    rendered.append(code);
    if(needs_padding) {
        rendered.push_back(' ');
    }
    rendered += fence;
    return rendered;
}

auto render_inlines(const std::vector<InlineFragment>& fragments) -> std::string;

auto is_identifier_start(char ch) -> bool {
    return std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

auto is_identifier_continue(char ch) -> bool {
    return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

auto trim_code_candidate(std::string_view text) -> std::string_view {
    while(!text.empty() &&
          (text.back() == '.' || text.back() == ',' || text.back() == ';' || text.back() == ':')) {
        text.remove_suffix(1);
    }
    return text;
}

auto has_camel_case_boundary(std::string_view text) -> bool {
    for(std::size_t i = 1; i < text.size(); ++i) {
        if(std::islower(static_cast<unsigned char>(text[i - 1])) != 0 &&
           std::isupper(static_cast<unsigned char>(text[i])) != 0) {
            return true;
        }
        if(i + 1 < text.size() && std::isupper(static_cast<unsigned char>(text[i - 1])) != 0 &&
           std::isupper(static_cast<unsigned char>(text[i])) != 0 &&
           std::islower(static_cast<unsigned char>(text[i + 1])) != 0) {
            return true;
        }
    }
    return false;
}

auto has_identifier_separator(std::string_view text) -> bool {
    return text.find("::") != std::string_view::npos || text.find('_') != std::string_view::npos ||
           text.find("()") != std::string_view::npos || text.find('.') != std::string_view::npos;
}

auto is_code_candidate(std::string_view raw) -> bool {
    auto text = trim_code_candidate(raw);
    if(text.size() < 3) {
        return false;
    }
    if(text.starts_with('.') && text.find("()") != std::string_view::npos) {
        return true;
    }
    if(text.find('<') != std::string_view::npos || text.find('>') != std::string_view::npos) {
        return true;
    }
    if(text.starts_with("operator")) {
        return true;
    }
    if(has_identifier_separator(text)) {
        return true;
    }
    if(has_camel_case_boundary(text)) {
        return true;
    }
    if(text.size() > 2 && text.front() == 'k' &&
       std::isupper(static_cast<unsigned char>(text[1])) != 0) {
        return true;
    }
    return false;
}

auto read_angle_suffix(std::string_view text, std::size_t start) -> std::size_t {
    if(start >= text.size() || text[start] != '<') {
        return start;
    }

    auto depth = 0;
    auto pos = start;
    while(pos < text.size()) {
        auto ch = text[pos];
        if(ch == '<') {
            ++depth;
        } else if(ch == '>') {
            --depth;
            ++pos;
            if(depth == 0) {
                return pos;
            }
            continue;
        } else if(depth > 0) {
            auto allowed = is_identifier_continue(ch) || ch == ':' || ch == ',' || ch == ' ' ||
                           ch == '\t' || ch == '&' || ch == '*' || ch == '.' || ch == '<';
            if(!allowed) {
                return start;
            }
        }
        ++pos;
    }
    return start;
}

auto read_call_suffix(std::string_view text, std::size_t start) -> std::size_t {
    if(start + 1 >= text.size() || text[start] != '(' || text[start + 1] != ')') {
        return start;
    }
    return start + 2;
}

auto read_operator_token(std::string_view text, std::size_t start) -> std::size_t {
    constexpr std::string_view name = "operator";
    if(text.substr(start, name.size()) != name) {
        return start;
    }

    auto pos = start + name.size();
    while(pos < text.size()) {
        auto ch = text[pos];
        auto allowed = std::string_view{"<>=!+-*/%&|^~[],()"};
        if(allowed.find(ch) == std::string_view::npos) {
            break;
        }
        ++pos;
    }
    return pos;
}

auto read_code_candidate(std::string_view text, std::size_t start) -> std::size_t {
    auto pos = start;
    if(text[start] == '.') {
        ++pos;
        if(pos >= text.size() || !is_identifier_start(text[pos])) {
            return start;
        }
    }

    if(auto operator_end = read_operator_token(text, pos); operator_end != pos) {
        return operator_end;
    }

    while(pos < text.size()) {
        auto ch = text[pos];
        if(is_identifier_continue(ch) || ch == ':' || ch == '~') {
            ++pos;
            continue;
        }
        break;
    }

    pos = read_angle_suffix(text, pos);
    pos = read_call_suffix(text, pos);
    return pos;
}

auto append_text_fragment(std::vector<InlineFragment>& fragments, std::string text) -> void {
    if(text.empty()) {
        return;
    }
    if(!fragments.empty()) {
        if(auto* previous = std::get_if<TextFragment>(&fragments.back())) {
            previous->text += text;
            return;
        }
    }
    fragments.push_back(make_text(std::move(text)));
}

auto append_code_aware_text(std::vector<InlineFragment>& fragments, std::string_view text) -> void {
    std::size_t plain_start = 0;
    std::size_t pos = 0;
    while(pos < text.size()) {
        auto ch = text[pos];
        if(ch != '.' && !is_identifier_start(ch)) {
            ++pos;
            continue;
        }

        auto end = read_code_candidate(text, pos);
        if(end == pos) {
            ++pos;
            continue;
        }

        auto candidate = trim_code_candidate(text.substr(pos, end - pos));
        if(!is_code_candidate(candidate)) {
            pos = end;
            continue;
        }

        append_text_fragment(fragments, std::string(text.substr(plain_start, pos - plain_start)));
        fragments.push_back(make_code(std::string(candidate)));
        plain_start = pos + candidate.size();
        pos = plain_start;
    }
    append_text_fragment(fragments, std::string(text.substr(plain_start)));
}

auto read_backtick_span(std::string_view text, std::size_t start) -> std::size_t;

auto read_link_span(std::string_view text, std::size_t start) -> std::size_t;

auto append_existing_code_span(std::vector<InlineFragment>& fragments,
                               std::string_view text,
                               std::size_t start,
                               std::size_t end) -> void {
    auto fence_size = std::size_t{0};
    while(start + fence_size < end && text[start + fence_size] == '`') {
        ++fence_size;
    }
    if(end < start + fence_size * 2) {
        append_text_fragment(fragments, std::string(text.substr(start, end - start)));
        return;
    }
    auto content_start = start + fence_size;
    auto content_size = end - start - fence_size * 2;
    fragments.push_back(make_code(std::string(text.substr(content_start, content_size))));
}

auto append_code_spanned_fragments(std::vector<InlineFragment>& fragments, std::string_view text)
    -> void {
    std::size_t plain_start = 0;
    std::size_t pos = 0;
    while(pos < text.size()) {
        if(text[pos] == '`') {
            append_code_aware_text(fragments, text.substr(plain_start, pos - plain_start));
            auto end = read_backtick_span(text, pos);
            append_existing_code_span(fragments, text, pos, end);
            pos = end;
            plain_start = pos;
            continue;
        }
        if(text[pos] == '[' || text[pos] == '!') {
            auto end = read_link_span(text, pos);
            if(end != pos) {
                append_code_aware_text(fragments, text.substr(plain_start, pos - plain_start));
                append_text_fragment(fragments, std::string(text.substr(pos, end - pos)));
                pos = end;
                plain_start = pos;
                continue;
            }
        }
        ++pos;
    }
    append_code_aware_text(fragments, text.substr(plain_start));
}

auto append_rendered_text(std::string& rendered, std::string_view text) -> void {
    auto fragments = code_spanned_fragments(text);
    rendered += render_inlines(fragments);
}

auto read_backtick_span(std::string_view text, std::size_t start) -> std::size_t {
    auto fence_size = std::size_t{0};
    while(start + fence_size < text.size() && text[start + fence_size] == '`') {
        ++fence_size;
    }
    auto fence = std::string_view{text.data() + start, fence_size};
    auto closing = text.find(fence, start + fence_size);
    if(closing == std::string_view::npos) {
        return text.size();
    }
    return closing + fence_size;
}

auto read_link_span(std::string_view text, std::size_t start) -> std::size_t {
    auto label_start = start;
    if(text[start] == '!') {
        if(start + 1 >= text.size() || text[start + 1] != '[') {
            return start;
        }
        label_start = start + 1;
    }
    if(text[label_start] != '[') {
        return start;
    }

    auto label_end = text.find(']', label_start + 1);
    if(label_end == std::string_view::npos || label_end + 1 >= text.size() ||
       text[label_end + 1] != '(') {
        return start;
    }
    auto target_end = text.find(')', label_end + 2);
    if(target_end == std::string_view::npos) {
        return start;
    }
    return target_end + 1;
}

auto append_code_spanned_line(std::string& rendered, std::string_view line) -> void {
    std::size_t plain_start = 0;
    std::size_t pos = 0;
    while(pos < line.size()) {
        if(line[pos] == '`') {
            append_rendered_text(rendered, line.substr(plain_start, pos - plain_start));
            auto end = read_backtick_span(line, pos);
            rendered.append(line.substr(pos, end - pos));
            pos = end;
            plain_start = pos;
            continue;
        }
        if(line[pos] == '[' || line[pos] == '!') {
            auto end = read_link_span(line, pos);
            if(end != pos) {
                append_rendered_text(rendered, line.substr(plain_start, pos - plain_start));
                rendered.append(line.substr(pos, end - pos));
                pos = end;
                plain_start = pos;
                continue;
            }
        }
        ++pos;
    }
    append_rendered_text(rendered, line.substr(plain_start));
}

auto is_fence_line(std::string_view line) -> bool {
    auto first = line.find_first_not_of(" \t");
    if(first == std::string_view::npos) {
        return false;
    }
    return line.substr(first).starts_with("```") || line.substr(first).starts_with("~~~");
}

auto render_inline(const InlineFragment& fragment) -> std::string {
    return std::visit(
        [](const auto& value) -> std::string {
            using T = std::decay_t<decltype(value)>;
            if constexpr(std::is_same_v<T, TextFragment>) {
                return value.text;
            } else if constexpr(std::is_same_v<T, CodeFragment>) {
                return render_code_span(value.code);
            } else {
                auto label = value.code_style ? render_code_span(value.label) : value.label;
                return "[" + label + "](" + value.target + ")";
            }
        },
        fragment);
}

auto render_inlines(const std::vector<InlineFragment>& fragments) -> std::string {
    std::string rendered;
    for(const auto& fragment: fragments) {
        rendered += render_inline(fragment);
    }
    return rendered;
}

auto render_node(const MarkdownNode& node) -> std::string;

auto render_section(const SemanticSection& section) -> std::string {
    std::string children;
    for(const auto& child: section.children) {
        children += render_node(child);
    }

    if(children.empty() && section.omit_if_empty) {
        return {};
    }

    auto clamped_level = std::clamp<int>(static_cast<int>(section.level), 1, 6);
    auto heading_prefix = std::string(static_cast<std::size_t>(clamped_level), '#');
    auto heading = section.code_style_heading ? render_code_span(section.heading) : section.heading;
    std::string rendered = std::format("{} {}\n\n", heading_prefix, heading);
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
                for(const auto& item: value.items) {
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
            } else if constexpr(std::is_same_v<T, BlockQuote>) {
                if(value.fragments.empty()) {
                    return {};
                }
                return ensure_double_newline(
                    prefix_blockquote_lines(render_inlines(value.fragments)));
            } else if constexpr(std::is_same_v<T, RawMarkdown>) {
                if(value.markdown.empty()) {
                    return {};
                }
                return ensure_double_newline(code_spanned_markdown(value.markdown));
            } else {
                return value ? render_section(*value) : std::string{};
            }
        },
        node.value);
}

}  // namespace

auto code_spanned_fragments(std::string_view text) -> std::vector<InlineFragment> {
    std::vector<InlineFragment> fragments;
    append_code_spanned_fragments(fragments, text);
    return fragments;
}

auto code_spanned_markdown(std::string_view markdown) -> std::string {
    std::string rendered;
    rendered.reserve(markdown.size());

    auto in_fence = false;
    std::size_t line_start = 0;
    while(line_start < markdown.size()) {
        auto line_end = markdown.find('\n', line_start);
        auto has_newline = line_end != std::string_view::npos;
        auto line = has_newline ? markdown.substr(line_start, line_end - line_start)
                                : markdown.substr(line_start);

        if(is_fence_line(line)) {
            in_fence = !in_fence;
            rendered.append(line);
        } else if(in_fence) {
            rendered.append(line);
        } else {
            append_code_spanned_line(rendered, line);
        }

        if(!has_newline) {
            break;
        }
        rendered.push_back('\n');
        line_start = line_end + 1;
    }

    return rendered;
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

    for(const auto& child: document.children) {
        rendered += render_node(child);
    }

    return rendered;
}

}  // namespace clore::generate
