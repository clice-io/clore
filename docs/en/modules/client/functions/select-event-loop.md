---
title: 'clore::net::detail::selecteventloop'
description: 'The function implements a simple fallback mechanism: it checks whether the provided loop pointer is non‑null, and if so, returns a reference to that object directly. If the pointer is null, it delegates to kota::event_loop::current(), which is expected to return a reference to the event loop active on the calling thread. The precondition is that a valid loop exists on the thread when the pointer is null; otherwise the behavior is undefined. The only external dependency is the kota::event_loop type and its static current() member.'
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

The function implements a simple fallback mechanism: it checks whether the provided `loop` pointer is non‑null, and if so, returns a reference to that object directly. If the pointer is null, it delegates to `kota::event_loop::current()`, which is expected to return a reference to the event loop active on the calling thread. The precondition is that a valid loop exists on the thread when the pointer is null; otherwise the behavior is undefined. The only external dependency is the `kota::event_loop` type and its static `current()` member.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `loop` (nullable pointer to `kota::event_loop`)
- result of `kota::event_loop::current()`

## Usage Patterns

- Resolves an optional event loop pointer into a guaranteed-valid reference for downstream async operations
- Allows callers to pass `nullptr` to request the current thread's event loop

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

