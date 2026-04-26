---
title: 'Namespace clore::net::anthropic::protocol'
description: 'clore::net::anthropic::protocol 命名空间负责实现与 Anthropic 消息 API 交互所需的协议层功能。它封装了请求和响应的序列化与反序列化逻辑，提供了一系列用于构建请求 URL、生成 JSON 请求体、解析响应文本及工具参数、追加工具输出结果的核心函数（如 build_messages_url、build_request_json、parse_response、parse_response_text、parse_tool_arguments、append_tool_outputs）。此外，该命名空间还声明了协议相关的常量与数据模型变量（如 stop_reason_value、model_value、tool_choice、tools 等），用于统一管理协议字段的取值和结构。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol`

## Summary

`clore::net::anthropic::protocol` 命名空间负责实现与 Anthropic 消息 API 交互所需的协议层功能。它封装了请求和响应的序列化与反序列化逻辑，提供了一系列用于构建请求 URL、生成 JSON 请求体、解析响应文本及工具参数、追加工具输出结果的核心函数（如 `build_messages_url`、`build_request_json`、`parse_response`、`parse_response_text`、`parse_tool_arguments`、`append_tool_outputs`）。此外，该命名空间还声明了协议相关的常量与数据模型变量（如 `stop_reason_value`、`model_value`、`tool_choice`、`tools` 等），用于统一管理协议字段的取值和结构。

在整体架构中，`clore::net::anthropic::protocol` 扮演着适配层的角色，位于底层网络通信和上层业务逻辑之间。它将原始的网络响应字符串转换为结构化的整数句柄或状态码，供上层模块（如 `clore::net::anthropic::detail::Protocol`）进一步使用，从而解耦了协议细节与应用逻辑。调用者通过该命名空间提供的函数即可完成与 Anthropic 协议相关的核心操作，而无需直接处理 JSON 或 HTTP 细节。

## Subnamespaces

- [`clore::net::anthropic::protocol::detail`](detail/index.md)

## Functions

### `clore::net::anthropic::protocol::append_tool_outputs`

Declaration: `network/anthropic.cppm:209`

Definition: `network/anthropic.cppm:628`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::append_tool_outputs` 用于将工具调用的输出结果追加到当前正在构造的协议消息中。它接收工具输出的必要数据，并按照 Anthropic 协议格式将其集成到消息结构中。调用者应当在构建需要包含工具结果的请求时调用此函数，确保所有工具输出在发送前被正确附加。该函数的返回值指示操作是否成功，通常为 `int` 类型的状态码。

#### Usage Patterns

- 将工具输出合并到消息历史中
- 在 Anthropic 协议处理中作为工具结果处理的封装

### `clore::net::anthropic::protocol::build_messages_url`

Declaration: `network/anthropic.cppm:201`

Definition: `network/anthropic.cppm:224`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/build-messages-url.md)

函数 `clore::net::anthropic::protocol::build_messages_url` 接受一个 `std::string_view` 参数并返回一个 `std::string` 表示的 URL。此函数负责根据提供的输入构造指向 Anthropic 消息 API 的完整端点地址。调用者必须确保传入有效的参数（例如资源标识符或请求上下文），以生成正确的 URL。生成的 URL 供上层组件（如 `clore::net::anthropic::detail::Protocol::build_url`）进一步使用。

#### Usage Patterns

- Called by `clore::net::anthropic::detail::Protocol::build_url` to produce the messages endpoint URL

### `clore::net::anthropic::protocol::build_request_json`

Declaration: `network/anthropic.cppm:203`

Definition: `network/anthropic.cppm:235`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::build_request_json` 接受一个 `const int &` 参数，返回一个 `int` 值。调用者应提供一个有效的整数引用，该函数将依据该引用构建对应的请求 JSON 并返回一个表示操作结果的整数。函数不直接修改传入的参数，调用方负责保证参数在调用期间有效。

#### Usage Patterns

- Called to create the JSON payload for an Anthropic messages API request
- Typically used before sending the HTTP request

### `clore::net::anthropic::protocol::parse_response`

Declaration: `network/anthropic.cppm:205`

Definition: `network/anthropic.cppm:460`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::parse_response` 接受一个 `std::string_view` 形式的原始响应数据，并将其解析为一个整数结果。调用者应提供有效且完整的响应内容。该函数返回的整数值代表解析后的状态或提取的数值，具体含义由协议定义的行为确定，调用者须依据该返回值进行后续流程的决策。

#### Usage Patterns

- Parsing raw JSON response from Anthropic API
- Extracting `CompletionResponse` from the JSON payload
- Error handling for malformed or error responses

### `clore::net::anthropic::protocol::parse_response_text`

Declaration: `network/anthropic.cppm:215`

Definition: `network/anthropic.cppm:636`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::parse_response_text` 是一个模板函数，负责从给定的响应对象中提取文本内容。它接受一个 `const` 引用参数（类型由模板参数 `T` 决定），并返回一个表示解析后文本的 `int` 值。该函数为调用者提供了与具体响应表示无关的文本获取接口，其行为假设传入的响应对象已按协议格式正确构建。

#### Usage Patterns

- Used by callers needing to parse the text content of an Anthropic API completion response into a specific type.

### `clore::net::anthropic::protocol::parse_tool_arguments`

Declaration: `network/anthropic.cppm:218`

Definition: `network/anthropic.cppm:641`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`parse_tool_arguments` 是一个模板函数，它接受一个类型为 `const int &` 的参数并返回 `int`。调用者负责传入一个有效的整数引用，该引用通常来自诸如 `parse_response` 或其他解析函数的输出，表示已解析的响应数据。该函数解析并提取工具参数，返回一个整数句柄，供后续如 `append_tool_outputs` 等函数使用。返回的句柄必须与整个工具调用生命周期一致，调用者应确保在不再需要时释放或重置该句柄。

#### Usage Patterns

- used to extract typed arguments from an Anthropic tool call

### `clore::net::anthropic::protocol::text_from_response`

Declaration: `network/anthropic.cppm:207`

Definition: `network/anthropic.cppm:623`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::text_from_response` 接受一个响应对象（`const int &`）并返回一个整数形式的文本数据。调用者负责提供有效的响应，此函数假定响应已按照协议格式正确构造。返回的整数代表从响应中提取的文本内容。

#### Usage Patterns

- Extracting text from a `CompletionResponse`

## Related Pages

- [Namespace clore::net::anthropic](../index.md)
- [Namespace clore::net::anthropic::protocol::detail](detail/index.md)

