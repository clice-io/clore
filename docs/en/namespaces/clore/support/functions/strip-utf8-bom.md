---
title: 'clore::support::striputf8bom'
description: 'The function clore::support::strip_utf8_bom examines the beginning of a std::string_view input for the UTF-8 Byte Order Mark (BOM) sequence (U+FEFF encoded as EF BB BF in UTF-8). If the BOM is present, it returns a new std::string_view that points to the input data with the BOM skipped; otherwise, it returns the original view unchanged. The caller can rely on the result being a valid view into the same storage, and no heap allocation or data copying occurs. This function is typically used by file-reading utilities to cleanly expose UTF-8 content without the BOM prefix.'
layout: doc
template: doc
---

# `clore::support::strip_utf8_bom`

Owner: [Namespace clore::support](../index.md)

Declaration: `support/logging.cppm:83`

Definition: `support/logging.cppm:470`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string_view;
```

The function `clore::support::strip_utf8_bom` examines the beginning of a `std::string_view` input for the UTF-8 Byte Order Mark (BOM) sequence (`U+FEFF` encoded as `EF BB BF` in UTF-8). If the BOM is present, it returns a new `std::string_view` that points to the input data with the BOM skipped; otherwise, it returns the original view unchanged. The caller can rely on the result being a valid view into the same storage, and no heap allocation or data copying occurs. This function is typically used by file-reading utilities to cleanly expose UTF-8 content without the BOM prefix.

## Usage Patterns

- Stripping the UTF‑8 BOM from file contents before processing in `clore::support::read_utf8_text_file`

## Called By

- function `clore::support::read_utf8_text_file`

