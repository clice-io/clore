---
title: 'Namespace clore::net::anthropic::schema'
description: '命名空间 clore::net::anthropic::schema 负责定义与 Anthropic API 交互所涉及的架构元素。它提供了两个核心函数模板：function_tool 用于向 API 注册可被模型调用的工具，需要提供工具名称和描述，并通过模板参数指定输入架构；response_format 则用于获取与特定预定义类型关联的响应格式标识符。此外，该命名空间还包含 name 和 description 变量，这些元素共同构成了定义工具调用和响应格式约定的结构化基础设施，在请求构造与响应解析中起到关键的契约作用。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::schema`

## Summary

命名空间 `clore::net::anthropic::schema` 负责定义与 Anthropic API 交互所涉及的架构元素。它提供了两个核心函数模板：`function_tool` 用于向 API 注册可被模型调用的工具，需要提供工具名称和描述，并通过模板参数指定输入架构；`response_format` 则用于获取与特定预定义类型关联的响应格式标识符。此外，该命名空间还包含 `name` 和 `description` 变量，这些元素共同构成了定义工具调用和响应格式约定的结构化基础设施，在请求构造与响应解析中起到关键的契约作用。

## Functions

### `clore::net::anthropic::schema::function_tool`

Declaration: `network/anthropic.cppm:755`

Definition: `network/anthropic.cppm:755`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::schema::function_tool` 是一个模板函数，用于向 Anthropic API 注册一个可被模型调用的函数工具。调用者需提供工具的名称和描述（均为 `std::string` 类型），并通过模板参数 `T` 指明该工具的输入架构（如表示函数参数的结构体类型）。函数返回一个 `int` 值，表示注册是否成功（例如非零值表示错误）。此函数是定义工具调用契约的关键入口，确保模型能够根据提供的名称和描述正确调用函数，并使用 `T` 所描述的结构化参数。

#### Usage Patterns

- Constructing a `FunctionToolDefinition` for tool use in Anthropic API calls

### `clore::net::anthropic::schema::response_format`

Declaration: `network/anthropic.cppm:750`

Definition: `network/anthropic.cppm:750`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数模板 `clore::net::anthropic::schema::response_format` 接受一个模板参数 `T` 并返回一个 `int`，该整数表示与 `T` 相关联的响应格式标识符。调用者需要保证 `T` 是库中预定义的可用于指定响应格式的类型；函数本身不接受任何运行时参数。返回的整数值可在后续操作（如设置请求选项或解析响应）中用于选择或标识对应的格式。

#### Usage Patterns

- Obtaining Anthropic API response format
- Configuration for API calls

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

