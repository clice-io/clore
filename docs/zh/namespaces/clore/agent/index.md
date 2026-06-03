---
title: 'Namespace clore::agent'
description: 'clore::agent 命名空间封装了基于工具调用的代理循环的主体逻辑，负责协调代码库探索并生成指南文档。其同步入口 run_agent 启动一个循环，通过多次工具调用完成探索，并在指定输出路径下产出文档；异步变体 run_agent_async 则集成 kota::event_loop，允许调用者将代理任务调度到事件循环上执行。辅助函数 build_tool_definitions 构建工具注册表，extract_string_arg 和 dispatch_tool_call 分别负责安全提取工具参数和按标识符分发工具调用，将协议细节从调用者处抽象出来。'
layout: doc
template: doc
---

# Namespace `clore::agent`

## Summary

`clore::agent` 命名空间封装了基于工具调用的代理循环的主体逻辑，负责协调代码库探索并生成指南文档。其同步入口 `run_agent` 启动一个循环，通过多次工具调用完成探索，并在指定输出路径下产出文档；异步变体 `run_agent_async` 则集成 `kota::event_loop`，允许调用者将代理任务调度到事件循环上执行。辅助函数 `build_tool_definitions` 构建工具注册表，`extract_string_arg` 和 `dispatch_tool_call` 分别负责安全提取工具参数和按标识符分发工具调用，将协议细节从调用者处抽象出来。

在架构上，该命名空间充当代理系统的执行与编排核心，同步与异步两种 API 分别适配阻塞与非阻塞场景。它定义了 `AgentError` 和 `ToolError` 错误类型，用于区分代理循环级别的失败与单次工具调用的失败，调用者应根据返回值或预期结果进行错误处理。变量如 `arguments`、`output_root` 等作为配置上下文在各函数间传递，确保工具调用环境的一致性。

## Types

### `clore::agent::AgentError`

Declaration: `src/agent/agent.cppm:35`

Definition: `src/agent/agent.cppm:35`

Implementation: [`Module agent`](../../../modules/agent/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The `message` member holds an arbitrary string describing the error.
- The struct is default-constructible and copyable (implicitly).
- No additional error codes or metadata are stored.

#### Key Members

- `message` (`std::string`) – stores the error description.

#### Usage Patterns

- Used to report failures in agent operations, such as during initialization or execution.
- Can be returned or thrown as an error value in function results.
- May be caught or inspected to retrieve the error string.

### `clore::agent::ToolError`

Declaration: `src/agent/tools.cppm:28`

Definition: `src/agent/tools.cppm:28`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` 不为空时表示发生了具体错误
- 类型仅包含一个字符串成员，无额外状态

#### Key Members

- `clore::agent::ToolError::message` 存储错误描述

#### Usage Patterns

- 作为工具执行失败时的返回类型或异常抛出对象
- 可通过读取 `message` 获取错误详情

## Functions

### `clore::agent::build_tool_definitions`

Declaration: `src/agent/tools.cppm:35`

Definition: `src/agent/tools.cppm:899`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

函数 `clore::agent::build_tool_definitions` 构造调用者可用的工具定义集合。它不接受任何参数，返回一个 `int` 值以表示操作结果——通常，非零值指示构建过程中发生了错误。调用此函数是使用任何工具调用或分发功能（如 `clore::agent::dispatch_tool_call`）的前提；在调用依赖于工具注册表的函数之前应确保该函数成功执行。

#### Usage Patterns

- Called to obtain tool definitions for an AI agent interaction

### `clore::agent::dispatch_tool_call`

Declaration: `src/agent/tools.cppm:38`

Definition: `src/agent/tools.cppm:914`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

`clore::agent::dispatch_tool_call` 将工具分派及执行的责任从调用者处抽象出来。调用者提供工具标识符（第一个 `std::string_view` 参数）、工具参数（`const json::Value &`）以及额外的上下文信息（三个后续参数：`const int &` 和两个 `std::string_view`）。函数根据标识符和参数执行相应的工具操作，并返回一个 `std::expected<std::string, ToolError>`；成功时包含工具的执行结果字符串，失败时则携带具体的 `ToolError` 错误信息。调用者必须确保所有输入参数有效且满足各工具所要求的契约。

#### Usage Patterns

- used by `run_agent` or `run_agent_async` to execute a tool call
- called with tool name and arguments extracted from LLM response
- return result string or error

### `clore::agent::extract_string_arg`

Declaration: `src/agent/tools.cppm:32`

Definition: `src/agent/tools.cppm:877`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

函数 `clore::agent::extract_string_arg` 负责从给定的 JSON 值中安全地提取一个字符串参数。调用者需提供一个 `const json::Value &` 和一个 `std::string_view` 作为参数名称；函数据此尝试获取对应字符串值。成功时返回 `std::expected<std::string, ToolError>` 包含提取的字符串；若参数缺失、类型不匹配或无法转换，则返回 `ToolError` 指示错误原因。它作为工具参数解析的基础操作，确保调用方在后续使用前获得合法的字符串值，避免未定义行为。

#### Usage Patterns

- 在 `dispatch_tool_call` 中用于从工具调用参数中提取字符串字段

### `clore::agent::run_agent`

Declaration: `src/agent/agent.cppm:41`

Definition: `src/agent/agent.cppm:538`

Implementation: [`Module agent`](../../../modules/agent/index.md)

`clore::agent::run_agent` 启动一个代理循环，该循环通过多次工具调用（tool calls）探索代码库，并在 `${output_root}/guides/` 下生成指南文档。调用者需提供配置参数：两个 `const int &`（可能分别表示最大步数或递归深度等限制）、一个 `std::string_view`（可能表示输出根路径）以及一个 `std::string`（可能表示目标代码库路径或额外标识）。函数返回 `std::expected<std::size_t, AgentError>`；成功时返回生成的文档数量（例如指南文件的计数），失败时返回 `AgentError` 错误对象，表示循环执行中发生的关键错误。调用者应检查返回值以确认操作成功并获取结果，或在错误情况下传播或处理 `AgentError`。

#### Usage Patterns

- used as synchronous entry point to run agent loop
- called with a `TaskConfig`, `ProjectModel`, LLM model name, and output root directory

### `clore::agent::run_agent_async`

Declaration: `src/agent/agent.cppm:48`

Definition: `src/agent/agent.cppm:521`

Implementation: [`Module agent`](../../../modules/agent/index.md)

The function `clore::agent::run_agent_async` initiates an asynchronous agent loop that runs on the provided `kota::event_loop`. The caller must schedule the returned value — an opaque task handle of type `int` — on that loop and then execute it; the agent loop will not progress otherwise. The first two `int` parameters and the two `std::string` parameters configure the agent; refer to the function’s documentation for their specific roles.

#### Usage Patterns

- 调度返回的任务在 `kota::event_loop` 上执行
- 作为异步 agent 循环的启动点

## Related Pages

- [Namespace clore](../index.md)

