---
title: 'Namespace clore::net::anthropic::protocol::detail'
description: 'clore::net::anthropic::protocol::detail 命名空间封装了与 Anthropic 协议消息构造和解析相关的内部实现细节。它提供了一系列底层辅助函数，例如 make_text_block、parse_json_text、make_role_message、make_tool_use_block、make_tool_result_block、append_text_with_gap、format_schema_instruction 和 validate_request，这些函数通常通过不透明的整数句柄来操作消息块，并返回状态码或句柄值。此外，命名空间还包含一个 constexpr 常量 kDefaultMaxTokens（值为 2048），用作请求中令牌数的默认上限。'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol::detail`

## Summary

`clore::net::anthropic::protocol::detail` 命名空间封装了与 Anthropic 协议消息构造和解析相关的内部实现细节。它提供了一系列底层辅助函数，例如 `make_text_block`、`parse_json_text`、`make_role_message`、`make_tool_use_block`、`make_tool_result_block`、`append_text_with_gap`、`format_schema_instruction` 和 `validate_request`，这些函数通常通过不透明的整数句柄来操作消息块，并返回状态码或句柄值。此外，命名空间还包含一个 `constexpr` 常量 `kDefaultMaxTokens`（值为 2048），用作请求中令牌数的默认上限。

在架构上，该命名空间充当实现细节层，为协议中更高级别的序列化、消息组合和请求验证提供原子操作。调用者负责确保传入参数的合法性（如角色字符串的规范性、JSON 格式的正确性、句柄的有效性），并通过返回的整数值判断操作是否成功。这些函数不公开具体实现，外部模块不应直接依赖，而是通过协议提供的公共接口间接使用。

## Variables

### `clore::net::anthropic::protocol::detail::kDefaultMaxTokens`

Declaration: `network/anthropic.cppm:23`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::kDefaultMaxTokens` is a `constexpr std::uint32_t` constant initialized to `2048`, providing a default maximum token count for Anthropic protocol requests.

#### Usage Patterns

- Used as default maximum token value in `build_request_json`

## Functions

### `clore::net::anthropic::protocol::detail::append_text_with_gap`

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/append-text-with-gap.md)

此函数用于将一段文本以协议所需的格式追加到目标字符串中，同时在其前后或内部插入必要的间隔符。调用者应当提供一个可修改的 `std::string` 对象作为第一个参数，该对象通常代表正在构建的协议消息；第二个参数是一个 `std::string_view`，表示要追加的文本内容。函数不返回任何值，而是直接修改目标字符串，并且要求调用者确保目标字符串在调用前后保持有效状态。

#### Usage Patterns

- called by `build_request_json` to assemble request body
- used for appending text blocks with a separating gap

### `clore::net::anthropic::protocol::detail::format_schema_instruction`

Declaration: `network/anthropic.cppm:176`

Definition: `network/anthropic.cppm:176`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::format_schema_instruction` 接受一个 `const int&` 参数，并返回一个 `int`。该函数用于格式化模式指令（schema instruction），其具体行为由传入的整数参数决定。返回的整数表示格式化操作的状态或结果。调用者负责传入有效的引用，并根据返回值判断是否需要进一步处理。

#### Usage Patterns

- called to create the system instruction for a structured response from the model
- used when constructing a request with a specified response schema

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:154`

Definition: `network/anthropic.cppm:154`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

该函数根据提供的 `role` 字符串和消息内容构造一条角色消息，并返回该消息的整数标识符。调用者负责传入有效的角色值（如 `"user"`、`"assistant"`）以及对应的内容——可以是 `json::Array` 形式的多块内容，也可以是纯文本的 `std::string_view`。返回的标识符可用于后续引用或组合请求。该函数假定传入的内容已经符合 Anthropic 协议的消息结构，不执行内部的校验或格式化。

#### Usage Patterns

- Building request messages for the Anthropic API
- Internal helper for constructing role-based message objects

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:130`

Definition: `network/anthropic.cppm:130`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::make_role_message` 根据指定的角色和内容构造一条角色消息，用于 Anthropic 协议的请求构建。调用者需提供角色字符串（例如 `"user"` 或 `"assistant"`）和内容：内容可以是纯文本（`std::string_view`），也可以是 `json::Array` 形式的结构化数据。该函数返回一个整数标识符，代表所创建的消息，该标识符可用于后续协议构造（例如添加到消息序列中）。调用者负责确保角色和内容符合协议约定，且标识符的有效性在调用方上下文中维持。

