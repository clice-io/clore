---
title: 'clore::generate::makerelativelinktarget'
description: 'The function clore::generate::make_relative_link_target computes a relative filesystem path from one documentation page to another. It normalizes both current_page_path and target_page_path using std::filesystem::path::lexically_normal, then determines the base directory by taking parent_path of the current path (falling back to . if none exists). The relative path is calculated via lexically_relative from that base; if the resulting path is empty (indicating the target is already identical to the base), the function falls back to returning the target’s generic_string form. Otherwise it returns the relative path in generic format. The implementation relies entirely on std::filesystem operations and uses no external dependencies beyond the C++ standard library.'
layout: doc
template: doc
---

# `clore::generate::make_relative_link_target`

Owner: [Module generate:common](../common.md)

Declaration: `src/generate/render/common.cppm:68`

Definition: `src/generate/render/common.cppm:68`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto make_relative_link_target(std::string_view current_page_path,
                               std::string_view target_page_path) -> std::string {
    namespace fs = std::filesystem;

    auto current = fs::path(current_page_path).lexically_normal();
    auto target = fs::path(target_page_path).lexically_normal();
    auto base = current.has_parent_path() ? current.parent_path() : fs::path{"."};
    auto rel = target.lexically_relative(base);
    if(rel.empty()) {
        return target.generic_string();
    }
    return rel.generic_string();
}
```

The function `clore::generate::make_relative_link_target` computes a relative filesystem path from one documentation page to another. It normalizes both `current_page_path` and `target_page_path` using `std::filesystem::path::lexically_normal`, then determines the base directory by taking `parent_path` of the current path (falling back to `.` if none exists). The relative path is calculated via `lexically_relative` from that base; if the resulting path is empty (indicating the target is already identical to the base), the function falls back to returning the target’s `generic_string` form. Otherwise it returns the relative path in generic format. The implementation relies entirely on `std::filesystem` operations and uses no external dependencies beyond the C++ standard library.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `current_page_path`
- `target_page_path`

## Writes To

- return value

## Usage Patterns

- Called by `clore::generate::make_link_target` to produce a relative link target for documentation pages.

## Called By

- function `clore::generate::make_link_target`

