---
title: 'Module extract:filter'
description: 'extract:filter 模块负责提供路径过滤与解析的核心工具集，用于在图形数据结构中筛选、规范化及比较路径。其公开实现围绕一组以整数标识符抽象路径的函数展开，包括路径前缀匹配、相对路径计算、规范化转换、目录下路径解析以及过滤判断，同时定义一个 PathResolveError 异常类型用于表示解析失败。模块内部维护多个缓存变量（如 filter_root、relative_str）以支持高效的状态管理，并依赖 config 和 std 模块提供配置与基础支持。'
layout: doc
template: doc
---

# Module `extract:filter`

## Summary

`extract:filter` 模块负责提供路径过滤与解析的核心工具集，用于在图形数据结构中筛选、规范化及比较路径。其公开实现围绕一组以整数标识符抽象路径的函数展开，包括路径前缀匹配、相对路径计算、规范化转换、目录下路径解析以及过滤判断，同时定义一个 `PathResolveError` 异常类型用于表示解析失败。模块内部维护多个缓存变量（如 `filter_root`、`relative_str`）以支持高效的状态管理，并依赖 `config` 和 `std` 模块提供配置与基础支持。

## Imports

- [`config`](../config/index.md)
- `std`

## Imported By

- [`extract:merge`](merge.md)

## Types

### `clore::extract::PathResolveError`

Declaration: `extract/filter.cppm:8`

Definition: `extract/filter.cppm:8`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::PathResolveError` 的实现仅包含一个 `std::string message` 数据成员，用以承载错误描述文本。该结构体本身是平凡可复制、可移动的，其复制、移动、赋值和析构行为完全由 `std::string` 的对应操作隐式提供。内部不存在额外的对齐控制或自定义分配器逻辑，因此消息的存储与管理完全委托给标准字符串。作为错误类型，该结构体不维护超过 `message` 内容之外的任何状态，也不要求 `message` 必须非空——调用者可以将其构造为默认的空字符串。这一简单设计使 `PathResolveError` 可以在 `clore::extract` 内部作为轻量级错误载体使用，直接通过字符串内容传达失败原因，无需额外的错误码或枚举。

## Functions

### `clore::extract::canonical_graph_path`

Declaration: `extract/filter.cppm:21`

Definition: `extract/filter.cppm:103`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数尝试将输入路径转换为规范的通用字符串形式，核心采用分级回退策略。首先通过 `std::filesystem::absolute` 将路径转为绝对形式，若成功则应用 `std::filesystem::weakly_canonical` 做弱规范化，并结合 `lexically_normal` 处理多余分隔符或 `.`/`..` 成分；若任一阶段出现 `std::error_code`，则回退到仅对原始路径执行 `lexically_normal`，再用 `weakly_canonical` 尝试第二次规范化。所有成功路径最终均调用 `generic_string` 输出以正斜杠分隔的字符串。

实现依赖标准库 `<filesystem>` 的 `absolute`、`weakly_canonical`、`lexically_normal` 以及 `generic_string`，并通过 `error_code` 捕获可能的环境错误（如路径不存在或权限问题）。当 `weakly_canonical` 失败时，函数会回退到规范化后的字符串形式，确保始终返回一个可用的表示，不抛出异常。

#### Side Effects

- Filesystem queries to resolve symlinks and determine absolute path (via `std::filesystem::absolute` and `std::filesystem::weakly_canonical`)

#### Reads From

- filesystem (via `std::filesystem::absolute` and `std::filesystem::weakly_canonical`)
- input parameter `path` of type `const std::filesystem::path&`

#### Usage Patterns

- Normalizing file paths for consistent representation in a graph
- Canonicalizing paths for use as identifiers or keys
- Handling filesystem errors gracefully by falling back to lexical normalization

### `clore::extract::filter_root_path`

Declaration: `extract/filter.cppm:27`

Definition: `extract/filter.cppm:161`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该实现首先检查 `config.workspace_root` 是否为空；若不为空，则直接返回 `std::filesystem::path(config.workspace_root)` 经过 `lexically_normal()` 规范化的结果。否则，回退到 `config.project_root` 并执行相同的规范化操作。整个流程仅依赖 `config::TaskConfig` 的两个字段和 `<filesystem>` 的路径规范化方法，无额外分支或错误处理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `config::TaskConfig::workspace_root`
- `config::TaskConfig::project_root`

#### Usage Patterns

- Used to obtain a normalized root directory path from task configuration
- Provides the effective base path for filtering or extraction operations

### `clore::extract::matches_filter`

Declaration: `extract/filter.cppm:23`

Definition: `extract/filter.cppm:124`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先通过 `canonical_graph_path` 规范化输入参数 `file` 和 `filter_root`，得到 `file_path` 与 `root_path`，再调用 `project_relative_path` 计算相对于过滤根目录的项目相对路径。若相对路径计算失败（返回 `std::nullopt`），则直接返回 `false`。随后将相对路径转换为通用字符串 `relative_str`，依次在 `filter.include` 和 `filter.exclude` 上应用 `path_prefix_matches` 进行前缀匹配：若 `include` 列表非空且无任何模式匹配，立即返回 `false`；若 `exclude` 中任一模式匹配，同样返回 `false`；否则返回 `true`。

控制流为典型的“先用 include 白名单筛选，再用 exclude 黑名单剔除”。所有路径操作依赖 `canonical_graph_path` 和 `project_relative_path`，匹配逻辑依赖 `path_prefix_matches` 与 `config::FilterRule` 的结构字段。未处理文件系统错误（如路径不存在），但 `project_relative_path` 内部可能抛异常或返回空值。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `file` parameter
- `filter` parameter (including `filter.include` and `filter.exclude`)
- `filter_root` parameter

#### Usage Patterns

- Filtering source files during project extraction
- Evaluating include/exclude rules for scanning
- Deciding whether to process a compilation entry

### `clore::extract::path_prefix_matches`

Declaration: `extract/filter.cppm:12`

Definition: `extract/filter.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::path_prefix_matches` 通过纯 `std::string_view` 操作判断 `relative` 是否以 `pattern` 为路径前缀，内部不依赖任何外部项目函数。算法首先处理空输入和尾部斜杠：若 `pattern` 为空直接返回 `false`；然后循环删除尾部 `/`，再次检查空字符串。之后按 `pattern` 是否包含 `'/'` 分两支：若包含，要求 `relative` 必须以 `pattern` 开头且长度要么完全相等，要么紧跟 `/`；若不包含，则除精确匹配外，还需确保 `relative` 长度至少比 `pattern` 多 1，且后一个字符为 `/`，从而避免匹配部分文件名（如 `pattern` 为 `foo` 而 `relative` 为 `foobar`）。控制流仅依赖 `std::string_view` 的成员函数（`empty`、`back`、`remove_suffix`、`find`、`starts_with`、`size`、`operator[]`），无其他依赖性。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `relative` of type `std::string_view`
- parameter `pattern` of type `std::string_view`

#### Usage Patterns

- used as a predicate to filter file paths that belong to a given directory prefix
- likely called from path-matching logic in `clore::extract::matches_filter` or other filter functions

### `clore::extract::project_relative_path`

Declaration: `extract/filter.cppm:14`

Definition: `extract/filter.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先通过调用 `std::filesystem::path::lexically_relative` 计算输入参数 `file` 相对于 `root_path` 的词汇相对路径，结果存入 `rel`。随后检查 `rel` 是否为空——若为空则直接返回 `std::nullopt`，表示无法建立相对关系。接着遍历 `rel` 的各组成部分，若发现任意一部分等于 `".."`，表明路径试图跳出 `root_path` 目录范围，此时同样返回 `std::nullopt`。仅当上述两项检查均通过时，才返回 `rel` 作为有效的相对路径。

