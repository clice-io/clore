---
title: 'Module config:validate'
description: 'config:validate 模块负责对 Clore 配置值执行校验，确保其符合预定义的约束（如非零、非空、有效范围等）。模块公开了一个 ValidationError 结构体用于表示验证错误的信息，以及一个 validate 函数作为验证的入口点。该函数接受一个 const int & 类型的配置值，返回一个 int 结果：返回 0 表示验证通过，非零值对应特定错误码或失败原因。内部通过两个匿名命名空间中的辅助函数 validate_nonzero 和 validate_nonempty 实现具体的校验逻辑，但它们对外部是不可见的。该模块与 config:schema 等模块协作，形成完整的配置处理流水线，保证最终使用的配置合法可靠。'
layout: doc
template: doc
---

# Module `config:validate`

## Summary

`config:validate` 模块负责对 Clore 配置值执行校验，确保其符合预定义的约束（如非零、非空、有效范围等）。模块公开了一个 `ValidationError` 结构体用于表示验证错误的信息，以及一个 `validate` 函数作为验证的入口点。该函数接受一个 `const int &` 类型的配置值，返回一个 `int` 结果：返回 `0` 表示验证通过，非零值对应特定错误码或失败原因。内部通过两个匿名命名空间中的辅助函数 `validate_nonzero` 和 `validate_nonempty` 实现具体的校验逻辑，但它们对外部是不可见的。该模块与 `config:schema` 等模块协作，形成完整的配置处理流水线，保证最终使用的配置合法可靠。

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::ValidationError`

Declaration: `config/validate.cppm:8`

Definition: `config/validate.cppm:8`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

结构体 `clore::config::ValidationError` 只包含一个公开数据成员 `std::string message`，没有用户定义的构造函数、析构函数或赋值运算符，因此作为平凡可复制类型适合按值传递。其内部不变量是 `message` 必须持有描述验证失败的文本内容，但不对字符串长度或格式施加任何额外约束；所有错误信息完全由该字符串承载，且结构体本身不执行任何验证、转换或剪裁逻辑。由于 `clore::config::ValidationError` 的状态完全由 `message` 决定，其实现极为简单，仅作为错误文本的轻量容器。

#### Invariants

- 无额外不变量，结构体仅作为错误消息的容器。

#### Key Members

- `std::string message`：存储验证错误消息。

#### Usage Patterns

- 被验证函数返回或赋值以报告错误。
- 作为错误集合的一部分被收集和检查。

## Functions

### `clore::config::validate`

Declaration: `config/validate.cppm:12`

Definition: `config/validate.cppm:42`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

该函数通过一系列顺序检查对传入的 `TaskConfig` 对象执行基于规则的验证。首先依次验证 `compile_commands_path`、`project_root`、`output_root` 三个路径字段：使用 `std::filesystem::exists` 检查路径是否存在、使用 `is_regular_file` 或 `is_directory` 确认文件类型符合预期，若失败则返回包含相应错误消息的 `std::unexpected`（消息由 `std::format` 构造）。随后对 LLM 配置子对象调用同匿名命名空间中的 `validate_nonempty` 检查 `llm.system_prompt` 非空，调用 `validate_nonzero` 确保 `llm.retry_limit` 为非零值。所有检查均返回 `std::expected<void, clore::config::ValidationError>`，一旦任何验证项失败立即短路返回，全部通过则返回 `{}`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- config`.compile_commands_path`
- config`.project_root`
- config`.output_root`
- config`.llm``.system_prompt`
- config`.llm``.retry_limit`

#### Usage Patterns

- validate a loaded `TaskConfig` before use
- called after `load_config` or `load_config_from_string`

## Internal Structure

模块 `config:validate` 主要负责对配置值执行约束检查，其公开入口为函数 `clore::config::validate`。为支持复用和清晰分离，模块内部在匿名命名空间中拆解了 `validate_nonzero` 和 `validate_nonempty` 两个辅助函数，它们分别针对不同的验证规则（如非零、非空）。验证错误通过公开结构体 `ValidationError`（包含字符串字段 `message`）进行传递。模块导入依赖 `config:schema` 以获取配置类型定义，并直接依赖 `std` 作为底层基础设施。整体实现结构遵循“公开接口 + 内部工具函数”的简单分层，验证逻辑集中在匿名命名空间内，避免了外部可见的辅助符号。

## Related Pages

- [Module config:schema](schema.md)

