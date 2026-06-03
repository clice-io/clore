---
title: 'clore::net::detail::selecteventloop'
description: 'clore::net::detail::select_event_loop 的实现遵循一个简单的分支策略。它首先检查 loop 参数是否为非空指针；若不为空，则立即返回通过解引用该指针获得的 kota::event_loop 引用。否则，它将依赖项委托给静态成员函数 kota::event_loop::current()，该函数按约定返回一个与调用线程关联的事件循环引用。如果当前线程上没有已注册的事件循环，此回退路径的行为是未定义的。'
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

`clore::net::detail::select_event_loop` 的实现遵循一个简单的分支策略。它首先检查 `loop` 参数是否为非空指针；若不为空，则立即返回通过解引用该指针获得的 `kota::event_loop` 引用。否则，它将依赖项委托给静态成员函数 `kota::event_loop::current()`，该函数按约定返回一个与调用线程关联的事件循环引用。如果当前线程上没有已注册的事件循环，此回退路径的行为是未定义的。

该函数的核心控制流仅有两条路径，没有循环或递归。其正确性依赖于调用方要么提供一个有效的 `loop` 指针，要么确保 `kota::event_loop::current()` 在当前线程上下文中有可返回的合法对象。它没有额外的错误检查或日志记录，因此性能开销极低。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `loop` (pointer to `kota::event_loop`)
- current event loop via `kota::event_loop::current()`

## Usage Patterns

- callers like `clore::net::call_llm_async` and `clore::net::call_completion_async` pass an optional `kota::event_loop*` to obtain a guaranteed valid reference for async operations

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

