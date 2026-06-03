---
title: 'clore::generate::makerelativelinktarget'
description: '函数 clore::generate::make_relative_link_target 接受两个 std::string_view 参数，分别代表当前页面的路径和目标页面的路径，并返回一个 std::string，表示从当前页定位到目标页的相对链接路径。调用者应确保传入的路径是规范化后的、可比较的字符串；该函数仅计算两者之间的相对关系，不校验路径有效性或存在性。返回值可直接用于构建 LinkTarget 对象，例如在 clore::generate::make_link_target 内部被调用来生成实际链接。'
layout: doc
template: doc
---

# `clore::generate::make_relative_link_target`

Owner: [Namespace clore::generate](../index.md)

Declaration: `src/generate/render/common.cppm:68`

Definition: `src/generate/render/common.cppm:68`

Implementation: [`Module generate:common`](../../../../modules/generate/common.md)

## Declaration

```cpp
auto (std::string_view, std::string_view) -> std::string;
```

函数 `clore::generate::make_relative_link_target` 接受两个 `std::string_view` 参数，分别代表当前页面的路径和目标页面的路径，并返回一个 `std::string`，表示从当前页定位到目标页的相对链接路径。调用者应确保传入的路径是规范化后的、可比较的字符串；该函数仅计算两者之间的相对关系，不校验路径有效性或存在性。返回值可直接用于构建 `LinkTarget` 对象，例如在 `clore::generate::make_link_target` 内部被调用来生成实际链接。

## Usage Patterns

- Called by `clore::generate::make_link_target` to compute a relative path for a link target.

## Called By

- function `clore::generate::make_link_target`

