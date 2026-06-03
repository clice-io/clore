---
title: 'clore::extract::findsymbol'
description: 'The implementation of clore::extract::find_symbol delegates to clore::extract::find_symbols to collect all symbols matching the given qualified_name within the ProjectModel. If the returned container of matches contains exactly one element, that element (a pointer to SymbolInfo) is returned; otherwise, nullptr is returned to indicate either no match or an ambiguous result. The function serves as a restricted query that requires a unique resolution, relying entirely on the internal logic of find_symbols (which traverses the model’s symbol tables, namespace hierarchies, and module storage) for candidate collection. No additional caching or state management is performed at this level.'
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

The implementation of `clore::extract::find_symbol` delegates to `clore::extract::find_symbols` to collect all symbols matching the given `qualified_name` within the `ProjectModel`. If the returned container of matches contains exactly one element, that element (a pointer to `SymbolInfo`) is returned; otherwise, `nullptr` is returned to indicate either no match or an ambiguous result. The function serves as a restricted query that requires a unique resolution, relying entirely on the internal logic of `find_symbols` (which traverses the model’s symbol tables, namespace hierarchies, and module storage) for candidate collection. No additional caching or state management is performed at this level.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `model` (the project model's symbol index)
- `qualified_name` (the lookup string)

## Usage Patterns

- Used to resolve a uniquely identifiable symbol by qualified name.
- Called by the three-parameter overload to refine lookup by module.

## Called By

- function `clore::extract::find_symbol`

