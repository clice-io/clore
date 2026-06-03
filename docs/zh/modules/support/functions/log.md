---
title: 'clore::logging::log'
description: 'clore::logging::log 的实现首先检查静态可选变量 g_log_level 是否已设置，并比较传入的 lvl 参数是否低于该阈值；如果低于，函数立即返回，不执行任何日志输出。否则，函数调用 spdlog::default_logger_raw()->log(lvl, "{}", msg)，将原始格式化字符串 msg 传递给 spdlog 的默认日志记录器进行输出。该实现完全依赖于 spdlog 库的日志基础设施，并在调用前执行一次快速的优先级过滤。'
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

`clore::logging::log` 的实现首先检查静态可选变量 `g_log_level` 是否已设置，并比较传入的 `lvl` 参数是否低于该阈值；如果低于，函数立即返回，不执行任何日志输出。否则，函数调用 `spdlog::default_logger_raw()->log(lvl, "{}", msg)`，将原始格式化字符串 `msg` 传递给 spdlog 的默认日志记录器进行输出。该实现完全依赖于 spdlog 库的日志基础设施，并在调用前执行一次快速的优先级过滤。

## Side Effects

- 写入日志输出（通过 spdlog）
- 读取并比对全局日志级别 `g_log_level`

## Reads From

- `g_log_level` 全局变量
- `lvl` 参数
- `msg` 参数

## Writes To

- 日志输出（由 spdlog 管理）

## Usage Patterns

- 被 `clore::logging::LogProxy::operator()` 重载调用
- 用于在代理对象中转发日志消息并执行级别过滤

## Called By

- method `clore::logging::LogProxy::operator()`

