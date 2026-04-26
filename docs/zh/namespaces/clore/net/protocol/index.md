---
title: 'Namespace clore::net::protocol'
description: 'clore::net::protocol 命名空间提供了与 LLM 通信协议相关的一组核心工具函数。它主要承担三方面职责：解析响应（如 parse_response、parse_response_text、text_from_response）、构建请求（如 build_request_json）以及验证输出（如 validate_markdown_fragment_output、validate_json_output）。此外，还包含用于处理工具调用的辅助函数（parse_tool_arguments、append_tool_outputs），这些函数共同支持从 LLM 响应中提取结构化数据、将工具执行结果回填到协议格式中，并对生成的内容进行合规性检查。'
layout: doc
template: doc
---

# Namespace `clore::net::protocol`

## Summary

`clore::net::protocol` 命名空间提供了与 LLM 通信协议相关的一组核心工具函数。它主要承担三方面职责：**解析响应**（如 `parse_response`、`parse_response_text`、`text_from_response`）、**构建请求**（如 `build_request_json`）以及**验证输出**（如 `validate_markdown_fragment_output`、`validate_json_output`）。此外，还包含用于处理工具调用的辅助函数（`parse_tool_arguments`、`append_tool_outputs`），这些函数共同支持从 LLM 响应中提取结构化数据、将工具执行结果回填到协议格式中，并对生成的内容进行合规性检查。

从架构角度看，该命名空间充当协议层与上层业务逻辑之间的**适配器**：它封装了消息的序列化格式（如 JSON）、工具调用约定的解析与构造，以及响应内容的校验逻辑。调用方通过该命名空间的接口，无需直接操作底层协议细节，即可完成与 LLM 的交互执行流程（“发送请求 → 接收响应 → 解析工具调用 → 执行工具 → 追加结果 → 构造下一轮请求”）。因此，`clore::net::protocol` 是实现 LLM 对话循环中协议处理的关键组件。

## Functions

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:540`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

调用方使用 `clore::net::protocol::append_tool_outputs` 将工具执行结果附加到已有响应结构中。第一个 `int` 参数标识目标上下文（例如会话或请求句柄），`const CompletionResponse &` 提供来自 LLM 的完成响应，第二个 `int` 参数指示待追加的工具输出数据源（通过其他函数如 `parse_tool_arguments` 得到）。返回值 `int` 表示操作是否成功，零通常表示成功，非零表示错误代码。调用方需确保传入的 `CompletionResponse` 已包含 `ToolCall` 信息且工具输出已就绪，函数不会校验其有效性。

#### Usage Patterns

- appending tool outputs to conversation history
- constructing updated message list for LLM API

### `clore::net::protocol::build_request_json`

Declaration: `network/openai.cppm:457`

Definition: `network/openai.cppm:465`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

`clore::net::protocol::build_request_json` 基于传入的整数引用构造一个请求的 JSON 表示。调用者必须保证提供的 `const int &` 参数在调用期间保持有效且具有正确的语义含义（例如表示请求标识符或索引）。函数返回一个 `int` 值，用于指示操作的执行结果：非零值通常表示成功，零或负值表示错误。调用者应根据返回值执行相应的错误处理。

#### Usage Patterns

- used to generate `OpenAI`-compatible request JSON
- called before sending HTTP request to LLM API

### `clore::net::protocol::parse_response`

Declaration: `network/openai.cppm:459`

Definition: `network/openai.cppm:532`

Implementation: [`Module openai`](../../../../modules/openai/index.md)

`clore::net::protocol::parse_response` 接受一个 `std::string_view` 参数，表示待解析的原始响应数据，并返回一个整数结果。该整数通常指示解析是否成功或携带某种状态信息，调用者应据此判断后续逻辑。

调用者负责在函数调用期间确保传入的 `std::string_view` 内容有效且未被修改。函数不会接管字符串的所有权，调用者必须维护数据的生存期直到调用结束。

#### Usage Patterns

- Parse LLM API response JSON
- Deserialize completion response

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:475`

Definition: `network/protocol.cppm:588`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

该函数从给定的 `CompletionResponse` 中解析出响应文本，并返回一个 `int` 值。调用者应确保传入的响应对象有效且包含可解析的文本内容；返回值指示解析结果或提供后续处理所需的标识。作为模板函数，`parse_response_text` 支持类型泛化，但其返回类型固定为 `int`，调用者无需显式指定模板参数。

#### Usage Patterns

- extract and parse structured data from a LLM response
- deserialize JSON from `CompletionResponse`

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:478`

Definition: `network/protocol.cppm:603`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

`clore::net::protocol::parse_tool_arguments` 是一个模板函数，它接受一个 `ToolCall` 对象的常引用，解析其中包含的参数，并返回一个整数。返回的整数表示解析操作的成功或错误状态，调用者应根据其值判断后续处理是否应继续。该函数的模板参数 `T` 允许调用者指定参数期望的类型或解析策略，但具体的行为由协议上下文定义。

#### Usage Patterns

- Convert tool call arguments to a typed object for further processing

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:524`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

`clore::net::protocol::text_from_response` 函数接收一个 `const CompletionResponse &` 引用，返回一个 `int`。调用者调用此函数以从给定的 `CompletionResponse` 中提取文本，返回的整数值指示提取操作的结果。该函数是协议模块中处理响应文本的核心入口之一，调用者应根据返回的整数值判断操作是否成功，并继续后续处理（如通过 `append_tool_outputs` 或 `build_request_json` 等进行下一步调用）。

#### Usage Patterns

- used to obtain a plain text response from an LLM completion
- called after ensuring the response is not a tool call or refusal

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:463`

Definition: `network/protocol.cppm:484`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

函数 `clore::net::protocol::validate_json_output` 负责验证调用者提供的 JSON 输出字符串。它接受一个 `std::string_view` 参数，该参数包含待验证的 JSON 文本，并返回一个 `int` 值，通常用于指示验证结果：返回零表示通过验证，非零值表示验证失败或存在特定错误。调用者应确保传入的字符串是合法的 JSON 格式，并在调用后检查返回值以判断输出是否符合预期。

#### Usage Patterns

- Used to validate LLM output before further processing.

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:465`

Definition: `network/protocol.cppm:493`

Implementation: [`Module protocol`](../../../../modules/protocol/index.md)

函数 `clore::net::protocol::validate_markdown_fragment_output` 负责验证一个 markdown 片段的有效性。调用者需传入一个 `std::string_view` 表示待校验的 markdown 片段，函数会检查其是否符合预期的格式或内容约束。

函数返回一个 `int` 类型的值，该值指示验证结果。调用方应依赖返回值判断输入是否通过验证，而非假设其语义细节（例如零表示成功，非零表示失败）。

#### Usage Patterns

- validating LLM output before further processing
- ensuring response matches expected markdown fragment format

## Related Pages

- [Namespace clore::net](../index.md)

