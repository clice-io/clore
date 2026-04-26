---
title: 'clore::support::ensureutf8'
description: 'The function clore::support::ensure_utf8 accepts a std::string_view and returns a std::string that is guaranteed to be a valid UTF-8 representation of the input. Callers can rely on the result being well-formed UTF-8, suitable for downstream operations that require UTF-8 encoding, such as writing to a file or performing truncation. The caller is responsible for providing the input text; the function handles any necessary normalization or conversion to ensure UTF-8 validity.'
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

The function `clore::support::ensure_utf8` accepts a `std::string_view` and returns a `std::string` that is guaranteed to be a valid UTF-8 representation of the input. Callers can rely on the result being well-formed UTF-8, suitable for downstream operations that require UTF-8 encoding, such as writing to a file or performing truncation. The caller is responsible for providing the input text; the function handles any necessary normalization or conversion to ensure UTF-8 validity.

## Usage Patterns

- called by `write_utf8_text_file` to ensure output is valid UTF-8
- called by `truncate_utf8` to sanitize input before truncation

## Calls

- function `clore::support::(anonymous namespace)::valid_utf8_sequence_length`

## Called By

- function `clore::support::truncate_utf8`
- function `clore::support::write_utf8_text_file`

