---
title: 'Namespace clore::agent'
description: 'clore::agent 命名空间封装了代理系统的核心机制，负责驱动基于工具调用的自动化代码探索与文档生成流程。其显著声明包括高层入口函数 run_agent 和 run_agent_async，前者同步运行代理循环并返回生成的文档数量（成功时以 std::size_t 表示，失败时以 AgentError 表示），后者在 kota::event_loop 上启动异步操作。此外，build_tool_definitions 提供工具定义集合的初始化入口，dispatch_tool_call 是工具调度的核心函数，依据工具名称和参数执行并返回结果或 ToolError，而 extract_string_arg 则负责从 JSON 参数中安全提取字符串值。ToolError 和 AgentError 结构体用于表达工具调用或代理运行过程中的特定错误。'
layout: doc
template: doc
---

# Namespace `clore::agent`

## Summary

`clore::agent` 命名空间封装了代理系统的核心机制，负责驱动基于工具调用的自动化代码探索与文档生成流程。其显著声明包括高层入口函数 `run_agent` 和 `run_agent_async`，前者同步运行代理循环并返回生成的文档数量（成功时以 `std::size_t` 表示，失败时以 `AgentError` 表示），后者在 `kota::event_loop` 上启动异步操作。此外，`build_tool_definitions` 提供工具定义集合的初始化入口，`dispatch_tool_call` 是工具调度的核心函数，依据工具名称和参数执行并返回结果或 `ToolError`，而 `extract_string_arg` 则负责从 JSON 参数中安全提取字符串值。`ToolError` 和 `AgentError` 结构体用于表达工具调用或代理运行过程中的特定错误。

该命名空间在架构中扮演代理系统的中枢角色，协调 LLM 模型、工具补全缓存、输出根目录及会话状态等组件。其内部的变量（如 `model`、`arguments`、`output_root`、`config`、`cache` 等）反映了代理运行时的关键上下文。通过 `dispatch_tool_call` 和 `build_tool_definitions` 等函数，它实现了工具调度层与代理控制逻辑的解耦，使上层应用能够专注于任务定义和结果处理，而无需直接操作底层工具执行细节。

## Types

### `clore::agent::AgentError`

Declaration: `agent/agent.cppm:21`

Definition: `agent/agent.cppm:21`

Implementation: [`Module agent`](../../../modules/agent/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- message should contain a non-empty string describing the error context

#### Key Members

- message

#### Usage Patterns

- returned as an error type from agent operations
- used to propagate error information via exceptions or result types

### `clore::agent::ToolError`

Declaration: `agent/tools.cppm:16`

Definition: `agent/tools.cppm:16`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

Insufficient evidence to summarize; provide more EVIDENCE.

## Functions

### `clore::agent::build_tool_definitions`

Declaration: `agent/tools.cppm:23`

Definition: `agent/tools.cppm:887`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

`clore::agent::build_tool_definitions` 为调用者提供构建工具定义集合的入口点。该函数不接收任何参数，返回一个 `int` 值，用于指示操作结果或定义的数量。调用者应在需要初始化或更新工具定义列表时调用此函数；返回值的具体含义（例如成功状态码或定义计数）由调用者根据契约进行解释。

#### Usage Patterns

- called to prepare tool definitions for a chat API request
- used in agent initialization

### `clore::agent::dispatch_tool_call`

Declaration: `agent/tools.cppm:26`

Definition: `agent/tools.cppm:902`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

`clore::agent::dispatch_tool_call` 是工具调度系统的核心入口，负责根据指定的工具名称和参数执行工具调用。调用者需提供工具名称、参数 JSON 值、会话标识符以及客户端和请求标识符。函数返回 `std::expected<std::string, ToolError>`，成功时包含工具执行的字符串结果，失败时包含详细的错误信息。调用者应始终检查返回值，并在出错时采取适当措施（例如向用户报告错误）。该函数不会直接修改会话状态，但执行结果可能影响后续代理步骤。

#### Usage Patterns

- Called to handle tool execution requests
- Used by agent loop to dispatch tool calls

### `clore::agent::extract_string_arg`

Declaration: `agent/tools.cppm:20`

Definition: `agent/tools.cppm:865`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

从给定的 `json::Value` 对象中提取由名称标识的字符串参数。如果名称对应的参数不存在或其值不是字符串类型，则返回一个表示失败的 `ToolError`。该函数是工具调用参数提取的核心契约，确保调用方获得一个经过验证的字符串值或明确的错误原因。

#### Usage Patterns

- Called by `dispatch_tool_call` to extract required string arguments from tool invocation JSON

### `clore::agent::run_agent`

Declaration: `agent/agent.cppm:27`

Definition: `agent/agent.cppm:524`

Implementation: [`Module agent`](../../../modules/agent/index.md)

函数 `clore::agent::run_agent` 负责运行代理循环：通过工具调用探索代码库，并在输出根目录下的 `guides/` 夹中生成指南文档。调用者需提供两个整数引用（`const int &`）、一个字符串视图（`std::string_view`，通常指向输出根目录路径）及一个字符串（`std::string`）。成功时该函数返回生成的文档数量（`std::size_t`），失败时返回 `AgentError`。调用者应检查 `std::expected` 的返回状态以处理潜在错误。

#### Usage Patterns

- Called to execute the full agent workflow synchronously from user-facing entry points
- Wraps the asynchronous `run_agent_async` in an event loop for blocking invocation

### `clore::agent::run_agent_async`

Declaration: `agent/agent.cppm:34`

Definition: `agent/agent.cppm:507`

Implementation: [`Module agent`](../../../modules/agent/index.md)

`clore::agent::run_agent_async` 启动一个异步代理循环，在给定的 `kota::event_loop` 上运行。调用者必须将返回的任务调度到该循环上并执行它。该函数接受两个 `const int &` 参数、两个 `std::string` 参数以及一个 `kota::event_loop &` 引用，返回一个 `int` 值，代表异步操作的结果或状态。

#### Usage Patterns

- Launch asynchronous agent with event loop
- Wrap synchronous agent with cache loading

## Related Pages

- [Namespace clore](../index.md)

