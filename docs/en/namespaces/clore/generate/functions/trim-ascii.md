---
title: 'clore::generate::trimascii'
description: 'clore::generate::trim_ascii is a utility function that accepts a std::string_view and returns a std::string_view pointing to the same underlying characters with all leading and trailing ASCII whitespace removed. The result is a non-owning view; no heap allocation or copying occurs. Callers use this function to normalize short text fragments—for example, after stripping inline Markdown syntax from a string, or before checking whether a raw text output is non‑empty. By trimming whitespace boundaries, the function helps ensure that further processing (such as appending content to a section or evaluating emptiness) operates only on meaningful content.'
layout: doc
template: doc
---

# `clore::generate::trim_ascii`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/common.cppm:34`

Definition: `src/generate/render/common.cppm:34`

Implementation: [`Module generate:common`](../../../../modules/generate/common.md)

## Declaration

```cpp
auto (std::string_view) -> std::string_view;
```

`clore::generate::trim_ascii` is a utility function that accepts a `std::string_view` and returns a `std::string_view` pointing to the same underlying characters with all leading and trailing ASCII whitespace removed. The result is a non-owning view; no heap allocation or copying occurs. Callers use this function to normalize short text fragments—for example, after stripping inline Markdown syntax from a string, or before checking whether a raw text output is non‑empty. By trimming whitespace boundaries, the function helps ensure that further processing (such as appending content to a section or evaluating emptiness) operates only on meaningful content.

## Usage Patterns

- Used to trim whitespace from prompt texts
- Used in stripping inline markdown
- Used when adding prompt output

## Called By

- function `clore::generate::add_prompt_output`
- function `clore::generate::build_prompt_section`
- function `clore::generate::strip_inline_markdown`

