---
title: 'Module extract:filter'
description: 'extract:filter 模块负责对路径标识符进行提取、过滤和规范化操作。它公开了一系列以整数参数传递路径标识的函数，包括将路径转换为过滤后的根路径标识、生成规范化的图路径、相对于指定目录解析路径、检查路径前缀匹配、应用内部过滤条件以及计算项目相对路径。模块依赖 config 获取配置数据，并依赖标准库实现底层路径处理。'
layout: doc
template: doc
---

# Module `extract:filter`

## Summary

`extract:filter` 模块负责对路径标识符进行提取、过滤和规范化操作。它公开了一系列以整数参数传递路径标识的函数，包括将路径转换为过滤后的根路径标识、生成规范化的图路径、相对于指定目录解析路径、检查路径前缀匹配、应用内部过滤条件以及计算项目相对路径。模块依赖 `config` 获取配置数据，并依赖标准库实现底层路径处理。

该模块拥有的公共实现范围包括：路径解析、前缀匹配、过滤判定和相对路径计算，同时提供 `PathResolveError` 错误类型用于报告路径解析失败时的信息。所有公开函数都操作预先注册的路径标识符，不直接执行文件系统 I/O，调用方需确保传入的标识符有效。

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

`clore::extract::PathResolveError` 是一个简单的错误类型，其内部仅包含一个 `message` 数据成员，类型为 `std::string`。该成员存储描述错误原因的可读文本。结构体没有用户定义的构造函数、析构函数或赋值操作符，完全依赖编译器生成的默认实现，因此其值语义与底层字符串一致。唯一隐含的不变量是 `message` 应包含有效的错误描述（通常由调用方保证），但结构体本身不强制执行任何约束。这种设计使其成为极轻量的结果封装，适合作为 `tl::expected` 或类似模式中的错误类型使用。

#### Invariants

- The `message` member contains a description of the error.

#### Key Members

- `message`

#### Usage Patterns

- Returned by functions that perform path resolution to indicate failure.
- Calling code can inspect `message` to display or log the error.

## Functions

### `clore::extract::canonical_graph_path`

Declaration: `extract/filter.cppm:21`

Definition: `extract/filter.cppm:103`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::canonical_graph_path` 实现了一个多级回退的路径规范化算法，旨在将给定的 `std::filesystem::path` 转换为尽可能规范的通用字符串表示。其核心流程首先通过 `std::filesystem::absolute` 将输入路径转为绝对形式，并使用 `std::error_code` 捕获可能的错误；若成功，则进一步调用 `std::filesystem::weakly_canonical` 配合 `lexically_normal` 生成规范路径，若全部成功则直接返回其通用字符串；若 `absolute` 阶段失败，则降级为对原始路径进行 `lexically_normal` 之后再尝试 `weakly_canonical`，同样通过错误码判断成功与否；若所有 `weakly_canonical` 尝试均失败，则最终返回仅经过 `lexically_normal` 处理的通用字符串。整个过程依赖 `std::filesystem` 库的路径操作函数，未引入外部项目内的依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path` parameter

#### Usage Patterns

- used to obtain a unique, normalised graph path for file-based nodes
- called during graph construction to canonicalize source file paths

### `clore::extract::filter_root_path`

Declaration: `extract/filter.cppm:27`

Definition: `extract/filter.cppm:161`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数依据 `config::TaskConfig` 中的工作区根路径配置进行决策：若 `config.workspace_root` 非空，则直接构造 `std::filesystem::path` 并调用 `lexically_normal()` 规范化后返回；否则回退至 `config.project_root`，同样经过 `lexically_normal()` 处理。其内部不涉及异常处理或路径存在性校验，仅通过条件分支选择规范化路径作为 `filter_root_path` 的结果。依赖包括 `std::filesystem::path` 的构造与规范化方法，以及 `config::TaskConfig` 的成员访问。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `config::TaskConfig::workspace_root`
- `config::TaskConfig::project_root`

#### Usage Patterns

- Used to obtain a canonical root path for filtering operations
- Called when a normalized base directory is required

### `clore::extract::matches_filter`

Declaration: `extract/filter.cppm:23`

