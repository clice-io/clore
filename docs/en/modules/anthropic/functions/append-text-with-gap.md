---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: 'The function clore::net::anthropic::protocol::detail::append_text_with_gap appends a given std::string_view text to a std::string& target while inserting a gap separator when both strings are non‑empty. The control flow begins with an early return if text is empty, preserving the existing content of target. If target is not already empty, a double newline ("\n\n") is appended to separate the previously stored content from the incoming text. Finally, the text itself is appended. This ensures that accumulated text blocks are visually separated by a blank line, while avoiding leading whitespace for the first block. The implementation relies solely on std::string and std::string_view operations, with no external dependencies beyond the standard library.'
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

The function `clore::net::anthropic::protocol::detail::append_text_with_gap` appends a given `std::string_view text` to a `std::string& target` while inserting a gap separator when both strings are non‑empty. The control flow begins with an early return if `text` is empty, preserving the existing content of `target`. If `target` is not already empty, a double newline (`"\n\n"`) is appended to separate the previously stored content from the incoming `text`. Finally, the `text` itself is appended. This ensures that accumulated text blocks are visually separated by a blank line, while avoiding leading whitespace for the first block. The implementation relies solely on `std::string` and `std::string_view` operations, with no external dependencies beyond the standard library.

## Side Effects

- Mutates the `target` string by appending `text` and optionally inserting a double-newline separator.

## Reads From

- `target` parameter (reads its current content to check if empty for separator insertion)
- `text` parameter (reads its content and checks emptiness)

## Writes To

- `target` parameter (appends separator and `text` content)

## Usage Patterns

- Used by `build_request_json` to accumulate JSON text blocks with gap separation.

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

