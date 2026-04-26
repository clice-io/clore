---
title: 'clore::net::detail::selecteventloop'
description: '该函数首先检查传入的 loop 指针是否为空；若非空则直接返回其引用，否则回退到通过 kota::event_loop::current() 获取当前线程的默认事件循环。这一选择逻辑依赖于调用方保证当 loop 为空时当前线程已关联一个有效的 kota::event_loop 实例，否则行为未定义。内部没有循环或异步操作，仅是一个简单的条件转发，将外部提供的循环或线程局部默认循环统一为引用返回。'
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

该函数首先检查传入的 `loop` 指针是否为空；若非空则直接返回其引用，否则回退到通过 `kota::event_loop::current()` 获取当前线程的默认事件循环。这一选择逻辑依赖于调用方保证当 `loop` 为空时当前线程已关联一个有效的 `kota::event_loop` 实例，否则行为未定义。内部没有循环或异步操作，仅是一个简单的条件转发，将外部提供的循环或线程局部默认循环统一为引用返回。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `loop` parameter
- `kota::event_loop::current()` result

## Usage Patterns

- Used by `call_completion_async` and `call_llm_async` to obtain a valid event loop reference from an optional pointer
- Invoked with a potentially null `kota::event_loop*` to default to the current loop

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

