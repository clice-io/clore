---
title: 'clore::logging::log'
description: 'The function first checks whether the global filtering level g_log_level has been set and, if so, whether the requested lvl is strictly lower than that threshold. When the condition holds, log returns immediately without performing any output. Otherwise, it forwards the lvl and the formatted msg to the underlying logging library by invoking spdlog::default_logger_raw()->log(lvl, "{}", msg). This two‑step control flow—guard against insufficient severity, then delegate to the spdlog default logger—provides a lightweight, configurable filtering mechanism without modifying the logger’s own level settings.'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Module support](../index.md)

Declaration: `src/support/logging.cppm:127`

Definition: `src/support/logging.cppm:127`

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

The function first checks whether the global filtering level `g_log_level` has been set and, if so, whether the requested `lvl` is strictly lower than that threshold. When the condition holds, `log` returns immediately without performing any output. Otherwise, it forwards the `lvl` and the formatted `msg` to the underlying logging library by invoking `spdlog::default_logger_raw()->log(lvl, "{}", msg)`. This two‑step control flow—guard against insufficient severity, then delegate to the spdlog default logger—provides a lightweight, configurable filtering mechanism without modifying the logger’s own level settings.

## Side Effects

- Logs a message via spdlog default logger, which may write to console or file.

## Reads From

- `g_log_level`
- lvl parameter
- msg parameter

## Writes To

- spdlog default logger output

## Usage Patterns

- Called by `LogProxy::operator()` to dispatch logging
- Used for conditional logging with level filtering

## Called By

- method `clore::logging::LogProxy::operator()`

