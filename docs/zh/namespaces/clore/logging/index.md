---
title: 'Namespace clore::logging'
description: 'clore::logging 命名空间是 Clore 库的日志基础设施，提供类型安全、编译时级别可控的日志代理及预定义级别常量（trace、debug、info、warn、err）。这些代理通过 LogProxy 结构体实现，支持格式化字符串和原始字符串视图两种重载，最终调用底层 log 函数将消息与严重性等级传递给 spdlog 后端。命名空间还包含全局日志级别阈值 g_log_level、直接写入标准错误的 stderr_logger 以及记录缓存命中率的 cache_hit_rate 辅助函数，共同构成一套灵活、可扩展的日志系统。'
layout: doc
template: doc
---

# Namespace `clore::logging`

## Summary

`clore::logging` 命名空间是 Clore 库的日志基础设施，提供类型安全、编译时级别可控的日志代理及预定义级别常量（`trace`、`debug`、`info`、`warn`、`err`）。这些代理通过 `LogProxy` 结构体实现，支持格式化字符串和原始字符串视图两种重载，最终调用底层 `log` 函数将消息与严重性等级传递给 spdlog 后端。命名空间还包含全局日志级别阈值 `g_log_level`、直接写入标准错误的 `stderr_logger` 以及记录缓存命中率的 `cache_hit_rate` 辅助函数，共同构成一套灵活、可扩展的日志系统。

## Types

### `clore::logging::LogProxy`

Declaration: `src/support/logging.cppm:135`

Definition: `src/support/logging.cppm:135`

