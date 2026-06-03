---
title: 'clore::generate::maketext'
description: 'clore::generate::make_text accepts a std::string and returns an InlineFragment. This function is the primary factory for creating plain, unformatted text fragments within the markdown generation pipeline. It is used by higher-level functions such as clore::generate::make_paragraph and clore::generate::make_blockquote to supply the textual content that those nodes wrap. Calling make_text with an arbitrary string guarantees an InlineFragment that represents exactly that string with no additional markdown interpretation or decoration.'
layout: doc
template: doc
---

# `clore::generate::make_text`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/markdown.cppm:110`

Definition: `src/generate/markdown.cppm:143`

Implementation: [`Module generate:markdown`](../../../../modules/generate/markdown.md)

## Declaration

```cpp
auto (std::string) -> InlineFragment;
```

`clore::generate::make_text` accepts a `std::string` and returns an `InlineFragment`. This function is the primary factory for creating plain, unformatted text fragments within the markdown generation pipeline. It is used by higher-level functions such as `clore::generate::make_paragraph` and `clore::generate::make_blockquote` to supply the textual content that those nodes wrap. Calling `make_text` with an arbitrary string guarantees an `InlineFragment` that represents exactly that string with no additional markdown interpretation or decoration.

## Usage Patterns

- used to embed plain text in inline fragments
- called by `make_paragraph`, `make_blockquote`, and `append_text_fragment`

## Called By

- function `clore::generate::(anonymous namespace)::append_text_fragment`
- function `clore::generate::make_blockquote`
- function `clore::generate::make_paragraph`

