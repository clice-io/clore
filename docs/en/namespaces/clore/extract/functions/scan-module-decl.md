---
title: 'clore::extract::scanmoduledecl'
description: 'The function clore::extract::scan_module_decl performs a fast scan of a C++ module declaration using Clang’s dependency directives scanner. It avoids running the full preprocessor and instead directly populates the module_name, is_interface_unit, and module_imports fields of a provided ScanResult object. The caller supplies a std::string_view containing the source text of the translation unit and a mutable reference to a ScanResult that will receive the extracted module information. The function returns void and relies on the caller to ensure the source text is valid and the ScanResult is properly initialized.'
layout: doc
template: doc
---

# `clore::extract::scan_module_decl`

Owner: [Namespace clore::extract](../index.md)

Declaration: `extract/scan.cppm:49`

Definition: `extract/scan.cppm:141`

Implementation: [`Module extract:scan`](../../../../modules/extract/scan.md)

## Declaration

```cpp
auto (std::string_view, ScanResult &) -> void;
```

The function `clore::extract::scan_module_decl` performs a fast scan of a C++ module declaration using Clang’s dependency directives scanner. It avoids running the full preprocessor and instead directly populates the `module_name`, `is_interface_unit`, and `module_imports` fields of a provided `ScanResult` object. The caller supplies a `std::string_view` containing the source text of the translation unit and a mutable reference to a `ScanResult` that will receive the extracted module information. The function returns `void` and relies on the caller to ensure the source text is valid and the `ScanResult` is properly initialized.

## Usage Patterns

- called by `scan_file` to fill `ScanResult` fields without full preprocessing
- used as a fast module detection step before heavy parsing

## Called By

- function `clore::extract::scan_file`

