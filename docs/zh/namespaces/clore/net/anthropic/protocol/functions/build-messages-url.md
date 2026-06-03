---
title: 'clore::net::anthropic::protocol::buildmessagesurl'
description: '该函数接受一个表示 API 端点基础路径的 std::string_view，并返回一个完整的 std::string 形式的消息 API URL。调用方有责任提供正确的基础路径（例如原始 API 服务器地址），而无需关心 URL 内部拼接逻辑。返回值可直接用于 HTTP 请求中的目标地址。'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::build_messages_url`

Owner: [Namespace clore::net::anthropic::protocol](../index.md)

Declaration: `src/network/anthropic.cppm:210`

Definition: `src/network/anthropic.cppm:233`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

## Declaration

```cpp
auto (std::string_view) -> std::string;
```

该函数接受一个表示 API 端点基础路径的 `std::string_view`，并返回一个完整的 `std::string` 形式的消息 API URL。调用方有责任提供正确的基础路径（例如原始 API 服务器地址），而无需关心 URL 内部拼接逻辑。返回值可直接用于 HTTP 请求中的目标地址。

## Usage Patterns

- used by `clore::net::anthropic::detail::Protocol::build_url` to produce the final messages request URL
- called with various API base `URLs` to generate the appropriate versioned endpoint

## Called By

- method `clore::net::anthropic::detail::Protocol::build_url`

