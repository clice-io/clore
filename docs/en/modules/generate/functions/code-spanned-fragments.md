---
title: 'clore::generate::codespannedfragments'
description: 'The function clore::generate::code_spanned_fragments is a thin wrapper that constructs a std::vector<InlineFragment> and passes the input string to the internal helper append_code_spanned_fragments. This helper, defined in an anonymous namespace, performs the actual line‑by‑line scanning and splitting of the markdown text into code‑spanned fragments by identifying backtick‑delimited spans, inline code candidates, and other syntactic patterns. By separating the allocation from the parsing logic, code_spanned_fragments provides a clean public interface while the core algorithm remains reusable across different call sites.'
layout: doc
template: doc
---

# `clore::generate::code_spanned_fragments`

Owner: [Module generate:markdown](../markdown.md)

Declaration: `src/generate/markdown.cppm:135`

Definition: `src/generate/markdown.cppm:704`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto code_spanned_fragments(std::string_view text) -> std::vector<InlineFragment> {
    std::vector<InlineFragment> fragments;
    append_code_spanned_fragments(fragments, text);
    return fragments;
}
```

The function `clore::generate::code_spanned_fragments` is a thin wrapper that constructs a `std::vector<InlineFragment>` and passes the input string to the internal helper `append_code_spanned_fragments`. This helper, defined in an anonymous namespace, performs the actual line‑by‑line scanning and splitting of the markdown text into code‑spanned fragments by identifying backtick‑delimited spans, inline code candidates, and other syntactic patterns. By separating the allocation from the parsing logic, `code_spanned_fragments` provides a clean public interface while the core algorithm remains reusable across different call sites.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- input parameter `text` of type `std::string_view`

## Writes To

- returns a newly allocated `std::vector<InlineFragment>`

## Usage Patterns

- Used by `clore::generate::(anonymous namespace)::append_rendered_text` to break up text into fragments for rendering

## Calls

- function `clore::generate::(anonymous namespace)::append_code_spanned_fragments`

## Called By

- function `clore::generate::(anonymous namespace)::append_rendered_text`

