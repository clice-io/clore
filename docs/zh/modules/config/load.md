---
title: 'Module config:load'
description: 'config:load 模块负责将 Clore 工具的配置从外部源加载到内存中，提供两个公开接口：load_config 从文件路径加载，load_config_from_string 从字符串直接加载。两者均返回一个整数结果码，指示加载成功或失败，后续可配合 config:validate 确认配置有效性。在内部，模块解析 TOML 格式内容，拒绝未知顶层键，并将原始数据转换为 RawTaskConfig 结构，依赖 config:schema 定义的类型以及 support 模块的文件读写与文本处理能力，确保配置加载过程的正确性与可移植性。'
layout: doc
template: doc
---

# Module `config:load`

## Summary

`config:load` 模块负责将 Clore 工具的配置从外部源加载到内存中，提供两个公开接口：`load_config` 从文件路径加载，`load_config_from_string` 从字符串直接加载。两者均返回一个整数结果码，指示加载成功或失败，后续可配合 `config:validate` 确认配置有效性。在内部，模块解析 TOML 格式内容，拒绝未知顶层键，并将原始数据转换为 `RawTaskConfig` 结构，依赖 `config:schema` 定义的类型以及 `support` 模块的文件读写与文本处理能力，确保配置加载过程的正确性与可移植性。

## Imports

- [`config:schema`](schema.md)
- `std`
- [`support`](../support/index.md)

## Types

### `clore::config::ConfigError`

Declaration: `config/load.cppm:15`

Definition: `config/load.cppm:15`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

结构 `clore::config::ConfigError` 内部仅包含一个 `std::string message` 成员，作为错误信息的载体。没有自定义构造函数、析构函数或赋值运算符，因此编译器会隐式生成这些特殊成员，使得该类型是一个可平凡复制、可移动的聚合类型。唯一的隐式不变量是 `message` 成员始终处于 `std::string` 所保证的有效状态（可为空字符串），不添加额外的约束或状态校验。所有对于该错误信息的访问和修改均直接通过公有成员 `message` 进行，无需依赖任何内部辅助函数或复杂的数据结构。

#### Invariants

- `message` 应当包含有意义的错误描述
- 结构体无其他状态约束

#### Key Members

- `message` 成员

#### Usage Patterns

- 在配置解析失败时构造并返回该错误
- 用户通过读取 `message` 获取错误信息

## Functions

### `clore::config::load_config`

Declaration: `config/load.cppm:19`

Definition: `config/load.cppm:81`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

该函数首先将传入的 `path` 转换为绝对路径并规范化，然后检查文件是否存在，若不存在则立即返回带有 `ConfigError` 的意外结果。文件存在时，依赖 `clore::support::read_utf8_text_file` 读取其完整内容为 `content`；读取失败同样返回错误。随后将 `content` 委托给 `load_config_from_string` 进行解析，后者内部通过 TOML 解析器加载 `toml_content` 表，校验顶层键的合法性（依赖 `reject_unknown_top_level_keys`），并构造 `RawTaskConfig` 结构，再经由 `to_config` 转换为最终配置对象。解析成功后，函数用 `config_path.parent_path` 字符串填充结果中的 `workspace_root` 字段，从而完成配置加载。

#### Side Effects

- reads a configuration file from the filesystem
- checks existence of the file via `std::filesystem::exists`
- reads the content of the file using `clore::support::read_utf8_text_file`
- sets the `workspace_root` field of the returned `TaskConfig` object

#### Reads From

- the `path` parameter
- the filesystem via `std::filesystem::exists`
- the file content via `clore::support::read_utf8_text_file`
- the result of `load_config_from_string`

#### Writes To

- the `workspace_root` field of the returned `TaskConfig` object
- the error state returned as `std::unexpected` (if any)

#### Usage Patterns

- loading application configuration from a configuration file at startup
- part of a configuration subsystem to parse config files with relative path resolution

### `clore::config::load_config_from_string`

Declaration: `config/load.cppm:21`

Definition: `config/load.cppm:110`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

该函数首先对输入的 `toml_content` 执行 BOM 剥离（调用 `clore::support::strip_utf8_bom`），再将结果传入 `::toml::parse` 进行解析。解析成功后会调用 `reject_unknown_top_level_keys` 校验表内是否存在未被 `RawTaskConfig` 字段覆盖的顶层键；若发现未知键则立即返回 `ConfigError`。随后通过 `toml_codec::from_toml` 将 `table` 解码为 `RawTaskConfig` 的实例 `raw`，该步骤若失败同样以 `ConfigError` 形式返回。最终调用 `to_config` 将 `raw` 转换为 `TaskConfig` 并作为 `std::expected` 的成功值返回。

整个函数完全依赖 `std::expected` 进行错误传播，未使用异常。错误处理覆盖三个节点：TOML 语法错误（`::toml::parse_error`）、未知顶层键（`reject_unknown_top_level_keys` 返回的 `ConfigError`）以及 `toml_codec::from_toml` 的字段级解码失败，每种情况均生成包含描述性消息的 `ConfigError`。核心依赖包括 `kota::codec::toml`（TOML 序列化/反序列化）、`::toml`（解析库）以及 `clore::support::strip_utf8_bom`（BOM 清理）。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `toml_content` (`std::string_view`)
- global function `clore::support::strip_utf8_bom`
- global function `reject_unknown_top_level_keys`
- global function `toml_codec::from_toml`
- global function `to_config`
- TOML library internals

#### Writes To

- local variable `normalized_toml`
- local variable `table`
- local variable `raw`
- local variable `result`
- return value (`std::expected<TaskConfig, ConfigError>`)

#### Usage Patterns

- called with a string containing TOML content
- used in config loaders that receive configuration as text
- part of the `clore::config` API for parsing configuration strings

## Internal Structure

模块 `config:load` 实现了从文件路径或内存字符串加载 TOML 格式配置的功能。它对外暴露 `load_config` 和 `load_config_from_string` 两个入口函数，它们均接受 `std::string_view` 并返回 `int` 状态码，后续可配合 `config:validate` 进行校验。模块通过 `import config:schema` 引入 `TaskConfig` 等核心类型，并利用 `import support` 中的文件读取与文本处理工具来获取原始 TOML 内容。

内部实现采用匿名命名空间进行分层：先通过 `RawTaskConfig` 结构体（包含 `llm` 与 `filter` 字段）暂存原始解析结果，再经 `to_config` 转换为成熟配置。解析过程中使用 `reject_unknown_top_level_keys` 对顶层键进行白名单校验，保证配置的严谨性。这种设计将底层 TOML 表的遍历、键值验证与上层配置对象的构建相分离，使得模块结构清晰、职责单一。

## Related Pages

- [Module config:schema](schema.md)
- [Module support](../support/index.md)

