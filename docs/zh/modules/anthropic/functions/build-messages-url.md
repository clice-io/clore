---
title: 'clore::net::anthropic::protocol::buildmessagesurl'
description: '该函数首先将输入的 api_base 复制到本地 url 字符串中，并通过循环移除尾部可能存在的连续斜杠字符，对基础 URL 进行标准化。接着判断标准化后的 url 是否已以 "/v1" 结尾：若是，则直接调用 clore::net::detail::append_url_path 追加 "messages" 路径段；否则追加 "v1/messages" 完整路径。整个流程的核心依赖是 clore::net::detail::append_url_path，用于安全拼接路径段并处理 / 分隔。'
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

该函数首先将输入的 `api_base` 复制到本地 `url` 字符串中，并通过循环移除尾部可能存在的连续斜杠字符，对基础 URL 进行标准化。接着判断标准化后的 `url` 是否已以 `"/v1"` 结尾：若是，则直接调用 `clore::net::detail::append_url_path` 追加 `"messages"` 路径段；否则追加 `"v1/messages"` 完整路径。整个流程的核心依赖是 `clore::net::detail::append_url_path`，用于安全拼接路径段并处理 `/` 分隔。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- 参数 `api_base`

## Usage Patterns

- 被 `clore::net::anthropic::detail::Protocol::build_url` 调用

## Called By

- method `clore::net::anthropic::detail::Protocol::build_url`