整个实现仅依赖标准库的 `std::filesystem` 组件，未调用其他 `clore::extract` 内部函数。核心逻辑围绕 `lexically_relative` 的返回值进行简单验证，无复杂控制流或外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `std::filesystem::path`& file
- const `std::filesystem::path`& `root_path`

#### Usage Patterns

- Compute a safe project-relative path for a file under a given root
- Ensure a file path does not escape a project directory by rejecting `..` components

### `clore::extract::resolve_path_under_directory`

Declaration: `extract/filter.cppm:18`

Definition: `extract/filter.cppm:79`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该实现首先对空路径进行防御性检查，若 `path` 为空则返回带有 `PathResolveError::message` 的意外结果。然后通过 `std::filesystem::path` 构造路径对象 `p`；若 `p` 为相对路径，则要求 `directory` 非空，否则同样返回错误。在 `directory` 非空时，将 `directory` 与 `p` 拼接形成绝对路径。最后对所有路径调用 `p.lexically_normal()` 以消除冗余的 `.` 和 `..` 组件，返回规范化后的 `std::filesystem::path`。整个流程无其他内部函数依赖，仅依赖于 `std::filesystem` 库和自定义错误类型 `PathResolveError`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `path` parameter (`std::string`)
- the `directory` parameter (`std::string`)

#### Writes To

- the returned `std::expected<std::filesystem::path, PathResolveError>` object

#### Usage Patterns

- resolving file paths from `compile_commands.json` entries
- normalizing relative paths against a project root directory

## Internal Structure

该模块实现了路径过滤的核心逻辑，依赖 `config` 模块提供过滤根目录和规则，依赖 `std` 提供基础类型和算法。内部按职责分为三层：底层是路径解析，通过 `canonical_graph_path`、`resolve_path_under_directory` 等函数将输入的路径标识符转换为规范化的内部表示；中层是匹配层，提供 `path_prefix_matches` 和 `matches_filter` 等谓词函数，用于判断路径是否符合配置中的过滤模式；上层是应用接口，如 `filter_root_path`、`project_relative_path`，将解析与匹配组合为可直接被调用者使用的功能。实现结构围绕一组公开变量（如 `filter_root`、`root_path`、`config`、`pattern`）和模块作用域的局部变量（如 `relative_str`、`rel_opt`、`matched`）展开，通过 `PathResolveError` 结构体规范化错误处理流程，确保路径操作的可靠性。

## Related Pages

- [Module config](../config/index.md)

