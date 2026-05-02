---
title: 'Namespace clore::agent'
description: 'The clore::agent namespace encapsulates the core agent functionality for exploring a codebase and generating guide documents. It provides the agent loop (run_agent and its asynchronous variant run_agent_async), tool dispatch (dispatch_tool_call), tool definition registration (build_tool_definitions), and utility functions for extracting arguments from JSON (extract_string_arg). Error handling is represented by the ToolError and AgentError structures.'
layout: doc
template: doc
---

# Namespace `clore::agent`

## Summary

The `clore::agent` namespace encapsulates the core agent functionality for exploring a codebase and generating guide documents. It provides the agent loop (`run_agent` and its asynchronous variant `run_agent_async`), tool dispatch (`dispatch_tool_call`), tool definition registration (`build_tool_definitions`), and utility functions for extracting arguments from JSON (`extract_string_arg`). Error handling is represented by the `ToolError` and `AgentError` structures.

Architecturally, this namespace serves as the central controller for agent‑driven analysis, managing the lifecycle of the agent session, the set of available tools, and the interactions between the language model and the codebase. It relies on an event loop (from the `kota` library) for asynchronous execution and writes its results to a specified output root directory.

## Types

### `clore::agent::AgentError`

Declaration: `agent/agent.cppm:21`

Definition: `agent/agent.cppm:21`

Implementation: [`Module agent`](../../../modules/agent/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The `message` member always contains a valid `std::string` object
- The `message` string may be empty

#### Key Members

- `std::string message` — the error description

#### Usage Patterns

- Created with a descriptive string when an error occurs in agent operations
- Likely used as a member of a `std::expected` or thrown as an exception

### `clore::agent::ToolError`

Declaration: `agent/tools.cppm:16`

Definition: `agent/tools.cppm:16`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- holds an error description in `message`
- aggregate-style struct with public data

#### Key Members

- `std::string message` field carrying the error text

#### Usage Patterns

- constructed to convey tool-related error details within `clore::agent`

## Functions

### `clore::agent::build_tool_definitions`

Declaration: `agent/tools.cppm:23`

Definition: `agent/tools.cppm:887`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The function `clore::agent::build_tool_definitions` constructs and registers the tool definitions that the agent can dispatch. It returns an `int` indicating the outcome of the build process, typically a success code or a count of definitions created. Callers must invoke this function before calling `clore::agent::dispatch_tool_call` or any other tool-dependent operation to ensure that the set of available tools is fully populated.

#### Usage Patterns

- Called during agent initialization to obtain tool definitions for network interactions

### `clore::agent::dispatch_tool_call`

Declaration: `agent/tools.cppm:26`

Definition: `agent/tools.cppm:902`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

The function `clore::agent::dispatch_tool_call` accepts a tool name as a `std::string_view`, tool arguments as a `const json::Value &`, a session identifier as a `const int &`, and two additional `std::string_view` parameters that provide tool call identification. It returns a `std::expected<std::string, ToolError>` containing either the tool’s output on success or a `ToolError` on failure. The caller is responsible for supplying a valid tool name that can be resolved within the session context, along with correctly structured arguments and identifiers; the function does not manage session lifecycle or tool registration.

#### Usage Patterns

- called by agent execution functions like `run_agent_async` or `run_agent`
- used to invoke named tools with automatic caching of successful results

### `clore::agent::extract_string_arg`

Declaration: `agent/tools.cppm:20`

Definition: `agent/tools.cppm:865`

Implementation: [`Module agent:tools`](../../../modules/agent/tools.md)

`clore::agent::extract_string_arg` retrieves a string value from a JSON object by a given key. The caller passes a `json::Value` object (expected to represent a JSON object) and a `std::string_view` naming the key. If the key exists and its associated value is a JSON string, the function returns that string wrapped in a `std::expected<std::string, ToolError>`. Otherwise it returns a `ToolError` indicating why extraction failed—for example, the key is missing or the value is not a string. The function never modifies the input JSON object.

#### Usage Patterns

- Called by `clore::agent::dispatch_tool_call` to extract a required string argument from a tool call's JSON parameters
- Used within the tool‑call dispatch flow to parse individual fields of the argument object

### `clore::agent::run_agent`

Declaration: `agent/agent.cppm:27`

Definition: `agent/agent.cppm:524`

Implementation: [`Module agent`](../../../modules/agent/index.md)

The `clore::agent::run_agent` function runs the agent loop, which explores the codebase via tool calls and generates guide documents under the output root directory. The caller provides two integer references, a string view, and a string; the function returns an `std::expected<std::size_t, AgentError>` indicating the number of guides produced or an error if the loop fails.

#### Usage Patterns

- Invoked as the synchronous top-level driver to execute the agent loop
- Callers branch on the returned `std::expected` to handle `AgentError` or consume the produced guide count

### `clore::agent::run_agent_async`

Declaration: `agent/agent.cppm:34`

Definition: `agent/agent.cppm:507`

Implementation: [`Module agent`](../../../modules/agent/index.md)

The function `clore::agent::run_agent_async` launches an asynchronous agent loop that operates on the provided `kota::event_loop`. The caller must schedule the returned `int` (interpreted as a task handle) on that event loop and run it to drive the agent. It accepts two `const int &` parameters (typically identifiers for the agent and conversation), two `std::string` parameters (likely a system prompt and user input), and a reference to a `kota::event_loop`. The asynchronous variant mirrors the contract of `clore::agent::run_agent` but is designed for non‑blocking execution in an event‑driven context; the caller owns the responsibility of properly integrating the returned task with the loop’s lifecycle.

#### Usage Patterns

- Called to start an asynchronous agent session with caching
- Callers schedule the returned task on the provided `kota::event_loop`
- Used in higher-level agent orchestration code

## Related Pages

- [Namespace clore](../index.md)

