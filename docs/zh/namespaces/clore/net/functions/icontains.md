---
title: 'clore::net::icontains'
description: '函数 clore::net::icontains 判断第一个字符串是否不区分大小写地包含第二个字符串。它接受两个 std::string_view 参数，并返回 bool 值：当且仅当 input 中（忽略大小写后）包含 substring 时返回 true。该函数是纯查询操作，不修改参数内容，其用途包括错误消息中的模式匹配，例如在 clore::net::is_feature_rejection_error 中用于检测特征拒绝模式。'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Namespace clore::net](../index.md)

Declaration: `network/protocol.cppm:758`

Definition: `network/protocol.cppm:758`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> bool;
```

函数 `clore::net::icontains` 判断第一个字符串是否不区分大小写地包含第二个字符串。它接受两个 `std::string_view` 参数，并返回 `bool` 值：当且仅当 `input` 中（忽略大小写后）包含 `substring` 时返回 `true`。该函数是纯查询操作，不修改参数内容，其用途包括错误消息中的模式匹配，例如在 `clore::net::is_feature_rejection_error` 中用于检测特征拒绝模式。

## Usage Patterns

- Used by `is_feature_rejection_error` to check if an error message contains a substring case-insensitively

## Called By

- function `clore::net::is_feature_rejection_error`

