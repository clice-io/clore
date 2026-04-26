---
title: 'clore::extract::normalizeentryfile'
description: '返回与指定 clore::extract::CompileEntry 关联的规范化源文件路径。该路径是条目的主要输入文件经过标准化后的表现形式，可用于唯一标识编译单元，并作为构建签名或缓存键的组成部分。调用方应依赖该函数提供的稳定标识符，而非原始的文件名或路径。'
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

返回与指定 `clore::extract::CompileEntry` 关联的规范化源文件路径。该路径是条目的主要输入文件经过标准化后的表现形式，可用于唯一标识编译单元，并作为构建签名或缓存键的组成部分。调用方应依赖该函数提供的稳定标识符，而非原始的文件名或路径。

## Usage Patterns

- Used by `build_compile_signature` to create a stable file key
- Used by `ensure_cache_key_impl` to normalize the file path before caching

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

