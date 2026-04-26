---
title: 'clore::logging::log'
description: '函数 clore::logging::log 首先根据全局日志级别 clore::logging::g_log_level 进行过滤。如果 g_log_level 已被设置且参数 lvl 的级别低于该阈值，函数会直接返回，避免不必要的日志输出。否则，它将通过调用 spdlog::default_logger_raw()->log 将消息 msg 传递到 spdlog 后端。该实现依赖于 spdlog 库的日志记录基础设施，并利用预编译的格式字符串 "{}" 将消息作为纯字符串记录。'
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

函数 `clore::logging::log` 首先根据全局日志级别 `clore::logging::g_log_level` 进行过滤。如果 `g_log_level` 已被设置且参数 `lvl` 的级别低于该阈值，函数会直接返回，避免不必要的日志输出。否则，它将通过调用 `spdlog::default_logger_raw()->log` 将消息 `msg` 传递到 spdlog 后端。该实现依赖于 spdlog 库的日志记录基础设施，并利用预编译的格式字符串 `"{}"` 将消息作为纯字符串记录。

## Side Effects

- 记录一条日志消息到`spdlog`默认日志器的输出目标（如控制台或文件）

## Reads From

- 参数`lvl`
- 参数`msg`
- 全局变量`g_log_level`

## Usage Patterns

- 由`LogProxy::operator()`在日志记录时调用
- 用于按级别过滤日志消息

## Called By

- method `clore::logging::LogProxy::operator()`

