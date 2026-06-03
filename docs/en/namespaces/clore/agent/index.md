---
title: 'Namespace clore::agent'
description: 'The clore::agent namespace encapsulates the primary agent loop and tool‑execution infrastructure for a code‑exploration and documentation‑generation system. It defines the core lifecycle functions run_agent and run_agent_async, which orchestrate a cycle of LLM‑driven tool calls and produce output artefacts (e.g., guide documents) under a designated output root. Supporting utilities include build_tool_definitions to initialise the set of available tools, extract_string_arg for safe retrieval of JSON‑encoded string parameters, and dispatch_tool_call to invoke a named tool with validated arguments. The namespace declares two error types (AgentError for loop‑level failures and ToolError for per‑call extraction or execution errors) and manages key state variables such as project_root, output_root, model, config, and cache. Architecturally, clore::agent is the orchestrator that bridges an external LLM interface with a tool‑based workspace; it coordinates the asynchronous or synchronous iteration, argument handling, and result accumulation that drive the agent’s behaviour.'
layout: doc
template: doc
---

# Namespace `clore::agent`

## Summary

The `clore::agent` namespace encapsulates the primary agent loop and tool‑execution infrastructure for a code‑exploration and documentation‑generation system. It defines the core lifecycle functions `run_agent` and `run_agent_async`, which orchestrate a cycle of LLM‑driven tool calls and produce output artefacts (e.g., guide documents) under a designated output root. Supporting utilities include `build_tool_definitions` to initialise the set of available tools, `extract_string_arg` for safe retrieval of JSON‑encoded string parameters, and `dispatch_tool_call` to invoke a named tool with validated arguments. The namespace declares two error types (`AgentError` for loop‑level failures and `ToolError` for per‑call extraction or execution errors) and manages key state variables such as `project_root`, `output_root`, `model`, `config`, and `cache`. Architecturally, `clore::agent` is the orchestrator that bridges an external LLM interface with a tool‑based workspace; it coordinates the asynchronous or synchronous iteration, argument handling, and result accumulation that drive the agent’s behaviour.

## Types

### `clore::agent::AgentError`

Declaration: `src/agent/agent.cppm:35`

Definition: `src/agent/agent.cppm:35`

Implementation: [`Module agent`](../../../modules/agent/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` is a valid `std::string` object

#### Key Members

- `message`

#### Usage Patterns

- Returned from functions to indicate an error condition
- Stores a human-readable error description

### `clore::agent::ToolError`

Declaration: `src/agent/tools.cppm:28`

Definition: `src/agent/tools.cppm:28`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The `message` string is not guaranteed to be non-empty.

#### Key Members

- `message`: stores the error description as a `std::string`.

#### Usage Patterns

- Used to convey tool-level errors in the agent's error handling flow.
- Likely returned or thrown when a tool operation fails.

## Functions

### `clore::agent::build_tool_definitions`

Declaration: `src/agent/tools.cppm:35`

Definition: `src/agent/tools.cppm:899`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The function `clore::agent::build_tool_definitions` constructs and returns an integer that represents the tool definitions available to the agent. It is designed to be called before any tool dispatch or extraction operations; callers should treat it as a prerequisite step that establishes the tool definition environment. The returned `int` indicates success or a count of definitions, and a non-negative value signals that the tool definitions are ready for use by functions such as `clore::agent::dispatch_tool_call` or `clore::agent::extract_string_arg`.

#### Usage Patterns

- called to collect all tool definitions for agent use
- used during initialization of `run_agent` and similar orchestration functions

### `clore::agent::dispatch_tool_call`

Declaration: `src/agent/tools.cppm:38`

Definition: `src/agent/tools.cppm:914`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The `clore::agent::dispatch_tool_call` function is the public interface for executing a tool by name. It accepts a tool name, its arguments as a JSON value, a context identifier as a `const int &`, a conversation identifier, and an additional string view. On success it returns a `std::string` containing the tool’s output; on failure it returns a `ToolError` indicating the reason. Callers must supply a valid tool name and well‑formed JSON arguments matching the tool’s expected schema; otherwise an error is reported. The returned string is not guaranteed to be stable across calls and the function does not provide transactional isolation for side effects.

#### Usage Patterns

- Used to dispatch tool calls with caching
- Called from agent execution loops like `run_agent` or `run_agent_async`

### `clore::agent::extract_string_arg`

Declaration: `src/agent/tools.cppm:32`

Definition: `src/agent/tools.cppm:877`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The function `clore::agent::extract_string_arg` is responsible for safely retrieving a string value identified by a given property name from a `json::Value` object. The caller provides a reference to the JSON object and a `std::string_view` naming the expected property. The function returns a `std::expected<std::string, ToolError>`, either yielding the extracted string or a `ToolError` if the property is missing or its value is not a string. Callers should use this function when they need to obtain a required string argument from a structured JSON input and handle extraction failures explicitly via the expected result.

#### Usage Patterns

- Extract required string field from tool call arguments

### `clore::agent::run_agent`

Declaration: `src/agent/agent.cppm:41`

Definition: `src/agent/agent.cppm:538`

Implementation: [`Module agent`](../../../modules/agent/index.md)

The function `clore::agent::run_agent` executes the agent loop, which explores the codebase by making tool calls and produces guide documents under the designated output root directory. Callers supply two integer references (typically representing agent limits or configuration), a `std::string_view` for the output root path, and a `std::string` for additional parameters. On success it returns a `std::expected<std::size_t, AgentError>` containing a count (e.g., number of produced documents), or an `AgentError` if the loop fails.

#### Usage Patterns

- called to start the agent from a synchronous context
- used as a wrapper around the asynchronous `run_agent_async`

### `clore::agent::run_agent_async`

Declaration: `src/agent/agent.cppm:48`

Definition: `src/agent/agent.cppm:521`

Implementation: [`Module agent`](../../../modules/agent/index.md)

The function `clore::agent::run_agent_async` initiates an asynchronous agent loop on the provided `kota::event_loop`. It accepts two integer references, two `std::string` arguments, and the event loop. The function returns a task handle that the caller must schedule on the loop and then run. This contract ensures the agent operates non‑blockingly within the given event‑driven context.

#### Usage Patterns

- Callers schedule the returned task on the event loop
- Used to orchestrate the asynchronous agent workflow

## Related Pages

- [Namespace clore](../index.md)

