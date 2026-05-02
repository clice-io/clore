---
title: 'Module config:validate'
description: 'config:validate 模块负责对 CLORE 工具的配置进行校验，确保其符合预定义的规则。其公开实现面向调用方提供统一的校验入口，通过 clore::config::validate 函数接受一个配置表示（以 const int & 传入），并在校验通过时返回 0，非零值则标志具体的失败原因。该模块还公开了 ValidationError 结构体，用于携带校验失败的错误信息（其 message 字段存储描述）。内部实现包含针对数值非零和容器非空的辅助校验逻辑，这些逻辑在库内部使用，不暴露给外部。整个模块依赖于 config:schema 模块提供的配置数据结构定义，可配合配置加载或归一化流程使用，以确保配置状态的安全性。'
layout: doc
template: doc
---

# Module `config:validate`

## Summary

`config:validate` 模块负责对 CLORE 工具的配置进行校验，确保其符合预定义的规则。其公开实现面向调用方提供统一的校验入口，通过 `clore::config::validate` 函数接受一个配置表示（以 `const int &` 传入），并在校验通过时返回 `0`，非零值则标志具体的失败原因。该模块还公开了 `ValidationError` 结构体，用于携带校验失败的错误信息（其 `message` 字段存储描述）。内部实现包含针对数值非零和容器非空的辅助校验逻辑，这些逻辑在库内部使用，不暴露给外部。整个模块依赖于 `config:schema` 模块提供的配置数据结构定义，可配合配置加载或归一化流程使用，以确保配置状态的安全性。

## Imports

- [`config:schema`](schema.md)
- `std`

## Types

### `clore::config::ValidationError`

Declaration: `config/validate.cppm:8`

Definition: `config/validate.cppm:8`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::ValidationError` 的内部结构极其精简，仅包含一个 `std::string` 类型的 `message` 数据成员。该成员是验证错误的唯一载体，其生命周期伴随整个结构体实例；通过默认生成的构造、析构、拷贝与移动操作，`message` 的字符串存储与管理完全委托给标准库。结构体不维护其他不变性：只要 `message` 持有有效的 `std::string` 对象（即使在默认构造的空字符串状态下），该结构体便处于合法状态。所有对错误信息的修改直接作用于 `message`，不涉及额外约束或状态同步。

#### Invariants

- `message` 存储验证失败时的描述文本
- 不包含任何运行时或编译期约束

#### Key Members

- `std::string message`

#### Usage Patterns

- 在验证函数中作为返回值类型使用，报告具体的验证失败原因
- 创建 `ValidationError` 实例并设置 `message` 后传递给调用方

## Functions

### `clore::config::validate`

Declaration: `config/validate.cppm:12`

Definition: `config/validate.cppm:42`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

函数 `clore::config::validate` 按照多阶段顺序检查 `TaskConfig` 各个字段的有效性。它首先利用 `std::filesystem` 验证 `compile_commands_path` 和 `project_root` 必须非空、必须存在且分别为常规文件和目录；`output_root` 必须非空，若存在则必须是目录。每项失败会立即返回一个包装在 `std::unexpected` 中的 `ValidationError`，其 `message` 字段包含 `std::format` 生成的描述。在路径校验之后，它依靠匿名命名空间中的辅助函数 `validate_nonempty` 和 `validate_nonzero` 对 LLM 子配置的 `system_prompt` 和 `retry_limit` 进行非空与非零检查。所有检查通过后返回空的 `expected<void>`。整个验证流程中，每次判断均会短路返回首个错误，使得错误报告清晰且顺序固定。

#### Side Effects

- reads filesystem metadata (exists, `is_regular_file`, `is_directory`) for `compile_commands_path`, `project_root`, `output_root`

#### Reads From

- `config.compile_commands_path`
- `config.project_root`
- `config.output_root`
- `config.llm.system_prompt`
- `config.llm.retry_limit`
- filesystem metadata for the above paths

#### Usage Patterns

- called after `load_config` to validate the configuration before use
- used in configuration parsing pipeline to ensure correctness

## Internal Structure

模块 `config:validate` 通过导入 `config:schema` 获得配置数据结构骨架，并依赖 `std` 提供基础设施。其公开入口为 `clore::config::validate`，接受配置的胶合表示（`const int &`）并返回整数以指示验证是否成功（0 为成功，非零为具体失败码）。实现内部利用匿名命名空间封装了 `validate_nonzero` 和 `validate_nonempty` 等私有辅助函数，分别负责检查字段值是否非零或容器是否非空，从而将验证规则分解为可独立测试的小步骤。该模块还定义了 `ValidationError` 结构体（内含 `message` 字符串），用于在验证失败时携带错误描述，但当前公开接口仅通过整数返回码报告结果，错误消息的传递方式可能仅限于内部或通过外部机制（如日志）暴露。整体上，实现层在匿名命名空间中隐藏细节，保持对外接口简洁，与 `config:schema` 的紧耦合确保了验证逻辑能与配置结构同步演进。

## Related Pages

- [Module config:schema](schema.md)

