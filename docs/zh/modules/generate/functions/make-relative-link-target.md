---
title: 'clore::generate::makerelativelinktarget'
description: '该函数使用 std::filesystem 库计算两个规范化路径之间的相对链接目标。首先将 current_page_path 和 target_page_path 通过 lexically_normal 标准化，消除冗余的 . 和 .. 成分。以当前路径的父目录（若无父目录则使用 "."）作为基础，调用 target.lexically_relative(base) 生成相对路径。若结果为空（表明目标路径与基础相同或无法相对表达），则直接返回目标路径的通用字符串形式；否则返回该相对路径的通用字符串形式（均使用正斜杠分隔），确保结果符合 URI 或文件系统链接规范。整个逻辑仅依赖标准文件系统设施，无外部依赖。'
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

该函数使用 `std::filesystem` 库计算两个规范化路径之间的相对链接目标。首先将 `current_page_path` 和 `target_page_path` 通过 `lexically_normal` 标准化，消除冗余的 `.` 和 `..` 成分。以当前路径的父目录（若无父目录则使用 `"."`）作为基础，调用 `target.lexically_relative(base)` 生成相对路径。若结果为空（表明目标路径与基础相同或无法相对表达），则直接返回目标路径的通用字符串形式；否则返回该相对路径的通用字符串形式（均使用正斜杠分隔），确保结果符合 URI 或文件系统链接规范。整个逻辑仅依赖标准文件系统设施，无外部依赖。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `current_page_path`
- `target_page_path`

## Usage Patterns

- Called by `clore::generate::make_link_target` to compute a relative path for a link target.

## Called By

- function `clore::generate::make_link_target`

