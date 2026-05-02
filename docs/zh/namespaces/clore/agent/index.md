---
title: 'Namespace clore::agent'
description: 'clore::agent 命名空间承担着智能代理系统的核心职责，提供了一套用于定义、注册和调用工具（tool）以及驱动代理循环执行的完整机制。它包含了多个关键函数：dispatch_tool_call 根据工具名称和 JSON 参数执行对应的工具调用并返回结果或错误；run_agent 和 run_agent_async 分别启动同步和异步的代理循环，其中同步版本会在指定输出根目录下生成指南文档；build_tool_definitions 汇总当前注册的工具规格并返回工具数量；extract_string_arg 则用于从工具调用的参数对象中安全地提取字符串参数。此外，命名空间中还定义了 ToolError 和 AgentError 两个错误类型，用于细化工具调用和代理执行过程中的错误处理。'
layout: doc
template: doc
---

# Namespace `clore::agent`

## Summary

`clore::agent` 命名空间承担着智能代理系统的核心职责，提供了一套用于定义、注册和调用工具（tool）以及驱动代理循环执行的完整机制。它包含了多个关键函数：`dispatch_tool_call` 根据工具名称和 JSON 参数执行对应的工具调用并返回结果或错误；`run_agent` 和 `run_agent_async` 分别启动同步和异步的代理循环，其中同步版本会在指定输出根目录下生成指南文档；`build_tool_definitions` 汇总当前注册的工具规格并返回工具数量；`extract_string_arg` 则用于从工具调用的参数对象中安全地提取字符串参数。此外，命名空间中还定义了 `ToolError` 和 `AgentError` 两个错误类型，用于细化工具调用和代理执行过程中的错误处理。

在架构上，`clore::agent` 充当了代理功能模块的命名空间层级组织者，它将工具调度、参数解析、代理生命周期管理等职责集中在一起，使外部调用者能够通过统一的接口与代理系统交互。该命名空间的函数共同构成了一个可扩展的工具调用框架，并支撑起自动化的代码库探索与文档生成流程，是 `clore` 项目中实现智能代理行为的关键组件。

## Types

### `clore::agent::AgentError`

Declaration: `agent/agent.cppm:21`

Definition: `agent/agent.cppm:21`

Implementation: [`Module agent`](../../../modules/agent/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 错误消息由调用方提供，无预设约束

#### Key Members

- `message`：存储错误描述字符串

#### Usage Patterns

- 作为返回值或异常的一部分传递错误信息

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

该函数负责构建并返回一个表示当前 Agent 工具集定义的数值状态。调用方调用 `clore::agent::build_tool_definitions` 后，可以获取到在内部 `tool_registry` 中注册的工具规格的汇总信息，通常用于验证、初始化或报告可用工具的数量。函数的返回值是一个整数，成功时代表已登记的工具数量，失败或不适用时可能返回其他约定值。调用方应假设该函数会访问全局或模块级的工具注册表，并在执行前确保所需工具已正确声明。

#### Usage Patterns

- Called to generate a complete set of tool definitions for network requests
- Used to prepare tool definitions before dispatching agent calls

### `clore::agent::dispatch_tool_call`

Declaration: `agent/tools.cppm:26`

Definition: `agent/tools.cppm:902`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

`clore::agent::dispatch_tool_call` 接受工具名称（`std::string_view`）、工具参数（`const json::Value &`）、一个整数标识符（`const int &`）以及两个附加上下文字符串（`std::string_view`），并尝试执行对应的工具调用。调用者有责任提供合法的工具名称和与工具定义一致的参数，以及正确的上下文数据。该函数返回 `std::expected<std::string, ToolError>`，成功时包含工具执行的结果字符串，失败时携带 `ToolError` 错误。调用者必须检查返回值以确定调用是否成功，并对 `ToolError` 进行适当的错误处理。

#### Usage Patterns

- invoked during agent execution to handle a tool call from an LLM
- used with caching to avoid duplicate tool executions with identical arguments

### `clore::agent::extract_string_arg`

Declaration: `agent/tools.cppm:20`

Definition: `agent/tools.cppm:865`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

函数 `clore::agent::extract_string_arg` 从给定的 `json::Value` 对象中提取指定名称的字符串参数。调用者传入一个 JSON 值（通常为工具调用的参数对象）和一个参数名（`std::string_view`）。若参数存在且为字符串，则返回该字符串；若参数缺失、类型不匹配或发生其他可恢复错误，则返回 `std::expected` 的错误状态，其错误类型为 `ToolError`。此函数是工具调用解包过程中可靠提取字符串参数的关键契约，调用者必须负责检查返回值或传播错误。

#### Usage Patterns

- extract string field from tool call arguments
- validate and retrieve string-typed JSON field
- used in `dispatch_tool_call` to parse tool arguments

### `clore::agent::run_agent`

Declaration: `agent/agent.cppm:27`

Definition: `agent/agent.cppm:524`

Implementation: [`Module agent`](../../../modules/agent/index.md)

`clore::agent::run_agent` 启动代理循环：它通过工具调用探索代码库，并在 `${output_root}/guides/` 下生成指南文档。该函数接受两个 `const int &`（通常表示配置或限制）、一个 `std::string_view`（可能指定输出根路径）和一个 `std::string`（其他参数），返回 `std::expected<std::size_t, AgentError>`。成功时产生一个无符号整数（例如已完成的指南数量），失败时携带 `AgentError` 错误信息。调用者应确保输入参数有效，并处理可能的错误结果。

#### Usage Patterns

- Entry point for synchronous agent execution
- Used to produce guide documents from codebase analysis

### `clore::agent::run_agent_async`

Declaration: `agent/agent.cppm:34`

Definition: `agent/agent.cppm:507`

Implementation: [`Module agent`](../../../modules/agent/index.md)

`clore::agent::run_agent_async` 启动一个异步代理循环，该循环运行在提供的 `kota::event_loop` 上。它接受两个 `const int &` 参数（通常代表上下文标识符）、两个 `std::string` 参数以及一个 `kota::event_loop &` 引用。函数返回一个 `int`，代表异步任务标识符；调用方必须将此返回的任务调度到事件循环上并运行它，以驱动代理循环的执行。

#### Usage Patterns

- callers schedule the returned `kota::task` on the provided `kota::event_loop`
- used as the entry point for starting an asynchronous agent execution with cache management

## Related Pages

- [Namespace clore](../index.md)

