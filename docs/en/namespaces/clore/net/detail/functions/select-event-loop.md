---
title: 'clore::net::detail::selecteventloop'
description: 'The function clore::net::detail::select_event_loop accepts a pointer to a kota::event_loop and returns a reference to a kota::event_loop. Its responsibility is to resolve an event loop for use in asynchronous operations: if the provided pointer is non‑null, the function returns a reference to that same loop; if the pointer is null, it selects and returns a default or fallback event loop. The caller can rely on the returned reference being valid for the duration of the asynchronous operation. No other pre‑conditions are imposed beyond providing a pointer that, when not null, points to a valid kota::event_loop.'
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

The function `clore::net::detail::select_event_loop` accepts a pointer to a `kota::event_loop` and returns a reference to a `kota::event_loop`. Its responsibility is to resolve an event loop for use in asynchronous operations: if the provided pointer is non‑null, the function returns a reference to that same loop; if the pointer is null, it selects and returns a default or fallback event loop. The caller can rely on the returned reference being valid for the duration of the asynchronous operation. No other pre‑conditions are imposed beyond providing a pointer that, when not null, points to a valid `kota::event_loop`.

## Usage Patterns

- Resolves an optional event loop pointer into a guaranteed-valid reference for downstream async operations
- Allows callers to pass `nullptr` to request the current thread's event loop

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

