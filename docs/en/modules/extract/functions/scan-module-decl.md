---
title: 'clore::extract::scanmoduledecl'
description: 'The function clore::extract::scan_module_decl leverages Clang''s low-level dependency directives scanner via clang::scanSourceForDependencyDirectives to parse the raw file content into a sequence of tokens and directives without invoking the full preprocessor. It iterates over the resulting directive list, handling two directive kinds: module declarations (cxx_export_module_decl and cxx_module_decl) and import declarations (cxx_import_decl). For module declarations, it skips the export and module tokens, then collects subsequent non‑whitespace tokens into the module name, stopping at a semicolon or early‑fragment indicator (e.g., ";" or ":"); it sets result.module_name and result.is_interface_unit accordingly. For import declarations, it extracts the import name after the import keyword, normalizes it via normalize_partition_import using the already‑established module name, and appends it to result.module_imports if not already present. The entire scan is fast because it operates on a pre‑tokenized stream and does no semantic analysis.'
layout: doc
template: doc
---

# `clore::extract::scan_module_decl`

Owner: [Module extract:scan](../scan.md)

Declaration: `src/extract/scan.cppm:67`

Definition: `src/extract/scan.cppm:159`

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

The function `clore::extract::scan_module_decl` leverages Clang's low-level dependency directives scanner via `clang::scanSourceForDependencyDirectives` to parse the raw file content into a sequence of tokens and directives without invoking the full preprocessor. It iterates over the resulting directive list, handling two directive kinds: module declarations (`cxx_export_module_decl` and `cxx_module_decl`) and import declarations (`cxx_import_decl`). For module declarations, it skips the `export` and `module` tokens, then collects subsequent non‑whitespace tokens into the module name, stopping at a semicolon or early‑fragment indicator (e.g., `";"` or `":"`); it sets `result.module_name` and `result.is_interface_unit` accordingly. For import declarations, it extracts the import name after the `import` keyword, normalizes it via `normalize_partition_import` using the already‑established module name, and appends it to `result.module_imports` if not already present. The entire scan is fast because it operates on a pre‑tokenized stream and does no semantic analysis.

## Side Effects

- Modifies `result.module_name` by assigning the detected module name
- Modifies `result.is_interface_unit` by setting it to true for export declarations
- Appends unique import names to `result.module_imports`

## Reads From

- `file_content` (`std::string_view` parameter)
- `result.module_name` and `result.module_imports` for duplicate checking
- Output of `clang::scanSourceForDependencyDirectives`

## Writes To

- `result.module_name`
- `result.is_interface_unit`
- `result.module_imports`

## Usage Patterns

- Called by `scan_file` to quickly obtain module metadata
- Used in the extraction pipeline for module scanning without full preprocessing

## Called By

- function `clore::extract::scan_file`

