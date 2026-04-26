---
title: 'clore::extract::normalizeentryfile'
description: '函数 clore::extract::normalize_entry_file 通过组合 std::filesystem 操作将 entry.file 规范化为绝对路径并解析链接依赖。首先检查路径是否为相对路径，若是则与 entry.directory 拼接；接着调用 std::filesystem::absolute 生成绝对形式，然后通过 lexically_normal 做词法上的规范化。为处理符号链接，进一步尝试 weakly_canonical：若失败（如路径不存在或权限不足）则回退到词法规范化结果，最终以 Unix 风格返回 generic_string。'
layout: doc
template: doc
---

# `clore::extract::normalize_entry_file`

Owner: [Module extract:compiler](../compiler.md)

Declaration: `extract/compiler.cppm:56`

Definition: `extract/compiler.cppm:91`

Declaration: [`Namespace clore::extract`](../../../namespaces/clore/extract/index.md)

## Implementation

```cpp
auto normalize_entry_file(const CompileEntry& entry) -> std::string {
    namespace fs = std::filesystem;
    auto path = fs::path(entry.file);
    if(path.is_relative()) {
        path = fs::path(entry.directory) / path;
    }
    std::error_code ec;
    auto absolute = fs::absolute(path, ec);
    if(!ec) {
        path = std::move(absolute);
    }
    path = path.lexically_normal();
    auto canonical = fs::weakly_canonical(path, ec);
    if(!ec) {
        return canonical.generic_string();
    }
    return path.generic_string();
}
```

函数 `clore::extract::normalize_entry_file` 通过组合 `std::filesystem` 操作将 `entry.file` 规范化为绝对路径并解析链接依赖。首先检查路径是否为相对路径，若是则与 `entry.directory` 拼接；接着调用 `std::filesystem::absolute` 生成绝对形式，然后通过 `lexically_normal` 做词法上的规范化。为处理符号链接，进一步尝试 `weakly_canonical`：若失败（如路径不存在或权限不足）则回退到词法规范化结果，最终以 Unix 风格返回 `generic_string`。

该函数主要依赖 `std::filesystem::path` 及其错误处理（通过 `std::error_code`），不涉及外部文件系统缓存或其他模块。控制流简洁：仅处理相对路径分支和回退分支，确保在无法解析真实路径时仍返回格式一致的结果。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `entry.file`
- `entry.directory`

## Usage Patterns

- Used by `build_compile_signature` to create a stable file key
- Used by `ensure_cache_key_impl` to normalize the file path before caching

## Called By

- function `clore::extract::build_compile_signature`
- function `clore::extract::ensure_cache_key_impl`

