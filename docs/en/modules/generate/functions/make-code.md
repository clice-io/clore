---
title: 'clore::generate::makecode'
description: 'The function clore::generate::make_code constructs and returns an InlineFragment representing a code span. Its implementation directly creates a CodeFragment by moving the input code string into the CodeFragment::code member, then returns that value as the polymorphic InlineFragment type. No additional parsing, validation, or transformation is performed; the algorithm is a straightforward constructor call. Dependencies are limited to the CodeFragment struct (defined in the same module) and the type alias InlineFragment, which is a std::variant over several fragment types. This function is invoked during the inline-level code span detection phase when the parser encounters backtick-enclosed code in the Markdown input, providing the foundational representation for code spans that later get rendered by render_inline.'
layout: doc
template: doc
---

# `clore::generate::make_code`

Owner: [Module generate:markdown](../markdown.md)

Declaration: `src/generate/markdown.cppm:112`

Definition: `src/generate/markdown.cppm:147`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto make_code(std::string code) -> InlineFragment {
    return CodeFragment{.code = std::move(code)};
}
```

The function `clore::generate::make_code` constructs and returns an `InlineFragment` representing a code span. Its implementation directly creates a `CodeFragment` by moving the input `code` string into the `CodeFragment::code` member, then returns that value as the polymorphic `InlineFragment` type. No additional parsing, validation, or transformation is performed; the algorithm is a straightforward constructor call. Dependencies are limited to the `CodeFragment` struct (defined in the same module) and the type alias `InlineFragment`, which is a `std::variant` over several fragment types. This function is invoked during the inline-level code span detection phase when the parser encounters backtick-enclosed code in the Markdown input, providing the foundational representation for code spans that later get rendered by `render_inline`.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `code` (via move)

## Writes To

- returned `InlineFragment` (constructed `CodeFragment`)

## Usage Patterns

- creating inline code fragments in `append_existing_code_span`
- creating inline code fragments in `append_code_aware_text`
- wrapping a string into a code span fragment

## Called By

- function `clore::generate::(anonymous namespace)::append_code_aware_text`
- function `clore::generate::(anonymous namespace)::append_existing_code_span`

