---
title: 'Namespace clore::logging'
description: 'clore::logging 命名空间负责为 clore 代码库提供统一、类型安全的日志基础设施。它封装了 spdlog 库，通过预定义的 LogProxy 常量（如 trace, debug, info, warn, err）提供不同严重级别的日志输出。此外，它还包含全局日志过滤级别 g_log_level 以及便捷函数 log、stderr_logger 和 cache_hit_rate，使得开发者可以灵活控制日志输出行为，并将缓存命中率等指标记录到日志系统。'
layout: doc
template: doc
---

# Namespace `clore::logging`

## Summary

`clore::logging` 命名空间负责为 `clore` 代码库提供统一、类型安全的日志基础设施。它封装了 `spdlog` 库，通过预定义的 `LogProxy` 常量（如 `trace`, `debug`, `info`, `warn`, `err`）提供不同严重级别的日志输出。此外，它还包含全局日志过滤级别 `g_log_level` 以及便捷函数 `log`、`stderr_logger` 和 `cache_hit_rate`，使得开发者可以灵活控制日志输出行为，并将缓存命中率等指标记录到日志系统。

在架构上，`clore::logging` 扮演着中央日志子系统的角色，通过 `LogProxy` 将可变参数消息格式化为字符串，并委托给底层的 `spdlog` 记录器。它使得所有 `clore` 模块能够以一致的方式生成、过滤和路由日志信息，同时保持对日志后端（如文件、控制台）的抽象能力。

## Types

### `clore::logging::LogProxy`

Declaration: `support/logging.cppm:112`

Definition: `support/logging.cppm:112`

Implementation: [`Module support`](../../../modules/support/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The log level `Level` is fixed at compile time per instantiation
- Every invocation produces exactly one call to `log` with the same `Level`
- Formatting uses `std::format` and requires `Args...` to be formattable

#### Key Members

- Template parameter `spdlog::level::level_enum Level`
- Operator `void operator()(std::string_view msg) const`
- Operator `void operator()(std::format_string<Args...> fmt, Args&&... args) const`

#### Usage Patterns

- Instantiated as type aliases for each log level (e.g., `using InfoProxy = LogProxy<spdlog::level::info>`)
- Used in conjunction with macros or wrapper functions to create level‑specific log calls
- Enables uniform formatting behavior across all severity levels

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

`clore::logging::debug` is a `constexpr inline` variable of type `LogProxy<spdlog::level::debug>`, declared in `support/logging.cppm` at line 125.

#### Usage Patterns

- Used to emit debug-level log messages via its call `operator`

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Implementation: [`Module support`](../../../modules/support/index.md)

变量 `clore::logging::err` 是一个公开的 `constexpr inline` 常量，类型为 `LogProxy<spdlog::level::err>`，在 `support/logging.cppm:128` 处声明。

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Implementation: [`Module support`](../../../modules/support/index.md)

变量 `clore::logging::g_log_level` 是一个全局可选的日志级别，类型为 `std::optional<spdlog::level::level_enum>`。它用于控制日志过滤，决定哪些级别的日志消息被处理。

#### Usage Patterns

- 在 `clore::logging::log` 中被读取以过滤日志级别
- 在 `clore::logging::stderr_logger` 中被读取以确定输出级别

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::info` is a public `constexpr inline LogProxy<spdlog::level::info>` declared in `support/logging.cppm` at line 126. It provides a logging proxy object for emitting messages at the info severity level.

#### Usage Patterns

- Called by `clore::logging::cache_hit_rate` to output info-level log messages

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Implementation: [`Module support`](../../../modules/support/index.md)

变量 `clore::logging::trace` 是一个 `constexpr inline` 的 `LogProxy<spdlog::level::trace>` 类型对象，在 `support/logging.cppm` 中声明。

#### Usage Patterns

- 作为 trace 级别日志的输出入口

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Implementation: [`Module support`](../../../modules/support/index.md)

变量 `clore::logging::warn` 是一个 `constexpr inline LogProxy<spdlog::level::warn>` 类型的全局对象，用于表示警告级别的日志记录。它作为 `clore` 日志系统的一部分，允许开发者以统一的方式输出警告信息。

#### Usage Patterns

- 使用 `clore::logging::warn("message {}", arg)` 记录警告级别日志
- 在函数 `clore::support::enable_utf8_console` 中可能被调用以输出警告信息

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Implementation: [`Module support`](../../../modules/support/index.md)

The `clore::logging::cache_hit_rate` function records a cache hit rate metric. The caller provides a cache identifier as a `std::string_view`, the number of hits as a `std::size_t`, and the total number of accesses as a `std::size_t`. The function computes the ratio and makes it available through the logging subsystem.

#### Usage Patterns

- used to report cache hit rate statistics
- called after cache operations to monitor efficiency

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../modules/support/index.md)

Declaration: [Declaration](functions/log.md)

`clore::logging::log` 接受一个 `spdlog::level::level_enum` 标识符和一个 `std::string_view` 消息，并将该消息以指定级别写入日志后端。调用者有责任保证传入的日志级别枚举值有效，并确保消息内容在调用期间保持有效（即底层字符串不被修改或销毁）。该函数不返回任何值，其契约在通过 `LogProxy` 间接调用时经历相同的参数约束。

#### Usage Patterns

- Called by `clore::logging::LogProxy::operator()(std::string_view)` to perform the actual logging after constructing the message
- Used directly by other components that need to log with an explicit severity level

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::stderr_logger` 是一个将日志消息输出到标准错误流的函数。调用者提供一个 `std::string_view` 表示的消息，该函数将其写入 `stderr`。此函数通常用作日志框架中的接收器（sink），或者用于直接记录消息而不需要日志级别过滤。它不返回任何值。

#### Usage Patterns

- used for initializing logging to stderr in application startup

## Related Pages

- [Namespace clore](../index.md)

