---
title: 'Namespace clore::net::protocol'
description: '命名空间 clore::net::protocol 提供了一组用于处理 LLM 网络通信协议的工具函数，专注于消息的解析、验证和请求构建。其核心职责包括：从 CompletionResponse 响应中提取文本或结构化内容（如工具调用）、将工具输出追加到会话上下文、验证 JSON 和 Markdown 片段输出是否符合预期格式，以及构建请求的 JSON 负载。该命名空间抽象了协议层的序列化与反序列化细节，使得上层模块可以专注于业务逻辑，而无需直接处理底层数据结构。'
layout: doc
template: doc
---

# Namespace `clore::net::protocol`

## Summary

命名空间 `clore::net::protocol` 提供了一组用于处理 LLM 网络通信协议的工具函数，专注于消息的解析、验证和请求构建。其核心职责包括：从 `CompletionResponse` 响应中提取文本或结构化内容（如工具调用）、将工具输出追加到会话上下文、验证 JSON 和 Markdown 片段输出是否符合预期格式，以及构建请求的 JSON 负载。该命名空间抽象了协议层的序列化与反序列化细节，使得上层模块可以专注于业务逻辑，而无需直接处理底层数据结构。

从架构上看，`clore::net::protocol` 位于网络层和业务逻辑层之间，扮演适配器角色。它利用 `parse_tool_arguments`、`append_tool_outputs`、`parse_response` 和 `text_from_response` 等函数，将原始的字符串响应转换为一组内部状态码（通常以 `int` 返回），从而统一了错误处理和验证流程。这些函数通常与消息历史、工具调用 ID 和完成理由等变量配合工作，确保整个协议处理链路的数据一致性。该命名空间通过提供标准化的验证入口（如 `validate_json_output` 和 `validate_markdown_fragment_output`），增强了对外部输入的健壮性。

## Functions

### `clore::net::protocol::append_tool_outputs`

Declaration: `src/network/protocol.cppm:485`

Definition: `src/network/protocol.cppm:556`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

函数 `clore::net::protocol::append_tool_outputs` 负责将一次工具调用的输出追加到由整数标识的上下文（如会话或消息序列）中。调用方提供一个会话的句柄（第一个 `int`）、一个描述完成响应的 `CompletionResponse` 对象，以及一个表示待附加输出数或起始位置的整数（第三个 `int`）；函数执行后返回一个 `int` 以指示操作结果（例如实际追加的数量或成功/失败状态）。该函数通常配合解析或构建响应流的其他协议函数使用，是调用方在接收工具输出后将其整合到协议状态中的关键步骤。

#### Usage Patterns

- 在完成响应包含工具调用时，将工具输出合并到对话历史中
- 作为构建请求或响应处理流程的一部分

### `clore::net::protocol::build_request_json`

Declaration: `src/network/openai.cppm:467`

Definition: `src/network/openai.cppm:475`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

函数 `clore::net::protocol::build_request_json` 接受一个 `const int &` 参数（表示请求标识符）并返回一个 `int` 值，该值指示构建 JSON 请求的结果。调用者负责提供有效的请求标识符，并检查返回的整数状态以确认 JSON 是否成功构建。该函数不修改其参数，且调用者应当根据返回代码处理可能的错误情况。

#### Usage Patterns

- 作为构造 API 请求 JSON 的核心函数被 `clore::net::protocol` 中的请求发送逻辑调用
- 调用方通常先构建 `CompletionRequest`，然后调用此函数获取 JSON 负载

### `clore::net::protocol::parse_response`

Declaration: `src/network/openai.cppm:469`

Definition: `src/network/openai.cppm:542`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

函数 `clore::net::protocol::parse_response` 接受一个 `std::string_view` 类型的响应数据，并返回一个 `int` 值。该值是解析过程的直接结果，供调用者判断响应是否有效或进行后续处理。调用者应确保传入的字符串包含完整且格式正确的响应内容；返回的非零值通常指示解析失败或需要特殊处理的特定条件。

#### Usage Patterns

- 用于解析 LLM API 返回的 JSON 响应
- 通常由上层通信或协议处理模块调用

### `clore::net::protocol::parse_response_text`

Declaration: `src/network/protocol.cppm:491`

Definition: `src/network/protocol.cppm:604`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

`clore::net::protocol::parse_response_text` 是一个模板函数，接受一个 `CompletionResponse` 常引用并返回 `int`。调用方应提供一个有效的 `CompletionResponse` 对象；函数会从该响应中解析文本部分，并返回一个指示处理结果的状态码（通常 0 表示成功，非零表示错误）。作为模板函数，它可针对不同的 `T` 类型实例化，但调用方通常无需显式指定模板参数，除非需要特定特化行为。

#### Usage Patterns

- 将 LLM 的 JSON 结构化响应转换为特定类型 `T`
- 在协议解析链中处理结构化输出
- 与 `build_request_json`、`parse_tool_arguments` 等函数配合使用

### `clore::net::protocol::parse_tool_arguments`

Declaration: `src/network/protocol.cppm:494`

Definition: `src/network/protocol.cppm:619`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

`clore::net::protocol::parse_tool_arguments` 是一个函数模板，负责解析给定的 `ToolCall` 中的参数。它返回一个 `int` 值，该值向调用方传达解析结果的状态（例如成功或错误条件）。调用方应根据返回的整数值判断参数是否有效或是否需要进一步处理。

#### Usage Patterns

- Used to extract typed tool arguments from a `ToolCall` object
- Commonly called in tool invocation handling code

### `clore::net::protocol::text_from_response`

Declaration: `src/network/protocol.cppm:483`

Definition: `src/network/protocol.cppm:540`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

函数 `clore::net::protocol::text_from_response` 从给定的 `CompletionResponse` 中提取文本信息，并将结果以 `int` 形式返回给调用者。调用者应当确保传入的 `CompletionResponse` 对象已经过正确构造且包含有效的响应数据；返回值的语义遵循协议模块内同类解析函数所使用的约定，通常用于表示操作成功与否或具体的错误状态。

#### Usage Patterns

- Extracting text from a completion response after validating no refusal or tool calls

### `clore::net::protocol::validate_json_output`

Declaration: `src/network/protocol.cppm:479`

Definition: `src/network/protocol.cppm:500`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

函数 `clore::net::protocol::validate_json_output` 验证所提供的 JSON 输出是否符合预期格式与契约。它接受一个 `std::string_view` 参数，并返回一个 `int` 值，调用者可根据该值判断验证是否通过。

调用者应确保传入的字符串为有效的 JSON 格式；函数的行为在输入为非标准 JSON 时未定义。返回值的具体含义由调用者依据上下文约定的状态码解释（通常 `0` 表示验证成功，非零表示验证失败）。

#### Usage Patterns

- Called after receiving an LLM response to ensure its content is valid JSON before further processing
- Likely used in conjunction with other protocol functions like `parse_response` or `validate_markdown_fragment_output`

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `src/network/protocol.cppm:481`

Definition: `src/network/protocol.cppm:509`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

函数 `clore::net::protocol::validate_markdown_fragment_output` 负责验证给定的 Markdown 片段输出是否符合协议约定。它接受一个 `std::string_view` 类型参数表示待验证的片段，并返回一个 `int` 值指示验证结果。调用者应确保传入的内容为预期的 Markdown 格式表达，并根据返回值判断验证是否通过，从而决定是否继续后续的协议处理流程。

#### Usage Patterns

- Validation of markdown fragments from LLM responses
- Rejecting JSON when markdown fragment is expected

## Related Pages

- [Namespace clore::net](../index.md)

