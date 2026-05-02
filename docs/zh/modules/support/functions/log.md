---
title: 'clore::logging::log'
description: '函数 clore::logging::log 的实现首先检查可选的全局日志级别 clore::logging::g_log_level：若该级别已设置且传入的 lvl 低于阈值，则立即返回，避免不必要的格式化开销。否则，直接委托给 spdlog::default_logger_raw() 的 log 方法，使用 "{}" 格式字符串将消息输出。该函数依赖 spdlog 库实现底层日志记录，并通过 g_log_level 提供静态的日志等级过滤控制。'
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

函数 `clore::logging::log` 的实现首先检查可选的全局日志级别 `clore::logging::g_log_level`：若该级别已设置且传入的 `lvl` 低于阈值，则立即返回，避免不必要的格式化开销。否则，直接委托给 `spdlog::default_logger_raw()` 的 `log` 方法，使用 `"{}"` 格式字符串将消息输出。该函数依赖 `spdlog` 库实现底层日志记录，并通过 `g_log_level` 提供静态的日志等级过滤控制。

## Side Effects

- Logs a message via `spdlog::default_logger_raw()->log()`

## Reads From

- global variable `g_log_level`
- function parameter `lvl`
- function parameter `msg`

## Writes To

- spdlog's default logger (log output)

## Usage Patterns

- Called by `clore::logging::LogProxy::operator()(std::string_view)` to perform the actual logging after constructing the message
- Used directly by other components that need to log with an explicit severity level

## Called By

- method `clore::logging::LogProxy::operator()`