Implementation: [`Module support`](../../../modules/support/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 模板参数 `Level` 必须为合法的 `spdlog::level::level_enum` 值
- 所有 `operator()` 调用均使用固定的 `Level` 级别
- 格式化重载要求参数包非空（`sizeof...(Args) > 0`）

#### Key Members

- `operator()(std::string_view)`
- `operator()(std::format_string<Args...>, Args&&...)`（变参格式化版本）

#### Usage Patterns

- 创建具体级别的实例，如 `static constexpr auto error = LogProxy<spdlog::level::err>{};`
- 像函数一样调用：`error("operation failed: {}", reason)`
- 自动完成格式化并转发到底层 `log` 函数，无需每次指定级别

#### Member Functions

##### `clore::logging::LogProxy::operator()`

Declaration: `src/support/logging.cppm:142`

Definition: `src/support/logging.cppm:142`

Implementation: [`Module support`](../../../modules/support/index.md)

###### Declaration

```cpp
void (std::format_string<Args...>, Args &&...) const;
```

##### `clore::logging::LogProxy::operator()`

Declaration: `src/support/logging.cppm:136`

Definition: `src/support/logging.cppm:136`

Implementation: [`Module support`](../../../modules/support/index.md)

###### Declaration

```cpp
void (std::string_view) const;
```

## Variables

### `clore::logging::debug`

Declaration: `src/support/logging.cppm:148`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::debug` is a `constexpr inline` `LogProxy<spdlog::level::debug>` instance. It provides a compile-time logging proxy for debug-level messages.

#### Usage Patterns

- calling `debug()` with a format string and variadic arguments
- used in conditional log emission checks based on global log level

### `clore::logging::err`

Declaration: `src/support/logging.cppm:151`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::err` is a `constexpr inline` instance of `LogProxy<spdlog::level::err>`, declared at `src/support/logging.cppm:151`. It serves as a type-safe proxy for emitting log messages at the error severity level.

#### Usage Patterns

- used to log error-level messages via `LogProxy::operator()` or stream insertion
- compared against global log level `clore::logging::g_log_level` to conditionally emit output

### `clore::logging::g_log_level`

Declaration: `src/support/logging.cppm:125`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::g_log_level` is a global `inline std::optional<spdlog::level::level_enum>` declared at `src/support/logging.cppm:125`. It serves as the configurable global log level threshold for the logging system.

#### Usage Patterns

- read by `clore::logging::log`
- read by `clore::logging::stderr_logger`
- compared against message level

### `clore::logging::info`

Declaration: `src/support/logging.cppm:149`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::info` is a `constexpr inline` object of type `LogProxy<spdlog::level::info>`, declared in `src/support/logging.cppm` at line 149. It serves as a log proxy for the info log level.

#### Usage Patterns

- Used as a log proxy in `clore::logging::cache_hit_rate` to emit info-level log messages.

### `clore::logging::trace`

Declaration: `src/support/logging.cppm:147`

Implementation: [`Module support`](../../../modules/support/index.md)

常量 `clore::logging::trace` 是命名空间 `clore::logging` 中一个 `constexpr inline` 变量，类型为 `LogProxy<spdlog::level::trace>`。

#### Usage Patterns

- 用于记录跟踪级别的日志信息

### `clore::logging::warn`

Declaration: `src/support/logging.cppm:150`

Implementation: [`Module support`](../../../modules/support/index.md)

变量 `clore::logging::warn` 是一个 `constexpr inline` 的 `LogProxy<spdlog::level::warn>` 对象，用于表示警告级别的日志记录代理。

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `src/support/logging.cppm:161`

Definition: `src/support/logging.cppm:161`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::cache_hit_rate` 用于记录某个缓存的命中率。它接受一个用于标识缓存或上下文的 `std::string_view`，以及两个 `std::size_t` 值：分别代表缓存命中次数和总访问次数。此函数会根据给定的命中数和总次数计算出百分比，并通过 `clore::logging::log` 或其他日志机制将结果输出。

调用者必须确保 `std::string_view` 指向的内容在函数执行期间保持有效。两个 `std::size_t` 应分别反映缓存命中的次数以及被查询的缓存总次数。函数不检查计算所用的数值范围（例如总次数为零的情况），调用者应在必要时自行验证。函数不会抛出异常，并在日志记录完成后立即返回。

#### Usage Patterns

- Monitoring cache performance
- Debugging cache efficiency
- Reporting hit/miss statistics

### `clore::logging::log`

Declaration: `src/support/logging.cppm:127`

Definition: `src/support/logging.cppm:127`

Implementation: [`Module support`](../../../modules/support/index.md)

Declaration: [Declaration](functions/log.md)

将指定的日志级别与消息字符串传递给底层的日志基础设施。`clore::logging::log` 负责将一条格式化完成的消息（以 `std::string_view` 形式给出）连同其严重性等级（由 `spdlog::level::level_enum` 指定）记录到当前配置的日志目标中。

调用者必须提供有效的日志级别枚举值；消息内容可以是任意非空字符串。该函数不检查消息的有效性，也不处理格式化或传参转换，这些责任由 `clore::logging::LogProxy` 的 `operator()` 重载承担，后者最终会调用 `clore::logging::log` 来执行实际的输出。

#### Usage Patterns

- 被 `clore::logging::LogProxy::operator()` 重载调用
- 用于在代理对象中转发日志消息并执行级别过滤

### `clore::logging::stderr_logger`

Declaration: `src/support/logging.cppm:153`

Definition: `src/support/logging.cppm:153`

Implementation: [`Module support`](../../../modules/support/index.md)

函数 `clore::logging::stderr_logger` 接收一个 `std::string_view` 格式的消息，并将其直接写入标准错误流（`stderr`）。调用者可将该函数作为自定义日志接收器使用，或将日志条目转发至此以输出到标准终端错误通道。函数保证对每条传入的消息同步、完整地写出，不进行缓存或格式化处理；调用者应自行确保字符串视图在函数返回前保持有效。

#### Usage Patterns

- 在日志系统初始化时调用以设置 stderr 日志器为默认
- 用于替换或配置全局默认日志器为 stderr 输出

## Related Pages

- [Namespace clore](../index.md)

