---
title: 'Namespace clore::net'
description: 'clore::net 命名空间封装了与外部大语言模型（LLM）服务的网络通信逻辑，负责构建请求、管理异步调用、探测模型能力以及解析常见响应错误。该命名空间定义了一套完整的消息类型体系（如 SystemMessage、UserMessage、AssistantMessage、ToolResultMessage 等）并通过 Message 变体统一管理对话上下文；同时提供 CompletionRequest、PromptRequest 等请求结构以灵活指定模型、工具选择策略和输出格式。核心异步函数如 call_llm_async、call_completion_async、call_structured_async 依赖 kota::event_loop 驱动非阻塞操作，并配合 initialize_llm_rate_limit / shutdown_llm_rate_limit 实现请求频率控制。ProbedCapabilities 与 sanitize_request_for_capabilities 确保请求与目标提供者的能力匹配，而 is_feature_rejection_error 和 parse_rejected_feature_from_error 辅助处理特性被拒绝时的诊断信息。整体上，clore::net 作为网络抽象层，隐藏了底层协议差异，集中处理 LLM 交互中的会话管理、速率限制和错误兜底，是上层业务逻辑与模型服务之间的关键桥梁。'
layout: doc
template: doc
---

# Namespace `clore::net`

## Summary

`clore::net` 命名空间封装了与外部大语言模型（LLM）服务的网络通信逻辑，负责构建请求、管理异步调用、探测模型能力以及解析常见响应错误。该命名空间定义了一套完整的消息类型体系（如 `SystemMessage`、`UserMessage`、`AssistantMessage`、`ToolResultMessage` 等）并通过 `Message` 变体统一管理对话上下文；同时提供 `CompletionRequest`、`PromptRequest` 等请求结构以灵活指定模型、工具选择策略和输出格式。核心异步函数如 `call_llm_async`、`call_completion_async`、`call_structured_async` 依赖 `kota::event_loop` 驱动非阻塞操作，并配合 `initialize_llm_rate_limit` / `shutdown_llm_rate_limit` 实现请求频率控制。`ProbedCapabilities` 与 `sanitize_request_for_capabilities` 确保请求与目标提供者的能力匹配，而 `is_feature_rejection_error` 和 `parse_rejected_feature_from_error` 辅助处理特性被拒绝时的诊断信息。整体上，`clore::net` 作为网络抽象层，隐藏了底层协议差异，集中处理 LLM 交互中的会话管理、速率限制和错误兜底，是上层业务逻辑与模型服务之间的关键桥梁。

## Diagram

```mermaid
graph TD
    NS["net"]
    T0["AssistantMessage"]
    NS --> T0
    T1["AssistantOutput"]
    NS --> T1
    T2["AssistantToolCallMessage"]
    NS --> T2
    T3["CompletionRequest"]
    NS --> T3
    T4["CompletionResponse"]
    NS --> T4
    T5["ForcedFunctionToolChoice"]
    NS --> T5
    T6["FunctionToolDefinition"]
    NS --> T6
    T7["LLMError"]
    NS --> T7
    T8["Message"]
    NS --> T8
    T9["ProbedCapabilities"]
    NS --> T9
    T10["PromptOutputContract"]
    NS --> T10
    T11["PromptRequest"]
    NS --> T11
    T12["ResponseFormat"]
    NS --> T12
    T13["SystemMessage"]
    NS --> T13
    T14["ToolCall"]
    NS --> T14
    T15["ToolChoice"]
    NS --> T15
    T16["ToolChoiceAuto"]
    NS --> T16
    T17["ToolChoiceNone"]
    NS --> T17
    T18["ToolChoiceRequired"]
    NS --> T18
    T19["ToolOutput"]
    NS --> T19
    T20["ToolResultMessage"]
    NS --> T20
    T21["UserMessage"]
    NS --> T21
    NSC0["anthropic"]
    NS --> NSC0
    NSC1["detail"]
    NS --> NSC1
    NSC2["openai"]
    NS --> NSC2
    NSC3["protocol"]
    NS --> NSC3
    NSC4["schema"]
    NS --> NSC4
```

## Subnamespaces

- [`clore::net::anthropic`](anthropic/index.md)
- [`clore::net::detail`](detail/index.md)
- [`clore::net::openai`](openai/index.md)
- [`clore::net::protocol`](protocol/index.md)
- [`clore::net::schema`](schema/index.md)

