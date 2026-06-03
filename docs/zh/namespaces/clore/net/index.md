---
title: 'Namespace clore::net'
description: 'clore::net 命名空间封装了与 LLM（大型语言模型）API 交互的核心网络层，职责包括异步请求的发起与完成、提供者能力的探测与请求适配、速率限制的初始化与清理，以及工具调用（Function Calling）的结构定义。它提供了 call_llm_async、call_completion_async 等模板函数用于执行异步调用，并通过 get_probed_capabilities、sanitize_request_for_capabilities 等函数确保请求与模型实际能力兼容。此外，initialize_llm_rate_limit / shutdown_llm_rate_limit 管理调用频率约束，validate_llm_provider_environment 检查运行环境配置，而 is_feature_rejection_error 和 parse_rejected_feature_from_error 则用于识别并提取提供商返回的功能拒绝错误。'
layout: doc
template: doc
---

# Namespace `clore::net`

## Summary

`clore::net` 命名空间封装了与 LLM（大型语言模型）API 交互的核心网络层，职责包括异步请求的发起与完成、提供者能力的探测与请求适配、速率限制的初始化与清理，以及工具调用（Function Calling）的结构定义。它提供了 `call_llm_async`、`call_completion_async` 等模板函数用于执行异步调用，并通过 `get_probed_capabilities`、`sanitize_request_for_capabilities` 等函数确保请求与模型实际能力兼容。此外，`initialize_llm_rate_limit` / `shutdown_llm_rate_limit` 管理调用频率约束，`validate_llm_provider_environment` 检查运行环境配置，而 `is_feature_rejection_error` 和 `parse_rejected_feature_from_error` 则用于识别并提取提供商返回的功能拒绝错误。

在架构上，`clore::net` 作为底层网络抽象层，向上层提供统一的 LLM 调用接口，隐藏不同提供商（如 GPT、Llama）的协议差异和限制。它通过 `CompletionRequest`、`PromptRequest`、`ToolChoice` 等数据类型标准化请求结构，并支持 Markdown 片段、JSON 等多种输出格式（由 `PromptOutputContract` 控制）。该命名空间还管理异步事件循环（`kota::event_loop`），使得调用者能够以非阻塞方式获取结果，并通过 `LLMError` 异常处理速率超限、功能拒绝等错误场景。

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

Declaration: `src/network/protocol.cppm:43`

Definition: `src/network/protocol.cppm:43`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `content` can be any valid `std::string` value, including empty strings.
- No operations or invariants beyond those of `std::string` are enforced.

#### Key Members

- `content` of type `std::string`

#### Usage Patterns

- Used as a payload type in `clore::net` messages to convey assistant-generated text.
- Likely to be serialized and deserialized as part of the network protocol.
- Can be assigned, copied, or moved like a regular `std::string` wrapper.

### `clore::net::AssistantOutput`

Declaration: `src/network/protocol.cppm:113`

Definition: `src/network/protocol.cppm:113`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `text` 和 `refusal` 可能同时为空
- `tool_calls` 可为空向量
- 未明确要求 `text` 与 `refusal` 互斥

#### Key Members

- `text`
- `refusal`
- `tool_calls`

#### Usage Patterns

- 用于解析助手 API 响应中的输出内容
- 调用方通常检查 `text` 或 `refusal` 以获取回复，或遍历 `tool_calls` 执行后续操作

### `clore::net::AssistantToolCallMessage`

Declaration: `src/network/protocol.cppm:47`

Definition: `src/network/protocol.cppm:47`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `content` may be empty (`std::nullopt`) or contain a non-empty string.
- `tool_calls` is always a valid vector, possibly empty.

#### Key Members

- `content`
- `tool_calls`

#### Usage Patterns

- Likely constructed when an assistant response includes both text and tool calls.
- Used in serialization/deserialization within `clore::net` protocol messages.

### `clore::net::CompletionRequest`

Declaration: `src/network/protocol.cppm:89`

Definition: `src/network/protocol.cppm:89`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `model` is default-initialized to an empty string
- `messages` is always present as a vector (can be empty)
- Optional fields (`response_format`, `tool_choice`, `parallel_tool_calls`) may be absent
- Struct is a value type with public members, no invariants enforced beyond type constraints

