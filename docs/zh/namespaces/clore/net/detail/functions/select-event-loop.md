---
title: 'clore::net::detail::selecteventloop'
description: '给定一个可选的 kota::event_loop 指针，返回一个有效的 kota::event_loop 引用。若指针非空，则返回该指针所指向的事件循环；若指针为空，则提供当前线程的默认事件循环。此函数常用于需要为异步操作提供事件循环的调用方，以便统一处理传入的 nullptr 与显式指定的循环实例。'
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

给定一个可选的 `kota::event_loop` 指针，返回一个有效的 `kota::event_loop` 引用。若指针非空，则返回该指针所指向的事件循环；若指针为空，则提供当前线程的默认事件循环。此函数常用于需要为异步操作提供事件循环的调用方，以便统一处理传入的 `nullptr` 与显式指定的循环实例。

## Usage Patterns

- callers like `clore::net::call_llm_async` and `clore::net::call_completion_async` pass an optional `kota::event_loop*` to obtain a guaranteed valid reference for async operations

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

