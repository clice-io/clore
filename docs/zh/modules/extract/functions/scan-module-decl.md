---
title: 'clore::extract::scanmoduledecl'
description: '函数首先调用 clang::scanSourceForDependencyDirectives，将文件内容解析为令牌与指令列表；若扫描失败则直接返回。随后定义 is_whitespace_only 和 is_punctuation_only 两个辅助 lambda，用于识别令牌类型。遍历每条指令：对于 cxx_export_module_decl 或 cxx_module_decl，跳过 export 和 module 关键字，收集后续令牌（连接标识符、点、冒号）作为模块名称；遇到分号或纯标点时终止，并将纯标点情形视为全局模块片段。若收集到有效名称，则设置 result.module_name，并根据指令类型设置 result.is_interface_unit。对于 cxx_import_decl，跳过 import 关键字，收集后续令牌作为导入名称，调用 normalize_partition_import 进行规范化，并仅在 result.module_imports 中尚未存在时添加该导入。整个算法的核心依赖是 Clang 的依赖指令扫描基础设施，无需运行完整预处理器即可快速提取模块声明信息。'
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

函数首先调用 `clang::scanSourceForDependencyDirectives`，将文件内容解析为令牌与指令列表；若扫描失败则直接返回。随后定义 `is_whitespace_only` 和 `is_punctuation_only` 两个辅助 lambda，用于识别令牌类型。遍历每条指令：对于 `cxx_export_module_decl` 或 `cxx_module_decl`，跳过 `export` 和 `module` 关键字，收集后续令牌（连接标识符、点、冒号）作为模块名称；遇到分号或纯标点时终止，并将纯标点情形视为全局模块片段。若收集到有效名称，则设置 `result.module_name`，并根据指令类型设置 `result.is_interface_unit`。对于 `cxx_import_decl`，跳过 `import` 关键字，收集后续令牌作为导入名称，调用 `normalize_partition_import` 进行规范化，并仅在 `result.module_imports` 中尚未存在时添加该导入。整个算法的核心依赖是 Clang 的依赖指令扫描基础设施，无需运行完整预处理器即可快速提取模块声明信息。

## Side Effects

- Modifies the `ScanResult` object passed by reference, setting `module_name`, `is_interface_unit`, and appending to `module_imports`.

## Reads From

- `file_content` parameter (string view of file source)
- `result.module_imports` member (to avoid duplicate entries)

## Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

## Usage Patterns

- Called by `clore::extract::scan_file` during source file scanning to populate module metadata.
- Used as a lightweight alternative to full preprocessing for extracting module information.

## Called By

- function `clore::extract::scan_file`

