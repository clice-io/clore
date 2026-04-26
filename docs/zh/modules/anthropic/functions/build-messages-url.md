---
title: 'clore::net::anthropic::protocol::buildmessagesurl'
description: '该函数首先将传入的 api_base 复制到本地 url 字符串，然后通过一个 while 循环去除尾部所有连续的斜杠字符，直到 url 为空或末尾不是斜杠。随后检查 url 是否以 "/v1" 结尾，若是则调用 clore::net::detail::append_url_path 将 "messages" 追加到路径后；否则追加 "v1/messages"。实现依赖于 clore::net::detail::append_url_path 完成路径拼接，整个流程仅包含一次去除尾部斜杠的循环和一次条件分支。'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::build_messages_url`

Owner: [Module anthropic](../index.md)

Declaration: `network/anthropic.cppm:201`

Definition: `network/anthropic.cppm:224`

Declaration: [`Namespace clore::net::anthropic::protocol`](../../../namespaces/clore/net/anthropic/protocol/index.md)

## Implementation

```cpp
auto build_messages_url(std::string_view api_base) -> std::string {
    std::string url(api_base);
    while(!url.empty() && url.back() == '/') {
        url.pop_back();
    }
    if(url.ends_with("/v1")) {
        return clore::net::detail::append_url_path(url, "messages");
    }
    return clore::net::detail::append_url_path(url, "v1/messages");
}
```

该函数首先将传入的 `api_base` 复制到本地 `url` 字符串，然后通过一个 `while` 循环去除尾部所有连续的斜杠字符，直到 `url` 为空或末尾不是斜杠。随后检查 `url` 是否以 `"/v1"` 结尾，若是则调用 `clore::net::detail::append_url_path` 将 `"messages"` 追加到路径后；否则追加 `"v1/messages"`。实现依赖于 `clore::net::detail::append_url_path` 完成路径拼接，整个流程仅包含一次去除尾部斜杠的循环和一次条件分支。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `api_base` parameter
- string literal `"/v1"`
- `clore::net::detail::append_url_path`

## Writes To

- local variable `url`
- returned `std::string`

## Usage Patterns

- Called by `clore::net::anthropic::detail::Protocol::build_url` to produce the messages endpoint URL

## Called By

- method `clore::net::anthropic::detail::Protocol::build_url`

