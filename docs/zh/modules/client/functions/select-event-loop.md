---
title: 'clore::net::detail::selecteventloop'
description: '该函数的行为基于一个简单的三元分支：如果传入的 loop 指针非空，则直接解引用并返回该 kota::event_loop 对象；否则，调用静态方法 kota::event_loop::current() 获取当前线程的活动事件循环引用。其内部依赖 kota::event_loop 的 current() 实现，该实现必须返回一个有效的事件循环——若当前线程无关联的活跃循环，行为未定义。此函数是 clore::net::detail 命名空间下的一个简化的返回引用工具，用于在其他异步调用函数（如 call_llm_async）中统一获取事件循环实例，避免重复执行空指针检查或默认回退逻辑。'
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

该函数的行为基于一个简单的三元分支：如果传入的 `loop` 指针非空，则直接解引用并返回该 `kota::event_loop` 对象；否则，调用静态方法 `kota::event_loop::current()` 获取当前线程的活动事件循环引用。其内部依赖 `kota::event_loop` 的 `current()` 实现，该实现必须返回一个有效的事件循环——若当前线程无关联的活跃循环，行为未定义。此函数是 `clore::net::detail` 命名空间下的一个简化的返回引用工具，用于在其他异步调用函数（如 `call_llm_async`）中统一获取事件循环实例，避免重复执行空指针检查或默认回退逻辑。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `loop` 参数
- `kota::event_loop::current()` 返回的当前线程事件循环状态

## Usage Patterns

- 用于将可选的 `event_loop*` 解析为确定的引用
- 被 `call_completion_async`、`call_llm_async` 等高层函数调用以获取事件循环

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

