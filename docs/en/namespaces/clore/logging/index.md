---
title: 'Namespace clore::logging'
description: 'The clore::logging namespace provides a structured logging framework built on top of spdlog. Its core declarations include a LogProxy template struct used to create constant proxy objects such as trace, debug, info, warn, and err―each statically bound to a specific severity level for compile‑time log filtering. The central dispatcher function log accepts a severity enum and a message string, routing entries to the appropriate sinks. Additional utilities include stderr_logger for immediate output to standard error and cache_hit_rate for monitoring cache effectiveness. A global optional g_log_level allows runtime level overrides. Architecturally, clore::logging serves as the single point of contact for all logging in the codebase, abstracting away the underlying logging implementation and promoting consistent, type‑safe, and performance‑aware logging.'
layout: doc
template: doc
---

# Namespace `clore::logging`

## Summary

The `clore::logging` namespace provides a structured logging framework built on top of spdlog. Its core declarations include a `LogProxy` template struct used to create constant proxy objects such as `trace`, `debug`, `info`, `warn`, and `err`―each statically bound to a specific severity level for compile‑time log filtering. The central dispatcher function `log` accepts a severity enum and a message string, routing entries to the appropriate sinks. Additional utilities include `stderr_logger` for immediate output to standard error and `cache_hit_rate` for monitoring cache effectiveness. A global optional `g_log_level` allows runtime level overrides. Architecturally, `clore::logging` serves as the single point of contact for all logging in the codebase, abstracting away the underlying logging implementation and promoting consistent, type‑safe, and performance‑aware logging.

## Types

### `clore::logging::LogProxy`

Declaration: `support/logging.cppm:112`

Definition: `support/logging.cppm:112`

Implementation: [`Module support`](../../../modules/support/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- template parameter `Level` is a `spdlog::level::level_enum`
- `operator()` overloads always call `log(Level, ...)`
- format-based overload requires `sizeof...(Args) > 0`

#### Key Members

- `operator()(std::string_view msg)`
- `operator()(std::format_string<Args...> fmt, Args&&... args)`

#### Usage Patterns

- used to create type-safe log callables for different log levels
- instances of `LogProxy` can be stored and passed as logging handlers, ensuring consistent level handling

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

A constant log proxy object for debug severity level, specialized with `spdlog::level::debug`. It provides a compile-time fixed log level for conditional logging.

#### Usage Patterns

- used to log debug messages
- invoked with format string and arguments

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::err` is a `constexpr inline` variable of type `LogProxy<spdlog::level::err>`, serving as a constant proxy object for logging messages at the error severity level.

#### Usage Patterns

- logged via call `operator` with format string and arguments

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::g_log_level` is a global `std::optional<spdlog::level::level_enum>` that provides an optional override for the log level used by the logging system.

#### Usage Patterns

- Read by `clore::logging::log` to check for a global log level override
- Read by `clore::logging::stderr_logger` to check for a global log level override

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Implementation: [`Module support`](../../../modules/support/index.md)

A constant inline proxy object for logging messages at the info severity level. It is declared as `constexpr inline LogProxy<spdlog::level::info> info` in the `clore::logging` namespace.

#### Usage Patterns

- used as a logging proxy for info level messages

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::trace` is a `constexpr inline` object of type `LogProxy<spdlog::level::trace>`, declared in the `clore::logging` namespace.

#### Usage Patterns

- emitting trace-level log messages

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::warn` is a `constexpr inline` variable of type `LogProxy<spdlog::level::warn>`, serving as a logging proxy for warning-level messages.

#### Usage Patterns

- used as a logging proxy for warning messages

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Implementation: [`Module support`](../../../modules/support/index.md)

Logs the cache hit rate for the operation or component identified by the given label. The caller provides two size parameters representing the number of cache hits and the total number of cache lookups (or equivalently, hits and misses). The function calculates the rate and records the result, typically at an informational logging level, allowing callers to monitor cache effectiveness without manually computing the ratio.

#### Usage Patterns

- Called to report cache hit/miss statistics
- Typically used in performance monitoring sections

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../modules/support/index.md)

Declaration: [Declaration](functions/log.md)

The function `clore::logging::log` accepts a `spdlog::level::level_enum` indicating the severity of the log message and a `std::string_view` containing the message content. The caller is responsible for providing a valid log level and the message to be recorded; the function then dispatches the message to the appropriate logging sinks. This function serves as the central logging entry point and is invoked by higher‑level logging proxies such as `clore::logging::LogProxy::operator()`.

#### Usage Patterns

- called by `LogProxy::operator()(std::string_view)`
- used directly for logging with an explicit level

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Implementation: [`Module support`](../../../modules/support/index.md)

The function `clore::logging::stderr_logger` is a simple logging callable that writes a given message to the standard error stream. It accepts a single `std::string_view` argument containing the text to be logged and returns `void`. This function is intended for use as a lightweight, immediate-output logger, typically within the logging framework or as a default sink for situations where no other logging backend is configured. The caller is responsible for providing a valid string view; no formatting or additional context is added by this function.

#### Usage Patterns

- Called at startup to configure stderr logging with a specific logger name
- Used to switch the default logger to stderr output
- Invocations optionally apply a previously stored log level

## Related Pages

- [Namespace clore](../index.md)

