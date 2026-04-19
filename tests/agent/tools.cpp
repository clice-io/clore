#include "generate/prelude.h"
#include "kota/codec/json/json.h"

import config;
import extract;
import generate;
import agent;

#include "generate/support.h"

namespace {

namespace json = kota::codec::json;

auto parse_json_value(std::string_view text) -> std::expected<json::Value, std::string> {
    auto parsed = json::Value::parse(text);
    if(!parsed.has_value()) {
        return std::unexpected(
            std::string(json::error_message(json::make_read_error(parsed.error()))));
    }
    return *parsed;
}

template <typename ToolRange>
auto has_tool_named(const ToolRange& tools, std::string_view name) -> bool {
    return std::ranges::any_of(tools, [name](const auto& tool) { return tool.name == name; });
}

}  // namespace

TEST_SUITE(agent_tools) {

TEST_CASE(extract_string_arg_validates_object_fields) {
    auto valid = parse_json_value(R"({"name":"math"})");
    auto invalid_type = parse_json_value(R"({"name":42})");
    auto invalid_shape = parse_json_value(R"(["math"])");

    ASSERT_TRUE(valid.has_value());
    ASSERT_TRUE(invalid_type.has_value());
    ASSERT_TRUE(invalid_shape.has_value());

    auto extracted = clore::agent::extract_string_arg(*valid, "name");
    auto type_error = clore::agent::extract_string_arg(*invalid_type, "name");
    auto shape_error = clore::agent::extract_string_arg(*invalid_shape, "name");

    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(*extracted, "math");
    EXPECT_FALSE(type_error.has_value());
    EXPECT_FALSE(shape_error.has_value());
    EXPECT_EQ(type_error.error().message, "field 'name' is not a string");
    EXPECT_EQ(shape_error.error().message, "arguments is not an object");
}

TEST_CASE(build_tool_definitions_exposes_agent_surface) {
    auto tools = clore::agent::build_tool_definitions();

    ASSERT_TRUE(tools.has_value());
    EXPECT_TRUE(has_tool_named(*tools, "project_overview"));
    EXPECT_TRUE(has_tool_named(*tools, "get_symbol"));
    EXPECT_TRUE(has_tool_named(*tools, "create_guide"));
    EXPECT_GE(tools->size(), 10u);
}

TEST_CASE(dispatch_tool_call_reports_overview_and_file_symbols) {
    ScopedTempDir temp("agent_tools_overview");
    fs::create_directories(temp.path / "src");

    auto model = make_basic_model(temp.path);
    add_module(model,
               extract::ModuleUnit{
                   .name = "math",
                   .is_interface = true,
                   .source_file = (temp.path / "src" / "math.cpp").generic_string(),
                   .imports = {},
                   .symbols = {extract::SymbolID{.hash = 1}},
               });

    auto empty_args = parse_json_value(R"({})");
    auto file_args = parse_json_value(R"({"name":"src/math.cpp"})");
    ASSERT_TRUE(empty_args.has_value());
    ASSERT_TRUE(file_args.has_value());

    auto overview = clore::agent::dispatch_tool_call("project_overview",
                                                     *empty_args,
                                                     model,
                                                     temp.path.generic_string(),
                                                     temp.path.generic_string());
    auto file_symbols = clore::agent::dispatch_tool_call("get_file_symbols",
                                                         *file_args,
                                                         model,
                                                         temp.path.generic_string(),
                                                         temp.path.generic_string());

    ASSERT_TRUE(overview.has_value());
    ASSERT_TRUE(file_symbols.has_value());
    EXPECT_NE(overview->find("Uses C++20 modules: yes"), std::string::npos);
    EXPECT_NE(file_symbols->find("File: src/math.cpp"), std::string::npos);
    EXPECT_NE(file_symbols->find("math::add"), std::string::npos);
}

TEST_CASE(dispatch_tool_call_reports_symbol_relationships) {
    ScopedTempDir temp("agent_tools_symbol");
    fs::create_directories(temp.path / "src");

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "demo.cpp").generic_string();

    auto callee =
        make_function_symbol(401, "callee", "demo::callee", "void callee()", file, "demo");
    auto caller =
        make_function_symbol(402, "caller", "demo::caller", "void caller()", file, "demo");
    caller.calls.push_back(callee.id);

    add_symbol(model, std::move(callee));
    add_symbol(model, std::move(caller));
    add_namespace(model, "demo", {extract::SymbolID{.hash = 401}, extract::SymbolID{.hash = 402}});

    auto symbol_args = parse_json_value(R"({"name":"demo::caller"})");
    ASSERT_TRUE(symbol_args.has_value());

    auto symbol = clore::agent::dispatch_tool_call("get_symbol",
                                                   *symbol_args,
                                                   model,
                                                   temp.path.generic_string(),
                                                   temp.path.generic_string());
    auto deps = clore::agent::dispatch_tool_call("get_dependencies",
                                                 *symbol_args,
                                                 model,
                                                 temp.path.generic_string(),
                                                 temp.path.generic_string());

    ASSERT_TRUE(symbol.has_value());
    ASSERT_TRUE(deps.has_value());
    EXPECT_NE(symbol->find("qualified_name: demo::caller"), std::string::npos);
    EXPECT_NE(symbol->find("calls (1):"), std::string::npos);
    EXPECT_NE(symbol->find("demo::callee"), std::string::npos);
    EXPECT_NE(deps->find("Dependencies of `demo::caller`"), std::string::npos);
    EXPECT_NE(deps->find("demo::callee"), std::string::npos);
}

