---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: 'The function clore::net::anthropic::protocol::detail::append_text_with_gap appends the content of text (a std::string_view) to the target std::string. It first guards against an empty input by returning immediately if text is empty. When target already contains data, it inserts a double newline separator ("\n\n") to create a visual gap before concatenating the new fragment. This ensures that the first piece of text is not preceded by an unnecessary separator while subsequent pieces are cleanly spaced. The implementation uses only standard library types and no external dependencies.'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Module anthropic](../index.md)

Declaration: `src/network/anthropic.cppm:34`

Definition: `src/network/anthropic.cppm:34`

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

The function `clore::net::anthropic::protocol::detail::append_text_with_gap` appends the content of `text` (a `std::string_view`) to the `target` `std::string`. It first guards against an empty input by returning immediately if `text` is empty. When `target` already contains data, it inserts a double newline separator (`"\n\n"`) to create a visual gap before concatenating the new fragment. This ensures that the first piece of text is not preceded by an unnecessary separator while subsequent pieces are cleanly spaced. The implementation uses only standard library types and no external dependencies.

## Side Effects

- mutates `target` by appending a gap and/or `text`

## Reads From

- `target` contents via `target.empty()`
- `text` via `text.empty()` and `text` value

## Writes To

- `target` string

## Usage Patterns

- called by `build_request_json` to concatenate text segments with separation

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

