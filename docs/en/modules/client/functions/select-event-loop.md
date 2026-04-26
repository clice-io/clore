---
title: 'clore::net::detail::selecteventloop'
description: 'The function clore::net::detail::select_event_loop provides a single dispatch point for obtaining an active event loop reference. It first checks whether the incoming pointer loop is non‑null; if so, it dereferences and returns that loop directly. When loop is nullptr, the function falls back to kota::event_loop::current(), which must return a valid loop—this precondition is undefined if no event loop is active on the calling thread. The implementation thus relies on the runtime state of the kota::event_loop singleton and ensures the caller always receives a usable loop reference without further validation.'
layout: doc
template: doc
---

# `clore::net::detail::select_event_loop`

Owner: [Module client](../index.md)

Declaration: `network/client.cppm:45`

Definition: `network/client.cppm:45`

Declaration: [`Namespace clore::net::detail`](../../../namespaces/clore/net/detail/index.md)

## Implementation

```cpp
auto select_event_loop(kota::event_loop* loop) -> kota::event_loop& {
    if(loop != nullptr) {
        return *loop;
    }
    // PRECONDITION: kota::event_loop::current() must return a valid loop.
    // If no event loop is active on this thread, behavior is undefined.
    return kota::event_loop::current();
}
```

The function `clore::net::detail::select_event_loop` provides a single dispatch point for obtaining an active event loop reference. It first checks whether the incoming pointer `loop` is non‑null; if so, it dereferences and returns that loop directly. When `loop` is `nullptr`, the function falls back to `kota::event_loop::current()`, which must return a valid loop—this precondition is undefined if no event loop is active on the calling thread. The implementation thus relies on the runtime state of the `kota::event_loop` singleton and ensures the caller always receives a usable loop reference without further validation.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `loop`
- thread-local state accessed by `kota::event_loop::current()`

## Usage Patterns

- Used by async functions like `call_completion_async` and `call_llm_async` to resolve an optional event loop pointer to a valid reference before passing it to internal logic.

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