Definition: `extract/filter.cppm:124`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::matches_filter` 首先通过 `canonical_graph_path` 规范化输入的两个路径 `file` 和 `filter_root`，生成 `file_path` 与 `root_path`。随后调用 `project_relative_path` 尝试计算从 `root_path` 到 `file_path` 的相对路径；若计算失败（返回空 optional），则直接返回 `false`。成功后将相对路径转换为通用字符串 `relative_str`。接着处理 `filter` 的 `include` 规则：若该列表非空，则遍历每个 `pattern` 并调用 `path_prefix_matches` 检查 `relative_str` 是否匹配，一旦发现匹配则设置标记并跳出；若全部遍历后未匹配任何 `include` 规则，函数返回 `false`。之后处理 `exclude` 规则：遍历每个 `pattern`，只要 `path_prefix_matches` 返回 `true` 即命中排除规则，立即返回 `false`。若所有检查均通过，最终返回 `true`。整个流程依赖于 `canonical_graph_path`、`project_relative_path` 和 `path_prefix_matches` 三个辅助函数完成路径解析与模式匹配。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `file` parameter
- `filter_root` parameter
- `filter.include` and `filter.exclude` member vectors

#### Usage Patterns

- Used to determine if a source file should be processed based on filtering rules

### `clore::extract::path_prefix_matches`

Declaration: `extract/filter.cppm:12`

Definition: `extract/filter.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数检查给定的 `relative` 路径是否以 `pattern` 为前缀，并确保匹配发生在完整的路径段边界上。算法首先处理 `pattern` 为空或尾部有多余斜杠的边缘情况，将其规范化。若 `pattern` 本身包含斜杠，则要求 `relative` 以 `pattern` 整体开头，且要么两者等长，要么紧随匹配部分之后是一个 `/` 分隔符。若 `pattern` 不含斜杠（仅匹配一个路径组件），则要求 `relative` 比 `pattern` 至少长一个字符、以 `pattern` 开头，且第 `pattern.size()` 个字符是 `/`，或者两者完全相等。

内部控制流围绕 `while` 循环移除尾部斜杠，随后根据 `pattern` 是否含有 `/` 分支处理。所有比较均通过 `std::string_view` 的 `starts_with` 和索引访问完成，未引入额外依赖。该函数是纯谓词，不涉及文件系统操作或外部状态，仅依赖标准字符串视图功能。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `relative` of type `std::string_view`
- parameter `pattern` of type `std::string_view`

#### Usage Patterns

- used to filter paths by a prefix pattern
- likely called during project model filtering or path matching

### `clore::extract::project_relative_path`

Declaration: `extract/filter.cppm:14`

Definition: `extract/filter.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::project_relative_path` 首先对输入的 `file` 路径调用 `std::filesystem::path::lexically_relative`，以 `root_path` 为基准进行词法相对化，结果存入 `rel`。若 `rel` 为空（表示两路径无公共前缀或相同），则立即返回 `std::nullopt`。随后遍历 `rel` 的每个组件，若任一组件等于 `..`，表明相对路径会跳出 `root_path` 范围，同样返回 `std::nullopt`。上述检查均通过后，直接返回 `rel`。

该实现仅依赖标准库的 `std::filesystem::path` 及相关操作，不调用任何自定义辅助函数。核心控制流为“词法计算 → 空值检验 → 向上遍历检验 → 返回结果”，保证了仅输出不包含父目录引用的严格相对路径。若无法计算或路径不合法，始终以 `std::nullopt` 表示失败。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `file` parameter
- the `root_path` parameter

#### Usage Patterns

- `project_relative_path(file_path, project_root)`
- determining whether a file is under a project root and its relative path

### `clore::extract::resolve_path_under_directory`

Declaration: `extract/filter.cppm:18`

Definition: `extract/filter.cppm:79`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先检查输入 `path` 是否为空字符串，若为空则立即以 `PathResolveError` 结构体返回错误，其中 `message` 字段包含描述信息。随后将 `path` 转换为 `std::filesystem::path` 对象 `p`，并依据其相对性判断：若 `p` 是相对路径（`p.is_relative()` 为真），则必须保证 `directory` 非空，否则返回另一条错误消息；若 `directory` 非空，则拼接为 `fs::path(directory) / p` 生成绝对路径。若 `p` 原本就是绝对路径则直接使用。最终对结果调用 `p.lexically_normal()` 得到标准化路径并返回。

整个流程仅依赖标准库中的 `std::filesystem` 路径处理函数（`is_relative`、`operator/`、`lexically_normal`），未涉及实际文件系统状态查询。错误处理均通过 `std::unexpected` 配合 `PathResolveError` 结构体传递，不抛出异常。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `path` parameter (string)
- `directory` parameter (string)

#### Usage Patterns

- resolve compilation database file paths
- normalize relative paths against a base directory

## Internal Structure

模块 `extract:filter` 将路径解析与过滤逻辑分解为两个层次。底层提供一组路径工具函数（`canonical_graph_path`、`resolve_path_under_directory`、`project_relative_path`、`path_prefix_matches`），负责路径的规范化、相对化及前缀匹配；上层实现过滤策略（`matches_filter`、`filter_root_path`），结合 `PathResolveError` 处理异常。它从 `config` 模块导入过滤所需的根路径和模式配置，依赖 `std` 模块提供基础类型与错误码支持。内部变量（如 `config`、`filter_root`、`pattern`）在各函数间共享，共同维护路径与过滤器的状态。

## Related Pages

- [Module config](../config/index.md)

