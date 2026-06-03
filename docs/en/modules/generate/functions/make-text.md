---
title: 'clore::generate::maketext'
description: 'The function clore::generate::make_text serves as a factory that wraps a plain string into the InlineFragment variant by constructing a TextFragment directly. It accepts a std::string, moves it into the TextFragment::text field, and returns the resulting InlineFragment object. The implementation is a single-expression return, with no branching, iteration, or conditional logic; it relies solely on the definitions of TextFragment and the InlineFragment type alias. No other functions or external dependencies are involved.'
layout: doc
template: doc
---

# `clore::generate::make_text`

Owner: [Module generate:markdown](../markdown.md)

Declaration: `src/generate/markdown.cppm:110`

Definition: `src/generate/markdown.cppm:143`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto make_text(std::string text) -> InlineFragment {
    return TextFragment{.text = std::move(text)};
}
```

The function `clore::generate::make_text` serves as a factory that wraps a plain string into the `InlineFragment` variant by constructing a `TextFragment` directly. It accepts a `std::string`, moves it into the `TextFragment::text` field, and returns the resulting `InlineFragment` object. The implementation is a single-expression return, with no branching, iteration, or conditional logic; it relies solely on the definitions of `TextFragment` and the `InlineFragment` type alias. No other functions or external dependencies are involved.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `text` parameter (moved from)

## Writes To

- returned `InlineFragment` containing a `TextFragment`

## Usage Patterns

- used to embed plain text in inline fragments
- called by `make_paragraph`, `make_blockquote`, and `append_text_fragment`

## Called By

- function `clore::generate::(anonymous namespace)::append_text_fragment`
- function `clore::generate::make_blockquote`
- function `clore::generate::make_paragraph`

