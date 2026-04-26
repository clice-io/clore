---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: 'The function checks whether text is empty and exits immediately if so, avoiding unnecessary work. When text is non‑empty, it inspects target: if target already contains some content, a double newline ("\n\n") separator is appended first, ensuring the new text is visually separated from any prior text. Finally, the content of text is appended to target. No external dependencies beyond the standard library’s std::string and std::string_view are required; the control flow is a straightforward linear sequence with a single conditional branch for the gap insertion.'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Module anthropic](../index.md)

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Declaration: [`Namespace clore::net::anthropic::protocol::detail`](../../../namespaces/clore/net/anthropic/protocol/detail/index.md)

## Implementation

```cpp
auto append_text_with_gap(std::string& target, std::string_view text) -> void {
    if(text.empty()) {
        return;
    }
    if(!target.empty()) {
        target += "\n\n";
    }
    target += text;
}
```

The function checks whether `text` is empty and exits immediately if so, avoiding unnecessary work. When `text` is non‑empty, it inspects `target`: if `target` already contains some content, a double newline (`"\n\n"`) separator is appended first, ensuring the new text is visually separated from any prior text. Finally, the content of `text` is appended to `target`. No external dependencies beyond the standard library’s `std::string` and `std::string_view` are required; the control flow is a straightforward linear sequence with a single conditional branch for the gap insertion.

## Side Effects

- Modifies target string by appending text and optionally a gap

## Reads From

- Parameter `target` (to check emptiness)
- Parameter `text` (to read content)

## Writes To

- Parameter `target` (modified in place)

## Usage Patterns

- Used in `build_request_json` to assemble text blocks with gaps

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

