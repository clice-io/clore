---
title: 'clore::extract::scanmoduledecl'
description: 'clore::extract::scan_module_decl 对 C++ 模块声明执行快速扫描，利用 Clang 的依赖指令扫描器来提取模块信息，而无需运行完整的预处理器。调用者必须提供一个 std::string_view 形式的待扫描源文本（例如模块单元的源代码片段）以及一个可写入的 ScanResult & 输出对象。该函数会填充 ScanResult 中的 module_name、is_interface_unit 和 module_imports 字段；如果输入不包含合法的模块声明，这些字段将保持其默认状态或为空，函数本身不会报告错误。调用者应根据填充后的 ScanResult 内容判断扫描结果是否有意义。'
layout: doc
template: doc
---

# `clore::extract::scan_module_decl`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/scan.cppm:67`

Definition: `src/extract/scan.cppm:159`

Implementation: [`Module extract:scan`](../../../../modules/extract/scan.md)

## Declaration

```cpp
auto (std::string_view, ScanResult &) -> void;
```

`clore::extract::scan_module_decl` 对 C++ 模块声明执行快速扫描，利用 Clang 的依赖指令扫描器来提取模块信息，而无需运行完整的预处理器。调用者必须提供一个 `std::string_view` 形式的待扫描源文本（例如模块单元的源代码片段）以及一个可写入的 `ScanResult &` 输出对象。该函数会填充 `ScanResult` 中的 `module_name`、`is_interface_unit` 和 `module_imports` 字段；如果输入不包含合法的模块声明，这些字段将保持其默认状态或为空，函数本身不会报告错误。调用者应根据填充后的 `ScanResult` 内容判断扫描结果是否有意义。

## Usage Patterns

- Called by `clore::extract::scan_file` to quickly extract module-level declarations without full preprocessing.
- Designed for scanning source files to determine module name, interface status, and imported modules.

## Called By

- function `clore::extract::scan_file`

