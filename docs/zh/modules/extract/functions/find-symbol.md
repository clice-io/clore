---
title: 'clore::extract::findsymbol'
description: '函数 clore::extract::find_symbol 的实现首先调用 clore::extract::find_symbols，传入相同的 model 和 qualified_name。若返回的匹配集合 matches 长度恰好为 1，则函数返回集合中的第一个元素，否则返回 nullptr。此实现依赖 clore::extract::find_symbols 完成实际的符号查找与匹配逻辑，自身仅负责将多匹配或零匹配场景统一转换为单指针返回值，从而简化调用方的歧义处理。'
layout: doc
template: doc
---

# `clore::extract::find_symbol`

Owner: [Module extract:model](../model.md)

Declaration: `src/extract/model.cppm:196`

Definition: `src/extract/model.cppm:388`

Declaration: [`Namespace clore::extract`](../../../namespaces/clore/extract/index.md)

## Implementation

```cpp
auto find_symbol(const ProjectModel& model, std::string_view qualified_name) -> const SymbolInfo* {
    auto matches = find_symbols(model, qualified_name);
    if(matches.size() != 1) {
        return nullptr;
    }
    return matches.front();
}
```

函数 `clore::extract::find_symbol` 的实现首先调用 `clore::extract::find_symbols`，传入相同的 `model` 和 `qualified_name`。若返回的匹配集合 `matches` 长度恰好为 1，则函数返回集合中的第一个元素，否则返回 `nullptr`。此实现依赖 `clore::extract::find_symbols` 完成实际的符号查找与匹配逻辑，自身仅负责将多匹配或零匹配场景统一转换为单指针返回值，从而简化调用方的歧义处理。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `model` (const `ProjectModel` reference)
- `qualified_name` (`std::string_view`)

## Usage Patterns

- Used as a convenience wrapper to retrieve a uniquely matching symbol by qualified name
- Called by the overload `clore::extract::find_symbol` that takes an additional `std::string_view` parameter

## Called By

- function `clore::extract::find_symbol`

