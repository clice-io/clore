---
title: 'clore::net::icontains'
description: '检查第一个 std::string_view 参数是否包含第二个，比较时忽略字母大小写。该函数返回 true 当且仅当在 haystack 中找到 needle 的匹配（不区分大小写），否则返回 false。'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Namespace clore::net](../index.md)

Declaration: `network/protocol.cppm:768`

Definition: `network/protocol.cppm:768`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> bool;
```

检查第一个 `std::string_view` 参数是否包含第二个，比较时忽略字母大小写。该函数返回 `true` 当且仅当在 `haystack` 中找到 `needle` 的匹配（不区分大小写），否则返回 `false`。

此函数主要供内部字符串匹配场景使用，例如 `clore::net::is_feature_rejection_error` 会用它来判断错误消息是否与已知的拒绝模式相匹配。调用方应确保两个字符串均有效，且比较行为对 ASCII 大小写不敏感。

## Usage Patterns

- Used by `clore::net::is_feature_rejection_error` to perform case-insensitive matching on error messages.

## Called By

- function `clore::net::is_feature_rejection_error`

