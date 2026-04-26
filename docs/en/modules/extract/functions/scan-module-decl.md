---
title: 'clore::extract::scanmoduledecl'
description: 'The implementation of clore::extract::scan_module_decl relies entirely on Clang''s dependency directives scanner (clang::scanSourceForDependencyDirectives) to quickly tokenize the file content without running the full preprocessor. This produces a list of dependency_directives_scan::Directive entries, which are then iterated to identify module and import declarations. For each cxx_module_decl or cxx_export_module_decl directive, the function walks the associated tokens past the module keyword, collecting name tokens (identifiers, dots, colons) while skipping whitespace and ignoring punctuation-only tokens that indicate a module fragment. The collected name becomes result.module_name, and result.is_interface_unit is set based on whether the declaration was export module. For each cxx_import_decl directive, tokens after the import keyword are concatenated into an import name, which is then normalized via normalize_partition_import and inserted into result.module_imports after a duplicate check. If the scanner fails (returns non‑zero), the function silently returns without modifying the result, leaving it in its default state.'
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

The implementation of `clore::extract::scan_module_decl` relies entirely on Clang's dependency directives scanner (`clang::scanSourceForDependencyDirectives`) to quickly tokenize the file content without running the full preprocessor. This produces a list of `dependency_directives_scan::Directive` entries, which are then iterated to identify module and import declarations. For each `cxx_module_decl` or `cxx_export_module_decl` directive, the function walks the associated tokens past the `module` keyword, collecting name tokens (identifiers, dots, colons) while skipping whitespace and ignoring punctuation-only tokens that indicate a module fragment. The collected name becomes `result.module_name`, and `result.is_interface_unit` is set based on whether the declaration was `export module`. For each `cxx_import_decl` directive, tokens after the `import` keyword are concatenated into an import name, which is then normalized via `normalize_partition_import` and inserted into `result.module_imports` after a duplicate check. If the scanner fails (returns non‑zero), the function silently returns without modifying the result, leaving it in its default state.

## Side Effects

- modifies the `module_name`, `is_interface_unit`, and `module_imports` fields of the provided `ScanResult` object

## Reads From

- the `file_content` string parameter
- tokens and directives produced by `clang::scanSourceForDependencyDirectives`

## Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

## Usage Patterns

- called by `clore::extract::scan_file` to extract module information from source content
- used as a fast alternative to full preprocessing for module detection

## Called By

- function `clore::extract::scan_file`

