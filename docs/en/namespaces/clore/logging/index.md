---
title: 'Namespace clore::logging'
description: 'The clore::logging namespace encapsulates the project’s logging infrastructure, providing a centralized, severity‑based logging system built on top of spdlog. It exposes a main log function that accepts a severity level and a string message, dispatching the output to a configurable backend. A global optional variable g_log_level controls the minimum severity threshold, allowing runtime filtering of log output. Additional utility functions such as stderr_logger and cache_hit_rate offer specialized logging sinks and higher‑level logging helpers.'
layout: doc
template: doc
---

# Namespace `clore::logging`

## Summary

The `clore::logging` namespace encapsulates the project’s logging infrastructure, providing a centralized, severity‑based logging system built on top of spdlog. It exposes a main `log` function that accepts a severity level and a string message, dispatching the output to a configurable backend. A global optional variable `g_log_level` controls the minimum severity threshold, allowing runtime filtering of log output. Additional utility functions such as `stderr_logger` and `cache_hit_rate` offer specialized logging sinks and higher‑level logging helpers.

The namespace also defines several `constexpr inline` instances of the `LogProxy` structure (e.g., `debug`, `trace`, `info`, `warn`, `err`). These stateless proxy objects serve as compile‑time entry points for emitting messages at specific severity levels, simplifying call‑site code and ensuring consistent level usage. Together, these elements form a modular logging subsystem that supports both flexible runtime configuration and concise, type‑safe invocation.

## Types

### `clore::logging::LogProxy`

Declaration: `support/logging.cppm:112`

Definition: `support/logging.cppm:112`

Implementation: [`Module support`](../../../modules/support/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The logging level `Level` is fixed at compile time via a non-type template parameter.
- All messages are forwarded to a free function `log` that must be defined in the surrounding scope.
- The variadic overload requires at least one argument (`sizeof...(Args) > 0`).

#### Key Members

- `void operator()(std::string_view msg) const`
- `void operator()(std::format_string<Args...> fmt, Args&&... args) const` (template, requires `sizeof...(Args) > 0`)

#### Usage Patterns

- Instantiated with a specific log level (e.g., `LogProxy<spdlog::level::info>`) to create a level‑specific logger object.
- Called with either a plain string or a format string and arguments to produce a formatted log message at the predetermined level.

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

Inline `constexpr` variable `clore::logging::debug` of type `LogProxy<spdlog::level::debug>` providing a stateless entry point for emitting debug-level log messages within the `clore::logging` namespace.

#### Usage Patterns

- invoked as a log proxy for `spdlog::level::debug` messages
- used alongside other severity-level proxies in `clore::logging`

### `clore::logging::err`

Declaration: `support/logging.cppm:128`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::err` is a `constexpr inline LogProxy<spdlog::level::err>` object, declared in `support/logging.cppm` at line 128. It is a compile-time constant that provides a logging proxy for error-level log messages.

### `clore::logging::g_log_level`

Declaration: `support/logging.cppm:102`

Implementation: [`Module support`](../../../modules/support/index.md)

The variable `clore::logging::g_log_level` is a public inline `std::optional<spdlog::level::level_enum>` declared at `support/logging.cppm:102`. It provides a global configuration point for controlling the minimum log severity threshold.

#### Usage Patterns

- read by `clore::logging::log` to decide message output
- read by `clore::logging::stderr_logger` to control logging behavior

### `clore::logging::info`

Declaration: `support/logging.cppm:126`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::info` is a `constexpr inline` instance of `LogProxy<spdlog::level::info>` declared at `support/logging.cppm:126`, providing a compile-time logging entry point for messages at the `spdlog::level::info` severity within the `clore::logging` namespace.

#### Usage Patterns

- invoked from `clore::logging::cache_hit_rate` to report informational messages
- used as a severity-tagged dispatcher built on `LogProxy<spdlog::level::info>`
- serves as the info-level counterpart to other `LogProxy` instances in the namespace

### `clore::logging::trace`

Declaration: `support/logging.cppm:124`

Implementation: [`Module support`](../../../modules/support/index.md)

A `constexpr inline` instance of `LogProxy<spdlog::level::trace>` exposed at namespace scope as `clore::logging::trace`, providing a convenient entry point for emitting trace-level log messages.

#### Usage Patterns

- invoked as a namespace-scope entry point for trace-level logging
- parallels other severity proxies like `debug`, `info`, `warn`, and `err`

### `clore::logging::warn`

Declaration: `support/logging.cppm:127`

Implementation: [`Module support`](../../../modules/support/index.md)

Compile-time logging proxy for warning-level messages within the `clore::logging` namespace.

#### Usage Patterns

- invoked as a logging entry point for warning-level messages
- used by `clore::support::enable_utf8_console`
- parallels other level proxies (`debug`, `trace`, `info`, `err`) in the namespace

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `support/logging.cppm:138`

Definition: `support/logging.cppm:138`

Implementation: [`Module support`](../../../modules/support/index.md)

This function records and emits a log message indicating the current cache hit rate. The caller supplies a cache identifier as a `std::string_view`, the number of cache hits as a `std::size_t`, and the total number of cache accesses as a `std::size_t`. The hit rate is internally calculated from these two counts and logged at the appropriate severity level.

#### Usage Patterns

- reporting cache hit rates
- logging after cache lookups

### `clore::logging::log`

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../modules/support/index.md)

Declaration: [Declaration](functions/log.md)

The function `clore::logging::log` accepts a severity level of type `spdlog::level::level_enum` together with a `std::string_view` message. It is the central logging sink in the `clore::logging` module, responsible for emitting the formatted message at the given severity. Callers must supply a valid level enumerator and a non-null message; the function guarantees that the message is dispatched to the configured logging backend (e.g., stderr, file, or other sinks) without further transformation. No special ownership of the string view is required—the function consumes the data synchronously before returning.

#### Usage Patterns

- Invoked by `clore::logging::LogProxy::operator()(std::string_view)` to route formatted messages to `spdlog`
- Used as the underlying logging primitive that respects the `g_log_level` threshold

### `clore::logging::stderr_logger`

Declaration: `support/logging.cppm:130`

Definition: `support/logging.cppm:130`

Implementation: [`Module support`](../../../modules/support/index.md)

The function `clore::logging::stderr_logger` accepts a `std::string_view` message and logs it to the standard error stream. It serves as a simple, direct logging sink for callers that need to output a text message to stderr without additional formatting or level filtering. The caller supplies only the message; the function handles output synchronously.

#### Usage Patterns

- Initializing logging configuration at startup
- Switching global logging to stderr with a specific logger name

## Related Pages

- [Namespace clore](../index.md)

