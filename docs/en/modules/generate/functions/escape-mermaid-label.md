---
title: 'clore::generate::escapemermaidlabel'
description: 'The function iterates character by character through the input std::string_view, pre-allocating a std::string with reserve to avoid reallocation. The core logic is a switch that transforms special characters: a backslash is doubled ("\\\\"), a double quote is preceded by a backslash ("\\\""), and line breaks (\n, \r) are replaced with a single space; all other characters are copied unchanged. This ensures that Mermaid labels are properly quoted and do not break the diagram syntax. The function relies solely on standard library types (std::string_view, std::string) and has no other dependencies, serving as a low-level utility called by the various Mermaid diagram rendering functions in the same translation unit.'
layout: doc
template: doc
---

# `clore::generate::escape_mermaid_label`

Owner: [Module generate:diagram](../diagram.md)

Declaration: `src/generate/render/diagram.cppm:27`

Definition: `src/generate/render/diagram.cppm:123`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto escape_mermaid_label(std::string_view text) -> std::string {
    std::string escaped;
    escaped.reserve(text.size());
    for(auto ch: text) {
        switch(ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n':
            case '\r': escaped += ' '; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}
```

The function iterates character by character through the input `std::string_view`, pre-allocating a `std::string` with `reserve` to avoid reallocation. The core logic is a `switch` that transforms special characters: a backslash is doubled (`"\\\\"`), a double quote is preceded by a backslash (`"\\\""`), and line breaks (`\n`, `\r`) are replaced with a single space; all other characters are copied unchanged. This ensures that Mermaid labels are properly quoted and do not break the diagram syntax. The function relies solely on standard library types (`std::string_view`, `std::string`) and has no other dependencies, serving as a low-level utility called by the various Mermaid diagram rendering functions in the same translation unit.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `text` (`std::string_view`)

## Writes To

- returned `std::string` (the escaped label)

## Usage Patterns

- Sanitizing labels for Mermaid diagrams
- Called by `clore::generate::render_namespace_diagram_code`

## Called By

- function `clore::generate::render_namespace_diagram_code`

