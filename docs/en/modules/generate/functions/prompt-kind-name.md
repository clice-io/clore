---
title: 'clore::generate::promptkindname'
description: 'The function clore::generate::prompt_kind_name maps each enumerator of clore::generate::PromptKind to a corresponding std::string_view string literal. It uses a simple switch statement over the kind parameter, returning a descriptive, underscore-separated name such as "namespace_summary", "module_architecture", "function_analysis", or "type_implementation_summary". If the kind does not match any known enumerator (which should not occur in normal usage), the fallback return is "unknown_prompt". No external dependencies or sub-calls are involved; the function is a pure, constant‑time lookup from enum to string.'
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

The function `clore::generate::prompt_kind_name` maps each enumerator of `clore::generate::PromptKind` to a corresponding `std::string_view` string literal. It uses a simple `switch` statement over the `kind` parameter, returning a descriptive, underscore-separated name such as `"namespace_summary"`, `"module_architecture"`, `"function_analysis"`, or `"type_implementation_summary"`. If the `kind` does not match any known enumerator (which should not occur in normal usage), the fallback return is `"unknown_prompt"`. No external dependencies or sub-calls are involved; the function is a pure, constant‑time lookup from enum to string.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `kind` parameter

## Usage Patterns

- Called by `clore::generate::prompt_request_key` to derive a string key for a given prompt request.

## Called By

- function `clore::generate::prompt_request_key`

