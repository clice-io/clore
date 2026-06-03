---
title: 'clore::generate::normalizefrontmattertitle'
description: 'The function first calls strip_inline_markdown on the input page_title. If the stripped result is non-empty, it is returned; otherwise, the function returns a string created by trim_ascii applied to the original page_title. The logic prefers a markdown-stripped version but falls back to a whitespace-trimmed version when stripping yields an empty string (e.g., if the title consisted solely of markdown formatting). Both helper functions (strip_inline_markdown and trim_ascii) are assumed to be in the same namespace and are the only dependencies.'
layout: doc
template: doc
---

# `clore::generate::normalize_frontmatter_title`

Owner: [Module generate:symbol](../symbol.md)

Declaration: `src/generate/render/symbol.cppm:51`

Definition: `src/generate/render/symbol.cppm:903`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto normalize_frontmatter_title(std::string_view page_title) -> std::string {
    auto plain = strip_inline_markdown(page_title);
    if(!plain.empty()) {
        return plain;
    }
    return std::string(trim_ascii(page_title));
}
```

The function first calls `strip_inline_markdown` on the input `page_title`. If the stripped result is non-empty, it is returned; otherwise, the function returns a string created by `trim_ascii` applied to the original `page_title`. The logic prefers a markdown-stripped version but falls back to a whitespace-trimmed version when stripping yields an empty string (e.g., if the title consisted solely of markdown formatting). Both helper functions (`strip_inline_markdown` and `trim_ascii`) are assumed to be in the same namespace and are the only dependencies.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `page_title`

## Writes To

- returned `std::string`

## Usage Patterns

- Used to normalize frontmatter titles for documentation pages

## Called By

- function `clore::generate::(anonymous namespace)::build_symbol_frontmatter`

