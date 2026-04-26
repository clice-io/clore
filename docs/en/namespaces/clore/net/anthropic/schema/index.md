---
title: 'Namespace clore::net::anthropic::schema'
description: 'The clore::net::anthropic::schema namespace encapsulates the data structures and functions that define the schema for interacting with the Anthropic API. It provides variables such as name and description, which likely serve as field identifiers or constants used in schema construction. The primary functional elements are the template functions function_tool and response_format. function_tool accepts two string arguments—intended as a tool’s name and its description—and returns an integer representing the outcome of registering or validating the tool, with the template parameter T enabling type‑safe input parameters. response_format returns an integer that designates the expected response format, relying on the template type argument T to determine the format identifier. Architecturally, this namespace forms a type‑safe schema layer for building Anthropic API tool definitions and response format specifications, isolating schema‑related logic from other networking and client code within the clore::net::anthropic hierarchy.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::schema`

## Summary

The `clore::net::anthropic::schema` namespace encapsulates the data structures and functions that define the schema for interacting with the Anthropic API. It provides variables such as `name` and `description`, which likely serve as field identifiers or constants used in schema construction. The primary functional elements are the template functions `function_tool` and `response_format`. `function_tool` accepts two string arguments—intended as a tool’s name and its description—and returns an integer representing the outcome of registering or validating the tool, with the template parameter `T` enabling type‑safe input parameters. `response_format` returns an integer that designates the expected response format, relying on the template type argument `T` to determine the format identifier. Architecturally, this namespace forms a type‑safe schema layer for building Anthropic API tool definitions and response format specifications, isolating schema‑related logic from other networking and client code within the `clore::net::anthropic` hierarchy.

## Functions

### `clore::net::anthropic::schema::function_tool`

Declaration: `network/anthropic.cppm:755`

Definition: `network/anthropic.cppm:755`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The function template `clore::net::anthropic::schema::function_tool` defines a callable tool within the Anthropic API schema. It expects two string arguments—typically the tool’s name and a description of its purpose—and returns an integer that represents the result of registering or validating the tool. The template parameter `T` allows the caller to specify the type of the tool’s input parameters schema, enabling type‑safe construction of the tool definition. Callers must supply both string values and a suitable `T` according to the schema contract; the returned integer indicates success or failure of the tool definition.

#### Usage Patterns

- Used to construct a `FunctionToolDefinition` for a specific type `T`
- Called with name and description strings to create a tool definition for the Anthropic API

### `clore::net::anthropic::schema::response_format`

Declaration: `network/anthropic.cppm:750`

Definition: `network/anthropic.cppm:750`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

The `clore::net::anthropic::schema::response_format` template function returns an integer value that designates the expected format of an Anthropic API response. Callers rely on this function to obtain an opaque format identifier when constructing or validating schema objects; the precise meaning of the returned integer is determined by the template type argument `T` and the associated schema definitions. This function provides no arguments and requires no precondition beyond the availability of a valid template type.

#### Usage Patterns

- Obtain a response format for the specified type T in the Anthropic context

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

