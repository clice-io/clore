---
title: 'Module config:normalize'
description: '模块 config:normalize 负责将 CLORE 的配置参数（尤其是数值或路径类配置项）转换为统一的规范化形式，确保后续处理的一致性。它公开了核心函数 clore::config::normalize，该函数接受一个 int 左值引用，对值进行规范化并返回状态码（0 表示成功，非零表示特定错误）；同时公开了 NormalizeError 结构体，用于携带规范过程中产生的错误信息。该模块依赖 config:schema 提供的配置类型骨架，并借助标准库实现路径分隔符归一化、绝对路径转换等内部逻辑。'
layout: doc
template: doc
---

# Module `config:normalize`

## Summary

模块 `config:normalize` 负责将 CLORE 的配置参数（尤其是数值或路径类配置项）转换为统一的规范化形式，确保后续处理的一致性。它公开了核心函数 `clore::config::normalize`，该函数接受一个 `int` 左值引用，对值进行规范化并返回状态码（0 表示成功，非零表示特定错误）；同时公开了 `NormalizeError` 结构体，用于携带规范过程中产生的错误信息。该模块依赖 `config:schema` 提供的配置类型骨架，并借助标准库实现路径分隔符归一化、绝对路径转换等内部逻辑。

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::NormalizeError`

Declaration: `config/normalize.cppm:10`

Definition: `config/normalize.cppm:10`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::NormalizeError` 是一个仅包含一个 `std::string` 类型成员 `message` 的平凡聚合结构体。其实现完全依赖于编译器生成的默认成员，未定义任何自定义构造函数、析构函数或赋值运算符，因此内部结构简单且无额外开销。该结构体不维护任何内部不变量；`message` 成员仅用于存储描述性错误文本，但其内容不受约束，完全由使用者决定。简而言之，实现的唯一职责就是直接持有这个字符串。

#### Invariants

- No documented invariants beyond the usual validity of `std::string`.

#### Key Members

- `message` – a `std::string` that stores the error description.

#### Usage Patterns

- Used as an error type in normalization-related operations.
- Likely returned or caught in code paths that validate or transform configuration data.

## Functions

### `clore::config::normalize`

Declaration: `config/normalize.cppm:14`

Definition: `config/normalize.cppm:22`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

该函数首先通过检查 `config.workspace_root` 是否为空来填充默认工作区根目录，若为空则将其设为 `std::filesystem::current_path` 的字符串表示。随后依次对 `workspace_root`、`compile_commands_path`、`project_root` 和 `output_root` 应用局部 lambda `make_absolute`，该 lambda 在路径为空时返回 `NormalizeError`，否则利用 `std::filesystem::path` 解析相对路径（可选的 `base` 参数由上一处理步骤中已确定的工作区根提供），最后通过 `lexically_normal` 进行词法规范化。每个调用通过返回的 `std::expected` 立即检查错误，若任一失败则提前返回未预期的错误。

在处理完绝对化和规范化后，第二个 lambda `normalize_separators` 将 `config` 中所有路径字段（包括 `compile_commands_path`、`project_root`、`output_root`、`workspace_root`）以及 `config.filter.include` 与 `config.filter.exclude` 向量中的每个字符串中的反斜杠 `\` 替换为正斜杠 `/`。此过程不涉及文件系统交互，仅进行字符串就地转换。函数最终返回一个表示成功的 `std::expected` 对象。

#### Side Effects

- Modifies the provided `TaskConfig` object by altering its string path fields.

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
- `config.filter.include` elements
- `config.filter.exclude` elements

#### Usage Patterns

- Called after `clore::config::load_config` to normalize configuration paths
- Used as part of configuration validation and preparation before further processing

## Internal Structure

模块 `config:normalize` 负责将配置值调整至内部标准形式，其实现依赖于 `config:schema` 提供的类型骨架以及标准库的基础设施。公开的接口包括 `NormalizeError` 错误类型和 `normalize` 函数，后者通过一个 `int&` 参数接收待规范化的值并返回状态码。内部层次上，模块将规范化逻辑分解为一组局部变量或函数对象（如 `normalize_separators`、`path`、`workspace_root` 和 `make_absolute`），这些元素封装了具体的规范化规则（分隔符处理、路径绝对化等），并通过多次出现的变量 `r` 暂存中间结果，从而在单一函数中实现多步骤的修正流程。这种分解使规范化规则集中管理，同时保持公开入口的简洁性。

## Related Pages

- [Module config:schema](schema.md)

