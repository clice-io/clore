---
title: 'Module config:load'
description: '模块 config:load 负责从字符串或文件路径读取配置数据，并将其解析为原始的内部表示结构。它公开了两个核心加载函数：load_config 接收文件路径，load_config_from_string 接收配置文本，两者均返回一个整数句柄，供调用者在后续处理管线中引用。模块内部维护了中间表示类型 RawTaskConfig（包含 LLM 和筛选规则等字段），并提供了对顶层键的未知键拒绝验证，但不执行最终的规范化和完整验证，该职责由依赖的 config:schema 模块承担。'
layout: doc
template: doc
---

# Module `config:load`

## Summary

模块 `config:load` 负责从字符串或文件路径读取配置数据，并将其解析为原始的内部表示结构。它公开了两个核心加载函数：`load_config` 接收文件路径，`load_config_from_string` 接收配置文本，两者均返回一个整数句柄，供调用者在后续处理管线中引用。模块内部维护了中间表示类型 `RawTaskConfig`（包含 LLM 和筛选规则等字段），并提供了对顶层键的未知键拒绝验证，但不执行最终的规范化和完整验证，该职责由依赖的 `config:schema` 模块承担。

## Imports

- [`config:schema`](schema.md)
- [`support`](../support/index.md)

## Types

### `clore::config::ConfigError`

Declaration: `src/config/load.cppm:30`

Definition: `src/config/load.cppm:30`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

`clore::config::ConfigError` 是一个轻量级结构，仅在内部用于传递配置解析或验证过程中的错误信息。它包含一个 `std::string message` 字段，没有其他成员或自定义构造函数、赋值操作。该字段直接存储描述错误的文本，其不变性由调用方保证，通常 `message` 应非空以提供有意义的错误上下文。`ConfigError` 本身不提供任何错误代码或分类，完全依赖字符串内容实现灵活的错误报告。

#### Invariants

- The `message` member may be empty or contain any string describing the error condition.
- No further invariants or guarantees are implied by the evidence.

#### Key Members

- `clore::config::ConfigError::message` – the text describing the error.

#### Usage Patterns

- Likely used as an exception type or error result when configuration parsing fails.
- Other code may construct `ConfigError` with a descriptive string and then throw or return it.

## Functions

### `clore::config::load_config`

Declaration: `src/config/load.cppm:34`

Definition: `src/config/load.cppm:96`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

函数 `clore::config::load_config` 首先对传入的路径 `path` 进行规范化处理：通过 `std::filesystem::path` 将相对路径转为绝对路径，并执行 `lexically_normal`。之后检查文件是否存在，若不存在则直接返回 `ConfigError`。若存在，调用 `clore::support::read_utf8_text_file` 以 `config_path` 为参数读取文件内容，读取失败同样返回错误。成功读取后，将字符串内容传递给 `load_config_from_string` 进行 TOML 解析并返回 `std::expected`。解析成功后，将 `config->workspace_root` 设置为 `config_path.parent_path().string()`，即配置文件所在的目录，作为工作空间的根目录。

该函数的核心流程围绕文件系统操作与字符串解析，依赖 `std::filesystem` 处理路径与文件存在性检验，依赖 `clore::support::read_utf8_text_file` 读取 UTF-8 文本，以及内部函数 `load_config_from_string` 完成 TOML 到 `TaskConfig` 的转换。错误路径统一通过 `ConfigError` 报告，其中包含描述性消息。

#### Side Effects

- Reads configuration file from the filesystem via `clore::support::read_utf8_text_file`
- Sets the `workspace_root` member on the returned `TaskConfig` object

#### Reads From

- The `path` parameter
- The file at the resolved absolute normalized path on the filesystem
- The file content read as a UTF-8 string via `clore::support::read_utf8_text_file`

#### Writes To

- The `workspace_root` member of the returned `TaskConfig` object

#### Usage Patterns

- Call to load a configuration from a file, obtaining a `TaskConfig` on success
- Used to parse configuration files and automatically set the workspace root to the file's directory

### `clore::config::load_config_from_string`

Declaration: `src/config/load.cppm:36`

Definition: `src/config/load.cppm:125`

Declaration: [`Namespace clore::config`](../../namespaces/clore/config/index.md)

实现首先通过 `clore::support::strip_utf8_bom` 去除输入字符串的 UTF-8 BOM，生成 `normalized_toml`，然后将其转换为 `std::string` 并调用 `::toml::parse` 解析为 `::toml::table`。若 `parse` 抛出异常，则直接构造 `ConfigError` 对象并返回 `std::unexpected`。解析成功后将 `table` 传入 `reject_unknown_top_level_keys`，通过预先定义的白名单（`allowed_keys`）检查是否存在未知顶层键；若存在，同样返回错误。

通过验证后，函数声明一个 `RawTaskConfig` 实例 `raw`，并调用 `toml_codec::from_toml(normalized_toml, raw)` 将原始 TOML 字符串解码为结构化的 `raw` 对象。此步骤对解析错误进行统一处理，若有误则返回带有 `result.error().message` 的 `ConfigError`。最后将 `raw` 通过 `to_config` 转换为最终的 `std::expected<TaskConfig, ConfigError>` 返回。整个过程依赖 `kota::codec::toml` 命名空间提供的编解码能力，以及内部辅助函数 `reject_unknown_top_level_keys` 和 `to_config`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The parameter `toml_content` (the TOML string)
- Internal data structures during parsing

#### Writes To

- The return value of type `std::expected<TaskConfig, ConfigError>`

#### Usage Patterns

- Used to load configuration from a TOML string
- Typically used in configuration deserialization pipelines

## Internal Structure

模块 `config:load` 负责将 TOML 格式的配置源（字符串或文件路径）解析并转换为运行时配置句柄。它通过匿名命名空间封装内部实现细节：`RawTaskConfig` 结构体承载过滤规则与 LLM 配置的原始字段，辅助函数 `reject_unknown_top_level_keys` 在加载阶段执行顶层键的白名单校验，而 `to_config` 将原始结构转换为模块 `config:schema` 定义的 `TaskConfig`。该模块依赖 `support` 提供的日志与通用工具，并向外暴露两个入口：`load_config`（从路径读取）和 `load_config_from_string`（直接解析字符串），两者均返回一个整型句柄，供后续验证与规范化管线使用。

内部层次清晰：先通过 TOML 解析器获得表结构，再校验未知键，然后映射到 `RawTaskConfig`，最后通过 `to_config` 生产最终配置。这种分解将格式解析、语义验证与数据结构转换分离，保持了模块内聚性。

## Related Pages

- [Module config:schema](schema.md)
- [Module support](../support/index.md)

