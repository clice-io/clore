---
title: 'clore::generate::escapemermaidlabel'
description: 'The function clore::generate::escape_mermaid_label takes a std::string_view and returns a std::string that is sanitized for safe inclusion as a label in Mermaid diagram markup. Callers should supply any raw text that may contain characters special to Mermaid label syntax—such as quotes, parentheses, newlines, or angle brackets—and the returned value can be directly embedded in Mermaid diagram code without breaking the syntax or causing rendering errors. This is required when generating diagram text from user‑facing or symbol‑derived strings, as in the render_namespace_diagram_code function, to ensure the output remains valid Mermaid.'
layout: doc
template: doc
---

# `clore::generate::escape_mermaid_label`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/diagram.cppm:27`

Definition: `src/generate/render/diagram.cppm:123`

Implementation: [`Module generate:diagram`](../../../../modules/generate/diagram.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

The function `clore::generate::escape_mermaid_label` takes a `std::string_view` and returns a `std::string` that is sanitized for safe inclusion as a label in Mermaid diagram markup. Callers should supply any raw text that may contain characters special to Mermaid label syntax—such as quotes, parentheses, newlines, or angle brackets—and the returned value can be directly embedded in Mermaid diagram code without breaking the syntax or causing rendering errors. This is required when generating diagram text from user‑facing or symbol‑derived strings, as in the `render_namespace_diagram_code` function, to ensure the output remains valid Mermaid.

## Usage Patterns

- Sanitizing labels for Mermaid diagrams
- Called by `clore::generate::render_namespace_diagram_code`

## Called By

- function `clore::generate::render_namespace_diagram_code`

