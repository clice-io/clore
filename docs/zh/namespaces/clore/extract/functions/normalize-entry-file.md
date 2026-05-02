---
title: 'clore::extract::normalizeentryfile'
description: 'clore::extract::normalize_entry_file 接受一个 CompileEntry 引用，并返回一个 std::string，表示该条目对应源文件的规范化路径。调用者可以依赖返回的字符串在文件系统的不同表示形式（如符号链接解析、路径分隔符统一、相对路径转换为绝对路径等）之间保持稳定且可重复，从而用于缓存键的生成或等价性检查。该函数不修改传入的 CompileEntry，其结果是纯函数式的。'
layout: doc
template: doc
---

# `clore::extract::normalize_entry_file`

Owner: [Namespace clore::extract](../index.md)

Declaration: `extract/compiler.cppm:56`

Definition: `extract/compiler.cppm:91`

Implementation: [`Module extract:compiler`](../../../../modules/extract/compiler.md)

## Declaration

```cpp
auto (const CompileEntry &) -> std::string;
```

`clore::extract::normalize_entry_file` 接受一个 `CompileEntry` 引用，并返回一个 `std::string`，表示该条目对应源文件的规范化路径。调用者可以依赖返回的字符串在文件系统的不同表示形式（如符号链接解析、路径分隔符统一、相对路径转换为绝对路径等）之间保持稳定且可重复，从而用于缓存键的生成或等价性检查。该函数不修改传入的 `CompileEntry`，其结果是纯函数式的。

## Usage Patterns

- used in `clore::extract::build_compile_signature` to generate a hash key
- used in `clore::extract::ensure_cache_key_impl` to normalize the entry file before caching

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

