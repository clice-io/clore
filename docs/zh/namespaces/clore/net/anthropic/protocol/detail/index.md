---
title: 'Namespace clore::net::anthropic::protocol::detail'
description: 'clore::net::anthropic::protocol::detail 是协议实现的内部命名空间，专注于提供构建和验证 Anthropic API 请求所需的底层辅助函数与常量。它封装了文本块、角色消息、工具使用块和工具结果块的构造（如 make_text_block、make_role_message、make_tool_use_block、make_tool_result_block），以及格式化指令、JSON 解析、请求校验和文本拼接等工具（如 format_schema_instruction、parse_json_text、validate_request、append_text_with_gap）。这些函数通常返回整型标识符用于内部引用，并依赖 kDefaultMaxTokens（2048）作为默认令牌上限。作为协议层的胶水代码，它将原始协议细节抽象为可组合的积木块，使上层代码无需直接处理 JSON 格式或协议语义，从而保持请求组装过程的一致性和可维护性。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol::detail`

## Summary

`clore::net::anthropic::protocol::detail` 是协议实现的内部命名空间，专注于提供构建和验证 Anthropic API 请求所需的底层辅助函数与常量。它封装了文本块、角色消息、工具使用块和工具结果块的构造（如 `make_text_block`、`make_role_message`、`make_tool_use_block`、`make_tool_result_block`），以及格式化指令、JSON 解析、请求校验和文本拼接等工具（如 `format_schema_instruction`、`parse_json_text`、`validate_request`、`append_text_with_gap`）。这些函数通常返回整型标识符用于内部引用，并依赖 `kDefaultMaxTokens`（2048）作为默认令牌上限。作为协议层的胶水代码，它将原始协议细节抽象为可组合的积木块，使上层代码无需直接处理 JSON 格式或协议语义，从而保持请求组装过程的一致性和可维护性。

## Variables

### `clore::net::anthropic::protocol::detail::kDefaultMaxTokens`

Declaration: `src/network/anthropic.cppm:32`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

常量 `kDefaultMaxTokens` 是命名空间 `clore::net::anthropic::protocol::detail` 中定义的 `constexpr std::uint32_t` 类型常量，其值为 `2048`，用作默认最大令牌数。

#### Usage Patterns

- 作为 `build_request_json` 函数中 `max_tokens` 的默认值

## Functions

### `clore::net::anthropic::protocol::detail::append_text_with_gap`

Declaration: `src/network/anthropic.cppm:34`

Definition: `src/network/anthropic.cppm:34`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/append-text-with-gap.md)

向目标字符串追加给定的文本视图，并在嵌入时自动维持适当的格式间隙（例如换行与缩进），使拼接结果符合 Anthropic 协议对 JSON 结构的布局要求。调用方须提供可变的 `std::string` 引用作为接收缓冲区，以及一个 `std::string_view` 作为待附加的文本片段；函数独立处理间隙逻辑，调用方无需关心 JSON 格式化细节。

#### Usage Patterns

- Called by `clore::net::anthropic::protocol::build_request_json` to add text blocks.

### `clore::net::anthropic::protocol::detail::format_schema_instruction`

Declaration: `src/network/anthropic.cppm:185`

Definition: `src/network/anthropic.cppm:185`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`format_schema_instruction` 接受一个架构指令标识符（通过 `const int &` 传入），并返回一个格式化的指令表示（以 `int` 形式返回）。调用者应保证提供的标识符在内部映射表中有效；返回的整数值可直接用于构造后续的协议块，例如 `make_tool_use_block` 或 `make_tool_result_block`。该函数是底层格式化步骤，不直接校验语义一致性——此类校验应由 `validate_request` 在其他地方执行。

#### Usage Patterns

- Called to generate system instruction text for LLM requests
- Used in constructing the final prompt when a response schema is specified
- Provides fallback default instruction when no schema is set

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `src/network/anthropic.cppm:139`

