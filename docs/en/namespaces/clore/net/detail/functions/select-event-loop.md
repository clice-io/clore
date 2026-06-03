---
title: 'clore::net::detail::selecteventloop'
description: 'The function clore::net::detail::select_event_loop accepts an optional pointer to a kota::event_loop and returns a reference to a valid kota::event_loop. When the pointer is non-null, it returns the referenced loop; when null, it returns a default event loop suitable for the calling context. This function is used by higher-level async operations to ensure they always operate on a live event loop without requiring callers to provide one explicitly.'
layout: doc
template: doc
---

# `clore::net::detail::select_event_loop`

Owner: [Namespace clore::net::detail](../index.md)

Declaration: `src/network/client.cppm:53`

Definition: `src/network/client.cppm:53`

Implementation: [`Module client`](../../../../../modules/client/index.md)

## Declaration

```cpp
auto (kota::event_loop *) -> kota::event_loop &;
```

The function `clore::net::detail::select_event_loop` accepts an optional pointer to a `kota::event_loop` and returns a reference to a valid `kota::event_loop`. When the pointer is non-null, it returns the referenced loop; when null, it returns a default event loop suitable for the calling context. This function is used by higher-level async operations to ensure they always operate on a live event loop without requiring callers to provide one explicitly.

## Usage Patterns

- used by `clore::net::call_llm_async` to resolve an event loop reference
- used by `clore::net::call_completion_async` to resolve an event loop reference
- provides fallback to current loop when caller passes null

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

