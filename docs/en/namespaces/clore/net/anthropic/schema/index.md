---
title: 'Namespace clore::net::anthropic::schema'
description: 'The clore::net::anthropic::schema namespace provides type‑safe, C++ abstractions for constructing and representing schema objects required by the Anthropic API. It exposes the function template function_tool, which accepts a tool’s name and description (both std::string) and returns an int status code, embodying the logic to produce a conformant function‑tool schema. The related function response_format returns an integer format identifier for a given type T, used to select the response structure expected by the API. Declarations such as name and description serve as placeholder variables or schematic constants within this namespace.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::schema`

## Summary

The `clore::net::anthropic::schema` namespace provides type‑safe, C++ abstractions for constructing and representing schema objects required by the Anthropic API. It exposes the function template `function_tool`, which accepts a tool’s name and description (both `std::string`) and returns an `int` status code, embodying the logic to produce a conformant function‑tool schema. The related function `response_format` returns an integer format identifier for a given type `T`, used to select the response structure expected by the API. Declarations such as `name` and `description` serve as placeholder variables or schematic constants within this namespace.

Architecturally, this namespace centralises schema construction and validation, insulating the rest of the codebase from direct API‑schema serialisation. It enforces invariants (e.g., non‑empty, unique tool names) and relies on template parameters to support varied tool‑parameter shapes and response formats, thereby maintaining extensibility while keeping the schema‑building logic cohesive and testable.

## Functions

### `clore::net::anthropic::schema::function_tool`

Declaration: `network/anthropic.cppm:762`

Definition: `network/anthropic.cppm:762`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function template `clore::net::anthropic::schema::function_tool` accepts two `std::string` arguments that specify the tool’s name and description, and returns an `int` status code. Its caller-facing responsibility is to construct a function tool schema object that conforms to the Anthropic API specification, allowing the model to invoke the tool. The caller must guarantee that the name is non‑empty and unique among the tools in the request, and that the description provides clear context for when the tool should be used. The template parameter `T` designates the type of the tool’s parameter schema; callers are responsible for providing a concrete type that matches the expected structure of the tool’s inputs.

#### Usage Patterns

- Creating a function tool definition for Anthropic schema
- Delegating to generic function tool creator

### `clore::net::anthropic::schema::response_format`

Declaration: `network/anthropic.cppm:757`

Definition: `network/anthropic.cppm:757`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::schema::response_format` returns an integer identifying the response format for the specified type `T`. This format code is used by the Anthropic API to determine how responses are structured. The caller may invoke it without arguments; the template parameter `T` selects the format variant.

#### Usage Patterns

- Callers use this function to obtain the response format configuration for the template type `T` when interacting with the Anthropic API.

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

