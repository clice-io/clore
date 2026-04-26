---
title: 'Namespace clore::net::schema'
description: 'The clore::net::schema namespace provides the core definitions and functions for specifying network‑level schemas and tool registrations. It declares fundamental variables such as name, description, and schema, which are used to describe individual schema elements. Two key function templates are exposed: response_format<T>(), which returns an integer identifier that selects the response encoding for a caller‑provided type T; and function_tool<T>(std::string, std::string), which registers or retrieves a callable tool within the schema, returning an integer success indicator or handle. Together, these declarations enable type‑safe integration of callable functions and response formatting into the network layer, forming the schema foundation for structured communication.'
layout: doc
template: doc
---

# Namespace `clore::net::schema`

## Summary

The `clore::net::schema` namespace provides the core definitions and functions for specifying network‑level schemas and tool registrations. It declares fundamental variables such as `name`, `description`, and `schema`, which are used to describe individual schema elements. Two key function templates are exposed: `response_format<T>()`, which returns an integer identifier that selects the response encoding for a caller‑provided type `T`; and `function_tool<T>(std::string, std::string)`, which registers or retrieves a callable tool within the schema, returning an integer success indicator or handle. Together, these declarations enable type‑safe integration of callable functions and response formatting into the network layer, forming the schema foundation for structured communication.

## Functions

### `clore::net::schema::function_tool`

Declaration: `network/schema.cppm:520`

Definition: `network/schema.cppm:584`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

The function template `clore::net::schema::function_tool` accepts two `std::string` arguments, typically representing the name and description of the tool, and returns an `int` that serves as a success indicator or a handle for subsequent operations. The caller is responsible for providing valid strings; the function registers or retrieves a function tool within the schema, making it available for use in network calls or tool-calling protocols. The template parameter `T` specifies the callable type or signature that the tool wraps, enabling type‑safe integration with the schema system.

#### Usage Patterns

- used to create function tool definitions for LLM calls with automatic schema generation
- typically called with a reflectable type as the template argument

### `clore::net::schema::response_format`

Declaration: `network/schema.cppm:517`

Definition: `network/schema.cppm:561`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

The template function `clore::net::schema::response_format<T>()` returns an integer identifier that represents the response format for the type `T`. It is the caller’s responsibility to provide a type `T` that is recognized within the schema system; the returned value is used to select the corresponding network-level response encoding.

#### Usage Patterns

- used to obtain a structured output schema for LLM API calls requiring a `ResponseFormat`

## Related Pages

- [Namespace clore::net](../index.md)

