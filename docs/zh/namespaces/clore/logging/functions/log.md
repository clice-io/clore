---
title: 'clore::logging::log'
description: '将指定的日志级别与消息字符串传递给底层的日志基础设施。clore::logging::log 负责将一条格式化完成的消息（以 std::string_view 形式给出）连同其严重性等级（由 spdlog::level::level_enum 指定）记录到当前配置的日志目标中。'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Namespace clore::logging](../index.md)

Declaration: `src/support/logging.cppm:127`

Definition: `src/support/logging.cppm:127`

Implementation: [`Module support`](../../../../modules/support/index.md)

将指定的日志级别与消息字符串传递给底层的日志基础设施。`clore::logging::log` 负责将一条格式化完成的消息（以 `std::string_view` 形式给出）连同其严重性等级（由 `spdlog::level::level_enum` 指定）记录到当前配置的日志目标中。

调用者必须提供有效的日志级别枚举值；消息内容可以是任意非空字符串。该函数不检查消息的有效性，也不处理格式化或传参转换，这些责任由 `clore::logging::LogProxy` 的 `operator()` 重载承担，后者最终会调用 `clore::logging::log` 来执行实际的输出。

## Usage Patterns

- 被 `clore::logging::LogProxy::operator()` 重载调用
- 用于在代理对象中转发日志消息并执行级别过滤

## Called By

- method `clore::logging::LogProxy::operator()`