Definition: `src/network/anthropic.cppm:139`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::make_role_message` 是一个重载函数，用于构造一条带有指定角色和内容的消息。该函数返回一个整型标识符，调用方应妥善保存该标识符并在后续操作中引用对应消息。第一个重载接受两个 `std::string_view` 参数：第一个表示消息角色（如 “user” 或 “assistant”），第二个表示文本内容。第二个重载接受一个 `std::string_view` 角色和一个 `json::Array` 作为内容块集合，适用于需要包含多个内容片段（如文本块、工具使用块等）的消息。调用方应保证角色值符合 Anthropic 协议的有效取值范围，且内容参数在语义上合法（例如文本内容不为空，或数组中的每个元素均为协议允许的内容块）。

#### Usage Patterns

- Constructing a user or assistant message for Anthropic API requests
- Building message objects with role and plain text content
- Internal call within `clore::net::anthropic::protocol` namespace

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `src/network/anthropic.cppm:163`

Definition: `src/network/anthropic.cppm:163`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

构造一个角色化消息，该消息将给定的角色标识（如 `"user"` 或 `"assistant"`）与内容关联起来。重载形式分别接收一个 `json::Array`（用于多块内容）或一个 `std::string_view`（作为单个文本块）。返回一个整型值，表示所创建消息的标识符或状态码；调用方应对此返回值进行检查以确认操作结果。此函数是协议层内部的构造器，调用方不应假设其具体的内存管理或生命周期行为。

#### Usage Patterns

- used to build message objects with a role and content blocks
- called when constructing user, assistant, or tool result messages

### `clore::net::anthropic::protocol::detail::make_text_block`

Declaration: `src/network/anthropic.cppm:44`

Definition: `src/network/anthropic.cppm:44`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::detail::make_text_block` 接受一个代表纯文本内容的 `std::string_view`，并返回一个整型值，该值表示所构造的文本块的标识符或句柄。它用于将文本片段转换为协议定义的块结构，以便在后续的 `role_message` 或 `tool_result_block` 等组合函数中使用。

调用者需保证传入的字符串视图在函数调用期间仍然有效；该函数不承担所有权或延长字符串生命周期的责任。此函数位于 `detail` 命名空间，属于协议实现的内部辅助，不应由外部代码直接使用。返回的整型值仅可用于同一协议构建流程中的其他函数，不具有跨流程的持久性。

#### Usage Patterns

- Creates a text block object for inclusion in Anthropic API message requests
- Called by higher‑level protocol functions that assemble message content

### `clore::net::anthropic::protocol::detail::make_tool_result_block`

Declaration: `src/network/anthropic.cppm:107`

Definition: `src/network/anthropic.cppm:107`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::make_tool_result_block` 负责将给定的工具结果标识符（以 `const int &` 形式提供）转换为一个协议层面的工具结果块。该函数是为构建 Anthropic API 请求而设计的内部辅助函数，调用者需要确保传入的标识符有效且与后续的 `make_tool_use_block` 等函数的用法一致。返回的整数代表新创建的工具结果块的内部索引，可用于在请求组合中引用该块。调用者不应直接依赖返回值的具体含义，而应仅将其作为后续协议装配步骤的输入。

#### Usage Patterns

- Constructing tool result blocks in Anthropic API requests
- Used when assembling a conversation turn that includes tool result data

### `clore::net::anthropic::protocol::detail::make_tool_use_block`

Declaration: `src/network/anthropic.cppm:67`

Definition: `src/network/anthropic.cppm:67`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::detail::make_tool_use_block` 负责构建一个表示工具使用（tool use）块的协议元素，并返回其标识符。调用者必须提供一个 `const int &` 类型的参数，该参数通常代表与工具调用相关的配置索引或描述符。返回的 `int` 值可用于后续协议组装步骤，例如将此块嵌入消息结构，或检查操作结果的状态。此函数是 `clore::net::anthropic::protocol::detail` 命名空间中一系列块构建辅助函数（如 `make_tool_result_block`）的一部分，共同用于合成符合 Anthropic 协议要求的工具交互消息。

#### Usage Patterns

- 用于构建 Anthropic API 的 `tool_use` 块
- 被更高级别的消息构建函数调用

### `clore::net::anthropic::protocol::detail::parse_json_text`

Declaration: `src/network/anthropic.cppm:180`

Definition: `src/network/anthropic.cppm:180`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::parse_json_text` 接受两个 `std::string_view` 参数，并返回一个 `int`。该函数负责解析由这两个字符串视图所标识或构成的 JSON 文本，并将解析结果以整数值的形式返回给调用方。调用方应确保提供的两个字符串视图共同描述了一份合法的 JSON 内容，具体的解析语义和返回值含义由调用的上下文约定。

#### Usage Patterns

- Called to parse a JSON object string into a structured `json::Object` within the Anthropic protocol implementation

### `clore::net::anthropic::protocol::detail::validate_request`

Declaration: `src/network/anthropic.cppm:202`

Definition: `src/network/anthropic.cppm:202`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::detail::validate_request` 接受一个 `const int &` 类型的请求标识符并返回一个 `int` 状态码。调用者应提供有效的请求标识符，函数将执行协议层面的请求校验，返回值指示请求是否满足预期约束（通常为 0 表示成功，非零表示特定错误）。该函数是 `clore::net::anthropic::protocol::detail` 名称空间中协议处理的底层工具，不应对其做实现假设。

#### Usage Patterns

- Invoked to check validity of a `CompletionRequest` before further processing
- Typically called internally before sending a request to the Anthropic API

## Related Pages

- [Namespace clore::net::anthropic::protocol](../index.md)

