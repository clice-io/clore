---
title: 'clore::generate::formatevidencetext'
description: 'The function clore::generate::format_evidence_text acts as a thin delegation wrapper around clore::generate::format_evidence_text_bounded. It calls that bounded function with std::numeric_limits<std::size_t>::max() as the maximum length parameter, effectively disabling any truncation of the generated evidence text. This design centralises the actual evidence‑text construction logic in the bounded variant, while providing a convenient unbounded interface for callers that do not need to enforce a length cap. No additional algorithm or control flow is introduced; the entire behaviour is determined by format_evidence_text_bounded, which handles the accumulation of text sections, symbol facts, source snippets, and analysis results under a configurable length constraint.'
layout: doc
template: doc
---

# `clore::generate::format_evidence_text`

Owner: [Module generate:evidence](../evidence.md)

Declaration: `src/generate/evidence.cppm:98`

Definition: `src/generate/evidence.cppm:592`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto format_evidence_text(const EvidencePack& pack) -> std::string {
    return format_evidence_text_bounded(pack, std::numeric_limits<std::size_t>::max());
}
```

The function `clore::generate::format_evidence_text` acts as a thin delegation wrapper around `clore::generate::format_evidence_text_bounded`. It calls that bounded function with `std::numeric_limits<std::size_t>::max()` as the maximum length parameter, effectively disabling any truncation of the generated evidence text. This design centralises the actual evidence‑text construction logic in the bounded variant, while providing a convenient unbounded interface for callers that do not need to enforce a length cap. No additional algorithm or control flow is introduced; the entire behaviour is determined by `format_evidence_text_bounded`, which handles the accumulation of text sections, symbol facts, source snippets, and analysis results under a configurable length constraint.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- pack parameter of type `const EvidencePack &`

## Usage Patterns

- called by `clore::generate::build_prompt` to obtain the full evidence text for a prompt

## Calls

- function `clore::generate::format_evidence_text_bounded`

## Called By

- function `clore::generate::build_prompt`

