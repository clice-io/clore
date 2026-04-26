---
title: 'Namespace clore::net::anthropic::protocol::detail'
description: '位于 clore::net::anthropic::protocol::detail 命名空间下的代码属于该协议实现中的内部细节层。该命名空间提供了一组底层辅助函数和常量，用于构建 Anthropic API 请求中的结构单元（如文本块、工具使用块、角色消息、工具结果块等），以及执行 JSON 解析、模式指令格式化、请求验证等任务。这些工具函数对外返回整数标识符作为不透明句柄，上层逻辑通过组合这些句柄来组装完整的请求载荷，同时依赖 kDefaultMaxTokens 这样的常量和 validate_request 等验证函数确保协议约束的正确性。其设计意图是将协议细节封装在独立的内部命名空间中，简化外部接口并保持协议实现的整洁性和可维护性。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol::detail`

## Summary

位于 `clore::net::anthropic::protocol::detail` 命名空间下的代码属于该协议实现中的内部细节层。该命名空间提供了一组底层辅助函数和常量，用于构建 Anthropic API 请求中的结构单元（如文本块、工具使用块、角色消息、工具结果块等），以及执行 JSON 解析、模式指令格式化、请求验证等任务。这些工具函数对外返回整数标识符作为不透明句柄，上层逻辑通过组合这些句柄来组装完整的请求载荷，同时依赖 `kDefaultMaxTokens` 这样的常量和 `validate_request` 等验证函数确保协议约束的正确性。其设计意图是将协议细节封装在独立的内部命名空间中，简化外部接口并保持协议实现的整洁性和可维护性。

## Variables

### `clore::net::anthropic::protocol::detail::kDefaultMaxTokens`

Declaration: `network/anthropic.cppm:23`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

A `constexpr std::uint32_t` constant defined in the `clore::net::anthropic::protocol::detail` namespace, initialized to `2048`. It serves as the default maximum token count for Anthropic API requests.

#### Usage Patterns

- used as default argument in `build_request_json`

## Functions

### `clore::net::anthropic::protocol::detail::append_text_with_gap`

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/append-text-with-gap.md)

函数 `clore::net::anthropic::protocol::detail::append_text_with_gap` 接受一个 `std::string &` 和一个 `std::string_view`。它将后者包含的文本内容追加到前者，并在追加的文本前后（或之间）按照协议要求插入一个间隙。调用方必须提供一个有效的、可修改的字符串引用作为目标，并确保 `std::string_view` 所引用的字符序列在调用期间保持有效。该函数不返回值，修改直接作用于目标字符串，用于构建请求 JSON 中需要保留特定间隔格式的文本块序列。

#### Usage Patterns

- called in `build_request_json` to accumulate text blocks with gaps

### `clore::net::anthropic::protocol::detail::format_schema_instruction`

Declaration: `network/anthropic.cppm:176`

Definition: `network/anthropic.cppm:176`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`format_schema_instruction` 根据传入的模式引用（类型为 `const int &`）生成并返回一条格式化的模式指令。调用者需确保提供的模式引用有效且内容符合协议要求；函数返回的 `int` 值可用于后续流程中对指令的引用或状态判断。

#### Usage Patterns

- Used when constructing system prompts for structured output
- Called by higher-level functions to enforce response format

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:154`

Definition: `network/anthropic.cppm:154`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_role_message` constructs a role‑based message for the Anthropic protocol. The caller provides a role identifier as a `std::string_view` (e.g., `"user"` or `"assistant"`) and the message content. Two overloads are available: one accepts a `json::Array` to supply structured content (such as multiple blocks), and the other accepts a plain `std::string_view` for simple text content. The function returns an `int` that identifies the newly created message, which can later be used in the request assembly pipeline.

The caller is responsible for ensuring that the role string is a valid role as defined by the protocol, and that the content (whether array or string) is well‑formed for that role. The return value must be treated as an opaque handle; its validity is guaranteed only within the scope of the current request construction flow. No validation of the protocol constraints on role–content combinations is performed by this function itself.

#### Usage Patterns

