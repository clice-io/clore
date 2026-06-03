---
title: 'clore::logging::log'
description: 'The function clore::logging::log accepts an spdlog::level::level_enum severity level and a std::string_view message, then dispatches the message to the configured logging sink(s) at that severity. The caller is responsible for providing a valid level enumerator and a non‑null message (the string view is expected to remain valid for the duration of the call). The logging framework respects the threshold and formatting set by the application; the function itself does not modify the message or add extra structure.'
layout: doc
template: doc
---

# `clore::logging::log`

Owner: [Namespace clore::logging](../index.md)

Declaration: `src/support/logging.cppm:127`

Definition: `src/support/logging.cppm:127`

Implementation: [`Module support`](../../../../modules/support/index.md)

The function `clore::logging::log` accepts an `spdlog::level::level_enum` severity level and a `std::string_view` message, then dispatches the message to the configured logging sink(s) at that severity. The caller is responsible for providing a valid level enumerator and a non‑null message (the string view is expected to remain valid for the duration of the call). The logging framework respects the threshold and formatting set by the application; the function itself does not modify the message or add extra structure.

## Usage Patterns

- Called by `LogProxy::operator()` to dispatch logging
- Used for conditional logging with level filtering

## Called By

- method `clore::logging::LogProxy::operator()`

