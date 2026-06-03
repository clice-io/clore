---
title: 'Module extract:filter'
description: 'extract:filter 模块负责处理文件系统路径的规范化、筛选和解析操作，为提取流程提供路径匹配与安全访问的基础设施。它公开了 canonical_graph_path、filter_root_path、matches_filter、project_relative_path、resolve_path_under_directory 和 path_prefix_matches 等函数，以及用于报告路径解析错误的 PathResolveError 结构体。这些接口共同支持将任意路径转换为图可用的规范形式、根据过滤器规则判断路径是否匹配、计算项目相对路径、安全地将路径限定在指定目录下，以及检查路径前缀匹配，从而确保文件系统操作的一致性和安全性。'
layout: doc
template: doc
---

# Module `extract:filter`

## Summary

`extract:filter` 模块负责处理文件系统路径的规范化、筛选和解析操作，为提取流程提供路径匹配与安全访问的基础设施。它公开了 `canonical_graph_path`、`filter_root_path`、`matches_filter`、`project_relative_path`、`resolve_path_under_directory` 和 `path_prefix_matches` 等函数，以及用于报告路径解析错误的 `PathResolveError` 结构体。这些接口共同支持将任意路径转换为图可用的规范形式、根据过滤器规则判断路径是否匹配、计算项目相对路径、安全地将路径限定在指定目录下，以及检查路径前缀匹配，从而确保文件系统操作的一致性和安全性。

## Imports

- [`config`](../config/index.md)

## Imported By

- [`extract:merge`](merge.md)

## Types

### `clore::extract::PathResolveError`

Declaration: `src/extract/filter.cppm:17`

Definition: `src/extract/filter.cppm:17`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::PathResolveError` 是一个仅包含单一成员 `message` 的聚合类型，用于存储路径解析失败时的错误描述。该成员为 `std::string` 类型，承担保存人类可读错误信息的职责。实现上未定义任何构造函数或赋值运算符，依赖编译器生成的默认成员函数，因此其不变量完全由调用方维护——`message` 的内容应在构造后保持有效且不包含未初始化状态。作为轻量级错误载体，该类型不提供任何比较或序列化操作，其内部结构直接反映了作为简单错误值传递的设计意图。

#### Invariants

- `message` 成员始终包含有效的字符串，可能为空但不应为未初始化状态

#### Key Members

- `message`（`std::string`）：存储路径解析错误的描述信息

#### Usage Patterns

- 被用作路径解析函数的错误返回类型或异常包装
- 其他代码通过检查 `message` 内容来获取错误详情

## Functions

### `clore::extract::canonical_graph_path`

Declaration: `src/extract/filter.cppm:30`

Definition: `src/extract/filter.cppm:112`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/canonical-graph-path.md)

该函数通过分层回退策略将任意文件系统路径转换为规范的图形路径表示形式。首先尝试使用 `std::filesystem::absolute` 和 `std::filesystem::weakly_canonical` 解析路径，同时通过 `std::error_code` 捕获错误。若成功，则返回弱规范化路径的 `generic_string`；若失败，则回退到绝对路径的 `lexically_normal` 版本。如果绝对路径解析本身失败，则直接对输入路径进行 `lexically_normal`，再次尝试 `weakly_canonical`，最终返回最有效的规范化结果。整个过程避免异常抛出，完全依赖于 `<filesystem>` 库提供的操作系统级路径解析能力。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- filesystem via `fs::absolute` (reads current working directory and path components)
- filesystem via `fs::weakly_canonical` (reads symlinks and directory existence)

#### Usage Patterns

- called by `matches_filter` to obtain a consistent path for filtering

### `clore::extract::filter_root_path`

Declaration: `src/extract/filter.cppm:36`

Definition: `src/extract/filter.cppm:170`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过检查 `config.workspace_root` 是否为空来决定返回路径：若不为空，则返回其 `lexically_normal()` 形式；否则返回 `config.project_root` 的规范化结果。过程依赖 `config::TaskConfig` 结构体的两个成员，并利用 `std::filesystem::path::lexically_normal` 进行路径标准化，没有分支合并或复杂算法。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- config`.workspace_root`
- config`.project_root`

#### Usage Patterns

- derive canonical root for path filtering
- obtain normalized workspace or project root

### `clore::extract::matches_filter`

Declaration: `src/extract/filter.cppm:32`

Definition: `src/extract/filter.cppm:133`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先通过 `clore::extract::canonical_graph_path` 规范化 `file` 和 `filter_root` 路径，得到 `file_path` 与 `root_path`，然后调用 `clore::extract::project_relative_path` 计算相对路径；若计算失败（例如文件不在根目录内），直接返回 `false`。将相对路径转为通用字符串 `relative_str` 后，依次处理 `filter.include` 中的模式：遍历每个 `pattern` 并借助 `clore::extract::path_prefix_matches` 检查是否匹配，若无一匹配则返回 `false`。接着处理 `filter.exclude` 中的模式，一旦有任意模式匹配同样返回 `false`。仅当 include 检查（若列表非空）与 exclude 检查均通过时，函数才返回 `true`。核心依赖为 `canonical_graph_path`、`project_relative_path` 与 `path_prefix_matches` 三个工具函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `file`
- parameter `filter` (a `config::FilterRule`)
- parameter `filter_root`
- `filter.include` (container of strings)
- `filter.exclude` (container of strings)
- return value of `canonical_graph_path`
- return value of `project_relative_path`

#### Usage Patterns

- called to decide whether a compilation entry's source file should be processed based on include/exclude rules
- used during extraction to filter files that match a given filter rule

### `clore::extract::path_prefix_matches`

Declaration: `src/extract/filter.cppm:21`

Definition: `src/extract/filter.cppm:42`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::path_prefix_matches` 判断一个路径前缀（`pattern`）是否匹配给定的相对路径（`relative`），其核心在于处理尾部斜杠并区分单组件与多组件前缀。该算法不依赖任何其他自定义函数，仅使用标准库的 `std::string_view` 操作（如 `empty`、`back`、`remove_suffix`、`find`、`starts_with` 以及下标访问）。

