---
title: 'Module config:normalize'
description: '模块 config:normalize 负责对 Clore 工具链中的配置值进行规范化处理，将其转换为标准形式以便后续验证和使用。其公开实现的核心是函数 clore::config::normalize，它接受一个 int 引用并就地更新该值，返回值通常指示操作状态；同时提供了 NormalizeError 结构体用于报告规范化过程中的错误。此外，该模块还暴露了多个公共变量，例如 workspace_root、make_absolute、normalize_separators 等，它们作为规范化过程的参数或上下文状态，共同构成模块的公开接口范围。'
layout: doc
template: doc
---

# Module `config:normalize`

## Summary

模块 `config:normalize` 负责对 Clore 工具链中的配置值进行规范化处理，将其转换为标准形式以便后续验证和使用。其公开实现的核心是函数 `clore::config::normalize`，它接受一个 `int` 引用并就地更新该值，返回值通常指示操作状态；同时提供了 `NormalizeError` 结构体用于报告规范化过程中的错误。此外，该模块还暴露了多个公共变量，例如 `workspace_root`、`make_absolute`、`normalize_separators` 等，它们作为规范化过程的参数或上下文状态，共同构成模块的公开接口范围。

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::NormalizeError`

Declaration: `config/normalize.cppm:10`

Definition: `config/normalize.cppm:10`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

结构体 `clore::config::NormalizeError` 仅包含一个公开的 `std::string` 成员 `message`，其内部状态唯一地由该字符串表示。该结构体不定义构造函数、析构函数或任何成员函数，因此其不变性完全依赖于调用方正确设置 `message` 的内容——通常应是一个描述规范化失败原因的非空字符串。由于没有额外的数据成员或资源管理需求，该类型可作为轻量级错误载体，在 `clore::config` 命名空间内传递规范化操作失败时的上下文信息。

#### Invariants

- The `message` field should typically be non-empty when an error is present.

#### Key Members

- `message`: a `std::string` describing the normalization error.

#### Usage Patterns

- Returned or thrown by normalization functions to indicate failure.
- Checked by callers to determine the reason for normalization failure.

## Functions

### `clore::config::normalize`

Declaration: `config/normalize.cppm:14`

Definition: `config/normalize.cppm:22`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

函数 `clore::config::normalize` 首先检查并设置 `config.workspace_root`：若为空，则将其赋值为 `std::filesystem::current_path().string()`。随后依次对 `workspace_root`、`compile_commands_path`、`project_root` 和 `output_root` 字段调用内部 lambda `make_absolute`。该 lambda 将空字符串视为错误并返回 `NormalizeError`；否则将路径转换为 `std::filesystem::path`，如果路径为相对形式，则基于可选的 `base` 参数（第一个调用使用 `std::nullopt`，后续调用使用已归一化的 `workspace_root` 作为基准）与 `p` 拼接或直接调用 `fs::absolute(p)`，最后通过 `lexically_normal().string()` 获取规范形式并写回原字段。若任一字段规范化失败，函数提前返回 `std::unexpected`。

在所有绝对路径处理完成后，第二个内部 lambda `normalize_separators` 将 `config` 中所有路径字段（包括 `compile_commands_path`、`project_root`、`output_root`、`workspace_root` 以及 `config.filter.include` 与 `config.filter.exclude` 列表中的每个元素）中的反斜杠 `\` 替换为正斜杠 `/`。函数最终返回成功 `std::expected<void, NormalizeError>`。整个过程依赖 `std::filesystem` 的路径操作和 `std::format` 构建错误消息。

#### Side Effects

- Mutates the `TaskConfig` struct passed by reference by modifying its path fields.
- Calls `std::filesystem::current_path()` which may involve a system call to retrieve the current working directory.

#### Reads From

- `config.workspace_root`
- `config.compile_commands_path`
- `config.project_root`
- `config.output_root`
- `config.filter.include`
- `config.filter.exclude`
- `std::filesystem::current_path()`

#### Writes To

- `config.workspace_root`
- `config.compile_commands_path`
- `config.project_root`
- `config.output_root`
- `config.filter.include`
- `config.filter.exclude`

#### Usage Patterns

- Called before using the configuration to ensure all paths are absolute and normalized.
- Used to prepare a `TaskConfig` for further processing or validation.

## Internal Structure

`config:normalize` 是 Clore 配置管线中的规范化模块，仅导入 `config:schema` 和 `std`。它对外暴露 `NormalizeError` 结构和 `normalize` 函数（接受 `int&` 并返回 `int`，用于表示状态或结果）。内部实现按职责拆分为多个可变辅助：`workspace_root` 提供工作区根路径，`make_absolute` 处理相对路径到绝对路径的转换，`normalize_separators` 负责分隔符标准化。这些辅助变量协同完成配置值的标准化；若过程中出现错误，则通过 `NormalizeError::message` 报告失败原因。整体采用原地修改引用参数的方式，使规范化行为可组合，并保持与 `config:validate` 等后续模块的解耦。

## Related Pages

- [Module config:schema](schema.md)

