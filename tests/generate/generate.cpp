#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

TEST_SUITE(generate_runtime) {

TEST_CASE(generate_dry_run_embeds_prompt_outputs) {
    ScopedTempDir temp("dry_run_prompts");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    auto model = make_basic_model(temp.path);

    auto result = generate_dry_run(config, model);

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(std::ranges::any_of(*result, [](const GeneratedPage& page) {
        return page.content.find("> Prompt (`") != std::string::npos;
    }));
}

TEST_CASE(generate_dry_run_emits_llms_txt) {
    ScopedTempDir temp("dry_run_llms");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    auto model = make_basic_model(temp.path);

    auto result = generate_dry_run(config, model);

    ASSERT_TRUE(result.has_value());
    auto llms_page = std::ranges::find_if(*result, [](const GeneratedPage& page) {
        return page.relative_path == "llms.txt";
    });
    ASSERT_TRUE(llms_page != result->end());
    EXPECT_NE(llms_page->content.find("[API Reference](index.md)"), std::string::npos);
    EXPECT_NE(llms_page->content.find("[math](namespaces/math/index.md)"), std::string::npos);
    EXPECT_NE(llms_page->content.find("[src/math.cpp](files/src/math.md)"), std::string::npos);
    EXPECT_NE(llms_page->content.find("[Dry Run Request Estimate](request-estimate.md)"),
              std::string::npos);
}

TEST_CASE(generate_dry_run_emits_request_estimate_page) {
    ScopedTempDir temp("dry_run_request_estimate");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    auto model = make_basic_model(temp.path);

    auto result = generate_dry_run(config, model);

    ASSERT_TRUE(result.has_value());
    auto estimate_page = std::ranges::find_if(*result, [](const GeneratedPage& page) {
        return page.relative_path == "request-estimate.md";
    });
    ASSERT_TRUE(estimate_page != result->end());
    EXPECT_NE(estimate_page->content.find("Total prompt tasks: 5"), std::string::npos);
    EXPECT_NE(estimate_page->content.find("Page prompt tasks: 2"), std::string::npos);
    EXPECT_NE(estimate_page->content.find("Symbol prompt tasks: 3"), std::string::npos);
    EXPECT_NE(estimate_page->content.find("Function symbols: 1 -> 3 requests"), std::string::npos);
}

TEST_CASE(write_pages_writes_relative_output_paths) {
    ScopedTempDir temp("write_pages");
    auto output_root = temp.path / "docs";

    std::vector<GeneratedPage> pages{
        GeneratedPage{
                      .relative_path = "src/math.md",
                      .content = "# Math\n",
                      },
    };

    auto result = write_pages(pages, output_root.generic_string());

    ASSERT_TRUE(result.has_value());

    auto target = output_root / "src" / "math.md";
    ASSERT_TRUE(fs::exists(target));
    EXPECT_EQ(read_text_file(target), "# Math\n");
}

TEST_CASE(write_pages_generates_directory_indexes) {
    ScopedTempDir temp("write_directory_indexes");
    auto output_root = temp.path / "docs";

    std::vector<GeneratedPage> pages{
        GeneratedPage{
                      .title = "API Reference",
                      .relative_path = "index.md",
                      .content = "# API Reference\n",
                      },
        GeneratedPage{
                      .title = "File `src/math.cpp`",
                      .relative_path = "files/src/math.md",
                      .content = "# Math\n",
                      },
    };

    auto result = write_pages(pages, output_root.generic_string());

    ASSERT_TRUE(result.has_value());

    auto files_index = output_root / "files" / "index.md";
    auto src_index = output_root / "files" / "src" / "index.md";
    ASSERT_TRUE(fs::exists(files_index));
    ASSERT_TRUE(fs::exists(src_index));
    EXPECT_NE(read_text_file(files_index).find("[Directory `files/src`](src/index.md)"),
              std::string::npos);
    EXPECT_NE(read_text_file(src_index).find("[File `src/math.cpp`](math.md)"), std::string::npos);
}

TEST_CASE(declaration_summary_prompt_rebuild_picks_up_completed_related_analyses) {
    ScopedTempDir temp("declaration_summary_prompt_rebuild");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    config.output_root = (temp.path / "docs").generic_string();
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "demo.cpp").generic_string();

    auto caller =
        make_function_symbol(920, "caller", "demo::caller", "void caller()", file, "demo");
    auto target =
        make_function_symbol(921, "target", "demo::target", "void target()", file, "demo");
    target.called_by = {caller.id};

    auto caller_key = make_symbol_target_key(caller);
    add_symbol(model, std::move(caller));
    add_symbol(model, std::move(target));

    auto* stored_target = extract::find_symbol(model, "demo::target", "void target()");
    auto* stored_caller = extract::find_symbol(model, "demo::caller", "void caller()");
    ASSERT_TRUE(stored_target != nullptr);
    ASSERT_TRUE(stored_caller != nullptr);

    SymbolAnalysisStore analyses_with_base;
    analyses_with_base.functions[caller_key] = FunctionAnalysis{
        .overview_markdown = "caller analysis summary",
        .details_markdown = "caller analysis details",
        .side_effects = {},
        .reads_from = {},
        .writes_to = {},
        .usage_patterns = {},
    };

    auto fallback_evidence = build_evidence_for_function_declaration_summary(*stored_target,
                                                                             model,
                                                                             SymbolAnalysisStore{},
                                                                             config.project_root);
    auto fallback_prompt = build_prompt(PromptKind::FunctionDeclarationSummary, fallback_evidence);
    ASSERT_TRUE(fallback_prompt.has_value());
    EXPECT_EQ(fallback_prompt->find("caller analysis summary"), std::string::npos);

    auto completed_evidence = build_evidence_for_function_declaration_summary(*stored_target,
                                                                              model,
                                                                              analyses_with_base,
                                                                              config.project_root);
    auto rebuilt_prompt = build_prompt(PromptKind::FunctionDeclarationSummary, completed_evidence);
    ASSERT_TRUE(rebuilt_prompt.has_value());
    EXPECT_NE(rebuilt_prompt->find("caller analysis summary"), std::string::npos);
}

};  // TEST_SUITE(generate_runtime)