#### Usage Patterns

- 构造简单文本角色消息
- 被上层消息构建函数调用

### `clore::net::anthropic::protocol::detail::make_text_block`

Declaration: `network/anthropic.cppm:35`

Definition: `network/anthropic.cppm:35`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::make_text_block` 将给定的字符串视图内容封装为一个文本块，并返回一个整数句柄。调用者必须确保传入的 `std::string_view` 在函数调用期间保持有效；函数不会延长字符串的生命周期。返回的整数值是对所创建文本块的不透明标识，不应被调用者直接解释或修改，而应传递给需要引用该文本块的后续操作（例如构造消息或请求）。

该函数的契约不保证对同一内容重复调用返回相同的整数值，也不承诺返回值一定非零或具有特定数值意义。调用者不应假设返回值与任何外部资源或状态直接对应。如果创建失败，返回值可能为零或表示错误的特殊值，具体行为由内部实现决定。

#### Usage Patterns

- called to create a text block in Anthropic message construction

### `clore::net::anthropic::protocol::detail::make_tool_result_block`

Declaration: `network/anthropic.cppm:98`

Definition: `network/anthropic.cppm:98`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

该函数构造一个表示工具结果的内容块，用于组装 Anthropic 协议消息。调用者提供对整数类型的常量引用（参数类型 `const int &`），该整数通常携带工具调用的标识符或结果索引。函数返回一个 `int` 值，表示操作结果或新块的句柄，调用者应据此确认块是否成功创建。该函数属于协议内部实现，不应被外部模块直接依赖，但在构建包含工具结果的消息流中可能被间接使用。

#### Usage Patterns

- 用于构建 Anthropic API 请求中的 `tool_result` 内容块
- 被上层消息组装函数调用以填充 tool 结果

### `clore::net::anthropic::protocol::detail::make_tool_use_block`

Declaration: `network/anthropic.cppm:58`

Definition: `network/anthropic.cppm:58`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::make_tool_use_block` 基于调用者提供的工具标识符构造一个工具使用块。调用者必须传入一个对整数的常量引用，该整数指示期望的工具调用；函数返回一个代表所构建块的整数标识符。该函数不接收输入内容或配置，调用者负责确保传入的标识符有效并与可用的工具声明对应。返回的整数可供后续协议序列化或由其他细节层函数使用。

#### Usage Patterns

- Converting a `ToolCall` into a JSON block for Anthropic request formatting
- Used alongside `make_tool_result_block` and `make_text_block` in request construction

### `clore::net::anthropic::protocol::detail::parse_json_text`

Declaration: `network/anthropic.cppm:171`

Definition: `network/anthropic.cppm:171`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::parse_json_text` 解析一个 JSON 文本，并返回一个表示解析结果的整数。该函数是 `clore::net::anthropic::protocol` 内部协议细节处理的一部分，供同命名空间下的其他函数（如 `make_text_block`、`append_text_with_gap` 等）在构建或处理消息块时调用。

调用者应提供两个 `std::string_view` 参数，通常第一个为待解析的 JSON 字符串，第二个为附加上下文（如标识符或类型指示）。返回的 `int` 值指示解析操作的状态，例如成功、失败或特定错误码。函数假定输入 JSON 符合预期格式，未定义行为可能发生在无效输入上。

#### Usage Patterns

- parsing raw JSON text with error context
- used in Anthropic protocol request/response handling

### `clore::net::anthropic::protocol::detail::validate_request`

Declaration: `network/anthropic.cppm:193`

Definition: `network/anthropic.cppm:193`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

函数 `clore::net::anthropic::protocol::detail::validate_request` 验证传入的请求参数，并返回一个表示验证结果的状态码。调用者应传入一个 `const int &` 类型的请求标识符或参数；函数根据内部规则判断其有效性，返回的整数值指示验证通过或失败（非零值可能代表具体错误）。该函数不修改传入参数，且未定义未验证的输入行为。

#### Usage Patterns

- 在发送请求前调用以验证 `CompletionRequest` 的完整性
- 作为协议层验证的入口点

## Related Pages

- [Namespace clore::net::anthropic::protocol](../index.md)

