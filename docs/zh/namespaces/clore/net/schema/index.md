---
title: 'Namespace clore::net::schema'
description: 'clore::net::schema 命名空间负责网络层中模式定义与标准化操作，主要提供响应格式和工具函数的抽象接口。其中，response_format 是一个模板函数，用于获取与特定类型关联的响应格式标识符；function_tool 则基于输入的字符串参数建立工具模式契约并返回执行结果。该命名空间在架构上充当网络模式交互的桥梁，支撑上层协议中格式选择与工具调用的统一处理。'
layout: doc
template: doc
---

# Namespace `clore::net::schema`

## Summary

`clore::net::schema` 命名空间负责网络层中模式定义与标准化操作，主要提供响应格式和工具函数的抽象接口。其中，`response_format` 是一个模板函数，用于获取与特定类型关联的响应格式标识符；`function_tool` 则基于输入的字符串参数建立工具模式契约并返回执行结果。该命名空间在架构上充当网络模式交互的桥梁，支撑上层协议中格式选择与工具调用的统一处理。

## Functions

### `clore::net::schema::function_tool`

Declaration: `network/schema.cppm:520`

Definition: `network/schema.cppm:584`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

函数模板 `clore::net::schema::function_tool` 接受两个 `std::string` 参数并返回 `int`。它在调用者与某个基于工具的模式之间建立契约，根据提供的字符串输入计算并返回一个整型结果。返回的 `int` 表示操作的成功状态或特定数值结果，具体含义由调用上下文确定。与同作用域内的 `clore::net::schema::response_format` 函数不同，`function_tool` 专注于工具相关的模式解析或执行。

#### Usage Patterns

- 用于自动生成 LLM 函数调用的工具定义
- 调用者传入函数名称、描述和可反射的类型来定义工具模式

### `clore::net::schema::response_format`

Declaration: `network/schema.cppm:517`

Definition: `network/schema.cppm:561`

Implementation: [`Module schema`](../../../../modules/schema/index.md)

函数 `clore::net::schema::response_format` 是一个模板函数，用于获取与给定类型 `T` 相关联的响应格式的整数标识符。调用者需要为 `T` 提供一个有效的模式类型，该函数返回一个整数值，通常对应某种预定义的响应格式枚举或常量。该函数是公开接口的一部分，旨在让调用方在构建网络模式时指定或查询所需的响应格式。

#### Usage Patterns

- 在需要为可反射类型自动生成 LLM 结构化输出模式时调用
- 结合 `clore::net::schema::function_tool` 等函数使用，为工具调用提供模式

## Related Pages

- [Namespace clore::net](../index.md)