TEST_CASE(dispatch_tool_call_get_dependencies_requires_signature_for_overloads) {
    ScopedTempDir temp("agent_tools_overload_dependencies");
    fs::create_directories(temp.path / "src");

    extract::ProjectModel model;
    auto file = (temp.path / "src" / "demo.cpp").generic_string();

    auto callee_int = make_function_symbol(410,
                                           "handle_int",
                                           "demo::handle_int",
                                           "void handle_int()",
                                           file,
                                           "demo");
    auto callee_int_id = callee_int.id;
    auto callee_double = make_function_symbol(411,
                                              "handle_double",
                                              "demo::handle_double",
                                              "void handle_double()",
                                              file,
                                              "demo");
    auto callee_double_id = callee_double.id;

    auto overload_int =
        make_function_symbol(412, "foo", "demo::foo", "int foo(int value)", file, "demo");
    overload_int.calls.push_back(callee_int_id);
    auto overload_int_id = overload_int.id;

    auto overload_double =
        make_function_symbol(413, "foo", "demo::foo", "double foo(double value)", file, "demo");
    overload_double.calls.push_back(callee_double_id);
    auto overload_double_id = overload_double.id;

    add_symbol(model, std::move(callee_int));
    add_symbol(model, std::move(callee_double));
    add_symbol(model, std::move(overload_int));
    add_symbol(model, std::move(overload_double));
    add_namespace(model,
                  "demo",
                  {callee_int_id, callee_double_id, overload_int_id, overload_double_id});

    auto ambiguous_args = parse_json_value(R"({"name":"demo::foo"})");
    auto exact_args =
        parse_json_value(R"json({"name":"demo::foo","signature":"double foo(double value)"})json");
    ASSERT_TRUE(ambiguous_args.has_value());
    ASSERT_TRUE(exact_args.has_value());

    auto ambiguous = clore::agent::dispatch_tool_call("get_dependencies",
                                                      *ambiguous_args,
                                                      model,
                                                      temp.path.generic_string(),
                                                      temp.path.generic_string());
    auto exact = clore::agent::dispatch_tool_call("get_dependencies",
                                                  *exact_args,
                                                  model,
                                                  temp.path.generic_string(),
                                                  temp.path.generic_string());

    ASSERT_TRUE(ambiguous.has_value());
    ASSERT_TRUE(exact.has_value());
    EXPECT_NE(ambiguous->find("Symbol 'demo::foo' is overloaded; provide the exact signature."),
              std::string::npos);
    EXPECT_NE(ambiguous->find("int foo(int value)"), std::string::npos);
    EXPECT_NE(ambiguous->find("double foo(double value)"), std::string::npos);
    EXPECT_NE(exact->find("Dependencies of `demo::foo` (double foo(double value))"),
              std::string::npos);
    EXPECT_NE(exact->find("demo::handle_double"), std::string::npos);
    EXPECT_EQ(exact->find("demo::handle_int"), std::string::npos);
}

TEST_CASE(create_guide_dispatch_writes_file) {
    ScopedTempDir temp("agent_tools_guide");

    auto args = parse_json_value(
        R"({"name":"architecture-overview","title":"Architecture","content":"# Guide"})");
    ASSERT_TRUE(args.has_value());

    auto result = clore::agent::dispatch_tool_call("create_guide",
                                                   *args,
                                                   extract::ProjectModel{},
                                                   temp.path.generic_string(),
                                                   temp.path.generic_string());

    ASSERT_TRUE(result.has_value());
    EXPECT_NE(result->find("Guide 'Architecture' created successfully"), std::string::npos);

    auto guide_path = temp.path / "guides" / "architecture-overview.md";
    EXPECT_TRUE(fs::exists(guide_path));
}

};  // TEST_SUITE(agent_tools)
