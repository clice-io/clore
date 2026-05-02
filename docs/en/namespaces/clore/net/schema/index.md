---
title: 'Namespace clore::net::schema'
description: 'The clore::net::schema namespace encapsulates the core schema definitions and operations for the networking layer. It declares metadata fields such as name, description, and schema, which are used to describe entities and their expected structures. Key template functions include response_format, which returns an integer identifier representing the expected response format for a given type (driving schema-level interpretation), and function_tool, which accepts two string arguments and returns an integer status or outcome, enabling tool-based interactions within the schema framework. Architecturally, this namespace provides a typed, declarative layer that defines how network messages are formatted, interpreted, and acted upon, serving as the contract between the networking infrastructure and higher-level application logic.'
layout: doc
template: doc
---

# Namespace `clore::net::schema`

## Summary

The `clore::net::schema` namespace encapsulates the core schema definitions and operations for the networking layer. It declares metadata fields such as `name`, `description`, and `schema`, which are used to describe entities and their expected structures. Key template functions include `response_format`, which returns an integer identifier representing the expected response format for a given type (driving schema-level interpretation), and `function_tool`, which accepts two string arguments and returns an integer status or outcome, enabling tool-based interactions within the schema framework. Architecturally, this namespace provides a typed, declarative layer that defines how network messages are formatted, interpreted, and acted upon, serving as the contract between the networking infrastructure and higher-level application logic.

## Functions

### `clore::net::schema::function_tool`

Declaration: `network/schema.cppm:520`

Definition: `network/schema.cppm:584`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

The function `clore::net::schema::function_tool` is a public template that accepts two `std::string` arguments and returns an `int`. It provides a tool mechanism within the network schema layer, allowing callers to pass string inputs and receive an integer result that typically represents a status code, identifier, or other numeric outcome.

Callers supply the two string parameters according to the schema’s contract; the function expects valid, non‑empty strings unless otherwise documented. The return value indicates success or failure and may be used in conjunction with related schema functions such as `clore::net::schema::response_format`.

#### Usage Patterns

- Instantiated with a reflectable type to generate a tool definition for an LLM function call

### `clore::net::schema::response_format`

Declaration: `network/schema.cppm:517`

Definition: `network/schema.cppm:561`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

The template function `clore::net::schema::response_format` returns an integer identifier that describes the expected response format for a given type `T`. Callers supply the type as an explicit template argument and receive a value that can be used to drive schema-level decisions about how responses of that type are interpreted or handled. The function itself takes no runtime parameters and has no side effects.

#### Usage Patterns

- used to configure structured output for LLM calls
- obtains a `ResponseFormat` for a reflectable type

## Related Pages

- [Namespace clore::net](../index.md)

