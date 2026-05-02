---
title: 'clore::support::ensureutf8'
description: 'The function clore::support::ensure_utf8 accepts a std::string_view and returns a std::string. It is responsible for ensuring that the returned string is a valid UTF-8 encoding of the input. The caller can rely on the result being a correctly encoded UTF-8 string, suitable for further processing or output. This function is used by other utilities such as clore::support::write_utf8_text_file and clore::support::truncate_utf8 to guarantee UTF-8 validity before performing operations that require correct encoding.'
layout: doc
template: doc
---

# `clore::support::ensure_utf8`

Owner: [Namespace clore::support](../index.md)

Declaration: `support/logging.cppm:75`

Definition: `support/logging.cppm:405`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

The function `clore::support::ensure_utf8` accepts a `std::string_view` and returns a `std::string`. It is responsible for ensuring that the returned string is a valid UTF-8 encoding of the input. The caller can rely on the result being a correctly encoded UTF-8 string, suitable for further processing or output. This function is used by other utilities such as `clore::support::write_utf8_text_file` and `clore::support::truncate_utf8` to guarantee UTF-8 validity before performing operations that require correct encoding.

## Usage Patterns

- Used by `write_utf8_text_file` to sanitize input before writing
- Used by `truncate_utf8` to ensure truncated result is valid UTF-8

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

