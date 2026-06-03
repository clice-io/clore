---
title: 'clore::extract::findsymbol'
description: '函数 clore::extract::find_symbol 在给定的 ProjectModel 中查找并返回一个指向 SymbolInfo 的指针，该指针表示与指定限定名称 std::string_view 匹配的符号。如果提供了可选的 std::string_view 签名参数，则函数会进一步筛选，返回第一个签名完全匹配的符号；若签名参数为空，则行为与只接受限定名称的重载相同，即返回任意一个匹配该名称的符号。未找到匹配符号时返回 nullptr。'
layout: doc
template: doc
---

# `clore::extract::find_symbol`

Owner: [Namespace clore::extract](../index.md)

Declaration: `src/extract/model.cppm:196`

Definition: `src/extract/model.cppm:388`

Implementation: [`Module extract:model`](../../../../modules/extract/model.md)

## Declaration

```cpp
auto (const ProjectModel &, std::string_view) -> const SymbolInfo *;
```

函数 `clore::extract::find_symbol` 在给定的 `ProjectModel` 中查找并返回一个指向 `SymbolInfo` 的指针，该指针表示与指定限定名称 `std::string_view` 匹配的符号。如果提供了可选的 `std::string_view` 签名参数，则函数会进一步筛选，返回第一个签名完全匹配的符号；若签名参数为空，则行为与只接受限定名称的重载相同，即返回任意一个匹配该名称的符号。未找到匹配符号时返回 `nullptr`。

## Usage Patterns

- Used as a convenience wrapper to retrieve a uniquely matching symbol by qualified name
- Called by the overload `clore::extract::find_symbol` that takes an additional `std::string_view` parameter

## Called By

- function `clore::extract::find_symbol`

