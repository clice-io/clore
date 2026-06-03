---
title: 'clore::generate::formatevidencetext'
description: 'The function clore::generate::format_evidence_text accepts a const EvidencePack & and returns a std::string containing a formatted textual representation of the evidence. Its caller, such as clore::generate::build_prompt, relies on this function to produce a consistent, human‑readable string that can be substituted into a prompt template. The caller is expected to provide a valid EvidencePack; the function guarantees that the returned string is suitable for downstream prompt construction, without any additional constraints on the structure of the evidence.'
layout: doc
template: doc
---

# `clore::generate::format_evidence_text`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/evidence.cppm:98`

Definition: `src/generate/evidence.cppm:592`

Implementation: [`Module generate:evidence`](../../../../modules/generate/evidence.md)

## Declaration

```cpp
auto (const EvidencePack &) -> std::string;
```

The function `clore::generate::format_evidence_text` accepts a `const EvidencePack &` and returns a `std::string` containing a formatted textual representation of the evidence. Its caller, such as `clore::generate::build_prompt`, relies on this function to produce a consistent, human‑readable string that can be substituted into a prompt template. The caller is expected to provide a valid `EvidencePack`; the function guarantees that the returned string is suitable for downstream prompt construction, without any additional constraints on the structure of the evidence.

## Usage Patterns

- Used by `build_prompt` to format evidence for prompts

## Calls

- function `clore::generate::format_evidence_text_bounded`

## Called By

- function `clore::generate::build_prompt`

