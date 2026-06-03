---
title: 'Namespace clore::net::anthropic::schema'
description: 'The clore::net::anthropic::schema namespace defines the core schema primitives for interacting with the Anthropic API. It provides type‑safe function templates such as function_tool, which registers a callable tool with a name and description and returns an integer identifier, and response_format, which returns an integer representing a compile‑time selected response format. The variables name and description are also part of this namespace, serving as the textual metadata for tool definitions.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::schema`

## Summary

The `clore::net::anthropic::schema` namespace defines the core schema primitives for interacting with the Anthropic API. It provides type‑safe function templates such as `function_tool`, which registers a callable tool with a name and description and returns an integer identifier, and `response_format`, which returns an integer representing a compile‑time selected response format. The variables `name` and `description` are also part of this namespace, serving as the textual metadata for tool definitions.

Architecturally, this namespace encapsulates the schema layer of the Anthropic client library, separating the definitions of API‑facing structures (tools and response formats) from the networking and serialization logic. By using template parameters, it allows callers to specialize behaviors at compile time while returning simple integer codes that can be used in subsequent API requests.

## Functions

### `clore::net::anthropic::schema::function_tool`

Declaration: `src/network/anthropic.cppm:771`

Definition: `src/network/anthropic.cppm:771`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The `clore::net::anthropic::schema::function_tool` function template creates a definition of a callable tool for use with the Anthropic API schema. It accepts two string arguments representing the tool’s name and description, and returns an integer that identifies the registered tool. The template parameter `T` allows the caller to specify the concrete type of the function that the tool will invoke. The caller is responsible for providing a valid name and description; the returned integer can be used in subsequent API interactions to reference this tool definition.

#### Usage Patterns

- Used to create tool definitions for a specific type T in the Anthropic provider schema

### `clore::net::anthropic::schema::response_format`

Declaration: `src/network/anthropic.cppm:766`

Definition: `src/network/anthropic.cppm:766`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function template `clore::net::anthropic::schema::response_format` returns an integer value that identifies a response format used with the Anthropic API. It takes no parameters; the template parameter `T` may specialize the behavior for distinct format types at compile time. Callers can rely on this function to obtain a numeric code representing the format that should be applied when processing or interpreting API responses.

#### Usage Patterns

- Used to obtain the response format for a specific type when interacting with the Anthropic API.
- Typically called as `response_format<SomeType>()`.

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

