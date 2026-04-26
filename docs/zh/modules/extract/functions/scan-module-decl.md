---
title: 'clore::extract::scanmoduledecl'
description: '函数 clore::extract::scan_module_decl 利用 Clang 的依赖指令扫描器（clang::scanSourceForDependencyDirectives）对给定的文件内容进行快速模块声明分析，避免运行完整预处理器。内部首先将源文本解析为 tokens 和 directives 向量；若扫描失败则直接返回，不修改 ScanResult。随后遍历每个指令：对于 cxx_export_module_decl 或 cxx_module_decl 类型的指令，跳过 export 和 module 关键字，通过收集后续标识符（排除空白和纯标点）构建模块名称，并据此设置 result.module_name 和 result.is_interface_unit（仅 export module 表示接口单元）；对于 cxx_import_decl 类型的指令，跳过 import 关键字后收集导入名称，调用 normalize_partition_import 进行规范化，并在去重后加入 result.module_imports。该函数依赖 ScanResult 结构体字段和 normalize_partition_import 辅助函数，同时借助两个内联 lambda（is_whitespace_only 和 is_punctuation_only）辅助令牌分类。'
layout: doc
template: doc
---

# `clore::extract::scan_module_decl`

Owner: [Module extract:scan](../scan.md)

Declaration: `extract/scan.cppm:49`

Definition: `extract/scan.cppm:141`

Declaration: [`Namespace clore::extract`](../../../namespaces/clore/extract/index.md)

## Implementation

```cpp
auto scan_module_decl(std::string_view file_content, ScanResult& result) -> void {
    // Use Clang's dependency directives scanner for fast module detection.
    llvm::SmallVector<clang::dependency_directives_scan::Token, 64> tokens;
    llvm::SmallVector<clang::dependency_directives_scan::Directive, 16> directives;

    if(clang::scanSourceForDependencyDirectives(file_content, tokens, directives)) {
        // Scanner failed; fall back to no module detection.
        return;
    }

    namespace dds = clang::dependency_directives_scan;
    auto is_whitespace_only = [](std::string_view text) -> bool {
        return !text.empty() && std::ranges::all_of(text, [](char ch) {
            return std::isspace(static_cast<unsigned char>(ch)) != 0;
        });
    };
    auto is_punctuation_only = [](std::string_view text) -> bool {
        return !text.empty() && std::ranges::all_of(text, [](char ch) {
            return std::ispunct(static_cast<unsigned char>(ch)) != 0;
        });
    };

    for(auto& dir: directives) {
        if(dir.Kind == dds::cxx_export_module_decl || dir.Kind == dds::cxx_module_decl) {
            // Collect module name from tokens: identifiers + '.' + ':'
            std::string module_name;

            // Skip 'export' and 'module' keywords
            bool past_module_keyword = false;
            bool saw_first_name_token = false;
            bool is_fragment = false;
            for(auto& tok: dir.Tokens) {
                auto tok_text = file_content.substr(tok.Offset, tok.Length);
                if(is_whitespace_only(tok_text)) {
                    continue;
                }

                if(!past_module_keyword) {
                    if(tok_text == "module") {
                        past_module_keyword = true;
                    }
                    continue;
                }

                if(!saw_first_name_token) {
                    saw_first_name_token = true;
                    if(tok_text == ";" || tok_text == ":" || tok_text.starts_with(':') ||
                       is_punctuation_only(tok_text)) {
                        is_fragment = true;
                        break;
                    }
                }

                // Stop at semicolon or end
                if(tok_text == ";")
                    break;

                module_name += tok_text;
            }

            if(!is_fragment && !module_name.empty()) {
                result.module_name = std::move(module_name);
                result.is_interface_unit = (dir.Kind == dds::cxx_export_module_decl);
            }
        } else if(dir.Kind == dds::cxx_import_decl) {
            // Collect import name
            std::string import_name;

            bool past_import_keyword = false;
            for(auto& tok: dir.Tokens) {
                auto tok_text = file_content.substr(tok.Offset, tok.Length);

                if(!past_import_keyword) {
                    if(tok_text == "import") {
                        past_import_keyword = true;
                    }
                    continue;
                }

                if(tok_text == ";")
                    break;

                import_name += tok_text;
            }

            if(!import_name.empty()) {
                auto normalized_import =
                    normalize_partition_import(result.module_name, std::move(import_name));
                if(std::ranges::find(result.module_imports, normalized_import) ==
                   result.module_imports.end()) {
                    result.module_imports.push_back(std::move(normalized_import));
                }
            }
        }
    }
}
```

函数 `clore::extract::scan_module_decl` 利用 Clang 的依赖指令扫描器（`clang::scanSourceForDependencyDirectives`）对给定的文件内容进行快速模块声明分析，避免运行完整预处理器。内部首先将源文本解析为 `tokens` 和 `directives` 向量；若扫描失败则直接返回，不修改 `ScanResult`。随后遍历每个指令：对于 `cxx_export_module_decl` 或 `cxx_module_decl` 类型的指令，跳过 `export` 和 `module` 关键字，通过收集后续标识符（排除空白和纯标点）构建模块名称，并据此设置 `result.module_name` 和 `result.is_interface_unit`（仅 `export module` 表示接口单元）；对于 `cxx_import_decl` 类型的指令，跳过 `import` 关键字后收集导入名称，调用 `normalize_partition_import` 进行规范化，并在去重后加入 `result.module_imports`。该函数依赖 `ScanResult` 结构体字段和 `normalize_partition_import` 辅助函数，同时借助两个内联 lambda（`is_whitespace_only` 和 `is_punctuation_only`）辅助令牌分类。

## Side Effects

- Modifies the `ScanResult` parameter by setting `module_name`, `is_interface_unit`, and `module_imports`.
- Allocates memory for `std::string` and `std::vector` members of `ScanResult`.

## Reads From

- `file_content` (`string_view`) - source text of a translation unit.
- `result` (`ScanResult`&) - reads `module_name` and `module_imports` for duplicate checking.
- Directives and tokens produced by `clang::scanSourceForDependencyDirectives`.

## Writes To

- `result.module_name` - set to the extracted module name.
- `result.is_interface_unit` - set to `true` if `export module` declaration, else `false`.
- `result.module_imports` - appended with normalized import names.

## Usage Patterns

- Called by `clore::extract::scan_file` to perform fast module scanning on source files.
- Used as a lightweight alternative to full preprocessing for module dependency discovery.

## Called By

- function `clore::extract::scan_file`

