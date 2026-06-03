---
title: 'clore::generate::formatevidencetext'
description: 'The function clore::generate::format_evidence_text accepts a reference to an EvidencePack and returns a std::string containing a formatted representation of the evidence data. It is designed to produce the complete evidence text that is later embedded into a prompt template by clore::generate::build_prompt. The caller provides the entire EvidencePack, and the function returns a single string that consolidates all evidence entries into a readable, prompt‑ready format. There is no limit on the output length; for size‑constrained use cases, the related clore::generate::format_evidence_text_bounded provides a bounded alternative.'
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

The function `clore::generate::format_evidence_text` accepts a reference to an `EvidencePack` and returns a `std::string` containing a formatted representation of the evidence data. It is designed to produce the complete evidence text that is later embedded into a prompt template by `clore::generate::build_prompt`. The caller provides the entire `EvidencePack`, and the function returns a single string that consolidates all evidence entries into a readable, prompt‑ready format. There is no limit on the output length; for size‑constrained use cases, the related `clore::generate::format_evidence_text_bounded` provides a bounded alternative.

## Usage Patterns

- called by `clore::generate::build_prompt` to obtain the full evidence text for a prompt

## Calls

- function `clore::generate::format_evidence_text_bounded`

## Called By

- function `clore::generate::build_prompt`

