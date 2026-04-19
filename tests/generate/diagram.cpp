#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

using namespace clore::generate;

TEST_SUITE(generate_diagram) {

TEST_CASE(should_emit_mermaid_false_for_small_graphs) {
    EXPECT_FALSE(should_emit_mermaid(3, 2));
    EXPECT_TRUE(should_emit_mermaid(10, 5));
}

TEST_CASE(escape_mermaid_label_escapes_quotes) {
    EXPECT_EQ(escape_mermaid_label("foo \"bar\""), "foo \\\"bar\\\"");
    EXPECT_EQ(escape_mermaid_label("a#b"), "a#b");
}

TEST_CASE(render_namespace_diagram_code_empty_namespace) {
    extract::ProjectModel model;
    auto result = render_namespace_diagram_code(model, "nonexistent");
    EXPECT_TRUE(result.empty());
}

TEST_CASE(render_module_dependency_diagram_code_empty_model) {
    extract::ProjectModel model;
    auto result = render_module_dependency_diagram_code(model);
    EXPECT_TRUE(result.empty());
}

TEST_CASE(render_module_dependency_diagram_code_recomputes_when_model_changes) {
    extract::ProjectModel model;

    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.alpha",
                   .is_interface = true,
                   .source_file = "src/alpha.cppm",
                   .imports = {"demo.beta"},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.beta",
                   .is_interface = true,
                   .source_file = "src/beta.cppm",
                   .imports = {},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.gamma",
                   .is_interface = true,
                   .source_file = "src/gamma.cppm",
                   .imports = {"demo.beta"},
                   .symbols = {},
               });
    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.delta",
                   .is_interface = true,
                   .source_file = "src/delta.cppm",
                   .imports = {},
                   .symbols = {},
               });

    auto first = render_module_dependency_diagram_code(model);
    ASSERT_FALSE(first.empty());
    EXPECT_EQ(first.find("demo.epsilon"), std::string::npos);

    add_module(model,
               extract::ModuleUnit{
                   .name = "demo.epsilon",
                   .is_interface = true,
                   .source_file = "src/epsilon.cppm",
                   .imports = {"demo.alpha"},
                   .symbols = {},
               });

    auto second = render_module_dependency_diagram_code(model);
    EXPECT_NE(second.find("demo.epsilon"), std::string::npos);
    EXPECT_NE(first, second);
}

};  // TEST_SUITE(generate_diagram)
