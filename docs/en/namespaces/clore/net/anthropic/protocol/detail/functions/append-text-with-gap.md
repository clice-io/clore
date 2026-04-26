---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: 'The function clore::net::anthropic::protocol::detail::append_text_with_gap is a helper used during request construction to incrementally build a string containing text content. It accepts a target string (by reference) and a text fragment to append. If the target string already holds content, the function inserts a gap—typically a newline or delimiter—before the new fragment, ensuring that separate text pieces are clearly separated in the final output. Callers provide the accumulating string and the next text fragment; after the call, the target string is extended with the fragment (and a preceding gap when needed), preserving correct formatting for the larger JSON request body.'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::detail::append_text_with_gap`

Owner: [Namespace clore::net::anthropic::protocol::detail](../index.md)

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Implementation: [`Module anthropic`](../../../../../../../modules/anthropic/index.md)

## Declaration

```cpp
auto (std::string &, std::string_view) -> void;
```

The function `clore::net::anthropic::protocol::detail::append_text_with_gap` is a helper used during request construction to incrementally build a string containing text content. It accepts a target string (by reference) and a text fragment to append. If the target string already holds content, the function inserts a gap—typically a newline or delimiter—before the new fragment, ensuring that separate text pieces are clearly separated in the final output. Callers provide the accumulating string and the next text fragment; after the call, the target string is extended with the fragment (and a preceding gap when needed), preserving correct formatting for the larger JSON request body.

## Usage Patterns

- Used in `build_request_json` to assemble text blocks with gaps

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

