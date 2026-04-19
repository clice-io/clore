#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

TEST_SUITE(generate_evidence) {

TEST_CASE(function_declaration_evidence_uses_signature_aware_related_summaries) {
    ScopedTempDir temp("function_summary_cache_keys");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "overloads.cpp").generic_string();

    auto caller_int =
        make_function_symbol(700, "caller", "demo::caller", "int caller(int value)", file, "demo");
    auto caller_int_id = caller_int.id;
    auto caller_int_key = make_symbol_target_key(caller_int);

    auto caller_double = make_function_symbol(701,
                                              "caller",
                                              "demo::caller",
                                              "double caller(double value)",
                                              file,
                                              "demo");
    auto caller_double_id = caller_double.id;
    auto caller_double_key = make_symbol_target_key(caller_double);

    auto target =
        make_function_symbol(702, "target", "demo::target", "void target()", file, "demo");
    target.called_by = {caller_int_id, caller_double_id};

    add_symbol(model, std::move(caller_int));
    add_symbol(model, std::move(caller_double));
    add_symbol(model, std::move(target));
    add_namespace(model, "demo", {caller_int_id, caller_double_id, extract::SymbolID{.hash = 702}});

    SymbolAnalysisStore analyses;
    analyses.functions[caller_int_key] = FunctionAnalysis{
        .overview_markdown = "int overload summary",
        .details_markdown = "",
        .side_effects = {},
        .reads_from = {},
        .writes_to = {},
        .usage_patterns = {},
    };
    analyses.functions[caller_double_key] = FunctionAnalysis{
        .overview_markdown = "double overload summary",
        .details_markdown = "",
        .side_effects = {},
        .reads_from = {},
        .writes_to = {},
        .usage_patterns = {},
    };

    auto* symbol = extract::find_symbol(model, "demo::target", "void target()");
    ASSERT_TRUE(symbol != nullptr);

    auto evidence = build_evidence_for_function_declaration_summary(*symbol,
                                                                    model,
                                                                    analyses,
                                                                    config.project_root);

    ASSERT_EQ(evidence.related_page_summaries.size(), 2u);
    EXPECT_TRUE(std::ranges::find(evidence.related_page_summaries, "int overload summary") !=
                evidence.related_page_summaries.end());
    EXPECT_TRUE(std::ranges::find(evidence.related_page_summaries, "double overload summary") !=
                evidence.related_page_summaries.end());
}

TEST_CASE(type_declaration_evidence_uses_signature_aware_related_summaries) {
    ScopedTempDir temp("type_summary_cache_keys");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    auto file = (temp.path / "src" / "types.cpp").generic_string();

    auto base = make_type_symbol(710, "Base", "demo::Base", file, "Base type.", "demo");
    auto base_id = base.id;
    auto base_key = make_symbol_target_key(base);

    auto derived = make_type_symbol(711, "Derived", "demo::Derived", file, "Derived type.", "demo");
    derived.bases = {base_id};

    add_symbol(model, std::move(base));
    add_symbol(model, std::move(derived));
    add_namespace(model, "demo", {base_id, extract::SymbolID{.hash = 711}});

    SymbolAnalysisStore analyses;
    analyses.types[base_key] = TypeAnalysis{
        .overview_markdown = "base type summary",
        .details_markdown = "",
        .invariants = {},
        .key_members = {},
        .usage_patterns = {},
    };

    auto* symbol = extract::find_symbol(model, "demo::Derived", "struct Derived");
    ASSERT_TRUE(symbol != nullptr);

    auto evidence =
        build_evidence_for_type_declaration_summary(*symbol, model, analyses, config.project_root);

    ASSERT_EQ(evidence.related_page_summaries.size(), 1u);
    EXPECT_EQ(evidence.related_page_summaries[0], "base type summary");
}

TEST_CASE(module_architecture_evidence_combines_symbol_and_module_summaries) {
    ScopedTempDir temp("module_architecture_summary_sources");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;

    auto util_file = (temp.path / "src" / "util.cppm").generic_string();
    auto math_file = (temp.path / "src" / "math.cppm").generic_string();

    auto api = make_type_symbol(715, "Widget", "demo::Widget", math_file, "Widget type.", "demo");
    auto api_id = api.id;
    auto api_key = make_symbol_target_key(api);
    add_symbol(model, std::move(api));

    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.util",
                   .is_interface = true,
                   .source_file = util_file,
                   .imports = {},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.math",
                   .is_interface = true,
                   .source_file = math_file,
                   .imports = {"demo.util"},
                   .symbols = {api_id},
               });

    SymbolAnalysisStore analyses;
    analyses.types[api_key] = TypeAnalysis{
        .overview_markdown = "widget summary",
        .details_markdown = "",
        .invariants = {},
        .key_members = {},
        .usage_patterns = {},
    };
    PageSummaryCache page_summaries{
        {"demo.util", "imported module summary"}
    };

    auto* module = extract::find_module_by_name(model, "demo.math");
    ASSERT_TRUE(module != nullptr);

    auto evidence = build_evidence_for_module_architecture(*module,
                                                           model,
                                                           analyses,
                                                           page_summaries,
                                                           config.project_root);

    EXPECT_TRUE(std::ranges::find(evidence.related_page_summaries, "widget summary") !=
                evidence.related_page_summaries.end());
    EXPECT_TRUE(std::ranges::find(evidence.related_page_summaries, "imported module summary") !=
                evidence.related_page_summaries.end());
}

