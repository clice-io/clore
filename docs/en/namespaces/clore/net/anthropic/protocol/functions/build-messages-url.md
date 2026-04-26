---
title: 'clore::net::anthropic::protocol::buildmessagesurl'
description: 'Constructs the complete URL for the Anthropic Messages API endpoint. Given a string view that identifies the target resource (for example, an API key or version token), clore::net::anthropic::protocol::build_messages_url returns the fully qualified URL as a std::string. The caller must supply a valid string view; no validation is performed within the function.'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::build_messages_url`

Owner: [Namespace clore::net::anthropic::protocol](../index.md)

Declaration: `network/anthropic.cppm:201`

Definition: `network/anthropic.cppm:224`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

Constructs the complete URL for the Anthropic Messages API endpoint. Given a string view that identifies the target resource (for example, an API key or version token), `clore::net::anthropic::protocol::build_messages_url` returns the fully qualified URL as a `std::string`. The caller must supply a valid string view; no validation is performed within the function.

## Usage Patterns

- Called by `clore::net::anthropic::detail::Protocol::build_url`

## Called By

- method `clore::net::anthropic::detail::Protocol::build_url`

