---
title: 'Namespace clore::net::protocol'
description: 'clore::net::protocol 命名空间封装了与网络通信协议相关的核心数据处理逻辑，主要负责请求构造、响应解析以及格式验证。该命名空间提供了一系列函数，例如使用 build_request_json 从配置参数构建 JSON 请求、通过 parse_response 和 parse_response_text 解析服务器返回的完成响应（CompletionResponse），并提取其中的文本内容或工具调用结果。此外，它还包含 validate_json_output 和 validate_markdown_fragment_output 等验证函数，用于确保协议数据的格式符合预期规范，从而保证上下游模块间的数据一致性。在架构上，该命名空间作为网络层与业务逻辑层之间的桥梁，将原始通信数据转换为结构化表示，并处理工具参数解析（如 parse_tool_arguments）与输出追加（如 append_tool_outputs），是 Clore 网络协议栈中实现可扩展交互的重要组件。'
layout: doc
template: doc
---

# Namespace `clore::net::protocol`

## Summary

`clore::net::protocol` 命名空间封装了与网络通信协议相关的核心数据处理逻辑，主要负责请求构造、响应解析以及格式验证。该命名空间提供了一系列函数，例如使用 `build_request_json` 从配置参数构建 JSON 请求、通过 `parse_response` 和 `parse_response_text` 解析服务器返回的完成响应（`CompletionResponse`），并提取其中的文本内容或工具调用结果。此外，它还包含 `validate_json_output` 和 `validate_markdown_fragment_output` 等验证函数，用于确保协议数据的格式符合预期规范，从而保证上下游模块间的数据一致性。在架构上，该命名空间作为网络层与业务逻辑层之间的桥梁，将原始通信数据转换为结构化表示，并处理工具参数解析（如 `parse_tool_arguments`）与输出追加（如 `append_tool_outputs`），是 Clore 网络协议栈中实现可扩展交互的重要组件。

## Functions

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:473`

Definition: `network/protocol.cppm:544`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

`clore::net::protocol::append_tool_outputs` 将指定 `CompletionResponse` 中的工具执行结果追加到由前两个整数参数标识的请求上下文中。调用者需确保传入的 `CompletionResponse` 包含有效的工具输出，且整数参数正确对应目标会话或阶段。函数返回一个整数值表示操作是否成功，调用者应根据返回值判断处理是否完成。

#### Usage Patterns

- Used to incorporate tool execution results back into the conversation history
- Called after a `CompletionResponse` with non‑empty `tool_calls` is received
- Replaces manual construction of assistant and tool result messages

### `clore::net::protocol::build_request_json`

Declaration: `network/openai.cppm:457`

Definition: `network/openai.cppm:465`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

`clore::net::protocol::build_request_json` 负责构造请求的 JSON 表示。调用者提供 `const int &` 类型的参数，该参数代表与请求相关联的标识符或配置；函数返回一个 `int` 值，表示构建操作的结果。调用者应确保传入的引用有效，并依据返回值判断请求是否成功构建。

#### Usage Patterns

- 通过 `build_request_json(request)` 将业务请求对象序列化为 JSON 字符串
- 用于构造发送给 LLM 的 HTTP 请求体

### `clore::net::protocol::parse_response`

Declaration: `network/openai.cppm:459`

Definition: `network/openai.cppm:532`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

调用 `clore::net::protocol::parse_response` 以解析以 `std::string_view` 形式提供的协议响应。该函数返回一个 `int` 值，表示解析结果或状态码。调用者应确保传入的字符串视图中包含格式正确的响应数据，并依据返回的整数判断解析是否成功或进一步处理。

#### Usage Patterns

- parse LLM API JSON response into `CompletionResponse`
- validate and convert raw response text from an AI provider

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:479`

Definition: `network/protocol.cppm:592`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

`clore::net::protocol::parse_response_text` 是一个模板函数，接受一个 `CompletionResponse` 类型的常量引用并返回 `int`。调用方使用此函数来解析完成响应中的文本部分。返回的整数值指示解析操作的结果或状态，调用方应检查该值以确定解析是否成功。

#### Usage Patterns

- parse structured LLM response into type `T`
- convert `CompletionResponse` to `std::expected<T, LLMError>`

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:482`

Definition: `network/protocol.cppm:607`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

The `clore::net::protocol::parse_tool_arguments` template function processes a `ToolCall` object to extract and validate its tool‑specific arguments. Its primary responsibility is to interpret the raw call data and produce a normalized representation suitable for downstream protocol handling. The function returns an `int` status code: zero signals successful parsing, while a non‑zero value indicates an error condition that the caller should handle.

#### Usage Patterns

- Invoked to deserialize tool call arguments into a concrete type for further processing
- Commonly used in tool execution or validation pipelines

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:471`

Definition: `network/protocol.cppm:528`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

函数 `clore::net::protocol::text_from_response` 接受一个 `const CompletionResponse &` 对象，并返回一个 `int` 值。调用者应当检查该返回值：一个非零值通常表示处理过程中发生的错误，而零值则代表成功。该函数旨在从响应对象中提取文本内容，但实际写入或获取文本的具体机制由调用者根据返回值进行后续处理。这是协议解析流程中的一个步骤，常与其他返回 `int` 的解析函数（如 `parse_response_text`）配合使用，以构成完整的响应处理管线。

#### Usage Patterns

- extracting text output from a `CompletionResponse`
- validating that a response is a text completion before further processing
- used in conjunction with error handling via `std::expected`

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:488`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

`clore::net::protocol::validate_json_output` 接受一个 `std::string_view` 类型的 JSON 文本，并返回一个 `int` 值。调用者应传入预期为合法 JSON 的字符串；函数的职责是验证该输入是否符合 JSON 格式规范。返回值为 0 表示验证通过，非零值表示存在格式错误或其它无效情形，具体错误码的含义由协议内部约定。此函数不修改输入字符串，且不解析 JSON 的语义内容，仅执行格式层面的校验。

#### Usage Patterns

- 用于校验 LLM 输出格式是否为合法 JSON

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:497`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

接受一个 Markdown 片段字符串（`std::string_view`），并返回一个 `int` 表示验证结果。调用者负责提供要验证的 Markdown 文本，该函数检查其格式是否符合协议规定的 Markdown 片段输出规范。返回值为零通常表示验证通过，非零值表示特定错误类型或失败原因；调用者应依赖返回码判断后续处理是否继续。

#### Usage Patterns

- Called during LLM response validation to ensure output is a valid markdown fragment

## Related Pages

- [Namespace clore::net](../index.md)

