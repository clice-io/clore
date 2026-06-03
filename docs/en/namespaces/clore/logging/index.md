---
title: 'Namespace clore::logging'
description: 'The clore::logging namespace provides a centralized, level‑based logging abstraction built on top of the spdlog library. It offers both a direct log function that dispatches messages at a specified severity level and a collection of constexpr inline LogProxy variables—trace, debug, info, warn, and err—that allow compile‑time selection of the logging level. The LogProxy struct supports two operator() overloads: one accepting a std::string_view literal and another accepting a std::format_string with variadic arguments, enabling both simple and formatted messages without runtime overhead for the level selection.'
layout: doc
template: doc
---

# Namespace `clore::logging`

## Summary

The `clore::logging` namespace provides a centralized, level‑based logging abstraction built on top of the spdlog library. It offers both a direct `log` function that dispatches messages at a specified severity level and a collection of `constexpr inline LogProxy` variables—`trace`, `debug`, `info`, `warn`, and `err`—that allow compile‑time selection of the logging level. The `LogProxy` struct supports two `operator()` overloads: one accepting a `std::string_view` literal and another accepting a `std::format_string` with variadic arguments, enabling both simple and formatted messages without runtime overhead for the level selection.

Architecturally, the namespace decouples logging configuration from the rest of the application by storing the active severity threshold in a global variable and providing utility functions such as `stderr_logger` for direct output and `cache_hit_rate` for structured cache performance logging. This design centralizes log routing, filtering, and formatting, making it straightforward for the application to adjust logging behavior at a single point while callers use familiar, type‑safe proxy objects.

## Types

### `clore::logging::LogProxy`

Declaration: `src/support/logging.cppm:135`

Definition: `src/support/logging.cppm:135`

Implementation: [`Module support`](../../../modules/support/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The log level is fixed at compile time and determined solely by the `Level` template parameter.
- All logging operations ultimately call `log(Level, ...)` which must be defined and accessible.

#### Key Members

- `operator()(std::string_view msg) const`
- `operator()(std::format_string<Args...> fmt, Args&&... args) const`

#### Usage Patterns

- Instantiated with specific log levels to create lightweight level-specific logging functors.
- Used in contexts where a callable object with a fixed log level is needed, such as in logger implementations or logging macros.
- Relies on an external `log` function that is not defined within the struct.

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

`clore::logging::debug` is a `constexpr inline` variable of type `LogProxy<spdlog::level::debug>`, declared at `src/support/logging.cppm:148`. It serves as a compile-time constant logger proxy for debug-level logging.

### `clore::logging::err`

Declaration: `src/support/logging.cppm:151`

Implementation: [`Module support`](../../../modules/support/index.md)

A constant inline variable of type `LogProxy<spdlog::level::err>`, providing a predefined logger proxy for error-level logging.

#### Usage Patterns

- logging error messages via member functions of `LogProxy`

### `clore::logging::g_log_level`

Declaration: `src/support/logging.cppm:125`

Implementation: [`Module support`](../../../modules/support/index.md)

An inline global variable of type `std::optional<spdlog::level::level_enum>` that stores the current logging threshold for the logging system.

#### Usage Patterns

- Referenced in `clore::logging::log` to filter log messages
- Referenced in `clore::logging::stderr_logger` to determine output level

### `clore::logging::info`

Declaration: `src/support/logging.cppm:149`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::info` is a `constexpr inline` variable of type `LogProxy<spdlog::level::info>` defined in the `clore::logging` namespace. It serves as a proxy for logging messages at the info severity level.

#### Usage Patterns

- emits info-level log messages via stream `operator`
- used in `clore::logging::cache_hit_rate`
- passed as argument to logging functions

### `clore::logging::trace`

Declaration: `src/support/logging.cppm:147`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::trace` is a `constexpr inline` variable of type `LogProxy<spdlog::level::trace>`, declared in `src/support/logging.cppm`.

#### Usage Patterns

- trace-level logging calls

### `clore::logging::warn`

Declaration: `src/support/logging.cppm:150`

Implementation: [`Module support`](../../../modules/support/index.md)

`clore::logging::warn` is a `constexpr inline` variable of type `LogProxy<spdlog::level::warn>`, serving as a logging proxy for warning-level messages.

#### Usage Patterns

- warning-level logging proxy

## Functions

### `clore::logging::cache_hit_rate`

Declaration: `src/support/logging.cppm:161`

Definition: `src/support/logging.cppm:161`

Implementation: [`Module support`](../../../modules/support/index.md)

The function `clore::logging::cache_hit_rate` accepts a cache identifier as a `std::string_view`, a count of hits as a `std::size_t`, and a total number of accesses as a `std::size_t`. It computes and logs the cache hit rate (hits divided by total) using the framework's logging infrastructure. The caller is responsible for providing a non‑zero total unless the cache has had no accesses; in that case the function may handle the zero case gracefully (e.g., by logging a special value or skipping the computation). The logged message is emitted through the active logging channel (such as `clore::logging::stderr_logger`) and includes the cache identifier and the computed rate.

#### Usage Patterns

- Used to monitor and log cache hit rate
- Called after cache access to record statistics

### `clore::logging::log`

Declaration: `src/support/logging.cppm:127`

Definition: `src/support/logging.cppm:127`

Implementation: [`Module support`](../../../modules/support/index.md)

Declaration: [Declaration](functions/log.md)

The function `clore::logging::log` accepts an `spdlog::level::level_enum` severity level and a `std::string_view` message, then dispatches the message to the configured logging sink(s) at that severity. The caller is responsible for providing a valid level enumerator and a non‑null message (the string view is expected to remain valid for the duration of the call). The logging framework respects the threshold and formatting set by the application; the function itself does not modify the message or add extra structure.

#### Usage Patterns

- Called by `LogProxy::operator()` to dispatch logging
- Used for conditional logging with level filtering

### `clore::logging::stderr_logger`

Declaration: `src/support/logging.cppm:153`

Definition: `src/support/logging.cppm:153`

Implementation: [`Module support`](../../../modules/support/index.md)

The function `clore::logging::stderr_logger` accepts a `std::string_view` message and writes it to the standard error stream. Its sole responsibility is to output the provided string literally to stderr, without applying any log‑level filtering, formatting, or additional metadata. Callers can rely on this function to perform a synchronous write of the given text to stderr; under normal conditions it does not throw exceptions and provides a simple, direct logging path.

#### Usage Patterns

- Used during logging initialization to direct output to stderr
- Called early in program startup to configure logging

## Related Pages

- [Namespace clore](../index.md)

