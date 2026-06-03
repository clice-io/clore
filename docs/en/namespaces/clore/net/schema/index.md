---
title: 'Namespace clore::net::schema'
description: 'The clore::net::schema namespace provides the core definitions for network schema components, serving as the public API for tool and response format identification. It declares two primary template functions: function_tool, which accepts a tool name and schema descriptor (both std::string) and returns an integer for tool identification and validation, and response_format, which returns an integer representing the response format indicator for a given schema type. These functions are designed to be instantiated with valid schema types, producing integer results used for validation or format identification within the network layer. The namespace also declares related variables (name, description, schema) that likely serve as input or output for these schema definitions, though their specific roles are not fully detailed.'
layout: doc
template: doc
---

# Namespace `clore::net::schema`

## Summary

The `clore::net::schema` namespace provides the core definitions for network schema components, serving as the public API for tool and response format identification. It declares two primary template functions: `function_tool`, which accepts a tool name and schema descriptor (both `std::string`) and returns an integer for tool identification and validation, and `response_format`, which returns an integer representing the response format indicator for a given schema type. These functions are designed to be instantiated with valid schema types, producing integer results used for validation or format identification within the network layer. The namespace also declares related variables (`name`, `description`, `schema`) that likely serve as input or output for these schema definitions, though their specific roles are not fully detailed.

## Functions

### `clore::net::schema::function_tool`

Declaration: `src/network/schema.cppm:530`

Definition: `src/network/schema.cppm:594`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

The `clore::net::schema::function_tool` template function is part of the public API for network schema definitions. It accepts two `std::string` arguments and returns an `int`. Its contract specifies that for a given pair of function name and schema descriptor, the function produces a corresponding integer result, typically used for tool identification or validation. The template parameter `T` allows customization of the tool type.

#### Usage Patterns

- used to generate `OpenAI`-compatible function tool definitions
- requires a reflectable class type as template argument

### `clore::net::schema::response_format`

Declaration: `src/network/schema.cppm:527`

Definition: `src/network/schema.cppm:571`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

The template function `response_format` accepts a type parameter `T` and returns an integer value that identifies the response format associated with `T`. Callers use this function to obtain the format indicator for a schema type within the network layer. The function must be instantiated with a type that is a valid schema component; otherwise the result is unspecified.

#### Usage Patterns

- Used to obtain a `ResponseFormat` for LLM structured output from a reflectable type
- Called in contexts where automatic JSON schema generation for response validation is needed

## Related Pages

- [Namespace clore::net](../index.md)

