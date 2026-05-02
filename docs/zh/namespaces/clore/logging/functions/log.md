---
title: 'clore::logging::log'
description: 'clore::logging::log 接受一个 spdlog::level::level_enum 标识符和一个 std::string_view 消息，并将该消息以指定级别写入日志后端。调用者有责任保证传入的日志级别枚举值有效，并确保消息内容在调用期间保持有效（即底层字符串不被修改或销毁）。该函数不返回任何值，其契约在通过 LogProxy 间接调用时经历相同的参数约束。'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Namespace clore::logging](../index.md)

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../../modules/support/index.md)

`clore::logging::log` 接受一个 `spdlog::level::level_enum` 标识符和一个 `std::string_view` 消息，并将该消息以指定级别写入日志后端。调用者有责任保证传入的日志级别枚举值有效，并确保消息内容在调用期间保持有效（即底层字符串不被修改或销毁）。该函数不返回任何值，其契约在通过 `LogProxy` 间接调用时经历相同的参数约束。

## Usage Patterns

- Called by `clore::logging::LogProxy::operator()(std::string_view)` to perform the actual logging after constructing the message
- Used directly by other components that need to log with an explicit severity level

## Called By

- method `clore::logging::LogProxy::operator()`

