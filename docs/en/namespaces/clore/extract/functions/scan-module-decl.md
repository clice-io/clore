---
title: 'clore::extract::scanmoduledecl'
description: 'clore::extract::scan_module_decl performs a fast scan of a C++ source file to extract module-related metadata without invoking the full preprocessor. It accepts the file''s contents as a std::string_view and a reference to a ScanResult object. On success, it populates the ScanResult''s module_name, is_interface_unit, and module_imports fields using Clang''s dependency directives scanner.'
layout: doc
template: doc
---

# `clore::extract::scan_module_decl`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/scan.cppm:67`

Definition: `src/extract/scan.cppm:159`

Implementation: [`Module extract:scan`](../../../../modules/extract/scan.md)

## Declaration

```cpp
auto (std::string_view, ScanResult &) -> void;
```

`clore::extract::scan_module_decl` performs a fast scan of a C++ source file to extract module-related metadata without invoking the full preprocessor. It accepts the file's contents as a `std::string_view` and a reference to a `ScanResult` object. On success, it populates the `ScanResult`'s `module_name`, `is_interface_unit`, and `module_imports` fields using Clang's dependency directives scanner.

Callers should use this function when they need a quick, lightweight pass to identify module declarations and import dependencies from source text. The function relies solely on the raw source content and does not require a `CompileEntry` or compilation database, making it suitable for early analysis stages or contexts where only the module structure is needed. The supplied `ScanResult` must be default-initialized; existing fields not mentioned above remain unchanged.

## Usage Patterns

- Called by `scan_file` to quickly obtain module metadata
- Used in the extraction pipeline for module scanning without full preprocessing

## Called By

- function `clore::extract::scan_file`

