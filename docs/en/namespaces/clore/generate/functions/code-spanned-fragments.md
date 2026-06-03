---
title: 'clore::generate::codespannedfragments'
description: 'The function clore::generate::code_spanned_fragments accepts a std::string_view and returns a std::vector<InlineFragment>. It is responsible for extracting inline code spans from Markdown source text, providing the caller with a decomposed list of fragments that correspond to code spans. This allows downstream processing to treat each code span as a distinct inline element.'
layout: doc
template: doc
---

# `clore::generate::code_spanned_fragments`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/markdown.cppm:135`

Definition: `src/generate/markdown.cppm:704`

Implementation: [`Module generate:markdown`](../../../../modules/generate/markdown.md)

## Declaration

```cpp
auto (std::string_view) -> std::vector<InlineFragment>;
```

The function `clore::generate::code_spanned_fragments` accepts a `std::string_view` and returns a `std::vector<InlineFragment>`. It is responsible for extracting inline code spans from Markdown source text, providing the caller with a decomposed list of fragments that correspond to code spans. This allows downstream processing to treat each code span as a distinct inline element.

## Usage Patterns

- Used by `clore::generate::(anonymous namespace)::append_rendered_text` to break up text into fragments for rendering

## Calls

- function `clore::generate::(anonymous namespace)::append_code_spanned_fragments`

## Called By

- function `clore::generate::(anonymous namespace)::append_rendered_text`

