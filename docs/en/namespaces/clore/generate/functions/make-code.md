---
title: 'clore::generate::makecode'
description: 'The function clore::generate::make_code accepts a std::string and returns an InlineFragment. It is responsible for creating an inline fragment that represents a code span in generated markdown content. The caller supplies the raw text that should be rendered as inline code; the returned InlineFragment encapsulates that text with the appropriate semantic markup.'
layout: doc
template: doc
---

# `clore::generate::make_code`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/markdown.cppm:112`

Definition: `src/generate/markdown.cppm:147`

Implementation: [`Module generate:markdown`](../../../../modules/generate/markdown.md)

## Declaration

```cpp
auto (std::string) -> InlineFragment;
```

The function `clore::generate::make_code` accepts a `std::string` and returns an `InlineFragment`. It is responsible for creating an inline fragment that represents a code span in generated markdown content. The caller supplies the raw text that should be rendered as inline code; the returned `InlineFragment` encapsulates that text with the appropriate semantic markup.

This function is a factory for building code spans, analogous to `clore::generate::make_text` for plain text or `clore::generate::make_link` for hyperlinks. It is used internally by code-aware formatting routines to preserve pre‑existing code spans in markdown fragments, ensuring they are correctly represented in the final output. The contract does not prescribe how the code span is formatted, only that the provided string is treated as literal code content.

## Usage Patterns

- creating inline code fragments in `append_existing_code_span`
- creating inline code fragments in `append_code_aware_text`
- wrapping a string into a code span fragment

## Called By

- function `clore::generate::(anonymous namespace)::append_code_aware_text`
- function `clore::generate::(anonymous namespace)::append_existing_code_span`

