---
title: 'clore::logging::log'
description: 'clore::logging::log 记录一条具有指定严重级别的日志消息。调用者必须提供有效的 spdlog::level::level_enum 表示日志级别，以及一个 std::string_view 作为消息内容。该函数不返回值，由便捷封装如 clore::logging::LogProxy::operator() 在内部调用，也可直接用于自定义日志记录场景。'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Namespace clore::logging](../index.md)

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../../modules/support/index.md)

`clore::logging::log` 记录一条具有指定严重级别的日志消息。调用者必须提供有效的 `spdlog::level::level_enum` 表示日志级别，以及一个 `std::string_view` 作为消息内容。该函数不返回值，由便捷封装如 `clore::logging::LogProxy::operator()` 在内部调用，也可直接用于自定义日志记录场景。

## Usage Patterns

- 由`LogProxy::operator()`在日志记录时调用
- 用于按级别过滤日志消息

## Called By

- method `clore::logging::LogProxy::operator()`

