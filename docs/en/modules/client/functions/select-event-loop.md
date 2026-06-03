---
title: 'clore::net::detail::selecteventloop'
description: 'The function first checks whether the incoming loop pointer is non-null. If it is, the function immediately dereferences it and returns that reference, preserving an explicitly provided loop. Otherwise, it falls back to kota::event_loop::current(), which returns a reference to the active event loop for the calling thread. The implementation relies on the precondition that this static method always yields a valid loop; if no loop is active on the thread, the behavior is undefined. This two‑step control flow allows callers to optionally supply a loop while ensuring that a valid reference is always obtained without dynamic allocation.'
layout: doc
template: doc
---

# `clore::net::detail::select_event_loop`

Owner: [Module client](../index.md)

Declaration: `src/network/client.cppm:53`

Definition: `src/network/client.cppm:53`

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

The function first checks whether the incoming `loop` pointer is non-null. If it is, the function immediately dereferences it and returns that reference, preserving an explicitly provided loop. Otherwise, it falls back to `kota::event_loop::current()`, which returns a reference to the active event loop for the calling thread. The implementation relies on the precondition that this static method always yields a valid loop; if no loop is active on the thread, the behavior is undefined. This two‑step control flow allows callers to optionally supply a loop while ensuring that a valid reference is always obtained without dynamic allocation.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `loop`
- current thread's event loop via `kota::event_loop::current()`

## Usage Patterns

- used by `clore::net::call_llm_async` to resolve an event loop reference
- used by `clore::net::call_completion_async` to resolve an event loop reference
- provides fallback to current loop when caller passes null

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

