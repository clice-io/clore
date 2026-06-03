---
title: 'clore::generate::formatevidencetextbounded'
description: 'The function clore::generate::format_evidence_text_bounded accepts a const EvidencePack & and a std::size_t limiting the total length of the output. It generates a single std::string that contains a human‑readable representation of all evidence items in the pack, formatted in a consistent textual style suitable for inclusion in prompt contexts or logs.'
layout: doc
template: doc
---

# `clore::generate::format_evidence_text_bounded`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/evidence.cppm:100`

Definition: `src/generate/evidence.cppm:596`

Implementation: [`Module generate:evidence`](../../../../modules/generate/evidence.md)

## Declaration

```cpp
auto (const EvidencePack &, std::size_t) -> std::string;
```

The function `clore::generate::format_evidence_text_bounded` accepts a `const EvidencePack &` and a `std::size_t` limiting the total length of the output. It generates a single `std::string` that contains a human‑readable representation of all evidence items in the pack, formatted in a consistent textual style suitable for inclusion in prompt contexts or logs.

The caller must provide the bound as the second argument; the resulting string is guaranteed not to exceed that length. If formatting all evidence would exceed the bound, the function truncates the output at the boundary, preserving as much content as possible from earlier evidence items. The function is primarily invoked by `clore::generate::format_evidence_text`, which supplies an effectively unbounded size to obtain the complete text.

## Usage Patterns

- called by `clore::generate::format_evidence_text`
- used to generate bounded evidence strings for prompts

## Calls

- function `clore::generate::(anonymous namespace)::append_if_fits`

## Called By

- function `clore::generate::format_evidence_text`

