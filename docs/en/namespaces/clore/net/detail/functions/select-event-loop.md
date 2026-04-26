---
title: 'clore::net::detail::selecteventloop'
description: 'The function clore::net::detail::select_event_loop accepts a pointer to a kota::event_loop and returns a reference to a kota::event_loop. If the provided pointer is non-null, the function dereferences it and returns that same loop. If the pointer is null, the function selects an internal default event loop and returns a reference to it. The caller must ensure that any non-null pointer passed to the function points to a valid, running kota::event_loop that remains alive for the duration of the operation. The return value is guaranteed to be a valid reference.'
layout: doc
template: doc
---

# `clore::net::detail::select_event_loop`

Owner: [Namespace clore::net::detail](../index.md)

Declaration: `network/client.cppm:45`

Definition: `network/client.cppm:45`

Implementation: [`Module client`](../../../../../modules/client/index.md)

## Declaration

```cpp
auto (kota::event_loop *) -> kota::event_loop &;
```

The function `clore::net::detail::select_event_loop` accepts a pointer to a `kota::event_loop` and returns a reference to a `kota::event_loop`. If the provided pointer is non-null, the function dereferences it and returns that same loop. If the pointer is null, the function selects an internal default event loop and returns a reference to it. The caller must ensure that any non-null pointer passed to the function points to a valid, running `kota::event_loop` that remains alive for the duration of the operation. The return value is guaranteed to be a valid reference.

## Usage Patterns

- Used by async functions like `call_completion_async` and `call_llm_async` to resolve an optional event loop pointer to a valid reference before passing it to internal logic.

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

