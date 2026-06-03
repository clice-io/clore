---
title: 'Namespace clore::net::anthropic::schema'
description: '该命名空间 clore::net::anthropic::schema 负责封装与 Anthropic 模型交互所需的模式定义和工具函数。其中，function_tool 模板函数用于将调用方提供的函数签名包装为模型可识别的工具描述，它接受工具名称和描述字符串并返回一个整型标识符，供后续消息引用或结果匹配；response_format 模板函数则用于查询或配置当前请求的响应格式，返回操作状态码或格式标识符。此外，命名空间内还包含 name 和 description 等变量，通常作为配置参数或返回结果的一部分。整体上，该命名空间在 clore::net::anthropic 层中承担着序列化与模式验证的职责，是连接高层调用与底层 Anthropic REST API 格式的桥梁。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::schema`

## Summary

该命名空间 `clore::net::anthropic::schema` 负责封装与 Anthropic 模型交互所需的模式定义和工具函数。其中，`function_tool` 模板函数用于将调用方提供的函数签名包装为模型可识别的工具描述，它接受工具名称和描述字符串并返回一个整型标识符，供后续消息引用或结果匹配；`response_format` 模板函数则用于查询或配置当前请求的响应格式，返回操作状态码或格式标识符。此外，命名空间内还包含 `name` 和 `description` 等变量，通常作为配置参数或返回结果的一部分。整体上，该命名空间在 `clore::net::anthropic` 层中承担着序列化与模式验证的职责，是连接高层调用与底层 Anthropic REST API 格式的桥梁。

## Functions

### `clore::net::anthropic::schema::function_tool`

Declaration: `src/network/anthropic.cppm:771`

Definition: `src/network/anthropic.cppm:771`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

该函数是一个模板重载，用于将指定的函数签名封装为 Anthropic 模型可识别的 `function_tool`。它接受两个字符串参数——通常对应工具的名称和描述——并返回一个整型标识符，供后续消息引用或结果匹配。调用方负责提供有效的函数名称与描述字符串；返回值的语义由模板参数 `T` 决定，不应假设为特定常量。此函数仅在 `clore::net::anthropic::schema` 命名空间内可用，且不抛出异常。

#### Usage Patterns

- Create a `FunctionToolDefinition` for Anthropic API schema
- Used to define tool call specifications for the Anthropic model

### `clore::net::anthropic::schema::response_format`

Declaration: `src/network/anthropic.cppm:766`

Definition: `src/network/anthropic.cppm:766`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::schema::response_format` 是一个模板函数，用于查询或配置当前请求的响应格式。调用者必须提供类型 `T` 来指定期望的格式类别。该函数返回 `int`，其值表示操作结果（例如，成功或失败的状态码）或当前格式的标识符。在调用此函数之前，应确保已正确初始化相关的网络和会话上下文。

#### Usage Patterns

- 用于获取特定类型 `T` 的响应格式，例如在构建 Anthropic 请求时配置输出格式。
- 作为 `clore::net::anthropic::schema` 命名空间下的便捷包装，隐藏底层 schema 实现细节。

## Related Pages

- [Namespace clore::net::anthropic](../index.md)

