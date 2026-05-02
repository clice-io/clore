---
title: 'clore::logging::log'
description: 'The function clore::logging::log accepts a severity level of type spdlog::level::level_enum together with a std::string_view message. It is the central logging sink in the clore::logging module, responsible for emitting the formatted message at the given severity. Callers must supply a valid level enumerator and a non-null message; the function guarantees that the message is dispatched to the configured logging backend (e.g., stderr, file, or other sinks) without further transformation. No special ownership of the string view is required—the function consumes the data synchronously before returning.'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Namespace clore::logging](../index.md)

Declaration: `support/logging.cppm:104`

Definition: `support/logging.cppm:104`

Implementation: [`Module support`](../../../../modules/support/index.md)

The function `clore::logging::log` accepts a severity level of type `spdlog::level::level_enum` together with a `std::string_view` message. It is the central logging sink in the `clore::logging` module, responsible for emitting the formatted message at the given severity. Callers must supply a valid level enumerator and a non-null message; the function guarantees that the message is dispatched to the configured logging backend (e.g., stderr, file, or other sinks) without further transformation. No special ownership of the string view is required—the function consumes the data synchronously before returning.

## Usage Patterns

- Invoked by `clore::logging::LogProxy::operator()(std::string_view)` to route formatted messages to `spdlog`
- Used as the underlying logging primitive that respects the `g_log_level` threshold

## Called By

- method `clore::logging::LogProxy::operator()`

