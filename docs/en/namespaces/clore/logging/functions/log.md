---
title: 'clore::logging::log'
description: 'The function clore::logging::log accepts a spdlog::level::level_enum indicating the severity of the log message and a std::string_view containing the message content. The caller is responsible for providing a valid log level and the message to be recorded; the function then dispatches the message to the appropriate logging sinks. This function serves as the central logging entry point and is invoked by higher‑level logging proxies such as clore::logging::LogProxy::operator().'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Namespace clore::logging](../index.md)

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../../modules/support/index.md)

The function `clore::logging::log` accepts a `spdlog::level::level_enum` indicating the severity of the log message and a `std::string_view` containing the message content. The caller is responsible for providing a valid log level and the message to be recorded; the function then dispatches the message to the appropriate logging sinks. This function serves as the central logging entry point and is invoked by higher‑level logging proxies such as `clore::logging::LogProxy::operator()`.

## Usage Patterns

- called by `LogProxy::operator()(std::string_view)`
- used directly for logging with an explicit level

## Called By

- method `clore::logging::LogProxy::operator()`

