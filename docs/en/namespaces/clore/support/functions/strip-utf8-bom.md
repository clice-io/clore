---
title: 'clore::support::striputf8bom'
description: 'clore::support::strip_utf8_bom accepts a std::string_view and returns a std::string_view that points to the same character data, except that a leading UTF‑8 byte order mark (BOM), if present, is skipped. The result is a non‑owning view that does not modify the original input; the caller retains ownership of the underlying storage.'
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

`clore::support::strip_utf8_bom` accepts a `std::string_view` and returns a `std::string_view` that points to the same character data, except that a leading UTF‑8 byte order mark (BOM), if present, is skipped. The result is a non‑owning view that does not modify the original input; the caller retains ownership of the underlying storage.

The function is intended to normalize UTF‑8 text before further processing, such as reading a file with `read_utf8_text_file`. The returned view is equivalent to the input when no BOM is found, or advances past the three‑byte sequence `0xEF BB BF` when it is detected. No validity checks are performed beyond the BOM pattern itself.

## Usage Patterns

- called by `clore::support::read_utf8_text_file` to strip BOM from file contents

## Called By

- function `clore::support::read_utf8_text_file`