#### Key Members

- `model`
- `messages`
- `response_format`
- `tools`
- `tool_choice`
- `parallel_tool_calls`

#### Usage Patterns

- Constructed and filled with request data before being sent over the network
- Used as input to a completion service or handler
- Optional fields are omitted by default and set only when needed

### `clore::net::CompletionResponse`

Declaration: `src/network/protocol.cppm:119`

Definition: `src/network/protocol.cppm:119`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::net::ForcedFunctionToolChoice`

Declaration: `src/network/protocol.cppm:82`

Definition: `src/network/protocol.cppm:82`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The `name` member must be a valid string; no additional invariants are implied by the evidence.

#### Key Members

- `name`

#### Usage Patterns

- Used to specify a forced tool choice in network protocol contexts.
- Likely populated and passed as an argument or stored within a larger protocol message.

### `clore::net::FunctionToolDefinition`

Declaration: `src/network/protocol.cppm:69`

Definition: `src/network/protocol.cppm:69`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::net::LLMError`

Declaration: `src/network/http.cppm:37`

Definition: `src/network/http.cppm:37`

Implementation: [`Module http`](../../../modules/http/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- member `message` always contains a valid `std::string` (possibly empty)
- construction from `kota::error` ensures `message` reflects the content of `err.message()`
- default construction leaves `message` empty

#### Key Members

- `message`: the stored error description
- `LLMError()`: default constructor
- `LLMError(std::string msg)`: constructs from a string
- `LLMError(kota::error err)`: constructs from a `kota::error`

#### Usage Patterns

- returned as an error type from LLM‑related network operations
- interoperates with `kota::error` by converting its error information
- default‑constructed to represent an unset or placeholder error

#### Member Functions

##### `clore::net::LLMError::LLMError`

Declaration: `src/network/http.cppm:42`

Definition: `src/network/http.cppm:42`

Implementation: [`Module http`](../../../modules/http/index.md)

###### Declaration

```cpp
clore::net::LLMError::LLMError(std::string msg);
```

##### `clore::net::LLMError::LLMError`

Declaration: `src/network/http.cppm:44`

Definition: `src/network/http.cppm:44`

Implementation: [`Module http`](../../../modules/http/index.md)

###### Declaration

```cpp
clore::net::LLMError::LLMError(kota::error err);
```

##### `clore::net::LLMError::LLMError`

Declaration: `src/network/http.cppm:40`

Definition: `src/network/http.cppm:40`

Implementation: [`Module http`](../../../modules/http/index.md)

###### Declaration

```cpp
clore::net::LLMError::LLMError();
```

### `clore::net::Message`

Declaration: `src/network/protocol.cppm:57`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::net::ProbedCapabilities`

Declaration: `src/network/protocol.cppm:131`

Definition: `src/network/protocol.cppm:131`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- All four capability flags are `std::atomic<bool>`.
- Each flag defaults to `true`.
- Flags can be set or read without data races.

#### Key Members

- `supports_json_schema`
- `supports_tool_choice`
- `supports_parallel_tool_calls`
- `supports_tools`

#### Usage Patterns

- Used to record the results of capability probing during network handshake.
- Other parts of the code examine these flags to decide whether to enable certain features, such as tool use or JSON schema validation.

### `clore::net::PromptOutputContract`

Declaration: `src/network/protocol.cppm:98`

Definition: `src/network/protocol.cppm:98`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 枚举值只能为 0、1 或 2
- 底层类型为 `std::uint8_t`，保证大小 1 字节

#### Key Members

- `clore::net::PromptOutputContract::Unspecified`
- `clore::net::PromptOutputContract::Json`
- `clore::net::PromptOutputContract::Markdown`

#### Usage Patterns

- 用于指定提示输出格式的选项
- 在请求或配置中传递，控制输出序列化方式
- 可能被 `clore::net` 命名空间中的其他函数或类作为参数使用

#### Member Variables

##### `clore::net::PromptOutputContract::Json`

Declaration: `src/network/protocol.cppm:100`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

###### Declaration

```cpp
Json
```

##### `clore::net::PromptOutputContract::Markdown`

Declaration: `src/network/protocol.cppm:101`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

###### Declaration

```cpp
Markdown
```

##### `clore::net::PromptOutputContract::Unspecified`

Declaration: `src/network/protocol.cppm:99`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

###### Declaration

```cpp
Unspecified
```

### `clore::net::PromptRequest`

Declaration: `src/network/protocol.cppm:104`

Definition: `src/network/protocol.cppm:104`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `prompt` is always a valid string, default empty
- `response_format` and `tool_choice` are optional, may be `std::nullopt`
- `output_contract` defaults to `PromptOutputContract::Unspecified`
- No other invariant constraints are specified

#### Key Members

- `prompt`
- `response_format`
- `tool_choice`
- `output_contract`

#### Usage Patterns

- Used as a request structure in network protocol, likely serialized and transmitted
- Constructed by clients to specify a prompt along with optional formatting and tool selection
- Examined by servers to extract and process the prompt and associated options

### `clore::net::ResponseFormat`

Declaration: `src/network/protocol.cppm:63`

Definition: `src/network/protocol.cppm:63`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `strict` defaults to `true` when not explicitly set
- `name` is a non-optional string, but no length constraint is enforced by the type
- `schema` may be absent, leaving the format unspecified

#### Key Members

- `clore::net::ResponseFormat::name`
- `clore::net::ResponseFormat::schema`
- `clore::net::ResponseFormat::strict`

#### Usage Patterns

- Likely used as a member of protocol configuration or response definitions
- Aggregate initialization allows concise creation of a `ResponseFormat` instance
- The `strict` boolean enables conditional enforcement of schema validation

### `clore::net::SystemMessage`

Declaration: `src/network/protocol.cppm:28`

Definition: `src/network/protocol.cppm:28`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `content` 成员可以包含任意有效的 `std::string` 值

#### Key Members

- `content`：存储消息文本的字符串

#### Usage Patterns

- 其他代码创建 `SystemMessage` 对象并设置 `content` 后，通过网络发送或接收

### `clore::net::ToolCall`

Declaration: `src/network/protocol.cppm:36`

Definition: `src/network/protocol.cppm:36`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

结构体 `clore::net::ToolCall` 表示一次具体的工具调用，通常由语言模型在聊天完成响应中发起。它作为 `AssistantToolCallMessage` 的一部分，用于携带调用指定工具所需的标识、名称和参数等信息。在 `clore::net` 命名空间中，它配合 `ToolChoice` 及相关定义（如 `FunctionToolDefinition`、`ToolOutput`）共同实现工具使用的工作流。

#### Invariants

- No invariants are explicitly documented in the evidence.
- The relationship between `arguments_json` and `arguments` is not specified in the evidence.

#### Key Members

- `id`
- `name`
- `arguments_json`
- `arguments`

#### Usage Patterns

- No usage patterns are documented in the provided evidence.

### `clore::net::ToolChoice`

Declaration: `src/network/protocol.cppm:86`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::ToolChoice` 是一个类型别名，用于表示语言模型调用中工具选择的不同策略。它通常作为 `CompletionRequest` 或 `PromptRequest` 等请求结构中的字段出现，允许开发者控制模型何时以及如何调用已注册的函数工具。该类型内部组合了多种选择模式，例如自动决定 (`ToolChoiceAuto`)、强制使用指定工具 (`ForcedFunctionToolChoice`)、禁止工具调用 (`ToolChoiceNone`) 或要求必须使用工具 (`ToolChoiceRequired`)，从而提供灵活的工具行为控制。

### `clore::net::ToolChoiceAuto`

Declaration: `src/network/protocol.cppm:76`

Definition: `src/network/protocol.cppm:76`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::ToolChoiceAuto` 是一个标记结构体，用于表示在调用大型语言模型时，由模型自动决定是否以及如何调用工具。它是 `clore::net::ToolChoice` 类型别名所表示的变体之一，通常与 `clore::net::ToolChoiceNone`、`clore::net::ToolChoiceRequired` 和 `clore::net::ForcedFunctionToolChoice` 一起使用，以配置对话补全请求中的工具调用行为。当选择 `ToolChoiceAuto` 时，模型会根据输入内容自行判断是否需要调用可用工具，而不强制要求工具调用。

#### Invariants

- 类型是平凡的，默认构造函数、析构函数、拷贝/移动操作均为编译器生成且无操作
- 不包含任何数据成员，因此不占用额外存储（空基类优化可能适用）
- 对象实例之间完全等价，没有可区分的状态

#### Key Members

- 无成员、嵌套类型或方法

#### Usage Patterns

- 作为标签分派中的标签，在函数重载或模板特化中区分自动工具选择行为
- 可能用于 `ToolChoice` 变体或策略类中，表示需要自动推断工具的场景
- 与其他 `ToolChoice*` 类型（如 `ToolChoiceFunction`）形成标记系列，供使用者通过类型选择行为

### `clore::net::ToolChoiceNone`

Declaration: `src/network/protocol.cppm:80`

Definition: `src/network/protocol.cppm:80`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 不包含任何数据成员
- 不可包含状态

#### Usage Patterns

- 证据中未展示具体使用模式

### `clore::net::ToolChoiceRequired`

Declaration: `src/network/protocol.cppm:78`

Definition: `src/network/protocol.cppm:78`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- 对象始终为空，不包含运行时状态
- 构造、复制和销毁均为平凡操作
- 类型本身不施加任何约束

#### Usage Patterns

- 作为模板参数或函数重载的标签，以启用或选择工具选择行为
- 在其他协议相关类型中作为标记或策略类型使用

### `clore::net::ToolOutput`

Declaration: `src/network/protocol.cppm:126`

Definition: `src/network/protocol.cppm:126`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `tool_call_id` 应为非空字符串，以唯一标识一次工具调用
- `output` 可以为空字符串，表示无输出
- 两个字段均为必需，且直接映射到网络协议中的对应字段

#### Key Members

- `tool_call_id`：工具调用的标识符
- `output`：工具执行后的输出内容

#### Usage Patterns

- 作为消息体在网络协议中序列化和反序列化
- 在工具调用完成后，用于封装结果并传递给后续处理逻辑

### `clore::net::ToolResultMessage`

Declaration: `src/network/protocol.cppm:52`

Definition: `src/network/protocol.cppm:52`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::net::UserMessage`

Declaration: `src/network/protocol.cppm:32`

Definition: `src/network/protocol.cppm:32`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `content` 可以是任意合法的 `std::string` 值，包括空字符串

#### Key Members

- `content`

#### Usage Patterns

- 其他代码通过直接读写 `content` 成员来设置或获取用户消息内容
- 可能被序列化或直接通过网络发送

## Functions

### `clore::net::call_completion_async`

Declaration: `src/network/client.cppm:24`

Definition: `src/network/client.cppm:65`

Implementation: [`Module client`](../../../modules/client/index.md)

`clore::net::call_completion_async` 是一个模板函数，由调用者通过 `typename Protocol` 实例化。调用者必须提供一个 `int` 标识符（代表要完成的操作或请求）以及一个 `kota::event_loop *` 或 `kota::event_loop &` 作为异步执行的事件循环。若传入空指针，函数会内部选择默认事件循环。函数返回一个 `int` 状态码，指示操作是否成功提交；调用者应根据约定检查该值以判断后续行为。

#### Usage Patterns

- Used by higher-level completion functions to handle feature probing transparently
- Called with an event loop to schedule async work

### `clore::net::call_completion_async`

Declaration: `src/network/network.cppm:31`

Definition: `src/network/network.cppm:157`

Implementation: [`Module network`](../../../modules/network/index.md)

函数 `clore::net::call_completion_async` 启动一个异步完成调用。调用者必须提供一个标识符（int）和一个 `kota::event_loop`，该函数通过引用或指针接受事件循环。函数返回一个 int，表示调用结果或请求标识。调用者负责保证所提供的事件循环在操作完成前保持有效且处于运行状态。

#### Usage Patterns

- called by higher-level completion `APIs` to initiate an LLM request asynchronously
- used in contexts where an event loop drives the asynchronous workflow

### `clore::net::call_llm_async`

Declaration: `src/network/client.cppm:35`

Definition: `src/network/client.cppm:165`

Implementation: [`Module client`](../../../modules/client/index.md)

`clore::net::call_llm_async` 是一个模板函数，用于向指定的 LLM 提供商发起异步请求。它接受三个 `std::string_view` 参数（分别表示提供商标识、模型名称和提示文本）以及一个指向 `kota::event_loop` 的可选指针。该函数返回一个 `int`，通常为 `0` 表示成功，或负数表示错误代码。调用者负责确保提供的字符串在异步操作完成前保持有效，并可通过传入 `nullptr` 或有效的事件循环指针来控制异步执行环境。

#### Usage Patterns

- used to asynchronously generate text from an LLM
- called with a model identifier, system prompt, and user prompt
- used within event-loop-driven applications

### `clore::net::call_llm_async`

Declaration: `src/network/client.cppm:28`

Definition: `src/network/client.cppm:146`

Implementation: [`Module client`](../../../modules/client/index.md)

启动目标模型（如 llama 或 gpt）的异步 LLM 调用。接受一个协议标识字符串、一个输入提示字符串、一个超时整数（秒）以及一个指向 `kota::event_loop` 的可选指针。当 `event_loop` 指针为空时，行为等同于传递当前线程的默认事件循环。返回一个整数，表示异步操作的句柄或错误码；调用者应通过 `event_loop` 监控该句柄的完成状态。该函数是模板化的，底层协议由隐式模板参数 `Protocol` 确定，调用者无需显式指定。

#### Usage Patterns

- 作为网络层公共接口，用于发起需要异步等待的 LLM 调用
- 通常被更上层的封装函数或业务逻辑直接 `co_await` 使用
- 配合错误处理、取消捕获等协程机制
- 重载版本接受 `int` 类型的请求，但证据中提供的实现使用 `PromptRequest`

### `clore::net::call_llm_async`

Declaration: `src/network/network.cppm:25`

Definition: `src/network/network.cppm:133`

Implementation: [`Module network`](../../../modules/network/index.md)

发起一个对大型语言模型的异步调用。调用者需提供目标模型标识符、输入文本提示、超时值（以秒为单位的 `int`）以及一个 `kota::event_loop &` 引用，该引用将用于调度回调和处理完成事件。函数返回一个 `int` 值，该值可用于后续通过 `clore::net::call_completion_async` 等函数跟踪请求的进度或检索结果。如果调用因速率限制、功能拒绝或其他环境问题而失败，函数会抛出 `clore::net::LLMError` 异常。

#### Usage Patterns

- call LLM asynchronously with a structured prompt request
- used in event-loop-based async workflows for text generation
- integrated with error handling via `LLMError`

### `clore::net::call_structured_async`

Declaration: `src/network/client.cppm:42`

Definition: `src/network/client.cppm:186`

Implementation: [`Module client`](../../../modules/client/index.md)

该函数是一个模板，用于发起一次异步结构化 LLM 调用。调用者必须指定 `Protocol` 和 `T`，分别表示结构化数据交换的协议（如预定义格式或模式）以及预期的返回类型。前三个 `std::string_view` 参数分别代表目标标识、模型名称和请求文本（或提示）。最后一个参数是一个指向 `kota::event_loop` 的指针，该事件循环必须保持有效且处于运行状态，直到请求完成或关联的回调被处理。函数返回一个 `int`，可用作请求句柄或表示提交状态。调用者应确保事件循环指针非空且生命周期覆盖整个异步操作。

#### Usage Patterns

- Calling an LLM with a structured output schema enforced by `response_format`
- Chaining `call_completion_async` followed by response parsing to obtain a typed result
- Using `kota::task` coroutine to await the external I/O operation

### `clore::net::get_probed_capabilities`

Declaration: `src/network/protocol.cppm:138`

Definition: `src/network/protocol.cppm:741`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

函数 `clore::net::get_probed_capabilities` 接受一个标识符字符串（通常与 LLM 端点或模型对应），并返回该标识符关联的已探测能力集合的可修改引用。调用者可通过此引用检查端点支持的特性（如功能标记），或将其传递给 `sanitize_request_for_capabilities` 等函数以对齐或调整后续请求参数。返回的引用指向内部存储的能力数据，在程序生命周期或直到重新探测前有效；修改此引用可能会影响后续请求的行为，调用者应谨慎操作。

#### Usage Patterns

- Used to obtain a cached `ProbedCapabilities` instance without repeated probing
- Callers pass a key derived from model, provider, or endpoint to reuse capability results

### `clore::net::icontains`

Declaration: `src/network/protocol.cppm:780`

Definition: `src/network/protocol.cppm:780`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

Declaration: [Declaration](functions/icontains.md)

`clore::net::icontains` 检查第一个 `std::string_view` 是否包含第二个 `std::string_view`，比较时不区分大小写。如果第二个参数是第一个参数的子串（忽略 ASCII 大小写）则返回 `true`，否则返回 `false`。调用者提供两个视图，函数不修改它们也不要求它们以空字符结尾。

#### Usage Patterns

- Used by `clore::net::is_feature_rejection_error` to perform case-insensitive substring matching on error strings.

### `clore::net::initialize_llm_rate_limit`

Declaration: `src/network/http.cppm:33`

Definition: `src/network/http.cppm:93`

Implementation: [`Module http`](../../../modules/http/index.md)

该函数初始化 LLM 请求的速率限制子系统。调用者必须传入一个 `std::uint32_t` 参数，该参数通常表示每秒允许的最大请求数量或某个时间窗口内的请求阈值。调用 `clore::net::initialize_llm_rate_limit` 是使用任何后续 LLM 调用（如 `clore::net::call_llm_async`）的先决条件，这些调用将受此速率限制的约束。该函数应在任何速率限制的 LLM 调用之前恰好调用一次；重复调用可能导致未定义的行为或重新配置限制。资源清理由对应的 `clore::net::shutdown_llm_rate_limit` 函数负责，调用者在不再需要速率限制时应当调用该函数。

#### Usage Patterns

- called to configure the maximum concurrent LLM requests
- used in initialization code

### `clore::net::is_feature_rejection_error`

Declaration: `src/network/protocol.cppm:147`

Definition: `src/network/protocol.cppm:800`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

检查给定的错误字符串是否表示某个被请求的 LLM 特性遭到下游提供方拒绝。返回 `true` 当字符串匹配常见的拒绝模式；否则返回 `false`。配合 `clore::net::parse_rejected_feature_from_error` 使用可从错误消息中提取被拒绝的具体特性名称。

#### Usage Patterns

- Determining if an LLM API error is a feature rejection
- Filtering errors that signal unsupported parameters before retrying
- Classifying network errors for structured error handling

### `clore::net::make_capability_probe_key`

Declaration: `src/network/protocol.cppm:140`

Definition: `src/network/protocol.cppm:755`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

函数 `clore::net::make_capability_probe_key` 根据三个字符串片段构造一个用于能力探测（capability probe）的键。调用方负责提供有效的 `std::string_view` 参数，这些参数共同唯一标识一次探测请求（例如模型、提供方和端点）；返回的 `std::string` 可安全用作映射键或缓存键，以便后续查询或去重探测结果。该函数不执行 I/O 或修改外部状态，其行为完全由输入决定。

#### Usage Patterns

- 用于生成 `ProbedCapabilities` 缓存的查找键
- 与其他 `std::unordered_map` 配合记录已探测的能力

### `clore::net::make_markdown_fragment_request`

Declaration: `src/network/protocol.cppm:111`

Definition: `src/network/protocol.cppm:156`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::make_markdown_fragment_request` 接受一个 `std::string` 并返回一个 `PromptRequest`。调用者提供一个纯文本输入，该函数构造一个专门用于生成 Markdown 片段响应的请求对象。返回的 `PromptRequest` 可直接传递给下游的 LLM 调用函数（例如 `call_llm_async` 或 `call_completion_async`），并预期会得到一个格式化为 Markdown 的答复。

该函数不执行网络调用或验证输入内容；它仅封装请求结构，使调用者无需手动组装 `PromptRequest` 所需的元信息。传入的字符串会作为该请求的有效载荷部分，用于描述需要转换为 Markdown 片段的内容。

#### Usage Patterns

- create a request for markdown fragment
- initialize `PromptRequest` with markdown contract

### `clore::net::parse_rejected_feature_from_error`

Declaration: `src/network/protocol.cppm:149`

Definition: `src/network/protocol.cppm:819`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

`clore::net::parse_rejected_feature_from_error` 接受一个表示错误消息的 `std::string_view` 参数，并尝试从中解析出被拒绝的功能名称。它返回一个 `std::optional<std::string>`：如果错误消息表明某个特定功能因策略或限制被拒绝，则返回该功能的名称；否则返回 `std::nullopt`。

调用者应仅在已确认错误属于功能拒绝类型时使用此函数，例如结合 `is_feature_rejection_error` 的判断。返回的字符串不含原始错误消息的额外内容，可直接用于标识被禁用的功能。

#### Usage Patterns

- Used to parse LLM error responses to identify which requested feature was rejected
- Called by error handling logic to determine feature-specific rejection information

### `clore::net::sanitize_request_for_capabilities`

Declaration: `src/network/protocol.cppm:144`

Definition: `src/network/protocol.cppm:761`

Implementation: [`Module protocol`](../../../modules/protocol/index.md)

函数 `clore::net::sanitize_request_for_capabilities` 接受一个原始的 `CompletionRequest` 和通过 `clore::net::get_probed_capabilities` 获得的 `ProbedCapabilities`，返回一个经过调整的 `CompletionRequest`。调用者负责提供请求和对应的能力信息；该函数会移除或修改请求中与已探测能力不兼容的部分，确保后续处理不会因不受支持的特性而拒绝。

#### Usage Patterns

- 在构造 LLM 调用请求后，传递完能力探测结果后调用，以清除不支持的选项
- 作为前置处理步骤，位于 `call_completion_async` 或类似调用之前

### `clore::net::shutdown_llm_rate_limit`

Declaration: `src/network/http.cppm:35`

Definition: `src/network/http.cppm:277`

Implementation: [`Module http`](../../../modules/http/index.md)

函数 `clore::net::shutdown_llm_rate_limit` 用于关闭之前通过 `clore::net::initialize_llm_rate_limit` 启动的 LLM 速率限制机制。此函数不接收任何参数，返回 `void`，并且声明为 `noexcept`，因此调用者可以在任何上下文中安全地调用它，无需处理异常。调用后，所有与 LLM 调用相关的速率限制状态和资源将被清理，使系统回到未初始化速率限制的状态。此函数是速率限制生命周期管理中的对称关闭操作，应在不再需要速率限制时调用，或作为程序正常关闭的一部分。

#### Usage Patterns

- Called during shutdown to disable LLM rate limiting
- Used to reset the rate limiter state when reinitializing

### `clore::net::validate_llm_provider_environment`

Declaration: `src/network/network.cppm:35`

Definition: `src/network/network.cppm:125`

Implementation: [`Module network`](../../../modules/network/index.md)

调用方应在使用任何 LLM 相关功能之前调用 `clore::net::validate_llm_provider_environment`，以确认当前运行环境已正确配置并满足所用提供者的要求。该函数不接受任何参数，并返回一个 `int` 值：零表示环境有效，非零表示存在不可恢复的配置或连接问题。调用方应检查返回值并在失败时中止后续 LLM 操作，或提供适当的用户反馈。

#### Usage Patterns

- Called to verify LLM provider configuration before making API requests
- Used to pre-check environment setup for LLM operations

## Related Pages

- [Namespace clore](../index.md)
- [Namespace clore::net::anthropic](anthropic/index.md)
- [Namespace clore::net::detail](detail/index.md)
- [Namespace clore::net::openai](openai/index.md)
- [Namespace clore::net::protocol](protocol/index.md)
- [Namespace clore::net::schema](schema/index.md)

