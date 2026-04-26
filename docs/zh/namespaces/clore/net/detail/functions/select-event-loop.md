---
title: 'clore::net::detail::selecteventloop'
description: 'clore::net::detail::select_event_loop 接收一个指向 kota::event_loop 的可选指针，并返回该事件循环的引用。如果指针非空，函数直接返回其指向的循环；如果指针为空，则返回一个实现定义的默认事件循环（通常是当前线程关联的默认实例）。调用者应确保在异步操作完成之前，所使用的 kota::event_loop 对象保持存活，无论是由调用者提供还是由默认实例提供。该函数主要用于将可选的 kota::event_loop* 参数转换为一个确定的引用，以便内部逻辑无需处理空指针。'
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

`clore::net::detail::select_event_loop` 接收一个指向 `kota::event_loop` 的可选指针，并返回该事件循环的引用。如果指针非空，函数直接返回其指向的循环；如果指针为空，则返回一个实现定义的默认事件循环（通常是当前线程关联的默认实例）。调用者应确保在异步操作完成之前，所使用的 `kota::event_loop` 对象保持存活，无论是由调用者提供还是由默认实例提供。该函数主要用于将可选的 `kota::event_loop*` 参数转换为一个确定的引用，以便内部逻辑无需处理空指针。

## Usage Patterns

- Used by `call_completion_async` and `call_llm_async` to obtain a valid event loop reference from an optional pointer
- Invoked with a potentially null `kota::event_loop*` to default to the current loop

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

