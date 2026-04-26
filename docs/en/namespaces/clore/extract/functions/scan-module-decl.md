---
title: 'clore::extract::scanmoduledecl'
description: 'The caller provides a source text (as std::string_view) and a mutable reference to a ScanResult object. The function performs a fast scan of the source to extract the module declaration, using Clang''s dependency directives scanner. It populates the module_name, is_interface_unit, and module_imports fields of the ScanResult without running the full preprocessor. This is a lightweight alternative to a full parse and is intended to be used during the initial scanning phase.'
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

The caller provides a source text (as `std::string_view`) and a mutable reference to a `ScanResult` object. The function performs a fast scan of the source to extract the module declaration, using Clang's dependency directives scanner. It populates the `module_name`, `is_interface_unit`, and `module_imports` fields of the `ScanResult` without running the full preprocessor. This is a lightweight alternative to a full parse and is intended to be used during the initial scanning phase.

The caller must ensure the `std::string_view` remains valid for the duration of the call, and the `ScanResult` is in a default‑initialized state (or at least that its relevant fields are overwritten). After the call, the caller can inspect the populated fields to determine module‑related properties of the source. The function does not throw exceptions; any failure to extract the declaration is reflected by the state of the `ScanResult` after the call, typically as an empty or invalid `module_name`.

## Usage Patterns

- called by `clore::extract::scan_file` to extract module information from source content
- used as a fast alternative to full preprocessing for module detection

## Called By

- function `clore::extract::scan_file`

