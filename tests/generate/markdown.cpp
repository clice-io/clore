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
