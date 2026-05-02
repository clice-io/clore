---
title: 'clore::net::anthropic::protocol::detail::appendtextwithgap'
description: 'This function appends the content of a std::string_view to a std::string, inserting a separator or “gap” as required by the protocol’s text‑block formatting rules. It is designed for internally composing multi‑segment text content (for example, within a message or content block) where a structural break must be placed between adjacent pieces of text. The caller supplies the target string as the first argument, which is mutated in place, and the incoming text as the second argument.'
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

This function appends the content of a `std::string_view` to a `std::string`, inserting a separator or “gap” as required by the protocol’s text‑block formatting rules. It is designed for internally composing multi‑segment text content (for example, within a message or content block) where a structural break must be placed between adjacent pieces of text. The caller supplies the target string as the first argument, which is mutated in place, and the incoming text as the second argument.

## Usage Patterns

- Used by `build_request_json` to accumulate JSON text blocks with gap separation.

## Called By

- function `clore::net::anthropic::protocol::build_request_json`

