---
title: 'clore::extract::ensurecachekey'
description: 'clore::extract::ensure_cache_key 保证给定的 CompileEntry 对象包含一个可用于后续缓存查询的唯一键。调用该函数后，条目可以通过该键在 query_toolchain_cached 等例程中查找对应的缓存数据。该函数会就地修改传入的 CompileEntry，其行为依赖于该条目已有的编译参数；调用者应确保传入的条目处于可被赋键的状态。'
layout: doc
template: doc
---

# `clore::extract::ensure_cache_key`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/compiler.cppm:76`

Definition: `src/extract/compiler.cppm:241`

Implementation: [`Module extract:compiler`](../../../../modules/extract/compiler.md)

## Declaration

```cpp
auto (CompileEntry &) -> void;
```

`clore::extract::ensure_cache_key` 保证给定的 `CompileEntry` 对象包含一个可用于后续缓存查询的唯一键。调用该函数后，条目可以通过该键在 `query_toolchain_cached` 等例程中查找对应的缓存数据。该函数会就地修改传入的 `CompileEntry`，其行为依赖于该条目已有的编译参数；调用者应确保传入的条目处于可被赋键的状态。

## Usage Patterns

- Called prior to `query_toolchain_cached`.

## Calls

- function `clore::extract::ensure_cache_key_impl`

## Called By

- function `clore::extract::query_toolchain_cached`

