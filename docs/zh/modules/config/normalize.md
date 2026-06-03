---
title: 'Module config:normalize'
description: '模块 config:normalize 负责对配置值进行规范化处理，当前公开的规范化函数面向整数类型的配置。它通过 clore::config::normalize 函数（接受 int&，返回 std::expected<void, NormalizeError>）对外提供服务，若规范化失败则返回含错误消息的 NormalizeError 结构体。此外，该模块公开了 normalize_separators 等变量，作为规范化过程中使用的配置参数。整体上，模块聚焦于将外部输入的配置值转换成内部一致、可用的形式，并为调用方提供明确的错误反馈。'
layout: doc
template: doc
---

# Module `config:normalize`

## Summary

模块 `config:normalize` 负责对配置值进行规范化处理，当前公开的规范化函数面向整数类型的配置。它通过 `clore::config::normalize` 函数（接受 `int&`，返回 `std::expected<void, NormalizeError>`）对外提供服务，若规范化失败则返回含错误消息的 `NormalizeError` 结构体。此外，该模块公开了 `normalize_separators` 等变量，作为规范化过程中使用的配置参数。整体上，模块聚焦于将外部输入的配置值转换成内部一致、可用的形式，并为调用方提供明确的错误反馈。

## Imports

- [`config:schema`](schema.md)

## Types

### `clore::config::NormalizeError`

Declaration: `src/config/normalize.cppm:17`

Definition: `src/config/normalize.cppm:17`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

结构体 `clore::config::NormalizeError` 内部仅包含一个 `std::string message` 成员，用作错误描述信息的载体。没有额外的数据成员或自定义构造函数、赋值运算符，因此整个类型是平凡可复制、可移动的。其实现不维护任何强约束性的不变式——`message` 可以持有任意字符串（包括空串），调用方可根据惯用法自行约定其内容格式（例如非空或可打印文本）。所有编译器生成的默认成员函数均被保留，使得该结构体可以直接通过聚合初始化或拷贝/移动语义快速构建和传递错误对象，无需额外的错误码或类型标记。

#### Invariants

- `message` 成员存储错误描述
- 无其他隐式约束或保证

#### Key Members

- `message` 成员

#### Usage Patterns

- 作为 `clore::config` 命名空间中规范化操作失败时的错误类型
- 可被 `catch` 或用于返回错误信息

## Functions

### `clore::config::normalize`

Declaration: `src/config/normalize.cppm:21`

Definition: `src/config/normalize.cppm:29`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

函数 `clore::config::normalize` 首先检查 `config.workspace_root` 是否为空，若为空则将其设置为 `fs::current_path().string()`。随后通过内部 lambda `make_absolute` 依次处理 `workspace_root`、`compile_commands_path`、`project_root` 和 `output_root`：`make_absolute` 对每个路径字符串进行非空校验（返回包含 `NormalizeError` 的 `std::unexpected`），将相对路径转换为绝对路径（若提供了可选的 `base` 参数则拼接到该基路径下，否则使用 `fs::absolute`），并应用 `lexically_normal` 进行词法归一化。完成绝对化后，使用第二个 lambda `normalize_separators` 遍历上述四个字段以及 `config.filter.include`、`config.filter.exclude` 中的所有路径，将反斜杠统一替换为正斜杠。整个流程依赖 `std::filesystem` 进行路径操作、`std::expected` 错误传递以及 `NormalizeError` 结构体承载错误消息。

#### Side Effects

- Modifies the provided `TaskConfig` object in place
- Calls `std::filesystem::current_path()` to set default workspace root

#### Reads From

- The `TaskConfig` object's fields: `workspace_root`, `compile_commands_path`, `project_root`, `output_root`, and `filter.include`, `filter.exclude`
- The current working directory via `std::filesystem::current_path()`

#### Writes To

- The same fields in the `TaskConfig` object: `workspace_root`, `compile_commands_path`, `project_root`, `output_root`, and `filter.include`, `filter.exclude`

#### Usage Patterns

- Used after loading a configuration to ensure paths are normalized before further processing
- Expected to be called on a `TaskConfig` instance that has been populated by functions like `load_config` or `load_config_from_string`

## Internal Structure

模块 `config:normalize` 依赖于 `config:schema` 提供的配置数据结构，将规范化操作封装为一个独立的公共函数 `normalize`，该函数接受一个 `int` 引用并返回 `expected<void, NormalizeError>`。内部实现通过变量如 `normalize_separators`、`make_absolute` 以及多个表示中间状态的 `r`、`config`、`path`、`base`、`workspace_root` 等，将路径处理与字段匹配分解为若干步骤，同时结构体 `NormalizeError` 通过 `message` 字段提供失败原因。这种分解使得整个规范化逻辑集中在一个入口中，调用者只需检查返回值即可获知操作是否成功。

## Related Pages

- [Module config:schema](schema.md)

