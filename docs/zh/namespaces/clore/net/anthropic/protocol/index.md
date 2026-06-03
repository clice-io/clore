---
title: 'Namespace clore::net::anthropic::protocol'
description: '命名空间 clore::net::anthropic::protocol 封装了与 Anthropic API 交互的协议层逻辑。它负责构建消息 API 的完整 URL（如 build_messages_url）、生成请求的 JSON 载荷（build_request_json），以及解析 API 返回的响应数据（parse_response）并提取其中包含的文本内容（text_from_response、parse_response_text）、工具调用参数（parse_tool_arguments），并支持追加工具输出到当前消息（append_tool_outputs）。该命名空间还包含多个与请求、响应、历史、状态、工具选择等协议相关的内部变量，构成了协议数据模型的骨架，为上层模块提供统一、可复用的交互接口。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol`

## Summary

命名空间 `clore::net::anthropic::protocol` 封装了与 Anthropic API 交互的协议层逻辑。它负责构建消息 API 的完整 URL（如 `build_messages_url`）、生成请求的 JSON 载荷（`build_request_json`），以及解析 API 返回的响应数据（`parse_response`）并提取其中包含的文本内容（`text_from_response`、`parse_response_text`）、工具调用参数（`parse_tool_arguments`），并支持追加工具输出到当前消息（`append_tool_outputs`）。该命名空间还包含多个与请求、响应、历史、状态、工具选择等协议相关的内部变量，构成了协议数据模型的骨架，为上层模块提供统一、可复用的交互接口。

## Subnamespaces

- [`clore::net::anthropic::protocol::detail`](detail/index.md)

## Functions

### `clore::net::anthropic::protocol::append_tool_outputs`

Declaration: `src/network/anthropic.cppm:218`

Definition: `src/network/anthropic.cppm:637`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::append_tool_outputs` 向当前正在构建的协议消息中追加一个工具调用的输出。调用者需提供该输出的标识符及具体内容（通过前两个参数），并指定一个与追加顺序或分组相关的整数。函数返回一个 `int` 状态值，指示追加操作是否成功；非零值通常表示错误。此函数不负责验证工具输出格式的有效性，只负责按协议要求将数据添加到消息中。

#### Usage Patterns

- Callers use this to incorporate tool outputs into a message history after receiving a response from the Anthropic API.
- Typically invoked after a completion response that requested tool use, to prepare the next request.

### `clore::net::anthropic::protocol::build_messages_url`

Declaration: `src/network/anthropic.cppm:210`

Definition: `src/network/anthropic.cppm:233`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/build-messages-url.md)

该函数接受一个表示 API 端点基础路径的 `std::string_view`，并返回一个完整的 `std::string` 形式的消息 API URL。调用方有责任提供正确的基础路径（例如原始 API 服务器地址），而无需关心 URL 内部拼接逻辑。返回值可直接用于 HTTP 请求中的目标地址。

#### Usage Patterns

- used by `clore::net::anthropic::detail::Protocol::build_url` to produce the final messages request URL
- called with various API base `URLs` to generate the appropriate versioned endpoint

### `clore::net::anthropic::protocol::build_request_json`

Declaration: `src/network/anthropic.cppm:212`

Definition: `src/network/anthropic.cppm:244`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::build_request_json` 接受一个 `const int &` 参数并返回一个 `int`。调用者应提供一个整数，该整数代表请求的配置或标识符；函数负责基于该输入构造一个请求 JSON 对象。返回的整数可能是操作的状态码或生成的 JSON 对象的句柄。该函数是 `clore::net::anthropic::protocol` 命名空间中协议处理逻辑的一部分，用于生成 Anthropic API 请求的 JSON 载荷。

#### Usage Patterns

- building HTTP request body for Anthropic API
- serializing a `CompletionRequest` to JSON before sending

### `clore::net::anthropic::protocol::parse_response`

Declaration: `src/network/anthropic.cppm:214`

Definition: `src/network/anthropic.cppm:469`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

调用者需传入一个来自 Anthropic API 的原始响应字符串（作为 `std::string_view`），该函数将解析该响应并返回一个整数标识符，代表解析后的内部状态。此标识符可被后续协议函数（如 `parse_response_text`、`parse_tool_arguments`）使用，以提取具体字段。调用者有责任确保传入的字符串是有效的、符合预期间结构（一般为 JSON 格式）的响应；函数未报告格式错误的语义（可能通过返回值表示失败）。返回的整数标识符仅在当前协议生命周期内有效，不应被持久化或跨上下文使用。

#### Usage Patterns

- Used to deserialize Anthropic API HTTP response bodies into structured types
- Called by higher-level network or request functions to convert raw JSON to `CompletionResponse`
- Utilized in error handling paths to map API error payloads to `LLMError`

### `clore::net::anthropic::protocol::parse_response_text`

Declaration: `src/network/anthropic.cppm:224`

Definition: `src/network/anthropic.cppm:645`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::parse_response_text` 是一个模板函数，负责从 Anthropic API 响应中解析并提取文本内容。调用者应提供响应对象（类型 `T`），函数返回解析后的文本数据；具体返回类型由响应内容和模板实例化决定。该函数是协议层对外暴露的接口，调用者应确保传入的响应对象符合预期格式。

#### Usage Patterns

- 从 Anthropic API 响应中提取结构化文本
- 作为 `protocol` 模块的通用解析入口

### `clore::net::anthropic::protocol::parse_tool_arguments`

Declaration: `src/network/anthropic.cppm:227`

Definition: `src/network/anthropic.cppm:650`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::parse_tool_arguments` 是一个模板函数，接受一个 `const int&` 类型的参数并返回一个 `int`。它的职责是从工具响应中解析参数，返回的整数值通常反映解析结果的状态或相关数据索引。调用者应提供一个有效的引用输入，并根据返回码进行后续处理。该函数与 `clore::net::anthropic::protocol` 命名空间中的其他解析函数共同构成工具交互流程的一部分。

#### Usage Patterns

- 从 tool call 中反序列化参数对象
- 作为解析 tool 调用结果的统一接口

### `clore::net::anthropic::protocol::text_from_response`

Declaration: `src/network/anthropic.cppm:216`

Definition: `src/network/anthropic.cppm:632`

Implementation: [`Module anthropic`](../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::text_from_response` 从给定的响应对象中提取文本内容。它接受一个 `const int&` 类型的响应标识，该标识应代表一个有效的、已解析的响应，并返回一个 `int` 类型的值，表示该响应中包含的文本数据（例如，文本长度或编码标识）。调用者负责确保传入的响应标识有效且符合预设格式，否则函数的行为未定义。

#### Usage Patterns

- 从 Anthropic 的 `CompletionResponse` 中提取文本
- 作为 `clore::net::protocol::text_from_response` 的 Anthropic 特定封装

## Related Pages

- [Namespace clore::net::anthropic](../index.md)
- [Namespace clore::net::anthropic::protocol::detail](detail/index.md)

