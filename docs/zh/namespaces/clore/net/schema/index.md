---
title: 'Namespace clore::net::schema'
description: 'clore::net::schema 命名空间专注于网络通信模式（schema）的定义与辅助。其中声明了两个核心模板函数：function_tool 接收两个字符串参数并返回整数，用于在网络模式的上下文中注册或处理工具函数；response_format 根据模板类型参数返回对应的响应格式标识符，帮助调用方查询该类型在模式中预期的响应格式。'
layout: doc
template: doc
---

# Namespace `clore::net::schema`

## Summary

`clore::net::schema` 命名空间专注于网络通信模式（schema）的定义与辅助。其中声明了两个核心模板函数：`function_tool` 接收两个字符串参数并返回整数，用于在网络模式的上下文中注册或处理工具函数；`response_format` 根据模板类型参数返回对应的响应格式标识符，帮助调用方查询该类型在模式中预期的响应格式。

该命名空间作为网络模式组件的基础设施，通过类型安全的接口抽象了工具注册与响应格式匹配的逻辑，简化了上层对模式元素的配置与操作。内部的变量（如 `description`、`schema`、`name`）暗示其同时管理着模式对象的元数据。整体而言，`clore::net::schema` 为网络通信模式的构建和运行提供了关键的通用原语。

## Functions

### `clore::net::schema::function_tool`

Declaration: `src/network/schema.cppm:530`

Definition: `src/network/schema.cppm:594`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

`clore::net::schema::function_tool` 是一个模板函数，它接受两个 `std::string` 参数并返回 `int`。该函数通常被用于在网络 schema 上下文中处理或注册一个工具函数，其返回值可能表示操作的结果状态（例如成功或错误码）。调用者应确保提供的字符串参数符合预期的格式和语义，因为模板参数 `T` 可能对参数的类型或行为施加额外的要求。

#### Usage Patterns

- 使用可反射类自动生成 LLM 工具函数定义
- 与 LLM 工具调用 API 集成

### `clore::net::schema::response_format`

Declaration: `src/network/schema.cppm:527`

Definition: `src/network/schema.cppm:571`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

函数模板 `clore::net::schema::response_format` 接受一个模板类型参数 `T`，返回一个整数，表示与类型 `T` 对应的响应格式标识符。调用方负责提供具体的 `T`，该函数会返回一个代表该类型在 schema 中预期的响应格式的整数值，该值可用于后续的模式匹配或序列化配置。

#### Usage Patterns

- Used to create a response format for structured LLM output
- Typically called with a reflectable class type in LLM tool or schema generation

## Related Pages

- [Namespace clore::net](../index.md)

