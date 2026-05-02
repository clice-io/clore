---
title: 'clore::extract::scanmoduledecl'
description: '函数 clore::extract::scan_module_decl 对给定的 C++ 源代码字符串执行快速的模块声明扫描，并将结果写入指定的 ScanResult & 对象。它使用 Clang 的依赖指令扫描器，只解析模块声明部分，而无需运行完整的预处理器，因此效率较高。函数会填充 ScanResult 中的 module_name、is_interface_unit 和 module_imports 字段，其余字段保持不变。'
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

函数 `clore::extract::scan_module_decl` 对给定的 C++ 源代码字符串执行快速的模块声明扫描，并将结果写入指定的 `ScanResult &` 对象。它使用 Clang 的依赖指令扫描器，只解析模块声明部分，而无需运行完整的预处理器，因此效率较高。函数会填充 `ScanResult` 中的 `module_name`、`is_interface_unit` 和 `module_imports` 字段，其余字段保持不变。

调用者需提供包含有效 C++ 模块声明源代码的 `std::string_view`，并保证 `ScanResult` 对象在函数调用期间可被写入。该函数为无异常安全契约（`void` 返回），若输入字符串不包含可识别的模块声明，相关字段可能保持默认值，函数本身不报告错误。实际使用时通常由 `clore::extract::scan_file` 等上层函数将整体文件扫描的结果封装为 `std::expected` 形式。

## Usage Patterns

- Called by `clore::extract::scan_file` during source file scanning to populate module metadata.
- Used as a lightweight alternative to full preprocessing for extracting module information.

## Called By

- function `clore::extract::scan_file`

