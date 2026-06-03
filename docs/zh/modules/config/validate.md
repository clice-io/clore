---
title: 'Module config:validate'
description: '模块 config:validate 负责对已加载并归一化的配置值执行约束检查，确保配置满足有效运行所需的条件。其公开接口包括函数 clore::config::validate，它接受 const int & 类型的配置值（通常由 load_config、load_config_from_string 或 normalize 等函数获得），并返回一个 std::expected<void, ValidationError>：若验证通过则返回空 expected，否则返回描述错误的 ValidationError 对象。ValidationError 结构体仅包含一个 message 字段，用于以类型安全的方式向调用方传递具体的验证失败原因。在内部实现中，模块使用匿名命名空间中的辅助函数（如 validate_nonzero 和 validate_nonempty）来检查数值非零、字符串非空等常见约束。该模块依赖 config:schema 模块提供的配置数据结构定义，是整个配置系统中负责数据合规性检查的核心组件。'
layout: doc
template: doc
---

# Module `config:validate`

## Summary

模块 `config:validate` 负责对已加载并归一化的配置值执行约束检查，确保配置满足有效运行所需的条件。其公开接口包括函数 `clore::config::validate`，它接受 `const int &` 类型的配置值（通常由 `load_config`、`load_config_from_string` 或 `normalize` 等函数获得），并返回一个 `std::expected<void, ValidationError>`：若验证通过则返回空 expected，否则返回描述错误的 `ValidationError` 对象。`ValidationError` 结构体仅包含一个 `message` 字段，用于以类型安全的方式向调用方传递具体的验证失败原因。在内部实现中，模块使用匿名命名空间中的辅助函数（如 `validate_nonzero` 和 `validate_nonempty`）来检查数值非零、字符串非空等常见约束。该模块依赖 `config:schema` 模块提供的配置数据结构定义，是整个配置系统中负责数据合规性检查的核心组件。

## Imports

- [`config:schema`](schema.md)

## Types

### `clore::config::ValidationError`

Declaration: `src/config/validate.cppm:14`

Definition: `src/config/validate.cppm:14`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::ValidationError` 是一个聚合类型，其唯一的数据成员是 `std::string message`。该结构体作为验证错误信息的载体，内部状态完全由 `message` 持有，除此之外不包含任何自定义构造函数、赋值操作或其它成员函数。所有特殊成员函数（默认构造、拷贝/移动构造、拷贝/移动赋值、析构）均由编译器隐式生成，因此该类型满足平凡可复制性（trivially copyable）的判定取决于 `std::string` 的平凡性——实际上它并非平凡类型，但隐式生成的特殊成员函数使得对象可以按常规方式传递和存储。实现中未定义任何不变式约束，`message` 的内容可以任意赋值，但典型用法要求其非空且描述了具体的验证失败原因。结构体的设计意图是保持极简，仅作为轻量级错误传递单元，避免引入额外逻辑或资源管理职责。

#### Key Members

- message

#### Usage Patterns

- Created by validation logic and returned or stored in a collection to report config problems.
- Consumed by error-handling code that reads the `message` field to present to the user or log.

## Functions

### `clore::config::validate`

Declaration: `src/config/validate.cppm:18`

Definition: `src/config/validate.cppm:48`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

函数 `clore::config::validate` 的实现采用顺序验证策略，对输入的 `TaskConfig` 对象逐字段执行约束检查。验证依次处理 `compile_commands_path`、`project_root` 和 `output_root`，利用 `std::filesystem` 库确保路径非空、存在且类型正确（文件或目录）；其中 `output_root` 允许不存在，但若存在则必须是目录。随后转入 LLM 子配置验证，分别委托匿名命名空间内的辅助函数 `validate_nonempty` 和 `validate_nonzero` 校验 `llm.system_prompt` 非空及 `llm.retry_limit` 非零。任何检查失败均立即返回 `std::unexpected` 携带 `ValidationError` 实例，成功则返回空 `std::expected<void, ValidationError>`。该实现无复杂分支或递归，依赖仅限于标准库文件系统与两个本地辅助函数。

#### Side Effects

No observable side effects are evident from the extracted code.

## Internal Structure

`config:validate` 模块提供核心的配置验证功能，对外仅暴露 `clore::config::validate` 函数。该模块通过导入 `config:schema` 获取配置数据结构定义，并利用 `std::expected` 实现无异常的错误返回。验证结果类型 `ValidationError` 包含一个描述性字符串字段 `message`，用于向调用者传达失败原因。

在实现上，模块使用匿名命名空间封装了两个内部验证辅助函数：`validate_nonzero`（检查 `uint32_t` 值非零）和 `validate_nonempty`（检查字符串非空）。这些辅助函数接受待验证的值和字段名称，返回与公开函数相同的错误类型。公开的 `validate` 函数接收一个 `const int&` 配置值，通过组合或调用这些内部验证逻辑来判定配置是否合规，从而保持模块内部层次清晰，验证逻辑可复用。

## Related Pages

- [Module config:schema](schema.md)

