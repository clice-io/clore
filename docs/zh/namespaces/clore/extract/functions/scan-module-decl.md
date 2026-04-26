---
title: 'clore::extract::scanmoduledecl'
description: '函数 clore::extract::scan_module_decl 接收一个 std::string_view 表示的待扫描内容和一个可变的 ScanResult 引用。调用者负责提供有效的输入字符串，并确保 ScanResult 对象在其生命周期内有效；函数会向该结果中填充模块名称、是否为接口单元以及模块导入列表等字段，而无需运行完整的预处理器。此函数专为快速扫描模块声明而设计，是更高层函数（如 clore::extract::scan_file）的底层依赖，调用者无需关心其内部实现，但应使用 ScanResult 中填充的信息来构建模块依赖关系。'
layout: doc
template: doc
---

# `clore::extract::scan_module_decl`

Owner: [Namespace clore::extract](../index.md)

Declaration: `extract/scan.cppm:49`

Definition: `extract/scan.cppm:141`

Implementation: [`Module extract:scan`](../../../../modules/extract/scan.md)

## Declaration

```cpp
auto (std::string_view, ScanResult &) -> void;
```

函数 `clore::extract::scan_module_decl` 接收一个 `std::string_view` 表示的待扫描内容和一个可变的 `ScanResult` 引用。调用者负责提供有效的输入字符串，并确保 `ScanResult` 对象在其生命周期内有效；函数会向该结果中填充模块名称、是否为接口单元以及模块导入列表等字段，而无需运行完整的预处理器。此函数专为快速扫描模块声明而设计，是更高层函数（如 `clore::extract::scan_file`）的底层依赖，调用者无需关心其内部实现，但应使用 `ScanResult` 中填充的信息来构建模块依赖关系。

## Usage Patterns

- Called by `clore::extract::scan_file` to perform fast module scanning on source files.
- Used as a lightweight alternative to full preprocessing for module dependency discovery.

## Called By

- function `clore::extract::scan_file`

