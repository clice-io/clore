#include "eventide/zest/zest.h"

#include <filesystem>
#include <fstream>

import clore.extract;

using namespace clore::extract;

TEST_SUITE(ast_extract) {
    TEST_CASE(extract_from_simple_source) {
        namespace fs = std::filesystem;

        auto temp_dir = fs::temp_directory_path() / "clore_test_ast";
        fs::create_directories(temp_dir);
        auto source_path = temp_dir / "test.cpp";

        {
            std::ofstream f(source_path);
            f << R"(
namespace mylib {

/// A simple class for testing.
class Widget {
public:
    int value;
    void update(int v);
};

enum class Color {
    Red,
    Green,
    Blue,
};

int global_func(int x, int y);

using IntAlias = int;

}  // namespace mylib
)";
        }

        CompileEntry entry;
        entry.file = source_path.string();
        entry.directory = temp_dir.string();
        entry.arguments = {"clang++", "-std=c++23", "-c", source_path.string()};

        auto result = extract_symbols(entry, 2048);
        ASSERT_TRUE(result.has_value());

        auto& symbols = result->symbols;
        EXPECT_GT(symbols.size(), 0u);

        // Check that we find the expected symbol kinds
        bool found_namespace = false;
        bool found_class = false;
        bool found_enum = false;
        bool found_function = false;
        bool found_field = false;

        for(auto& sym : symbols) {
            if(sym.name == "mylib" && sym.kind == SymbolKind::Namespace)
                found_namespace = true;
            if(sym.name == "Widget" && (sym.kind == SymbolKind::Class || sym.kind == SymbolKind::Struct))
                found_class = true;
            if(sym.name == "Color" && sym.kind == SymbolKind::Enum)
                found_enum = true;
            if(sym.name == "global_func" && sym.kind == SymbolKind::Function)
                found_function = true;
            if(sym.name == "value" && sym.kind == SymbolKind::Field)
                found_field = true;
        }

        EXPECT_TRUE(found_namespace);
        EXPECT_TRUE(found_class);
        EXPECT_TRUE(found_enum);
        EXPECT_TRUE(found_function);
        EXPECT_TRUE(found_field);

        fs::remove_all(temp_dir);
    }

    TEST_CASE(extract_with_inheritance) {
        namespace fs = std::filesystem;

        auto temp_dir = fs::temp_directory_path() / "clore_test_ast_inherit";
        fs::create_directories(temp_dir);
        auto source_path = temp_dir / "inherit.cpp";

        {
            std::ofstream f(source_path);
            f << R"(
struct Base {
    virtual void foo();
};

struct Derived : Base {
    void foo() override;
};
)";
        }

        CompileEntry entry;
        entry.file = source_path.string();
        entry.directory = temp_dir.string();
        entry.arguments = {"clang++", "-std=c++23", "-c", source_path.string()};

        auto result = extract_symbols(entry, 2048);
        ASSERT_TRUE(result.has_value());

        auto& symbols = result->symbols;

        // Find Derived and check it has a base
        bool found_derived_with_base = false;
        for(auto& sym : symbols) {
            if(sym.name == "Derived" && !sym.bases.empty()) {
                found_derived_with_base = true;
            }
        }
        EXPECT_TRUE(found_derived_with_base);

        fs::remove_all(temp_dir);
    }

    TEST_CASE(extract_with_template) {
        namespace fs = std::filesystem;

        auto temp_dir = fs::temp_directory_path() / "clore_test_ast_template";
        fs::create_directories(temp_dir);
        auto source_path = temp_dir / "template.cpp";

        {
            std::ofstream f(source_path);
            f << R"(
template <typename T>
T identity(T value) { return value; }

template <typename T, int N>
struct Array {
    T data[N];
};
)";
        }

        CompileEntry entry;
        entry.file = source_path.string();
        entry.directory = temp_dir.string();
        entry.arguments = {"clang++", "-std=c++23", "-c", source_path.string()};

        auto result = extract_symbols(entry, 2048);
        ASSERT_TRUE(result.has_value());

        auto& symbols = result->symbols;
        EXPECT_GT(symbols.size(), 0u);

        fs::remove_all(temp_dir);
    }

    TEST_CASE(extract_empty_arguments) {
        CompileEntry entry;
        entry.file = "test.cpp";
        entry.directory = "/tmp";

        auto result = extract_symbols(entry, 2048);
        EXPECT_FALSE(result.has_value());
    }
};
