---
title: 'clore::net::icontains'
description: 'clore::net::icontains 检查第一个 std::string_view 是否包含第二个 std::string_view，比较时不区分大小写。如果第二个参数是第一个参数的子串（忽略 ASCII 大小写）则返回 true，否则返回 false。调用者提供两个视图，函数不修改它们也不要求它们以空字符结尾。'
layout: doc
template: doc
---

# `clore::net::icontains`

Owner: [Namespace clore::net](../index.md)

Declaration: `src/network/protocol.cppm:780`

Definition: `src/network/protocol.cppm:780`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> bool;
```

`clore::net::icontains` 检查第一个 `std::string_view` 是否包含第二个 `std::string_view`，比较时不区分大小写。如果第二个参数是第一个参数的子串（忽略 ASCII 大小写）则返回 `true`，否则返回 `false`。调用者提供两个视图，函数不修改它们也不要求它们以空字符结尾。

## Usage Patterns

- Used by `clore::net::is_feature_rejection_error` to perform case-insensitive substring matching on error strings.

## Called By

- function `clore::net::is_feature_rejection_error`

