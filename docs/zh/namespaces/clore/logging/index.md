---
title: 'Namespace clore::logging'
description: 'clore::logging 命名空间提供了基于 spdlog 的分级日志记录基础设施。核心机制是通过一组编译时常量代理对象（LogProxy），如 trace、debug、info、warn、err，让调用者以类型安全且高效的方式按指定严重级别输出日志消息。该命名空间同时包含底层日志函数（log）、直接输出到标准错误的便捷函数（stderr_logger）、用于监控缓存命中率的专用函数（cache_hit_rate）以及一个全局日志级别控制变量（g_log_level），共同构成了 clore 库中统一且灵活的日志记录层。'
layout: doc
template: doc
---

# Namespace `clore::logging`

## Summary

`clore::logging` 命名空间提供了基于 `spdlog` 的分级日志记录基础设施。核心机制是通过一组编译时常量代理对象（`LogProxy`），如 `trace`、`debug`、`info`、`warn`、`err`，让调用者以类型安全且高效的方式按指定严重级别输出日志消息。该命名空间同时包含底层日志函数（`log`）、直接输出到标准错误的便捷函数（`stderr_logger`）、用于监控缓存命中率的专用函数（`cache_hit_rate`）以及一个全局日志级别控制变量（`g_log_level`），共同构成了 clore 库中统一且灵活的日志记录层。

## Types

### `clore::logging::LogProxy`

Declaration: `support/logging.cppm:112`

Definition: `support/logging.cppm:112`

Implementation: [`Module support`](../../../modules/support/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The template argument `Level` must be a valid `spdlog::level::level_enum` value.
- All calls to `operator()` eventually invoke `log` with the same `Level`.
- The formatted overload requires at least one argument to avoid ambiguity with the plain-string overload.

#### Key Members

- `void operator()(std::string_view msg) const`
- `void operator()(std::format_string<Args...> fmt, Args&&... args) const`

#### Usage Patterns

- Instantiated for each log level (e.g., `LogProxy<spdlog::level::info>`) to create level-specific logging functors.
- Used as a building block for higher-level logging macros or wrapper objects that capture a level.
- Invoked call-site code to log messages without specifying the level explicitly.

#### Member Functions

##### `clore::logging::LogProxy::operator()`

Declaration: `support/logging.cppm:119`

Definition: `support/logging.cppm:119`

Implementation: [`Module support`](../../../modules/support/index.md)

###### Declaration

```cpp
void (int, Args &&...) const;
```

##### `clore::logging::LogProxy::operator()`

Declaration: `support/logging.cppm:113`

Definition: `support/logging.cppm:113`

Implementation: [`Module support`](../../../modules/support/index.md)

###### Declaration

```cpp
void (std::string_view) const;
```

## Variables

### `clore::logging::debug`

Declaration: `support/logging.cppm:125`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::debug` 是一个 `constexpr inline LogProxy<spdlog::level::debug>` 类型的编译时常量，用于调试级别的日志记录。

#### Usage Patterns

- 作为日志代理，用于输出调试信息

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::err` is a `constexpr inline LogProxy<spdlog::level::err>` object, representing a compile-time constant proxy for logging messages at the error severity level.

#### Usage Patterns

- used as a proxy for logging at error level

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::g_log_level` 是一个类型为 `std::optional<spdlog::level::level_enum>` 的内联变量，用于存储全局日志级别。

#### Usage Patterns

- 在 `clore::logging::log` 中读取以过滤日志级别
- 在 `clore::logging::stderr_logger` 中读取以过滤日志级别

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Implementation: [`Module support`](../../../modules/support/index.md)

A `constexpr inline` global variable of type `LogProxy<spdlog::level::info>` declared at `support/logging.cppm:126`. It serves as a compile-time proxy object for emitting log messages at the info severity level.

#### Usage Patterns

- Invoked as a function-like object with a format string and arguments to log at info level

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Implementation: [`Module support`](../../../modules/support/index.md)

A `constexpr inline` variable of type `LogProxy<spdlog::level::trace>`, declared at `support/logging.cppm:124`. It serves as a compile-time constant logging proxy for trace-level messages.

#### Usage Patterns

- invoked with formatting arguments to log trace messages

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::warn` is a `constexpr inline` variable of type `LogProxy<spdlog::level::warn>`, defined at `support/logging.cppm:127`. It serves as a compile-time logging proxy for warning-level messages.

#### Usage Patterns

- used as a log proxy to emit warning-level log messages
- referenced in function `clore::support::enable_utf8_console`

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::cache_hit_rate` 接受一个标签、命中次数和总次数，负责计算并记录缓存命中率。调用者应提供具有语义含义的 `std::string_view` 标识该缓存，并确保两个 `std::size_t` 实参非负且总量大于零（以避免除零错误）。该函数不返回任何值，但会将格式化后的比率写入日志系统（通常为 `info` 级别），便于监控和性能分析。

#### Usage Patterns

- 用于报告缓存性能统计
- 在缓存实现中调用时确保 `hits + misses` 非零
- 通过 `info` 记录缓存命中率

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../modules/support/index.md)

Declaration: [Declaration](functions/log.md)

`clore::logging::log` 记录一条具有指定严重级别的日志消息。调用者必须提供有效的 `spdlog::level::level_enum` 表示日志级别，以及一个 `std::string_view` 作为消息内容。该函数不返回值，由便捷封装如 `clore::logging::LogProxy::operator()` 在内部调用，也可直接用于自定义日志记录场景。

#### Usage Patterns

- 由`LogProxy::operator()`在日志记录时调用
- 用于按级别过滤日志消息

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::stderr_logger` 是一个日志记录函数，它接受一条消息（以 `std::string_view` 形式提供）并将其直接输出到标准错误流。调用者只需提供消息内容，无需指定日志级别或额外配置；该函数负责将消息写入 `stderr`。它适用于不需要分级过滤、仅希望将日志文本发送到标准错误输出的场景。

#### Usage Patterns

- Called once at program startup to initialize logging
- Used to configure logging output to stderr

## Related Pages

- [Namespace clore](../index.md)

