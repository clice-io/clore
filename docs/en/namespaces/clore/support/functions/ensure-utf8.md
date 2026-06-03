---
title: 'clore::support::ensureutf8'
description: 'The function clore::support::ensure_utf8 accepts a std::string_view and returns a std::string that is guaranteed to be valid UTF-8. It is the caller’s responsibility to provide any std::string_view; the function handles any ill-formed byte sequences and produces a properly encoded UTF-8 result, suitable for further processing or output by callers such as clore::support::write_utf8_text_file and clore::support::truncate_utf8.'
layout: doc
template: doc
---

# `clore::support::ensure_utf8`

Owner: [Namespace clore::support](../index.md)

Declaration: `src/support/logging.cppm:98`

Definition: `src/support/logging.cppm:428`

Implementation: [`Module support`](../../../../modules/support/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

The function `clore::support::ensure_utf8` accepts a `std::string_view` and returns a `std::string` that is guaranteed to be valid UTF-8. It is the caller’s responsibility to provide any `std::string_view`; the function handles any ill-formed byte sequences and produces a properly encoded UTF-8 result, suitable for further processing or output by callers such as `clore::support::write_utf8_text_file` and `clore::support::truncate_utf8`.

## Usage Patterns

- Ensuring text is valid UTF-8 before passing to `write_utf8_text_file`
- Sanitizing input before truncation in `truncate_utf8`

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