## Types

### `clore::net::AssistantMessage`

Declaration: `network/protocol.cppm:31`

Definition: `network/protocol.cppm:31`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::AssistantMessage` 是一个结构体，用于表示大型语言模型（LLM）在对话中作为助手角色生成的回复消息。它通常包含模型输出的文本内容以及可能调用的工具（函数）信息。在 `clore::net::Message` 类型别名定义的变体集合中，`AssistantMessage` 与 `SystemMessage`、`UserMessage`、`ToolResultMessage` 等共同构成完整的对话消息序列。当构造 `CompletionRequest` 或 `PromptRequest` 时，`AssistantMessage` 用于传递来自之前模型调用的助手响应，以维持对话上下文。

#### Invariants

- The `content` member holds a valid `std::string` which may be empty.

#### Key Members

- `clore::net::AssistantMessage::content`

#### Usage Patterns

- Used as a simple value type to represent assistant messages in network protocols.
- Likely serialized or deserialized for transmission over the network.

### `clore::net::AssistantOutput`

Declaration: `network/protocol.cppm:101`

Definition: `network/protocol.cppm:101`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `text` 和 `refusal` 不能同时为非空（通常只有一个有值）
- `tool_calls` 可以为空

#### Key Members

- `text`
- `refusal`
- `tool_calls`

#### Usage Patterns

- 作为 `clore::net::Assistant` 的返回值
- 由 `clore::net::Assistant` 的 `generate` 或类似方法构造

### `clore::net::AssistantToolCallMessage`

Declaration: `network/protocol.cppm:35`

Definition: `network/protocol.cppm:35`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `content` 可能为空（`std::nullopt`）
- `tool_calls` 可为空向量

#### Key Members

- `content`
- `tool_calls`

#### Usage Patterns

- 作为 `AssistantMessage` 的一部分使用
- 用于处理助手生成的多工具调用响应

### `clore::net::CompletionRequest`

Declaration: `network/protocol.cppm:77`

Definition: `network/protocol.cppm:77`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::net::CompletionResponse`

Declaration: `network/protocol.cppm:107`

Definition: `network/protocol.cppm:107`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `raw_json` 包含与结构化字段对应的完整原始JSON
- `id` 和 `model` 与响应来源一致
- `message` 是从原始JSON解析得到的结构化表示

#### Key Members

- `id`
- `model`
- `message`
- `raw_json`

#### Usage Patterns

- 作为网络请求完成后解析结果的载体
- 由 `raw_json` 反序列化填充，供上层应用使用 `message`

### `clore::net::ForcedFunctionToolChoice`

Declaration: `network/protocol.cppm:70`

Definition: `network/protocol.cppm:70`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::ForcedFunctionToolChoice` 是 `ToolChoice` 类型别名所表示的变体之一，用于强制语言模型调用一个特定的函数工具。当构造 `CompletionRequest` 时，将 `tool_choice` 字段设为该结构体的实例，可以要求模型必须调用由某个 `FunctionToolDefinition` 指定的工具，而不会自行选择“无工具”或“自动选择”。该类型通常与 `ToolChoiceAuto`、`ToolChoiceNone` 和 `ToolChoiceRequired` 一同用于精确控制模型在对话响应中对函数工具的调用行为。

#### Invariants

- `name` 字段应包含有效的工具函数名称

#### Key Members

- `std::string name`

#### Usage Patterns

- 作为工具选择策略的一种强制模式，在其他代码中通过赋值 `name` 来使用

### `clore::net::FunctionToolDefinition`

Declaration: `network/protocol.cppm:57`

Definition: `network/protocol.cppm:57`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::net::LLMError`

Declaration: `network/http.cppm:23`

Definition: `network/http.cppm:23`