内部控制流首先排除空 `pattern` 并移除尾部所有 `/` 字符，若去除后为空则直接返回 `false`。随后根据 `pattern` 中是否包含 `/` 分为两路：若存在（多组件），则要求 `relative` 必须以 `pattern` 开头，且后续字符要么不存在（完全相等），要么是 `/`；若不存在（单组件），则在相等性检测失败后，额外检查 `relative` 长度是否至少比 `pattern` 多 1，以及 `relative` 是否以 `pattern` 开头且第 `pattern.size()` 个字符为 `/`。两种分支都确保 `pattern` 仅作为完整路径段的前缀，而非部分文件名。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `relative` (`std::string_view`)
- `pattern` (`std::string_view`)

#### Usage Patterns

- Filtering relative file paths against a directory or component prefix
- Path matching in `matches_filter` or similar predicate functions

### `clore::extract::project_relative_path`

Declaration: `src/extract/filter.cppm:23`

Definition: `src/extract/filter.cppm:73`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过调用 `file.lexically_relative(root_path)` 计算相对路径 `rel`。若 `rel.empty()` 成立（即 `file` 与 `root_path` 之间不存在词典上的相对关系），或 `rel` 的任一路径部件等于 `".."`（表明文件位于 `root_path` 的目录树之外），则立即返回 `std::nullopt`；否则直接返回 `rel`。算法完全依赖于 `std::filesystem::path` 的成员函数，不调用任何其他 `clore::extract` 内部函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `file` parameter
- `root_path` parameter
- the relative path `rel` computed by `lexically_relative`

#### Writes To

- local variable `rel`
- return value of type `std::optional<std::filesystem::path>`

#### Usage Patterns

- Convert absolute paths to project-relative paths for consistent representation
- Ensure a file path does not escape a given project root directory

### `clore::extract::resolve_path_under_directory`

Declaration: `src/extract/filter.cppm:27`

Definition: `src/extract/filter.cppm:88`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数首先检查传入的 `path` 是否为空，若为空则直接返回一个 `PathResolveError`，其中 `message` 指示“compilation database entry has empty file path”。将 `path` 转换为 `std::filesystem::path` 对象 `p` 后，若 `p.is_relative()` 为真，则校验 `directory` 是否非空：若为空则返回错误，提示需要提供非空目录；否则将 `directory` 与 `p` 拼接形成绝对路径。最后对所有情况都调用 `p.lexically_normal()` 进行词法规范化，并返回规范化后的路径。该实现依赖 `std::filesystem` 的路径操作和 `PathResolveError` 结构体，未使用其他内部辅助函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `path` parameter of type `const std::string&`
- the `directory` parameter of type `const std::string&`

#### Writes To

- the returned `std::filesystem::path` value (no external mutation)

#### Usage Patterns

- resolving file paths from compilation database entries
- combining a relative path with a base directory
- validating and normalizing path inputs before further processing

## Internal Structure

该模块 `extract:filter` 被分解为若干公开的路径查询与过滤函数，以及一些内部辅助工具，它们共同负责将文件系统路径转化为可用于过滤与匹配的规范化形式。对外接口包括 `matches_filter`、`filter_root_path`、`project_relative_path`、`resolve_path_under_directory`，以及一个错误类型 `PathResolveError`；内部则依赖 `canonical_graph_path`、`path_prefix_matches` 等私有函数实现路径标准化与前缀匹配。模块通过导入 `config` 依赖获得过滤规则（如 `FilterRule`），形成从配置到路径过滤的清晰分层，内部实现按照“路径规范化 → 相对化 → 前缀匹配”的步骤组织，并使用 `std::expected` 与自定义错误类型处理边界情况。

## Related Pages

- [Module config](../config/index.md)

