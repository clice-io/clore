---
title: 'clore::extract::scanmoduledecl'
description: 'The implementation of clore::extract::scan_module_decl relies on Clang’s dependency directives scanner to quickly parse a source file’s module-related constructs without invoking the full preprocessor. It calls clang::scanSourceForDependencyDirectives on the input file_content, producing vectors of Token and Directive. If the scanner fails (returns a non‑zero value), the function returns early, leaving the provided ScanResult unchanged. Otherwise, it iterates over each directive. For directives of kind cxx_export_module_decl or cxx_module_decl, the function extracts the module name by skipping the export and module keywords and concatenating subsequent non‑whitespace, non‑punctuation tokens until a semicolon. It checks for a global module fragment (e.g., a :) and, if the fragment is absent and a module name was found, sets result.module_name and result.is_interface_unit (true only for cxx_export_module_decl). For directives of kind cxx_import_decl, the function collects the import name after the import keyword, normalizes it using normalize_partition_import (which handles partition specifiers), and appends the result to result.module_imports if it is not already present. Helper lambdas is_whitespace_only and is_punctuation_only are used to filter tokens during both module‑name and import‑name extraction.'
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

The implementation of `clore::extract::scan_module_decl` relies on Clang’s dependency directives scanner to quickly parse a source file’s module-related constructs without invoking the full preprocessor. It calls `clang::scanSourceForDependencyDirectives` on the input `file_content`, producing vectors of `Token` and `Directive`. If the scanner fails (returns a non‑zero value), the function returns early, leaving the provided `ScanResult` unchanged. Otherwise, it iterates over each directive. For directives of kind `cxx_export_module_decl` or `cxx_module_decl`, the function extracts the module name by skipping the `export` and `module` keywords and concatenating subsequent non‑whitespace, non‑punctuation tokens until a semicolon. It checks for a global module fragment (e.g., a `:`) and, if the fragment is absent and a module name was found, sets `result.module_name` and `result.is_interface_unit` (true only for `cxx_export_module_decl`). For directives of kind `cxx_import_decl`, the function collects the import name after the `import` keyword, normalizes it using `normalize_partition_import` (which handles partition specifiers), and appends the result to `result.module_imports` if it is not already present. Helper lambdas `is_whitespace_only` and `is_punctuation_only` are used to filter tokens during both module‑name and import‑name extraction.

## Side Effects

- populates the provided `ScanResult` with module name, interface unit flag, and imports list
- calls `normalize_partition_import` to normalize import names

## Reads From

- `file_content` (`string_view`)
- `result.module_imports` (when checking for duplicate imports)
- `result.module_name` (passed to `normalize_partition_import`)
- directives and tokens returned by `clang::scanSourceForDependencyDirectives`

## Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

## Usage Patterns

- called by `scan_file` to fill `ScanResult` fields without full preprocessing
- used as a fast module detection step before heavy parsing

## Called By

- function `clore::extract::scan_file`