Implementation: [`Module http`](../../../modules/http/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` 始终包含有效的错误描述字符串
- 默认构造的 `message` 为空字符串

#### Key Members

- `message`
- 默认构造函数 `LLMError()`
- `explicit LLMError(std::string msg)`
- `explicit LLMError(kota::error err)`

#### Usage Patterns

- 作为函数返回值表示 LLM 操作失败
- 从 `kota::error` 或字符串直接构造
- 通过 `message` 成员获取错误详情

#### Member Functions

##### `clore::net::LLMError::LLMError`

Declaration: `network/http.cppm:30`

Definition: `network/http.cppm:30`

Implementation: [`Module http`](../../../modules/http/index.md)

###### Declaration

```cpp
clore::net::LLMError::LLMError(kota::error err);
```

##### `clore::net::LLMError::LLMError`

Declaration: `network/http.cppm:28`

Definition: `network/http.cppm:28`

Implementation: [`Module http`](../../../modules/http/index.md)

###### Declaration

```cpp
clore::net::LLMError::LLMError(std::string msg);
```

##### `clore::net::LLMError::LLMError`

Declaration: `network/http.cppm:26`

Definition: `network/http.cppm:26`

Implementation: [`Module http`](../../../modules/http/index.md)

###### Declaration

```cpp
clore::net::LLMError::LLMError();
```

### `clore::net::Message`

Declaration: `network/protocol.cppm:45`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::Message` 是聊天对话中所有可能消息类型的变体类型，用于统一表示 `SystemMessage`、`UserMessage`、`AssistantMessage`、`ToolResultMessage`、`AssistantToolCallMessage` 等角色特定的消息。通过该别名，代码可以将不同消息结构体以类型安全的方式组合，并借助模式匹配（如 `std::visit`）按实际消息类型分派处理逻辑，简化了多消息类型的存储与传递。

#### Invariants

- Always contains exactly one of the five alternative types
- Each alternative is a distinct message kind in the protocol

#### Key Members

- `SystemMessage`
- `UserMessage`
- `AssistantMessage`
- `AssistantToolCallMessage`
- `ToolResultMessage`

#### Usage Patterns

- Used as a generic message type in network communication
- Passed to `std::visit` to handle each message kind
- Stored in queues or passed as function arguments

### `clore::net::ProbedCapabilities`

Declaration: `network/protocol.cppm:119`

Definition: `network/protocol.cppm:119`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All members are atomically updatable and initially default to `true`.

#### Key Members

- `supports_json_schema`
- `supports_tool_choice`
- `supports_parallel_tool_calls`
- `supports_tools`

#### Usage Patterns

- Used to track which capabilities are available after probing an LLM endpoint.
- Members are atomically read and written by different threads during capability probing.

### `clore::net::PromptOutputContract`

Declaration: `network/protocol.cppm:86`

Definition: `network/protocol.cppm:86`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

证据不足，无法总结；请提供更多证据。

#### Invariants

- 枚举值仅限于 `Unspecified`、`Json`、`Markdown`
- 底层类型为 `std::uint8_t`

#### Key Members

- `Unspecified`
- `Json`
- `Markdown`

#### Usage Patterns

- 用于指定或查询输出格式协议
- 可转换为 `std::uint8_t` 进行序列化或比较

#### Member Variables

##### `clore::net::PromptOutputContract::Json`

Declaration: `network/protocol.cppm:88`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

###### Declaration

```cpp
Json
```

##### `clore::net::PromptOutputContract::Markdown`

Declaration: `network/protocol.cppm:89`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

###### Declaration

```cpp
Markdown
```

##### `clore::net::PromptOutputContract::Unspecified`

Declaration: `network/protocol.cppm:87`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

###### Declaration

```cpp
Unspecified
```

### `clore::net::PromptRequest`

Declaration: `network/protocol.cppm:92`

Definition: `network/protocol.cppm:92`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `prompt` defaults to empty string; no non-empty guarantee is enforced.
- `output_contract` always has a value, defaulting to `Unspecified`.
- `response_format` and `tool_choice` are optional and may be `std::nullopt`.

#### Key Members

- `prompt`: the main input text.
- `response_format`: optional expected format of the response.
- `tool_choice`: optional tool selection for the model.
- `output_contract`: contract specifying output structure.

#### Usage Patterns

- Constructed with prompt string and optionally set `response_format`, `tool_choice`, or `output_contract`.
- Passed as argument to functions that send requests to a language model service.
- Typically aggregated into higher-level request structures or serialized for network transmission.

### `clore::net::ResponseFormat`

Declaration: `network/protocol.cppm:51`

Definition: `network/protocol.cppm:51`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 如果 `schema` 存在，则必须是一个合法的 JSON Object
- `strict` 控制验证行为的严格程度

#### Key Members

- `name`
- `schema`
- `strict`

#### Usage Patterns

- 作为 HTTP 响应解析的配置选项
- 在客户端请求中指定期望的响应格式

### `clore::net::SystemMessage`

Declaration: `network/protocol.cppm:16`

Definition: `network/protocol.cppm:16`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- content 成员始终有效，但可能为空字符串
- 没有额外的约束条件

#### Key Members

- content: 存储系统消息文本的字符串

#### Usage Patterns

- 作为网络协议中系统消息的数据载体
- 可被序列化或直接访问 content 成员

### `clore::net::ToolCall`

Declaration: `network/protocol.cppm:24`

Definition: `network/protocol.cppm:24`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `arguments` 是 `arguments_json` 的解析结果
- `arguments_json` 与 `arguments` 保持语义一致

#### Key Members

- `id`
- `name`
- `arguments_json`
- `arguments`

#### Usage Patterns

- 作为工具调用消息的组成部分传递
- 通过 `arguments_json` 进行序列化，通过 `arguments` 进行结构化访问

### `clore::net::ToolChoice`

Declaration: `network/protocol.cppm:74`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::ToolChoice` 是一个类型别名，用于表示在 Completion 或 Prompt 请求中控制语言模型如何选择调用工具的策略。它通常是一个可辨识联合类型，涵盖 `clore::net::ToolChoiceAuto`、`clore::net::ToolChoiceNone`、`clore::net::ToolChoiceRequired` 以及 `clore::net::ForcedFunctionToolChoice` 等选项，从而允许调用方分别指定自动决定、禁止工具、强制工具调用或强制调用特定函数。

### `clore::net::ToolChoiceAuto`

Declaration: `network/protocol.cppm:64`

Definition: `network/protocol.cppm:64`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::ToolChoiceAuto` 是一个用于指定模型自动决定是否需要调用工具的选项。当作为 `ToolChoice` 的一种变体使用时，它指示语言模型根据输入自行判断是否需要使用已注册的工具，以及选择哪个工具来执行任务。该结构体通常被填入 `CompletionRequest` 或 `PromptRequest` 的 `tool_choice` 字段，以启用模型自主的工具选择行为。

#### Invariants

- No invariants; it is an empty struct with no data or behavior.

#### Usage Patterns

- Used as a type tag in template parameters to indicate automatic behavior.
- Likely used in conjunction with other `ToolChoice` types for selection logic.

### `clore::net::ToolChoiceNone`

Declaration: `network/protocol.cppm:68`

Definition: `network/protocol.cppm:68`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::ToolChoiceNone` 是一个用于指示不调用任何工具的空结构体。它通常作为 `clore::net::ToolChoice` 联合体（variant）的一个替代项，与 `clore::net::ToolChoiceAuto`、`clore::net::ToolChoiceRequired` 以及 `clore::net::ForcedFunctionToolChoice` 并列，用于在请求中显式禁用工具调用。当希望模型仅生成自然语言回复而不执行任何函数时，可以使用该类型。

#### Invariants

- Empty struct with no state
- Default-constructible

#### Key Members

- (none)

#### Usage Patterns

- Used as a type in a variant to indicate no tool selected
- Likely a sentinel or tag in generic code

### `clore::net::ToolChoiceRequired`

Declaration: `network/protocol.cppm:66`

Definition: `network/protocol.cppm:66`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- Default constructible
- Trivially copyable

#### Usage Patterns

- Passed as a template argument to specialize behavior
- Used as a function parameter type to enforce tool choice requirement

### `clore::net::ToolOutput`

Declaration: `network/protocol.cppm:114`

Definition: `network/protocol.cppm:114`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- Both `tool_call_id` and `output` are free-form strings with no specified constraints.

#### Key Members

- `clore::net::ToolOutput::tool_call_id`
- `clore::net::ToolOutput::output`

#### Usage Patterns

- Constructed and populated by tool execution logic.
- Serialized or consumed as part of a larger protocol message.

### `clore::net::ToolResultMessage`

Declaration: `network/protocol.cppm:40`

Definition: `network/protocol.cppm:40`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 字段均为字符串类型
- 无默认值或约束条件

#### Key Members

- `tool_call_id`
- `content`

#### Usage Patterns

- 作为消息的一部分在网络协议中传递
- 在工具调用完成后填充结果

### `clore::net::UserMessage`

Declaration: `network/protocol.cppm:20`

Definition: `network/protocol.cppm:20`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- content 可以包含任意字符串，没有格式或长度约束

#### Key Members

- content

#### Usage Patterns

- 作为网络协议中用户消息的有效负载类型
- 在发送或接收消息时用于携带文本数据

## Functions

### `clore::net::call_completion_async`

Declaration: `network/client.cppm:16`

Definition: `network/client.cppm:57`

Implementation: [`Module client`](../../../modules/client/index.md)

函数 `clore::net::call_completion_async` 是一个模板函数，用于异步发起补全请求。调用者需传入一个整数参数（通常表示请求标识或配置）以及一个指向 `kota::event_loop` 的可选指针。若指针为 `nullptr`，函数将使用由 `clore::net::detail::select_event_loop` 选择的默认事件循环。返回值是一个整数，指示操作的启动结果或返回句柄。

模板参数 `Protocol` 指定用于处理请求的协议类型。调用者必须保证在异步操作完成之前，所关联的 `kota::event_loop` 对象保持有效且不被销毁。该函数不提供同步等待机制；完成状态通过事件循环调度回调传递。

#### Usage Patterns

- Called as a coroutine with `co_await`
- Used to perform completion requests with automatic capability probing
- Integrates with `clore::net::detail::perform_http_request_async` for HTTP transport

### `clore::net::call_completion_async`

Declaration: `network/network.cppm:24`

Definition: `network/network.cppm:150`

Implementation: [`Module network`](../../../modules/network/index.md)

`clore::net::call_completion_async` 启动一次异步完成调用。调用者必须提供一个整数参数（通常代表请求句柄或上下文标识符）以及一个 `kota::event_loop` 的引用或指针；该事件循环用于调度异步操作的完成回调。函数返回一个整数值，可用于检查操作状态或跟踪结果。调用者应确保所传递的 `kota::event_loop` 在异步操作完成前保持有效。

#### Usage Patterns

- Called to perform an asynchronous LLM completion with automatic provider detection from environment
- Typically used in async contexts that provide a `kota::event_loop`

### `clore::net::call_llm_async`

Declaration: `network/network.cppm:18`

Definition: `network/network.cppm:126`

Implementation: [`Module network`](../../../modules/network/index.md)

`clore::net::call_llm_async` 启动一个针对 LLM 服务的异步调用。调用者需传入两个 `std::string_view` 参数（分别表示请求内容和目标模型或端点标识）、一个 `int` 参数（指定超时或限流令牌），以及一个 `kota::event_loop &` 引用（用于驱动异步事件循环）。函数返回一个 `int` 值，成功时代表异步操作的标识符（后续可用于查询状态或取消），失败时返回负错误码。调用者有责任确保 `kota::event_loop` 在调用期间保持有效并处于运行状态，且在调用前应通过 `clore::net::validate_llm_provider_environment` 完成环境校验。本函数不负责同步等待结果；结果可通过事件循环的回调机制（如 `kota::event_loop` 上的事件）获取。

#### Usage Patterns

- 异步调用 LLM 生成文本
- 配合事件循环使用
- 错误处理通过 `LLMError`

### `clore::net::call_llm_async`

Declaration: `network/client.cppm:20`

Definition: `network/client.cppm:137`

Implementation: [`Module client`](../../../modules/client/index.md)

`clore::net::call_llm_async` 是一个模板函数，用于发起一次异步的大语言模型（LLM）调用。调用者需要提供协议类型 `Protocol`，该类型决定了与哪个 LLM 服务进行交互（例如 `OpenAI` 或 Anthropic）。此函数会立即返回一个整型标识符，可用于后续跟踪请求状态或取消操作；实际响应通过 `kota::event_loop` 异步交付。

函数的第一个 `std::string_view` 参数通常指定端点或模型标识符，第二个 `std::string_view` 参数是输入提示（prompt），第三个 `int` 参数表示请求超时时间（单位通常为毫秒），最后一个参数是 `kota::event_loop*` 指针——若传入空指针，函数内部会退而使用当前线程的事件循环默认实例。返回值 `int` 表示请求的句柄（非零表示成功发起，零或负值可能表示调用层错误）；调用者应确保事件循环在请求完成前保持活跃。
---

#### Usage Patterns

- Used by callers requiring asynchronous text completion from an LLM
- Often invoked with a custom event loop for non-blocking I/O

### `clore::net::call_llm_async`

Declaration: `network/client.cppm:27`

Definition: `network/client.cppm:156`

Implementation: [`Module client`](../../../modules/client/index.md)

发起一个异步 LLM 调用，该调用由 `Protocol` 模板参数指定的协议驱动。函数接受三个字符串视图：第一个通常标识 LLM 提供商或端点，第二个指定模型名称，第三个为请求负载；此外还需提供一个指向 `kota::event_loop` 的指针，用于调度回调。返回一个 `int` 值，表示该异步操作的标识符，调用者可利用此标识符（例如与 `shutdown_llm_rate_limit` 配合）管理调用状态。调用者必须确保 `event_loop` 指针非空且指向一个活跃的事件循环，因为函数内部会通过 `detail::select_event_loop` 获取其引用。

#### Usage Patterns

- 用于异步获取 LLM 生成的文本回复
- 与其他 `call_llm_async` 重载类似，但支持自定义事件循环

### `clore::net::call_structured_async`

Declaration: `network/client.cppm:34`

Definition: `network/client.cppm:177`

Implementation: [`Module client`](../../../modules/client/index.md)

`clore::net::call_structured_async` 是一个函数模板，模板参数 `Protocol` 和 `T` 分别指定通信协议和结构化输出类型。调用者无需在参数中显式提供这些类型，它们通常由编译器从调用上下文推导或由调用者显式指定。

该函数期望四个参数：三个 `std::string_view` 分别表示 LLM 提供者标识、模型名称和输入提示文本，以及一个指向 `kota::event_loop*` 的非空指针，用于调度异步操作的完成事件。返回值 `int` 指示调用是否成功启动：`0` 表示操作已排队，非零值表示错误码。调用者必须确保 `loop` 指向的事件循环在操作完成前保持存活，且返回的错误码应通过 `clore::net::LLMError` 或类似机制转换以获取人类可读消息。

#### Usage Patterns

- Used to make structured asynchronous LLM calls where the response is parsed into a specific C++ type.
- Typically invoked with a concrete `Protocol` and `T` type arguments.
- Often called within other coroutines or event loop contexts.

### `clore::net::get_probed_capabilities`

Declaration: `network/protocol.cppm:126`

Definition: `network/protocol.cppm:725`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

函数 `clore::net::get_probed_capabilities` 接受一个 `std::string_view` 标识符，并返回对其内部缓存的 `ProbedCapabilities` 对象的引用。调用者可以通过该引用查询或修改与特定标识符相关联的探测能力集合，例如在调用 `clore::net::sanitize_request_for_capabilities` 之前获取该能力集。返回的引用在程序生命周期内保持有效，标识符的格式应与 `clore::net::validate_llm_provider_environment` 及其他能力相关函数期望的值一致。若标识符未被识别，行为由实现定义（可能抛出异常或创建默认能力集）。

#### Usage Patterns

- used to lazily initialize and retrieve per-provider capability probes
- called before constructing requests that depend on provider capabilities

### `clore::net::icontains`

Declaration: `network/protocol.cppm:758`

Definition: `network/protocol.cppm:758`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Declaration: [Declaration](functions/icontains.md)

函数 `clore::net::icontains` 判断第一个字符串是否不区分大小写地包含第二个字符串。它接受两个 `std::string_view` 参数，并返回 `bool` 值：当且仅当 `input` 中（忽略大小写后）包含 `substring` 时返回 `true`。该函数是纯查询操作，不修改参数内容，其用途包括错误消息中的模式匹配，例如在 `clore::net::is_feature_rejection_error` 中用于检测特征拒绝模式。

#### Usage Patterns

- Used by `is_feature_rejection_error` to check if an error message contains a substring case-insensitively

### `clore::net::initialize_llm_rate_limit`

Declaration: `network/http.cppm:19`

Definition: `network/http.cppm:78`

Implementation: [`Module http`](../../../modules/http/index.md)

函数 `clore::net::initialize_llm_rate_limit` 负责为 LLM 请求设置速率限制机制。调用者必须提供一个非零的 `std::uint32_t` 参数，表示每秒允许的最大请求数。该函数应于任何涉及 LLM 通信的网络操作之前调用，且对应的 `clore::net::shutdown_llm_rate_limit` 应在后续适当时机以清理内部状态。此声明只定义初始化契约；实际限流语义（如是否针对每端点、是否支持动态调整）取决于底层实现。

#### Usage Patterns

- Called during initialization to set the concurrency limit for LLM requests
- Used to enable or disable rate limiting by passing zero

### `clore::net::is_feature_rejection_error`

Declaration: `network/protocol.cppm:131`

Definition: `network/protocol.cppm:778`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

函数 `clore::net::is_feature_rejection_error` 接受一个 `std::string_view`，返回 `bool`，用于判断给定的错误消息是否表示一个“特性被拒绝”的异常。如果错误消息包含指示 LLM 拒绝所请求特性的典型短语，则返回 `true`；否则返回 `false`。该函数是纯检查性质的，不修改任何状态，调用者可在决定如何解析或处理错误时使用它，以区分特性拒绝与其他类型的错误。

#### Usage Patterns

- Determines if an LLM error response corresponds to a feature rejection (e.g., unsupported parameter)

### `clore::net::make_markdown_fragment_request`

Declaration: `network/protocol.cppm:99`

Definition: `network/protocol.cppm:140`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

函数 `clore::net::make_markdown_fragment_request` 接受一个 `std::string` 参数（代表 markdown 片段的内容），并返回一个 `PromptRequest` 对象。调用者负责提供该字符串；函数将其封装为可在后续 LLM 交互中使用的请求结构。该函数不验证输入内容的业务意义，仅负责构造请求格式。

#### Usage Patterns

- Used to create a request for markdown-only responses from LLM
- Called when the caller expects the response to be formatted as markdown

### `clore::net::parse_rejected_feature_from_error`

Declaration: `network/protocol.cppm:133`

Definition: `network/protocol.cppm:797`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::parse_rejected_feature_from_error` 接受一个字符串视图类型的错误消息，并尝试从中解析出被拒绝的特征标识。如果输入确实对应一个特征拒绝错误，返回值会包含该特征的名称或说明；否则返回空值。

调用者应当先通过 `clore::net::is_feature_rejection_error` 确认错误属于特征拒绝类型，再使用本函数提取被拒绝的特征信息。函数返回的 `std::optional<std::string>` 在不含有效特征时保持为空，调用者需检查其是否有值。

#### Usage Patterns

- Used in error handling to identify rejected features from LLM provider responses

### `clore::net::sanitize_request_for_capabilities`

Declaration: `network/protocol.cppm:128`

Definition: `network/protocol.cppm:739`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::sanitize_request_for_capabilities` 接收一个 `CompletionRequest` 和一组 `ProbedCapabilities`，并返回一个经过调整的 `CompletionRequest`。该函数确保请求中只包含由探测到的能力所支持的特性，从而消除因请求不兼容而引发的运行时错误。调用方应在向提供者发起实际调用之前使用此函数，以获取一个符合目标端点实际能力的请求副本。

#### Usage Patterns

- Called before sending a completion request to ensure compatibility with provider capabilities
- Used to prune unsupported features like JSON schema, tool choice, parallel tool calls, and tools

### `clore::net::shutdown_llm_rate_limit`

Declaration: `network/http.cppm:21`

Definition: `network/http.cppm:223`

Implementation: [`Module http`](../../../modules/http/index.md)

调用 `clore::net::shutdown_llm_rate_limit` 用于终止当前 LLM 请求的速率限制机制。此函数必须在先前调用 `clore::net::initialize_llm_rate_limit` 初始化之后调用，以便安全地释放相关内部状态。在调用之后，任何后续的 LLM 异步调用不再受速率限制约束。此函数不接收参数且不抛出异常，适合在清理或重置环境中使用。

#### Usage Patterns

- 应用关闭时调用
- 重置速率限制状态时调用

### `clore::net::validate_llm_provider_environment`

Declaration: `network/network.cppm:28`

Definition: `network/network.cppm:118`

Implementation: [`Module network`](../../../modules/network/index.md)

调用 `clore::net::validate_llm_provider_environment` 以确认当前运行环境（例如所需的配置、凭证或环境变量）是否满足 LLM 提供者的要求。该函数不接收参数并返回一个 `int` 状态码：零表示环境有效，非零值表示缺失或不正确的配置，调用方应据此决定是否继续执行后续的 LLM 调用。此函数是使用任何 LLM 服务前的先决条件检查。

#### Usage Patterns

- Called before making LLM API calls to ensure provider is configured

## Related Pages

- [Namespace clore](../index.md)
- [Namespace clore::net::anthropic](anthropic/index.md)
- [Namespace clore::net::detail](detail/index.md)
- [Namespace clore::net::openai](openai/index.md)
- [Namespace clore::net::protocol](protocol/index.md)
- [Namespace clore::net::schema](schema/index.md)

