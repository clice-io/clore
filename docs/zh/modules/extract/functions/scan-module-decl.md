---
title: 'clore::extract::scanmoduledecl'
description: 'The function clore::extract::scan_module_decl implements a lightweight module‑declaration scanner that avoids the overhead of a full preprocessor run by using Clang’s dependency‑directives scanner (clang::scanSourceForDependencyDirectives). It operates directly on the raw file content (std::string_view file_content) and populates the supplied ScanResult with the module name, interface‑unit flag, and a deduplicated list of module imports.'
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

The function `clore::extract::scan_module_decl` implements a lightweight module‑declaration scanner that avoids the overhead of a full preprocessor run by using Clang’s dependency‑directives scanner (`clang::scanSourceForDependencyDirectives`). It operates directly on the raw file content (`std::string_view file_content`) and populates the supplied `ScanResult` with the module name, interface‑unit flag, and a deduplicated list of module imports.

The algorithm first tokenises the file content into `tokens` and `directives` via the Clang scanner. Two local lambdas, `is_whitespace_only` and `is_punctuation_only`, help classify token text. It then iterates over each directive. For directives of kind `cxx_export_module_decl` or `cxx_module_decl`, it skips the `export` and `module` keywords, then concatenates subsequent tokens until a semicolon is encountered. A special case detects a module‑fragment declaration (e.g. `module;` or `module :...`) by checking if the first non‑whitespace token after `module` is punctuation; in that case the module name is left empty and `is_interface_unit` is not set. Otherwise the collected name is stored in `result.module_name`, and `result.is_interface_unit` is set to `true` only for an `export module` declaration. For directives of kind `cxx_import_decl`, the scanner extracts the import name, normalises it via `normalize_partition_import`, and appends it to `result.module_imports` if not already present. The function makes no changes to the `ScanResult` if the scanner itself fails, effectively falling back to no module detection.

## Side Effects

- Modifies the `ScanResult` object passed by reference by setting `module_name`, `is_interface_unit`, and appending to `module_imports`.

## Reads From

- Reads from the `file_content` string view.
- Uses Clang's `clang::scanSourceForDependencyDirectives` to parse tokens and directives internally.

## Writes To

- Writes to `result.module_name` (`std::string`).
- Writes to `result.is_interface_unit` (`bool`).
- Writes to `result.module_imports` via `push_back` (`std::vector<std::string>`).

## Usage Patterns

- Called by `clore::extract::scan_file` to quickly extract module-level declarations without full preprocessing.
- Designed for scanning source files to determine module name, interface status, and imported modules.

## Called By

- function `clore::extract::scan_file`

