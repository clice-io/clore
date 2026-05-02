---
title: 'Module config:load'
description: '该模块提供了配置文件加载的核心功能，支持从文件路径或字符串读取 TOML 格式的配置数据。它解析输入内容，验证顶层键的合法性（通过 reject_unknown_top_level_keys 检查未知字段），并将解析结果转换为 config:schema 定义的配置结构（如 FilterRule、LLMConfig 等），最终通过 to_config 完成结构填充。公开接口包括 load_config 和 load_config_from_string，均返回 int 表示成功（0）或错误，错误信息通过 ConfigError 结构中的 message 字段传递。内部使用 RawTaskConfig 作为中间表示，逐步构建最终配置对象，并依赖 support 模块提供的文本处理工具进行健壮的输入处理。'
layout: doc
template: doc
---

# Module `config:load`

## Summary

该模块提供了配置文件加载的核心功能，支持从文件路径或字符串读取 TOML 格式的配置数据。它解析输入内容，验证顶层键的合法性（通过 `reject_unknown_top_level_keys` 检查未知字段），并将解析结果转换为 `config:schema` 定义的配置结构（如 `FilterRule`、`LLMConfig` 等），最终通过 `to_config` 完成结构填充。公开接口包括 `load_config` 和 `load_config_from_string`，均返回 `int` 表示成功（0）或错误，错误信息通过 `ConfigError` 结构中的 `message` 字段传递。内部使用 `RawTaskConfig` 作为中间表示，逐步构建最终配置对象，并依赖 `support` 模块提供的文本处理工具进行健壮的输入处理。

## Imports

- [`config:schema`](schema.md)
- `std`
- [`support`](../support/index.md)

## Types

### `clore::config::ConfigError`

Declaration: `config/load.cppm:15`

Definition: `config/load.cppm:15`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

结构体 `clore::config::ConfigError` 仅包含一个 `std::string` 类型的公开数据成员 `message`，用于存储错误描述。内部结构极为简单，没有自定义构造函数、析构函数或赋值运算符，完全依赖编译器生成的默认实现来管理资源。不变量隐含在 `message` 应包含有意义的错误信息这一语义中，但类型本身未施加任何强制约束（如非空性检查）。由于不提供任何格式化或解析逻辑，该结构体实质上是对 `std::string` 的一层薄包装，方便在配置解析上下文中传递错误文本。

#### Invariants

- The `message` member is always a valid `std::string` (default-constructible).

#### Key Members

- `std::string message`

#### Usage Patterns

- Returned or thrown to indicate a configuration loading failure.
- Inspected by callers to retrieve the error description.

## Functions

### `clore::config::load_config`

Declaration: `config/load.cppm:19`

Definition: `config/load.cppm:81`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

函数 `clore::config::load_config` 首先将传入的 `path` 参数标准化：若为相对路径则转换为绝对路径，再调用 `lexically_normal` 进行规范化。接着检查文件是否存在，若不存在则立即返回一个携带 `ConfigError` 的 `std::unexpected`，错误消息包含原路径。文件存在后，依赖 `clore::support::read_utf8_text_file` 读取整个文件内容为 UTF-8 字符串 `content`；若读取失败，同样返回 `ConfigError`。读取成功后，将内容转发给 `load_config_from_string` 进行 TOML 解析与转换，得到初步的 `TaskConfig` 对象。最后，将解析出的配置对象的 `workspace_root` 字段设置为配置文件的父目录路径（即 `config_path.parent_path().string()`），然后返回该配置对象。整个过程借助 `std::expected` 传递成功值或错误，错误类型为 `ConfigError`，其唯一成员 `message` 记录描述信息。

#### Side Effects

- reads configuration file from filesystem
- modifies the `workspace_root` field of the returned `TaskConfig`

#### Reads From

- filesystem: file at the provided `path`
- `clore::support::read_utf8_text_file` (reads file content)

#### Writes To

- the `workspace_root` member of the returned `TaskConfig`

#### Usage Patterns

- loading configuration for application startup
- parsing a user-specified configuration file

### `clore::config::load_config_from_string`

Declaration: `config/load.cppm:21`

Definition: `config/load.cppm:110`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

该函数首先通过 `clore::support::strip_utf8_bom` 移除输入字符串中可能存在的 UTF-8 BOM 标记，得到 `normalized_toml`。随后它调用 `::toml::parse` 将规范化后的内容解析为 `::toml::table`，若抛出 `::toml::parse_error` 异常则立即返回一个包含错误描述的 `ConfigError`。解析成功后，它通过 `reject_unknown_top_level_keys` 验证表中仅包含允许的顶层键，检查失败时同样返回错误。接着，它借助 `kota::codec::toml::from_toml` 将 `table` 反序列化为内部结构体 `RawTaskConfig`，该步骤若失败则封装 `ConfigError` 返回。最后，调用 `to_config` 将 `RawTaskConfig` 转换为最终的 `TaskConfig` 并返回。整个流程依赖 TOML 解析库、自定义 BOM 处理函数以及模块内部的配置验证与转换逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `toml_content`
- 通过 `::toml::parse` 读取已解析的字符串内容
- 通过 `toml_codec::from_toml` 读取 `table`

#### Writes To

- 局部变量 `normalized_toml`
- 局部变量 `table`
- 局部变量 `raw`
- 返回值 `std::expected<TaskConfig, ConfigError>`

#### Usage Patterns

- 从外部源获取TOML字符串后调用
- 可能由 `clore::config::load_config` 调用以适应文件读取逻辑
- 在需要从内存中解析配置时作为入口点

## Internal Structure

模块 `config:load` 负责从文件或字符串加载并解析 CLORE 配置。它导入 `config:schema` 以复用配置数据结构定义，导入 `support` 获取文本处理、路径规范化等工具，以及标准库。内部采用匿名命名空间封装实现细节：`RawTaskConfig` 作为中间数据结构，存储 `llm` 和 `filter` 字段；辅助函数 `reject_unknown_top_level_keys` 基于预定义的 `allowed_keys` 验证 TOML 表，确保配置健壮性；`to_config` 将 `RawTaskConfig` 转换为最终配置对象。公开函数 `load_config` 和 `load_config_from_string` 分别从文件路径和内存字符串出发，依次完成 TOML 解析、键验证与配置构建，形成清晰的单次解析流程。

## Related Pages

- [Module config:schema](schema.md)
- [Module support](../support/index.md)

