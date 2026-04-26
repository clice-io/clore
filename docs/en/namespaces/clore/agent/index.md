---
title: 'Namespace clore::agent'
description: 'The clore::agent namespace encapsulates the core agent logic for a codebase exploration and documentation generation tool. It provides functions to build and register tool definitions (build_tool_definitions), dispatch tool calls (dispatch_tool_call), extract string arguments from tool-call JSON (extract_string_arg), and run the main agent loop both synchronously (run_agent) and asynchronously (run_agent_async). The namespace also defines error types (ToolError and AgentError) to communicate failures during tool execution or overall agent operation.'
layout: doc
template: doc
---

# Namespace `clore::agent`

## Summary

The `clore::agent` namespace encapsulates the core agent logic for a codebase exploration and documentation generation tool. It provides functions to build and register tool definitions (`build_tool_definitions`), dispatch tool calls (`dispatch_tool_call`), extract string arguments from tool-call JSON (`extract_string_arg`), and run the main agent loop both synchronously (`run_agent`) and asynchronously (`run_agent_async`). The namespace also defines error types (`ToolError` and `AgentError`) to communicate failures during tool execution or overall agent operation.

Architecturally, `clore::agent` acts as the orchestrator of an LLM‑driven agent that issues tool calls to analyze a codebase. It manages an internal tool registry, a result cache for tool invocations, and configuration variables such as `output_root` (for generated guides), `model` (the LLM backend), and `config` (operational parameters). The agent loop iterates over tool call results, producing structured documents under the output root directory. This namespace serves as the primary interface for initiating and controlling the agent’s workflow within the larger `clore` system.

## Types

### `clore::agent::AgentError`

Declaration: `agent/agent.cppm:21`

Definition: `agent/agent.cppm:21`

Implementation: [`Module agent`](../../../modules/agent/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- message should be a valid, non-empty string when describing an error

#### Key Members

- message

#### Usage Patterns

- used as a return type or thrown as an exception to indicate agent-related errors

### `clore::agent::ToolError`

Declaration: `agent/tools.cppm:16`

Definition: `agent/tools.cppm:16`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The `clore::agent::ToolError` struct represents an error type specifically for tool-related operations within the agent framework. It is used to capture and communicate failures that occur during the execution or handling of tools, and it complements the more general `clore::agent::AgentError` struct.

#### Invariants

- No explicit invariants beyond the presence of a message string.

#### Key Members

- `std::string message`

#### Usage Patterns

- Returned or thrown by tool-related functions to indicate failure with a descriptive message.

## Functions

### `clore::agent::build_tool_definitions`

Declaration: `agent/tools.cppm:23`

Definition: `agent/tools.cppm:887`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The function `clore::agent::build_tool_definitions` constructs and registers the complete set of tool definitions used by the agent. It takes no arguments and returns an `int` indicating success (zero) or failure (a non‑zero error code). Callers must invoke this function during agent initialization to populate the internal tool registry; after a successful call, the agent can dispatch tool calls via `clore::agent::dispatch_tool_call` and execute tool‑based workflows with `clore::agent::run_agent` or `clore::agent::run_agent_async`.

#### Usage Patterns

- Obtain tool definitions for sending to a language model
- Called by agent initialization or tool call dispatch logic

### `clore::agent::dispatch_tool_call`

Declaration: `agent/tools.cppm:26`

Definition: `agent/tools.cppm:902`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The function `clore::agent::dispatch_tool_call` is the caller-facing dispatcher for executing a tool identified by name. It accepts the tool name as a `std::string_view`, the tool’s arguments as a `const json::Value &`, an integer (likely a request or sequence identifier), and two additional `std::string_view` parameters (typically representing a session or invocation context). It returns a `std::expected<std::string, ToolError>`, either yielding the tool’s output as a string or reporting a `ToolError` on failure.

The caller is responsible for ensuring that the tool name corresponds to a valid, built tool definition and that the arguments are well-formed JSON. The function may internally interact with a per-session result cache (`tool_result_cache`) to avoid redundant execution. The contract expects the caller to handle both the success value and the error case appropriately.

#### Usage Patterns

- called to handle tool calls in agent execution loops
- used by functions such as `run_agent_async` and `run_agent` to process tool dispatch requests

### `clore::agent::extract_string_arg`

Declaration: `agent/tools.cppm:20`

Definition: `agent/tools.cppm:865`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The function `clore::agent::extract_string_arg` retrieves a string-valued argument from a JSON object based on the argument name provided as `std::string_view`. It serves as a validation and extraction helper for tool-call handling, ensuring the requested argument exists and is of the expected type. On success it returns the extracted string; on failure it returns a `ToolError` indicating the specific error (e.g., missing key or type mismatch). The caller must supply a valid `json::Value` (typically the arguments object of a tool call) and the name of the argument to extract.

#### Usage Patterns

- used by `dispatch_tool_call` to extract string arguments from tool call argument objects

### `clore::agent::run_agent`

Declaration: `agent/agent.cppm:27`

Definition: `agent/agent.cppm:524`

Implementation: [`Module agent`](../../../modules/agent/index.md)

The `clore::agent::run_agent` function executes the core agent loop. It explores the codebase by issuing tool calls and, on success, produces guide documents under the configured output root directory. Callers supply four arguments — two `const int &` values, a `std::string_view`, and a `std::string` — which collectively define the agent’s operational parameters (e.g., concurrency limits, output path). The function returns a `std::expected<std::size_t, AgentError>`: on success it yields a count (likely the number of guides generated), and on failure it provides a descriptive `AgentError`. The function is synchronous and does not throw exceptions; callers must handle the expected result to determine success or failure.

#### Usage Patterns

- synchronous entry point for running the agent
- callers use it to execute the agent loop and obtain a result
- typically called with a configuration, project model, LLM model name, and output directory

### `clore::agent::run_agent_async`

Declaration: `agent/agent.cppm:34`

Definition: `agent/agent.cppm:507`

Implementation: [`Module agent`](../../../modules/agent/index.md)

The function `clore::agent::run_agent_async` initiates an asynchronous agent loop that runs on a caller-provided `kota::event_loop`. It accepts two `const int &` parameters, two `std::string` parameters, and a `kota::event_loop &`. The function returns a task representing the loop; callers must schedule this task on the supplied event loop and run it. The caller is responsible for ensuring the event loop remains active while the task executes.

#### Usage Patterns

- callers must schedule the returned task on the event loop and run it
- used to start an asynchronous agent loop with cache loading

## Related Pages

- [Namespace clore](../index.md)