TEST_CASE(function_implementation_preserves_source_snippet) {
    ScopedTempDir temp("snippet_truncation_budget");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "trim.cpp").generic_string();
    auto target = make_function_symbol(720, "trim", "demo::trim", "void trim()", file, "demo");
    target.source_snippet = "int trim() { return 0; }";

    add_symbol(model, target);
    add_namespace(model, "demo", {target.id});

    auto evidence =
        build_evidence_for_function_implementation_summary(target, model, config.project_root);

    ASSERT_EQ(evidence.source_snippets.size(), 1u);
    EXPECT_EQ(evidence.source_snippets[0], "int trim() { return 0; }");
}

TEST_CASE(namespace_summary_prompt_uses_namespace_subject) {
    ScopedTempDir temp("namespace_summary_prompt");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);
    extract::ProjectModel model;
    extract::NamespaceInfo ns;
    ns.name = "demo::config";

    auto file = (temp.path / "src" / "config.cppm").generic_string();
    for(std::uint64_t i = 0; i < 3; ++i) {
        auto name = std::format("Type{}", i);
        auto qualified_name = std::format("demo::config::{}", name);
        auto symbol = make_type_symbol(100 + i,
                                       name,
                                       qualified_name,
                                       file,
                                       "Type documentation.",
                                       "demo::config");
        ns.symbols.push_back(symbol.id);
        add_symbol(model, std::move(symbol));
    }
    model.namespaces.emplace(ns.name, ns);

    SymbolAnalysisStore analyses;
    auto evidence = build_evidence_for_namespace_summary(model.namespaces.at(ns.name),
                                                         model,
                                                         analyses,
                                                         config.project_root);
    EXPECT_EQ(evidence.subject_name, "demo::config");
    EXPECT_EQ(evidence.subject_kind, "namespace");

    auto prompt = build_prompt(PromptKind::NamespaceSummary, evidence);

    ASSERT_TRUE(prompt.has_value());
    EXPECT_NE(prompt->find("Write a summary for the namespace `demo::config`."), std::string::npos);
}

TEST_CASE(function_declaration_summary_deduplicates_reverse_usage_context) {
    ScopedTempDir temp("function_reverse_usage");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "demo.cpp").generic_string();

    auto target =
        make_function_symbol(900, "target", "demo::target", "void target()", file, "demo");
    auto caller_a =
        make_function_symbol(901, "callerA", "demo::callerA", "void callerA()", file, "demo");
    auto caller_b =
        make_function_symbol(902, "callerB", "demo::callerB", "void callerB()", file, "demo");
    auto ref_only =
        make_function_symbol(903, "refOnly", "demo::refOnly", "void refOnly()", file, "demo");

    target.called_by = {caller_a.id, caller_b.id};
    target.referenced_by = {caller_b.id, ref_only.id};

    add_symbol(model, target);
    add_symbol(model, caller_a);
    add_symbol(model, caller_b);
    add_symbol(model, ref_only);
    add_namespace(model, "demo", {target.id, caller_a.id, caller_b.id, ref_only.id});

    SymbolAnalysisStore analyses;
    auto evidence = build_evidence_for_function_declaration_summary(target,
                                                                    model,
                                                                    analyses,
                                                                    config.project_root);

    ASSERT_EQ(evidence.reverse_usage_context.size(), 3u);
    EXPECT_EQ(evidence.reverse_usage_context[0].qualified_name, "demo::callerA");
    EXPECT_EQ(evidence.reverse_usage_context[1].qualified_name, "demo::callerB");
    EXPECT_EQ(evidence.reverse_usage_context[2].qualified_name, "demo::refOnly");
}

TEST_CASE(type_declaration_summary_deduplicates_reverse_usage_context) {
    ScopedTempDir temp("type_reverse_usage");
    fs::create_directories(temp.path / "src");

    auto config = make_config(temp.path);

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "widget.cpp").generic_string();

    auto target = make_type_symbol(910, "Widget", "demo::Widget", file, "Widget doc.", "demo");
    auto use_a = make_function_symbol(911, "useA", "demo::useA", "void useA()", file, "demo");
    auto use_b = make_function_symbol(912, "useB", "demo::useB", "void useB()", file, "demo");
    auto derived = make_type_symbol(913, "Derived", "demo::Derived", file, "Derived doc.", "demo");

    target.called_by = {use_a.id};
    target.referenced_by = {use_a.id, derived.id};
    target.derived = {derived.id, use_b.id};

    add_symbol(model, target);
    add_symbol(model, use_a);
    add_symbol(model, use_b);
    add_symbol(model, derived);
    add_namespace(model, "demo", {target.id, use_a.id, use_b.id, derived.id});

    SymbolAnalysisStore analyses;
    auto evidence =
        build_evidence_for_type_declaration_summary(target, model, analyses, config.project_root);

    ASSERT_EQ(evidence.reverse_usage_context.size(), 3u);
    EXPECT_EQ(evidence.reverse_usage_context[0].qualified_name, "demo::useA");
    EXPECT_EQ(evidence.reverse_usage_context[1].qualified_name, "demo::Derived");
    EXPECT_EQ(evidence.reverse_usage_context[2].qualified_name, "demo::useB");
}

};  // TEST_SUITE(generate_evidence)
