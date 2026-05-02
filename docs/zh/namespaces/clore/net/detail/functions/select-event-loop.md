---
title: 'clore::net::detail::selecteventloop'
description: '函数 clore::net::detail::select_event_loop 接受一个可选的 kota::event_loop * 指针，并返回一个 kota::event_loop & 引用。如果传入的指针非空，则返回该指针所指向的事件循环；若指针为空，函数会选择一个默认的事件循环并返回其引用。调用者无需关心默认事件循环的具体选择策略，只需保证在需要自定义事件循环时传入有效指针，否则可传入 nullptr 以使用默认实例。该函数主要用于内部异步操作的调度环境决策，确保调用方总能获得一个有效的事件循环引用来注册回调或等待完成。'
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

函数 `clore::net::detail::select_event_loop` 接受一个可选的 `kota::event_loop *` 指针，并返回一个 `kota::event_loop &` 引用。如果传入的指针非空，则返回该指针所指向的事件循环；若指针为空，函数会选择一个默认的事件循环并返回其引用。调用者无需关心默认事件循环的具体选择策略，只需保证在需要自定义事件循环时传入有效指针，否则可传入 `nullptr` 以使用默认实例。该函数主要用于内部异步操作的调度环境决策，确保调用方总能获得一个有效的事件循环引用来注册回调或等待完成。

## Usage Patterns

- 用于将可选的 `event_loop*` 解析为确定的引用
- 被 `call_completion_async`、`call_llm_async` 等高层函数调用以获取事件循环

## Called By

- function `clore::net::call_completion_async`
- function `clore::net::call_llm_async`
- function `clore::net::call_llm_async`

