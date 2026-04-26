---
title: 'clore::logging::log'
description: 'The implementation of clore::logging::log applies a two-step dispatch: it first checks the global filter clore::logging::g_log_level, and only proceeds to the underlying logger if the requested severity meets the threshold. When g_log_level has a value and the incoming lvl is strictly less than that stored level, the function returns immediately, performing no I/O or formatting. Otherwise, it forwards the call directly to spdlog::default_logger_raw()->log, passing lvl and formatting the msg argument as the sole format string parameter ("{}"). This avoids the overhead of constructing a formatted output when the message would be suppressed.'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Module support](../index.md)

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Declaration: [`Namespace clore::logging`](../../../namespaces/clore/logging/index.md)

## Implementation

```cpp
inline void log(spdlog::level::level_enum lvl, std::string_view msg) {
    if(g_log_level.has_value() && lvl < *g_log_level) {
        return;
    }
    spdlog::default_logger_raw()->log(lvl, "{}", msg);
}
```

The implementation of `clore::logging::log` applies a two-step dispatch: it first checks the global filter `clore::logging::g_log_level`, and only proceeds to the underlying logger if the requested severity meets the threshold. When `g_log_level` has a value and the incoming `lvl` is strictly less than that stored level, the function returns immediately, performing no I/O or formatting. Otherwise, it forwards the call directly to `spdlog::default_logger_raw()->log`, passing `lvl` and formatting the `msg` argument as the sole format string parameter (`"{}"`). This avoids the overhead of constructing a formatted output when the message would be suppressed.

The only external dependency is the `spdlog` library for the logger instance and level type. The filtering state is held in the file‑scoped variable `clore::logging::g_log_level`, which is a `std::optional<spdlog::level::level_enum>`. No additional string processing, caching, or level‑name resolution is performed inside `log`; those responsibilities are delegated to callers (e.g., `LogProxy::operator()`) or to other functions in the `clore::logging` namespace.

## Side Effects

- Logs a message to the spdlog default logger output.

## Reads From

- `g_log_level` global
- `lvl` parameter
- `msg` parameter

## Writes To

- spdlog default logger output

## Usage Patterns

- called by `LogProxy::operator()(std::string_view)`
- used directly for logging with an explicit level

## Called By

- method `clore::logging::LogProxy::operator()`

