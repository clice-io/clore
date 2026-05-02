---
title: 'Namespace clore::net::anthropic::schema'
description: '命名空间 clore::net::anthropic::schema 负责定义与 Anthropic API 交互所需的数据结构和辅助函数，专注于模式层面的抽象。其显著声明包括模板函数 function_tool（用于配置函数工具，接受两个字符串参数并返回整数状态码）和 response_format（用于获取响应格式标识符，返回整数），以及 name 和 description 等变量。在架构上，该命名空间充当 API 请求构建与响应对接的中间层，通过模板支持多场景扩展，将底层的模式细节封装起来，为上层调用提供一致的类型接口。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::schema`

## Summary

命名空间 `clore::net::anthropic::schema` 负责定义与 Anthropic API 交互所需的数据结构和辅助函数，专注于模式层面的抽象。其显著声明包括模板函数 `function_tool`（用于配置函数工具，接受两个字符串参数并返回整数状态码）和 `response_format`（用于获取响应格式标识符，返回整数），以及 `name` 和 `description` 等变量。在架构上，该命名空间充当 API 请求构建与响应对接的中间层，通过模板支持多场景扩展，将底层的模式细节封装起来，为上层调用提供一致的类型接口。

## Functions

### `clore::net::anthropic::schema::function_tool`

Declaration: `network/anthropic.cppm:762`

Definition: `network/anthropic.cppm:762`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::schema::function_tool` 是一个模板函数，接受两个 `std::string` 参数并返回 `int`。调用方需提供两个字符串参数，并根据返回的 `int` 值判断调用是否成功。模板参数 `T` 用于指定函数工具的类型或上下文，具体行为取决于该模板参数。

#### Usage Patterns

- convenience wrapper for creating tool definitions
- templated on type T to specify tool schema
- called with a name and description for the tool

### `clore::net::anthropic::schema::response_format`

Declaration: `network/anthropic.cppm:757`

Definition: `network/anthropic.cppm:757`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::schema::response_format` 是一个模板函数，返回一个 `int` 值，表示与当前调用上下文关联的响应格式标识符。调用者可以调用此函数（不传递参数）以获取整数代码，该代码用于配置或解析 Anthropic API 的响应格式。模板参数 `T` 允许该函数在多种场景下使用，但返回值本身不直接依赖 `T` 的具体类型。

#### Usage Patterns

- 在 Anthropic 命名空间下获取响应格式配置

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