- Called to build a user or assistant role message in API request construction.
- Used by higher-level functions that assemble message lists for chat completions.

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:130`

Definition: `network/anthropic.cppm:130`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

构造一个表示角色消息的协议消息。调用者提供角色标识符（如 `"user"` 或 `"assistant"`）以及消息内容，内容可以是纯文本字符串或 `json::Array` 格式的结构化数据。函数返回一个整数标识符，该标识符可用于后续操作（例如添加到请求或与其他块组合）。该函数是 `clore::net::anthropic::protocol::detail` 命名空间中消息构建工具的一部分，调用者应确保传入的角色字符串符合 Anthropic 协议支持的值，且内容格式与协议要求一致。

#### Usage Patterns

- used to construct a message object with a role and plain text content
- called by higher-level protocol functions that need to build messages

### `clore::net::anthropic::protocol::detail::make_text_block`

Declaration: `network/anthropic.cppm:35`

Definition: `network/anthropic.cppm:35`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::make_text_block` 接受一个 `std::string_view` 文本内容，构造一个内部表示文本块的资源，并返回一个整数标识符。调用者应将此标识符视为该文本块的不透明句柄，用于后续的消息组装或处理流程中。该函数不负责检查内容有效性或解析格式；它仅将给定字符串封装为标准文本块单元。

#### Usage Patterns

- constructing text content block for Anthropic protocol messages

### `clore::net::anthropic::protocol::detail::make_tool_result_block`

Declaration: `network/anthropic.cppm:98`

Definition: `network/anthropic.cppm:98`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::detail::make_tool_result_block` 接收一个 `const int &` 类型的工具结果标识符，并返回一个整数代表构造的工具结果块。调用者应确保传入的标识符有效；函数不会修改该参数。返回的整数值可用于后续处理，例如嵌入到消息结构中。

该函数是 `clore::net::anthropic::protocol` 实现细节的一部分，用于构建与Anthropic协议兼容的工具结果块。

#### Usage Patterns

- Convert a `ToolResultMessage` to a JSON block for inclusion in API request content

### `clore::net::anthropic::protocol::detail::make_tool_use_block`

Declaration: `network/anthropic.cppm:58`

Definition: `network/anthropic.cppm:58`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::make_tool_use_block` 是一个辅助函数，用于构造一个表示工具使用（tool use）的内容块。它通过传入的 `const int &` 参数（通常代表工具调用的标识符）来生成对应的协议块。

调用者应提供一个有效的工具调用整数标识，函数返回一个整数结果（可能表示新创建块在内容数组中的位置或操作状态）。该函数不直接修改传入的参数，调用者需自行处理返回的整数值以适应后续的消息组装流程。

#### Usage Patterns

- Called to convert a `ToolCall` into a JSON block for Anthropic request messages

### `clore::net::anthropic::protocol::detail::parse_json_text`

Declaration: `network/anthropic.cppm:171`

Definition: `network/anthropic.cppm:171`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::detail::parse_json_text` 接受两个 `std::string_view` 参数，返回一个 `int` 值。通常第一个参数是待解析的 JSON 文本，第二个参数提供解析所需的上下文或标识符。调用方需确保传入的字符串视图在函数调用期间保持有效且内容不被修改。返回值表示解析得到的整数结果或操作状态码，调用方应根据具体场景检查并处理该返回值。此函数是协议细节命名空间的一部分，供其他消息构造函数（如 `make_text_block` 或 `make_tool_use_block`）内部使用。

#### Usage Patterns

- Used internally to parse JSON content from API responses or local data.
- Called as a convenience wrapper for unified error handling with `LLMError`.

### `clore::net::anthropic::protocol::detail::validate_request`

Declaration: `network/anthropic.cppm:193`

Definition: `network/anthropic.cppm:193`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::detail::validate_request` 接受一个 `const int &` 类型的请求标识符，返回一个 `int` 值指示验证结果。调用者负责提供合法的请求标识符；函数会检查请求是否符合协议约束，返回零表示通过验证，非零值对应特定的失败原因。

#### Usage Patterns

- validates a completion request before Anthropic API call
- used as a wrapper around generic validation

## Related Pages

- [Namespace clore::net::anthropic::protocol](../index.md)

