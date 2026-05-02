---
title: 'Namespace clore::net::anthropic::protocol'
description: 'clore::net::anthropic::protocol 命名空间封装了与 Anthropic 消息 API 进行交互的底层协议逻辑。它负责将上层请求转换为符合 API 规范的 JSON 格式（通过 build_request_json），构造正确端点地址（build_messages_url），并解析 API 返回的原始响应。该命名空间提供了从响应中提取文本内容（parse_response_text、text_from_response）和处理工具调用结果（parse_tool_arguments、append_tool_outputs）的函数，同时暴露了协议相关的常量变量（如 model_value、stop_reason_value）以支持配置和验证。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol`

## Summary

`clore::net::anthropic::protocol` 命名空间封装了与 Anthropic 消息 API 进行交互的底层协议逻辑。它负责将上层请求转换为符合 API 规范的 JSON 格式（通过 `build_request_json`），构造正确端点地址（`build_messages_url`），并解析 API 返回的原始响应。该命名空间提供了从响应中提取文本内容（`parse_response_text`、`text_from_response`）和处理工具调用结果（`parse_tool_arguments`、`append_tool_outputs`）的函数，同时暴露了协议相关的常量变量（如 `model_value`、`stop_reason_value`）以支持配置和验证。

作为 `clore::net::anthropic` 库的核心协议层，该命名空间将上游调用与 Anthropic 的 REST 接口细节隔离，确保数据格式和状态码的正确性。其函数通常返回整数句柄或状态码，由上层模块（如 `clore::net::anthropic::detail::Protocol`）调度使用，从而构建起从请求构建到响应解析的完整通信管线。

## Subnamespaces

- [`clore::net::anthropic::protocol::detail`](detail/index.md)

## Functions

### `clore::net::anthropic::protocol::append_tool_outputs`

Declaration: `network/anthropic.cppm:209`

Definition: `network/anthropic.cppm:628`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

调用方使用 `append_tool_outputs` 将一个工具调用的输出追加到当前正在构建的协议消息中。该函数接受三个参数，分别代表工具调用标识符、输出内容以及一个可选的索引或状态值，并在操作完成后返回一个 `int` 值以指示结果（例如成功或失败，或更新后的元素计数）。契约要求：调用方必须在调用前确保工具调用的标识符已在消息上下文中注册，且提供的输出内容格式符合 Anthropic API 规范；返回值的语义由上层调用约定定义，调用方应根据此返回值判断后续处理流程。

#### Usage Patterns

- Used to incorporate tool call results into the message history
- Called after parsing a response that contains tool use blocks

### `clore::net::anthropic::protocol::build_messages_url`

Declaration: `network/anthropic.cppm:201`

Definition: `network/anthropic.cppm:224`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/build-messages-url.md)

此函数负责构建与 Anthropic Messages API 交互所用的目标 URL。它接受一个 `std::string_view` 参数，通常代表模型标识符或资源路径，并返回一个 `std::string` 类型的完整 URL。调用者应将此 URL 直接用于 HTTP 请求的构造。

该函数是 `clore::net::anthropic::protocol` 命名空间的一部分，由更上层的 `clore::net::anthropic::detail::Protocol::build_url` 方法调用。作为合约，调用方只需确保传入的字符串视图指向有效的标识符；函数保证返回一个格式正确、可用于 Messages API 的端点地址。返回的 URL 不包含查询参数或认证信息，这些应由调用方在后续步骤中自行添加。

#### Usage Patterns

- 被 `clore::net::anthropic::detail::Protocol::build_url` 调用

### `clore::net::anthropic::protocol::build_request_json`

Declaration: `network/anthropic.cppm:203`

Definition: `network/anthropic.cppm:235`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::build_request_json` 接受一个 `const int &` 参数并返回一个 `int`。调用者应提供与该协议相关的整数引用（可能表示请求标识或参数），该函数将据此构建请求 JSON。返回值指示操作结果，通常用于成功或错误报告。

#### Usage Patterns

- Called by higher‑level network code to prepare the Anthropic request body
- Consumed as the payload for HTTP POST requests to the Anthropic Messages API

### `clore::net::anthropic::protocol::parse_response`

Declaration: `network/anthropic.cppm:205`

Definition: `network/anthropic.cppm:460`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::parse_response` 接受一个表示原始响应的 `std::string_view`，解析后返回一个整数句柄。调用者应保留该句柄，并传递给同命名空间下的其他函数（如 `text_from_response`、`parse_response_text` 等）来提取响应中的特定数据。返回值为负表示解析失败，调用者无需关心内部细节。

#### Usage Patterns

- After receiving an Anthropic API response body
- To convert raw JSON to `CompletionResponse`

### `clore::net::anthropic::protocol::parse_response_text`

Declaration: `network/anthropic.cppm:215`

Definition: `network/anthropic.cppm:636`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::parse_response_text` 是一个函数模板，它解析给定的响应对象并返回一个整数，该整数代表从该响应中提取的文本内容。调用者负责提供有效的响应引用；返回的整数值应根据程序中该类型上下文的具体约定进行解释。

#### Usage Patterns

- Used to deserialize the text portion of an Anthropic API response into a specific type.

### `clore::net::anthropic::protocol::parse_tool_arguments`

Declaration: `network/anthropic.cppm:218`

Definition: `network/anthropic.cppm:641`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

此函数解析来自工具使用响应的参数，将原始参数数据转换为目标类型 `T`。调用者应确保提供的整数引用表示有效的工具参数负载；函数返回一个整数状态码，指示解析是否成功。模板参数 `T` 必须满足协议指定的可反序列化要求，否则行为未定义。

#### Usage Patterns

- Delegates to `clore::net::protocol::parse_tool_arguments`
- Parses tool arguments into type `T`

### `clore::net::anthropic::protocol::text_from_response`

Declaration: `network/anthropic.cppm:207`

Definition: `network/anthropic.cppm:623`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::text_from_response` 从给定的 API 响应中提取文本内容。它接受一个响应对象的常量引用（`const int &`），返回一个整数，该整数表示提取的结果（例如状态码或指向文本的句柄）。调用者必须确保传入的响应引用已由 `parse_response` 或类似函数正确解析，且包含有效的文本有效载荷。返回的非零值可能指示错误或需要进一步处理的特殊条件。

#### Usage Patterns

- Called by higher-level Anthropic protocol code when a completion response is received
- Used to obtain the textual content of a response for further processing

## Related Pages

- [Namespace clore::net::anthropic](../index.md)
- [Namespace clore::net::anthropic::protocol::detail](detail/index.md)

