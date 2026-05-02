---
title: 'clore::logging::log'
description: 'The function clore::logging::log implements a level‑gated dispatch to the spdlog library. Its control flow begins by checking whether the module‑level global g_log_level has been set (via its std::optional state). If a threshold exists and the incoming lvl is strictly less than that threshold, the function returns immediately without performing any logging work. This early‑exit optimization avoids unnecessary formatting and I/O when the message would be filtered out by the current log level configuration. When the level passes the gate, the call is forwarded to spdlog::default_logger_raw()->log(lvl, "{}", msg), which writes the formatted message through the default spdlog logger. No additional buffering or transformation is applied; the raw spdlog::level::level_enum and the std::string_view message are passed directly. The function therefore depends on g_log_level (a std::optional<spdlog::level::level_enum> defined in the same clore::logging namespace) and on the spdlog runtime API for the actual output. Its behaviour is fully synchronous and non‑allocating for the threshold check; allocation may occur inside spdlog depending on the sink configuration.'
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

The function `clore::logging::log` implements a level‑gated dispatch to the spdlog library. Its control flow begins by checking whether the module‑level global `g_log_level` has been set (via its `std::optional` state). If a threshold exists and the incoming `lvl` is strictly less than that threshold, the function returns immediately without performing any logging work. This early‑exit optimization avoids unnecessary formatting and I/O when the message would be filtered out by the current log level configuration. When the level passes the gate, the call is forwarded to `spdlog::default_logger_raw()->log(lvl, "{}", msg)`, which writes the formatted message through the default spdlog logger. No additional buffering or transformation is applied; the raw `spdlog::level::level_enum` and the `std::string_view` message are passed directly. The function therefore depends on `g_log_level` (a `std::optional<spdlog::level::level_enum>` defined in the same `clore::logging` namespace) and on the spdlog runtime API for the actual output. Its behaviour is fully synchronous and non‑allocating for the threshold check; allocation may occur inside spdlog depending on the sink configuration.

## Side Effects

- Writes a log record through `spdlog::default_logger_raw()` at the specified severity level

## Reads From

- parameter `lvl`
- parameter `msg`
- global optional `g_log_level`
- the `spdlog` default logger via `spdlog::default_logger_raw()`

## Writes To

- the `spdlog` default logger's output sinks

## Usage Patterns

- Invoked by `clore::logging::LogProxy::operator()(std::string_view)` to route formatted messages to `spdlog`
- Used as the underlying logging primitive that respects the `g_log_level` threshold

## Called By

- method `clore::logging::LogProxy::operator()`

