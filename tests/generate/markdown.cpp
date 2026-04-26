#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

TEST_SUITE(generate_markdown) {

TEST_CASE(render_markdown_clamps_heading_levels_and_prefixes_multiline_blockquotes) {
    MarkdownDocument document;
    auto section = make_section(SemanticKind::Section, "demo", "Deep Section", 9, false);
    section->children.push_back(make_blockquote("first line\nsecond line"));
    document.children.push_back(MarkdownNode{std::move(section)});

    auto rendered = render_markdown(document);

    EXPECT_NE(rendered.find("###### Deep Section\n\n"), std::string::npos);
    EXPECT_NE(rendered.find("> first line\n> second line\n\n"), std::string::npos);
    EXPECT_EQ(rendered.find("####### Deep Section"), std::string::npos);
}

TEST_CASE(render_markdown_wraps_code_style_headings_and_links) {
    MarkdownDocument document;
    auto section = make_section(SemanticKind::Function,
                                "demo::value",
                                "demo::value<std::vector<int>>",
                                2,
                                false,
                                true);
    Paragraph paragraph;
    paragraph.fragments.push_back(make_link("demo::value<std::vector<int>>", "value.md", true));
    section->children.push_back(MarkdownNode{std::move(paragraph)});
    document.children.push_back(MarkdownNode{std::move(section)});

    auto rendered = render_markdown(document);

    EXPECT_NE(rendered.find("## `demo::value<std::vector<int>>`\n\n"), std::string::npos);
    EXPECT_NE(rendered.find("[`demo::value<std::vector<int>>`](value.md)"), std::string::npos);
}

TEST_CASE(code_spanned_fragments_wraps_code_tokens_in_plain_text) {
    auto fragments = code_spanned_fragments(
        "Calls call_llm_async with std::vector<Message>, LLMError, and .or_fail().");
    Paragraph paragraph{.fragments = std::move(fragments)};
    MarkdownDocument document;
    document.children.push_back(MarkdownNode{std::move(paragraph)});

    auto rendered = render_markdown(document);

    EXPECT_NE(
        rendered.find(
            "Calls `call_llm_async` with `std::vector<Message>`, `LLMError`, and `.or_fail()`."),
        std::string::npos);
    EXPECT_EQ(rendered.find("std::vector<Message> and"), std::string::npos);
}

TEST_CASE(code_spanned_markdown_preserves_existing_markup_and_code_fences) {
    auto rendered = code_spanned_markdown(
        "Use [the API](https://example.test/std::vector<T>) with call_llm_async.\n"
        "Existing `std::span<T>` stays wrapped.\n"
        "```cpp\n"
        "std::vector<Message> values;\n"
        "```\n");

    EXPECT_NE(rendered.find("[the API](https://example.test/std::vector<T>)"), std::string::npos);
    EXPECT_NE(rendered.find("with `call_llm_async`."), std::string::npos);
    EXPECT_NE(rendered.find("Existing `std::span<T>` stays wrapped."), std::string::npos);
    EXPECT_NE(rendered.find("std::vector<Message> values;"), std::string::npos);
    EXPECT_EQ(rendered.find("``std::span<T>``"), std::string::npos);
}

TEST_CASE(format_evidence_text_renders_source_snippets_without_code_fences) {
    EvidencePack pack;
    pack.source_snippets = {
        "int x = 1;\n",
        "const char* s = \"```\";\n",
    };

    auto text = format_evidence_text(pack);

    EXPECT_NE(text.find("### Source Snippets\n"), std::string::npos);
    EXPECT_NE(text.find("    int x = 1;"), std::string::npos);
    EXPECT_NE(text.find("    const char* s = \"``\\`\";"), std::string::npos);
    EXPECT_EQ(text.find("```"), std::string::npos);
}

};  // TEST_SUITE(generate_markdown)
