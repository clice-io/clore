---
title: 'clore::generate::promptkindname'
description: '函数 clore::generate::prompt_kind_name 的实现基于一个纯粹的 switch 语句，将每个 PromptKind 枚举值映射到一个预定义的 snake_case 字符串标识符。该映射覆盖了所有已定义的 PromptKind 枚举成员（如 PromptKind::NamespaceSummary、PromptKind::FunctionAnalysis 等），并为未识别的枚举值返回回退字符串 "unknown_prompt"。内部不涉及任何条件分支以外的控制流，也不依赖其他函数或外部状态，完全依赖于 PromptKind 枚举的定义。'
layout: doc
template: doc
---

# `clore::generate::prompt_kind_name`

Owner: [Module generate:model](../model.md)

Declaration: `src/generate/model.cppm:48`

Definition: `src/generate/model.cppm:289`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto prompt_kind_name(PromptKind kind) -> std::string_view {
    switch(kind) {
        case PromptKind::NamespaceSummary: return "namespace_summary";
        case PromptKind::ModuleSummary: return "module_summary";
        case PromptKind::ModuleArchitecture: return "module_architecture";
        case PromptKind::IndexOverview: return "index_overview";
        case PromptKind::FunctionAnalysis: return "function_analysis";
        case PromptKind::TypeAnalysis: return "type_analysis";
        case PromptKind::VariableAnalysis: return "variable_analysis";
        case PromptKind::FunctionDeclarationSummary: return "function_declaration_summary";
        case PromptKind::FunctionImplementationSummary: return "function_implementation_summary";
        case PromptKind::TypeDeclarationSummary: return "type_declaration_summary";
        case PromptKind::TypeImplementationSummary: return "type_implementation_summary";
    }
    return "unknown_prompt";
}
```

函数 `clore::generate::prompt_kind_name` 的实现基于一个纯粹的 `switch` 语句，将每个 `PromptKind` 枚举值映射到一个预定义的 `snake_case` 字符串标识符。该映射覆盖了所有已定义的 `PromptKind` 枚举成员（如 `PromptKind::NamespaceSummary`、`PromptKind::FunctionAnalysis` 等），并为未识别的枚举值返回回退字符串 `"unknown_prompt"`。内部不涉及任何条件分支以外的控制流，也不依赖其他函数或外部状态，完全依赖于 `PromptKind` 枚举的定义。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- the `PromptKind` parameter `kind`

## Usage Patterns

- called by `clore::generate::prompt_request_key` to produce a string-based key for a prompt request

## Called By

- function `clore::generate::prompt_request_key`

