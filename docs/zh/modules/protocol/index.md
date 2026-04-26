---
title: 'Module protocol'
description: '该模块定义了与大型语言模型（LLM）交互所需的协议层，涵盖了请求与响应的数据结构、工具调用机制以及输出格式验证。其公共接口包括 CompletionRequest、CompletionResponse、PromptRequest、SystemMessage、UserMessage、AssistantMessage、ToolCall、ToolOutput 等核心类型，以及用于工具解析、响应提取、能力探测和输出契约检查的独立函数（如 parse_tool_arguments、text_from_response、get_probed_capabilities、validate_markdown_fragment_output 等）。detail 子命名空间提供了 JSON 验证与克隆工具及视图类型，作为协议实现的基础支撑。'
layout: doc
template: doc
---

# Module `protocol`

## Summary

该模块定义了与大型语言模型（LLM）交互所需的协议层，涵盖了请求与响应的数据结构、工具调用机制以及输出格式验证。其公共接口包括 `CompletionRequest`、`CompletionResponse`、`PromptRequest`、`SystemMessage`、`UserMessage`、`AssistantMessage`、`ToolCall`、`ToolOutput` 等核心类型，以及用于工具解析、响应提取、能力探测和输出契约检查的独立函数（如 `parse_tool_arguments`、`text_from_response`、`get_probed_capabilities`、`validate_markdown_fragment_output` 等）。`detail` 子命名空间提供了 JSON 验证与克隆工具及视图类型，作为协议实现的基础支撑。

该模块封装了协议的构造、校验与转换逻辑，使调用方能够以类型安全的方式组合消息、配置工具选择策略、解析模型输出，并根据探测到的提供者能力调整请求。通过 `Message` 变体类型统一管理多种角色消息，`ToolChoice` 别名为工具调用策略提供灵活选择。同时，该模块负责解析特征拒绝错误、推断输出合约，并提供了同步/异步请求的简化入口，从而建立了一个完整的 LLM 通信协议实现层。

## Imports

- [`http`](../http/index.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`agent:tools`](../agent/tools.md)
- [`anthropic`](../anthropic/index.md)
- [`client`](../client/index.md)
- [`generate:cache`](../generate/cache.md)
- [`generate:scheduler`](../generate/scheduler.md)
- [`openai`](../openai/index.md)
- [`provider`](../provider/index.md)
- [`schema`](../schema/index.md)

## Types

### `clore::net::AssistantMessage`

Declaration: `network/protocol.cppm:31`

Definition: `network/protocol.cppm:31`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::AssistantMessage` 的内部实现仅包含一个 `std::string content` 字段，用于存储助手消息的文本负载。该字段是唯一的数据成员，因此该结构体本质上是一个围绕 `std::string` 的轻微封装，不维护额外的内部不变性 —— 消息内容的有效性完全依赖于调用方构造时所赋予的字符串值。所有必要的构造函数、赋值操作以及析构行为均由编译器隐式生成，无需自定义实现。作为 POD-like 类型，其内存布局简单连续，可直接进行内存拷贝或序列化操作。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体是一个纯聚合，通过三个公有字段表示不同类型的输出：`text`、`refusal` 和 `tool_calls`。内部仅依赖 `std::optional` 和 `std::vector` 管理存储，不引入额外的不变量——默认构造后所有字段均为空，而每个字段的 `std::optional` 语义允许调用方独立设置任意子集。`tool_calls` 采用 `std::vector` 以实现零开销的未调用状态（空向量）以及运行时的动态增长，与 `text` 和 `refusal` 的可选字符串保持一致的零开销默认表示。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

struct `clore::net::AssistantToolCallMessage` 是一个简单的聚合类型，内部仅包含两个数据字段。`content` 是一个 `std::optional<std::string>`，用于容纳可选的文本回复，其值可为空或包含一个非空字符串。`tool_calls` 是一个 `std::vector<ToolCall>`，存储并发执行的工具调用序列。该结构不提供自定义构造函数、析构函数或赋值运算符，因此完全依赖编译器生成的默认实现。不变量要求 `tool_calls` 中的每个元素都是有效构造的 `ToolCall` 对象，且 `content` 与 `tool_calls` 之间没有隐含的互斥约束——二者可以同时有值（表示混合响应），也可以其中之一为空。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体是一个纯数据聚合，其所有字段均直接映射 HTTP API 的请求参数。`model` 存储模型标识符（默认为空字符串），`messages` 保存对话历史消息列表，不支持单独表示。`tools` 是一个函数工具定义列表，默认为空；`tool_choice` 和 `parallel_tool_calls` 通过 `std::optional` 表示可选性，分别控制工具选择模式与是否允许并行工具调用。`response_format` 用于指定输出格式（如 JSON 对象或文本）。该结构体未定义任何自定义构造函数、赋值运算符或成员函数，因此依赖编译器生成的默认语义，并强制所有字段处于有效但可能未充分初始化的状态——调用方需自行保证 `model` 非空且 `messages` 不空才能构建有意义的请求。

### `clore::net::CompletionResponse`

Declaration: `network/protocol.cppm:107`

Definition: `network/protocol.cppm:107`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::CompletionResponse` 是一个简单的数据聚合结构体，它将完成 API 响应的关键部分直接存储在四个公共字段中：`id`、`model`、`message`（类型为 `AssistantOutput`）以及原始 JSON 字符串 `raw_json`。由于所有字段均可直接访问，该结构体不维护任何内部不变量，其设计意图是作为解析后的响应容器，同时保留 `raw_json` 以供需要原始数据的调用者使用。没有自定义构造函数或成员函数，因此其行为完全由聚合初始化决定。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ForcedFunctionToolChoice` 是一个轻量包装，其唯一内部状态是一个 `std::string` 类型的成员 `name`，用于存储强制选择的工具名称。该结构体不包含任何自定义构造函数、析构函数或赋值操作，完全依赖编译器生成的特殊成员函数，因此其行为与一个普通的字符串包装器一致。一个隐含的不变式是 `name` 应当非空，因为强制选择语义要求指定一个具体的工具；尽管代码本身不强制此检查，调用方应保证其有效性。该结构体以最小开销实现了模型的强制选择模式，所有操作均直接委托给底层 `std::string`。

#### Invariants

- `name` 字段应包含有效的工具函数名称

#### Key Members

- `std::string name`

#### Usage Patterns

- 作为工具选择策略的一种强制模式，在其他代码中通过赋值 `name` 来使用

### `clore::net::FunctionToolDefinition`

Declaration: `network/protocol.cppm:57`

Definition: `network/protocol.cppm:57`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体是一个纯数据聚合体，包含四个公开成员：`name` 与 `description` 均为 `std::string` 类型，分别存储工具的标识名称与人类可读描述；`parameters` 为 `kota::codec::json::Object` 类型，用于保存工具调用所需的 JSON Schema 参数定义；`strict` 为 `bool` 类型且默认初始化为 `true`，表示工具执行时默认启用严格模式。所有成员均为直接访问，无显式构造函数或成员函数，也不存在额外的内部不变式约束，其完整性仅依赖外部写入的数值有效性。

### `clore::net::Message`

Declaration: `network/protocol.cppm:45`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::Message` 是一个类型别名，其底层类型为 `std::variant`，包含五种具体消息类型：`SystemMessage`、`UserMessage`、`AssistantMessage`、`AssistantToolCallMessage` 和 `ToolResultMessage`。该别名将不同类型的消息统一为单一值类型，利用 `std::variant` 的存储和访问机制实现类型安全的联合体。

实现层面，该变体不保证各备选类型的大小对齐以外的不变量，但通过 `std::variant` 的运行时代理确保每次只有一个活跃成员。关键成员实现继承自 `std::variant` 的构造函数、赋值操作符和访问器，其中 `std::visit` 是常用的模式匹配入口；没有自定义析构、拷贝或移动逻辑，完全依赖标准库的默认实现。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构的内部表示完全由四个 `std::atomic<bool>` 成员组成，分别是 `supports_json_schema`、`supports_tool_choice`、`supports_parallel_tool_calls` 和 `supports_tools`，所有成员均在定义处初始化为 `true`。这种设计使对象天生是轻量级且可共享的——只要 `ProbedCapabilities` 实例存活，每个成员都可以通过原子操作单独读取或写入，无需额外同步。初始全 `true` 的默认状态体现了乐观假设：在远端能力被实际探测之前，所有特性都被视为支持；探测逻辑随后可以独立地将任一标志置为 `false` 以反映实际限制。由于结构体所有成员均为 public 且无自定义构造函数，它也是一个聚合类型，支持花括号初始化或指定初始化器语法（C++20），但实践中更常见的用法是通过成员访问直接修改原子值。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::PromptOutputContract` 是一个基于 `std::uint8_t` 的强类型枚举，用于紧凑地表示提示输出的格式约定。其成员 `Unspecified`、`Json` 和 `Markdown` 按此顺序隐式赋值为 0、1、2，形成了从未指定到具体格式的自然线性顺序。该枚举仅作为轻量级区分器，不附加任何方法或额外存储，确保在协议层传递时开销最小。值的单调递增顺序为向下兼容的扩展提供了基础，同时底层无符号整数类型保证了跨平台布局的一致性。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
Json
```

##### `clore::net::PromptOutputContract::Markdown`

Declaration: `network/protocol.cppm:89`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
Markdown
```

##### `clore::net::PromptOutputContract::Unspecified`

Declaration: `network/protocol.cppm:87`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
Unspecified
```

### `clore::net::PromptRequest`

Declaration: `network/protocol.cppm:92`

Definition: `network/protocol.cppm:92`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构是一个简单的聚合体，包含四个数据成员，用于封装向 LLM 端点发起提示请求所需的所有参数。其中 `prompt` 是必需的字符串，存储提示文本；`response_format` 和 `tool_choice` 是可选的，分别表示期望的响应格式和工具选择；`output_contract` 默认为 `PromptOutputContract::Unspecified`，显式指定输出契约类型。所有可选字段均使用 `std::optional` 包装，允许调用方按需提供配置，而默认值则简化了常见用例。该结构不维护额外的不变式，仅作为值类型承载请求负载，其设计意图是清晰分离必选与可选参数，并避免在协议层进行复杂的初始化逻辑。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ResponseFormat` 是一个简单聚合体，包含三个数据成员：`name`（类型 `std::string`）标识响应格式的名称；`schema`（类型 `std::optional<kota::codec::json::Object>`）保存可选的 JSON 模式定义；`strict`（类型 `bool`，默认值为 `true`）控制格式匹配的严格程度。该结构体的各成员均通过直接初始化提供，未定义任何用户提供的构造函数、析构函数或赋值运算符，因此其生存期管理完全依赖编译器生成的默认操作。

在实现层面，`strict` 的默认值 `true` 是最重要的不变量：当使用者未显式设置该字段时，内部始终采用严格匹配模式。`schema` 的 `std::optional` 封装允许在无模式约束时通过空值表示，避免动态分配或特殊哨兵。所有成员均保持公开访问，结构体本身不维护额外的不变量或校验逻辑，其正确性由调用方保证。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

实现上，`clore::net::SystemMessage` 仅包含一个 `std::string` 类型的数据成员 `content`，构成一个平凡的聚合体。结构体未定义任何用户提供的构造函数、析构函数或赋值运算符，因此依赖编译器合成的默认特殊成员函数。所有成员函数均来自隐式生成，使得对象可以默认初始化（`content` 为空字符串）、逐成员复制与移动，以及通过聚合初始化语法直接构造。不变量仅受 `std::string` 自身的语义约束——`content` 始终为一个合法的 UTF-8 字符串（未指定编码时视为字节序列）。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

内部结构由四个字段构成：`id`、`name`、`arguments_json` 和 `arguments`。前三个均为 `std::string`，最后一个为 `kota::codec::json::Value`，用于存储已解析的 JSON 参数。`arguments_json` 与 `arguments` 之间隐含一个同步不变式：后者应始终是对前者 JSON 内容的解析结果；任何对 `arguments_json` 的写入或更新都必须相应重建 `arguments`，反之对 `arguments` 的修改也应序列化回 `arguments_json`。此外，`id` 和 `name` 作为调用标识与函数名，在构造后通常不应更改。由于缺少 RAII 成员或自定义构造函数，该结构体依赖外部代码来维护该不变式，因此在序列化、反序列化或手动修改字段时需要显式同步。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoice` 是 `std::variant` 的类型别名，其备选类型包括 `ToolChoiceAuto`、`ToolChoiceRequired`、`ToolChoiceNone` 和 `ForcedFunctionToolChoice`。该别名直接复用标准库 `variant` 的实现机制——内部通过联合体（union）存储一个备选类型的实例，并通过索引或 `std::in_place_type` 标签确保类型安全。variant 的析构函数、移动构造函数和赋值运算符自动管理当前活跃对象的生命周期，不引入额外的虚表或堆分配。开发者可通过 `std::get`、`std::get_if` 或 `std::visit` 访问具体值，这些操作在编译时验证类型有效性，而运行时则通过索引检查避免未定义行为。

### `clore::net::ToolChoiceAuto`

Declaration: `network/protocol.cppm:64`

Definition: `network/protocol.cppm:64`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体是一个空类型，内部不包含任何数据成员、基类或成员函数。此实现选择将其用作标记类型或标签（tag），在 `clore::net` 命名空间中可能用于模板特化、重载决议或策略选择。由于没有成员，其语义完全由编译器生成的默认构造函数、析构函数、拷贝与移动操作提供，且不施加任何额外的不变式。

#### Invariants

- No invariants; it is an empty struct with no data or behavior.

#### Usage Patterns

- Used as a type tag in template parameters to indicate automatic behavior.
- Likely used in conjunction with other `ToolChoice` types for selection logic.

### `clore::net::ToolChoiceNone`

Declaration: `network/protocol.cppm:68`

Definition: `network/protocol.cppm:68`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoiceNone` 是一个空标记结构体，本身不包含任何数据成员或成员函数。其内部结构为空，不维护任何不变量（因为没有状态需要保护）。所有成员实现均不存在，这一类型纯粹用作类型标签，在模板或重载决议中标识“不进行工具选择”的策略。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoiceRequired` 是一个没有任何数据成员或基类的空结构体，其实现完全平凡。该类型不维护任何不变量或运行时状态，仅作为一个明确的类型标识或标签，用于在 `clore::net` 命名空间的类型系统中区分特定的语义选项。

#### Invariants

- Default constructible
- Trivially copyable

#### Usage Patterns

- Passed as a template argument to specialize behavior
- Used as a function parameter type to enforce tool choice requirement

### `clore::net::ToolOutput`

Declaration: `network/protocol.cppm:114`

Definition: `network/protocol.cppm:114`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolOutput` 是一个简单的聚合体，用于存储单个工具调用的执行结果。其内部仅包含两个 `std::string` 字段：`tool_call_id` 用于标识对应的工具调用请求，`output` 则保存工具返回的文本输出。该结构体没有定义任何构造函数或赋值操作，完全依赖编译器生成的默认成员函数，因此不变量仅由外部使用方保证——在有效使用中，`tool_call_id` 应匹配某个已发出的工具调用标识符，而 `output` 应包含该次调用的完整响应内容。作为一个值类型，它主要用于在网络层与上层逻辑之间传递工具调用的结果，并可通过简单的成员访问完成数据的读取与写入。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolResultMessage` 的内部结构由两个 `std::string` 成员组成：`tool_call_id` 和 `content`，按此顺序声明。作为聚合类型，它支持聚合初始化，成员按声明顺序初始化，且由于没有用户提供的构造函数、析构函数或拷贝/移动赋值运算符，它保持了平凡的拷贝、移动和析构语义。结构体未定义额外的成员函数或友元，其所有数据成员均为公有，使得 `std::is_aggregate_v<ToolResultMessage>` 为真，并允许结构化绑定。不变量方面，`tool_call_id` 用于关联特定的工具调用请求，但结构体本身不强制任何非空或格式约束——调用方负责确保该标识符与 `clore::net::ToolCallMessage` 中的 `tool_call_id` 对应。`content` 可以是任意文本，包括空字符串，表示工具执行返回的结果或错误信息。这种简单的值语义设计使其可以高效地在网络层传递和序列化。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::UserMessage` 的实现仅包含一个 `public` 数据成员 `content`，其类型为 `std::string`。该结构体未显式定义任何构造函数、析构函数或赋值运算符，因此编译器会为其隐式生成默认构造、析构、拷贝、移动及赋值等特殊成员函数，这些函数均以平凡的逐成员方式实现。固有的不变量仅在于 `content` 可以持有任意合法的 `std::string` 值（包括空串），不存在额外的验证或约束逻辑。此设计将 `UserMessage` 定位为单纯的数据容器，所有操作均委托给标准字符串类型的语义，不引入任何自定义的行为或资源管理。

#### Invariants

- content 可以包含任意字符串，没有格式或长度约束

#### Key Members

- content

#### Usage Patterns

- 作为网络协议中用户消息的有效负载类型
- 在发送或接收消息时用于携带文本数据

### `clore::net::detail::ArrayView`

Declaration: `network/protocol.cppm:174`

Definition: `network/protocol.cppm:174`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

struct `clore::net::detail::ArrayView` 是一个轻量级视图类型，内部仅包含一个指向 `const kota::codec::json::Array` 的非拥有指针 `value`。所有成员函数（`empty`、`size`、`begin`、`end`、`operator[]`、`operator->` 和 `operator*`）均直接转发到底层数组的对应操作，不执行任何额外验证或资源管理。该结构的唯一不变量是 `value` 在调用任何成员时必须指向一个有效对象，否则将导致未定义行为。其设计目的是为 JSON 数组提供零开销的只读访问，尤其适合作为迭代器或范围访问的轻量代理。

#### Invariants

- `value` must point to a valid, non-null `kota::codec::json::Array`
- the underlying array must outlive the view
- the view provides read-only access only

#### Key Members

- `value` field
- `operator[]`
- `begin` / `end`
- `size`
- `empty`

#### Usage Patterns

- passing a reference to a JSON array without copying
- iterating over array elements using range-for loops
- returning a non-owning view from functions to avoid lifetime issues

#### Member Functions

##### `clore::net::detail::ArrayView::begin`

Declaration: `network/protocol.cppm:185`

Definition: `network/protocol.cppm:185`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto begin() const noexcept {
        return value->begin();
    }
```

##### `clore::net::detail::ArrayView::empty`

Declaration: `network/protocol.cppm:177`

Definition: `network/protocol.cppm:177`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto empty() const noexcept -> bool {
        return value->empty();
    }
```

##### `clore::net::detail::ArrayView::end`

Declaration: `network/protocol.cppm:189`

Definition: `network/protocol.cppm:189`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto end() const noexcept {
        return value->end();
    }
```

##### `clore::net::detail::ArrayView::operator*`

Declaration: `network/protocol.cppm:201`

Definition: `network/protocol.cppm:201`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator*() const noexcept -> const kota::codec::json::Array& {
        return *value;
    }
```

##### `clore::net::detail::ArrayView::operator->`

Declaration: `network/protocol.cppm:197`

Definition: `network/protocol.cppm:197`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator->() const noexcept -> const kota::codec::json::Array* {
        return value;
    }
```

##### `clore::net::detail::ArrayView::operator[]`

Declaration: `network/protocol.cppm:193`

Definition: `network/protocol.cppm:193`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator[](std::size_t index) const -> const kota::codec::json::Value& {
        return (*value)[index];
    }
```

##### `clore::net::detail::ArrayView::size`

Declaration: `network/protocol.cppm:181`

Definition: `network/protocol.cppm:181`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto size() const noexcept -> std::size_t {
        return value->size();
    }
```

### `clore::net::detail::ObjectView`

Declaration: `network/protocol.cppm:152`

Definition: `network/protocol.cppm:152`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

结构体 `clore::net::detail::ObjectView` 是一个非拥有视图，其唯一成员 `value` 为指向 `kota::codec::json::Object` 的指针，默认初始化为 `nullptr`。它通过 `operator*` 和 `operator->` 提供对底层对象的直接解引用，并委托 `begin` 和 `end` 来暴露底层对象的迭代器接口。核心成员 `get` 接受 `std::string_view` 键并返回 `std::optional<kota::codec::json::Cursor>`，在内部查询 `value` 所指向的 JSON 对象，为键查找提供了类型安全的包装。该结构体假定 `value` 在被调用任何非空方法前均已指向一个有效的 `Object`，否则解引用或迭代器操作会导致未定义行为；它自身不负责生命周期管理。

#### Invariants

- `value` must point to a valid `kota::codec::json::Object` when accessing its members
- The underlying object is not owned by `ObjectView` and must outlive the view

#### Key Members

- `value`: pointer to the underlying JSON object
- `get(std::string_view)`: retrieves cursor for a key
- `begin()` / `end()`: iteration over object entries
- `operator->()` / `operator*()`: direct access to the underlying object

#### Usage Patterns

- Used to pass a JSON object by reference with a consistent interface
- Provides safe key-based access via `get()` without exposing the raw pointer
- Can be iterated using range-based for loops

#### Member Functions

##### `clore::net::detail::ObjectView::begin`

Declaration: `network/protocol.cppm:157`

Definition: `network/protocol.cppm:157`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto begin() const noexcept {
        return value->begin();
    }
```

##### `clore::net::detail::ObjectView::end`

Declaration: `network/protocol.cppm:161`

Definition: `network/protocol.cppm:161`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto end() const noexcept {
        return value->end();
    }
```

##### `clore::net::detail::ObjectView::get`

Declaration: `network/protocol.cppm:155`

Definition: `network/protocol.cppm:276`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto ObjectView::get(std::string_view key) const -> std::optional<json::Cursor> {
    auto* item = value->find(key);
    if(item == nullptr) {
        return std::nullopt;
    }
    return item->cursor();
}
```

##### `clore::net::detail::ObjectView::operator*`

Declaration: `network/protocol.cppm:169`

Definition: `network/protocol.cppm:169`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator*() const noexcept -> const kota::codec::json::Object& {
        return *value;
    }
```

##### `clore::net::detail::ObjectView::operator->`

Declaration: `network/protocol.cppm:165`

Definition: `network/protocol.cppm:165`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator->() const noexcept -> const kota::codec::json::Object* {
        return value;
    }
```

## Functions

### `clore::net::detail::clone_array`

Declaration: `network/protocol.cppm:264`

Definition: `network/protocol.cppm:438`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::clone_array` 创建一个新的 `json::Array` 作为传入 `source` 所引用的底层数组的副本。内部控制流极为简单：它解引用 `source` 的 `value` 成员（一个指向底层数组的指针），并用其构造并返回一个 `std::expected<json::Array, LLMError>`。第二个参数（一个 `std::string_view` 上下文标识符）未被使用。该操作依赖于 `json::Array` 的复制构造函数，因此结果是底层数组的独立拷贝。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- source`.value` (pointer to the underlying `json::Array`)

#### Usage Patterns

- deep copy of array data from an `ArrayView`
- cloning an array for further processing or modification

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:258`

Definition: `network/protocol.cppm:442`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function directly invokes the copy constructor of `json::Object` to produce a duplicate of the source JSON object, then returns the result wrapped in a success value of `std::expected<json::Object, LLMError>`. No iteration, validation, or error handling is performed within the function itself, making it a straightforward passthrough that relies entirely on the copy semantics of the underlying JSON library. The unused `std::string_view` parameter is ignored, indicating the function is designed to accept a context label for diagnostic purposes without affecting the cloning logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- source (const `json::Object` &)

#### Usage Patterns

- Called when a mutable or independently owned copy of an object is required
- Used in contexts where the original object must remain unchanged after modification of the copy

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:261`

Definition: `network/protocol.cppm:447`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数根据给定的 `ObjectView` 实例创建一个 `json::Object` 的深层副本。`ObjectView` 是一个轻量包装类型，其 `value` 成员保存指向底层 `json::Object` 的指针。`clone_object` 通过解引用该指针并调用 `json::Object` 的复制构造函数，返回一个包含该副本的 `std::expected<json::Object, LLMError>`。第二个参数 `std::string_view` 在实现中未被使用，可能是为将来的错误上下文预留。整个操作不涉及条件分支或迭代，依赖仅限于 `ObjectView` 和 `json::Object` 的复制语义。

#### Side Effects

- Allocates a new `json::Object` instance by copying the source object.

#### Reads From

- `source.value` (the `json::Object` pointed to by the `ObjectView`)

#### Writes To

- The newly constructed `json::Object` that is returned.

#### Usage Patterns

- Clone a JSON object for mutation or independent ownership.
- Used when a separate copy of an object view is needed.

### `clore::net::detail::clone_value`

Declaration: `network/protocol.cppm:267`

Definition: `network/protocol.cppm:451`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::clone_value` 通过直接调用 `json::Value` 的拷贝构造函数来创建 `source` 的深层副本。它忽略 `context` 参数，仅用于辅助错误报告而不影响逻辑。函数总是返回一个持有副本的 `std::expected` 成功值，没有解析或验证步骤。它是内部克隆工具链中的通用后备，在不需要特定结构（如对象或数组）处理时使用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- source (const `json::Value`& parameter)

#### Usage Patterns

- duplicate a JSON value for safe mutation
- clone a JSON value before validation to avoid modifying the original

### `clore::net::detail::excerpt_for_error`

Declaration: `network/protocol.cppm:219`

Definition: `network/protocol.cppm:312`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数实现了一个简单的截断操作：它将输入字符串 `body` 通过 `clore::support::truncate_utf8` 裁剪至最多 `kMaxBytes`（200 字节），并返回截断后的结果。内部不含分支或循环，直接依赖 `clore::support::truncate_utf8` 来完成 UTF-8 安全的字节截断，确保在多字节字符边界处正确切分。此函数用作错误上下文摘要的生成器，将长错误消息限制在固定长度内，避免输出过长的原始信息。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- body parameter (`std::string_view`)

#### Writes To

- return value (`std::string`)

#### Usage Patterns

- Used to create safe excerpts of response bodies for error messages

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:249`

Definition: `network/protocol.cppm:411`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function calls `value.get_array()` to attempt a dynamic cast of the underlying JSON node. If the result is `nullptr` — meaning the node is not a JSON array — it immediately returns a `std::unexpected` with an `LLMError` carrying a formatted message that repeats the caller-supplied `context` string. On success, it moves the non‑null pointer into a newly constructed `ArrayView` and returns that view as a success value.

`clore::net::detail::expect_array` depends on the `json::Cursor` abstraction and the `ArrayView` wrapper defined elsewhere in the same detail namespace. Its sole internal control flow is a null check; there is no iteration or recursion. The function is a small, focused building block used by higher‑level protocol parsing routines (such as those processing tool definitions or response messages) where an array‑typed field is expected and anything else is treated as a parse error.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `value` of type `json::Cursor`
- parameter `context` of type `std::string_view`
- internal call to `value.get_array()`

#### Usage Patterns

- validating JSON cursors as arrays before further processing
- converting a JSON cursor to an `ArrayView` for iteration or indexing

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:246`

Definition: `network/protocol.cppm:402`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该实现首先通过调用 `value.get_array()` 获取底层 JSON 数组的原始指针。如果指针为空，表示当前值并非数组类型，函数立即构造一个 `std::unexpected` 包装的 `LLMError`，错误消息使用 `std::format` 将传入的 `context` 字符串嵌入到描述中。若指针非空，则直接返回一个 `ArrayView`，将获取到的数组指针赋值给其 `value` 成员。整个流程仅依赖 `json::Value::get_array()` 的空值检测，以及 `ArrayView` 的简单构造。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `json::Value`& value
- `std::string_view` context

#### Usage Patterns

- Used to safely extract a JSON array from a value before processing its elements.
- Called by functions that expect an array in a specific context, similar to `expect_object` or `expect_string`.
- Often used in combination with `ArrayView` methods like `begin`, `end`, `size`, and `operator[]`.

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:240`

Definition: `network/protocol.cppm:384`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数首先调用 `value.get_object()` 以尝试从给定的 `json::Value` 中提取底层 `json::Object` 指针。若返回空指针，表示当前值并非 JSON 对象，则直接构造一个带有格式化错误消息的 `LLMError`（包含 `context` 字符串），并通过 `std::unexpected` 返回失败预期。若指针非空，则使用该指针初始化一个 `ObjectView` 结构体（直接设置其 `value` 成员），并将其包装为 `std::expected` 的成功值返回。整个流程不依赖其他内部函数，仅依赖 `json::Value::get_object`、`ObjectView` 的聚合初始化以及 `LLMError` 的构造。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (const `json::Value`&)
- `context` (`std::string_view`)

#### Usage Patterns

- Used to extract a JSON object from a Value with error reporting
- Called by validation functions to ensure a value is an object

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:243`

Definition: `network/protocol.cppm:393`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::expect_object` 尝试将给定的 `json::Cursor` 值向下转型为 JSON 对象。它调用 `value.get_object()` 以获取一个 `const kota::codec::json::Object*` 指针；如果该指针为空（即当前值不是对象）则失败，并返回一个通过 `std::format` 构造的包含传入 `context` 字符串的错误消息（包装在 `std::unexpected` 中）。成功时，函数使用聚合初始化直接构造一个 `ObjectView` 实例，将其内部指针成员设置为取得的对象指针。整个操作仅依赖于 `json::Cursor` 的类型查询方法以及 `ObjectView` 的简单指针包装，不涉及递归或深层验证。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor` value
- `std::string_view` context

#### Usage Patterns

- Used to validate that a JSON value is an object, returning an `ObjectView` for further field access
- Commonly called from higher-level parsing functions that require a JSON object at a given path

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:252`

Definition: `network/protocol.cppm:420`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::expect_string` 尝试从给定的 `json::Value` 中提取一个 `std::string_view`，调用 `json::Value::get_string` 方法获取可选的字符串结果。如果 `get_string` 返回空（即该 JSON 值不是字符串类型），函数立即返回一个 `std::unexpected` 包装的 `LLMError`，其错误信息通过 `std::format` 包含传入的 `context` 参数，帮助调用者定位问题。成功时则直接解引用 `get_string` 的返回结果，透传提取到的字符串视图。该函数是 JSON 验证层中的基础期望工具，依赖 `json::Value` 的类型查询能力与项目统一的错误类型 `LLMError`，以简洁统一的方式处理类型不匹配的常见场景。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (const `json::Value` &)
- `context` (`std::string_view`) for error message

#### Usage Patterns

- Validate that a JSON value is a string before processing
- Extract a string view from a JSON value with error handling

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:255`

Definition: `network/protocol.cppm:429`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::expect_string` 使用 `json::Cursor::get_string` 尝试将传入的 `json::Cursor` 值解析为字符串。若该方法返回的可选值无有效内容（即 `std::optional` 为空），则立即返回一个 `std::unexpected(LLMError)`，其错误消息通过 `std::format` 将 `context` 参数嵌入 “{} is not a JSON string” 模板中。否则解引用可选值并返回底层的 `std::string_view`。该函数不会对字符串内容进行进一步校验，完全依赖 `json::Cursor` 自身的类型检查。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- value (`json::Cursor`)
- context (`std::string_view`)

#### Usage Patterns

- Used when a JSON value must be a string, e.g., extracting fields from JSON objects or arrays
- Called by validation functions like `validate_completion_request`

### `clore::net::detail::infer_output_contract`

Declaration: `network/protocol.cppm:627`

Definition: `network/protocol.cppm:644`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数首先检查 `request.response_format` 是否存在。若存在且 `request.output_contract` 为 `PromptOutputContract::Markdown`，则立即返回一个 `std::unexpected` 错误，指示 `PromptRequest` 中 `output_contract` 与 `response_format` 的 JSON 约定冲突；否则直接返回 `PromptOutputContract::Json`。若 `request.response_format` 不存在，则继续检查 `request.output_contract` 是否为 `PromptOutputContract::Unspecified`，若是则返回另一个 `std::unexpected` 错误，要求显式设置为 `Markdown` 或 `Json`；若已显式设置则原样返回该合约值。该函数不依赖外部网络或协议层，仅基于 `PromptRequest` 的字段进行纯逻辑推导，用于决定最终输出格式。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- request`.response_format`
- request`.output_contract`

#### Usage Patterns

- Called to determine the output contract before sending a prompt request
- Used to validate consistency between `response_format` and `output_contract`

### `clore::net::detail::insert_string_field`

Declaration: `network/protocol.cppm:211`

Definition: `network/protocol.cppm:299`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::insert_string_field` 将一个字符串字段直接插入到 `json::Object` 中：它从 `key` 和 `value` 构造 `std::string` 副本，并调用 `object.insert` 完成操作。`context` 参数在实现中未被使用，可能预留给错误报告或日志，但当前版本简单忽略了它。函数始终返回一个空的 `std::expected<void, LLMError>`，表示插入操作假定不会失败。该实现是内联的、无分支的，其唯一依赖是底层的 JSON 对象插入方法以及 `std::string` 的复制构造。

#### Side Effects

- mutates the provided `json::Object` by inserting a new key-value pair

#### Reads From

- `key` parameter
- `value` parameter

#### Writes To

- `object` parameter (`json::Object`)

#### Usage Patterns

- adding a simple string field to a JSON object
- building JSON request bodies

### `clore::net::detail::make_empty_array`

Declaration: `network/protocol.cppm:227`

Definition: `network/protocol.cppm:344`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::make_empty_array` 通过解析字符串字面量 `"[]"` 来生成一个空的 JSON 数组。它调用 `json::parse<json::Array>("[]")`，如果解析结果包含错误，则立即将上下文传递给 `clore::net::detail::unexpected_json_error` 并返回该错误；否则，解引用成功的 `std::expected` 对象并返回底层的 `json::Array`。整个过程的控制流是线性的：一次解析尝试，两次可能的结果分支。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `context` parameter
- string literal `"[]"`

#### Usage Patterns

- Obtain an empty JSON array for a response or data structure
- Safely produce an initialized empty array

### `clore::net::detail::make_empty_object`

Declaration: `network/protocol.cppm:224`

Definition: `network/protocol.cppm:336`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数通过调用 `json::parse<json::Object>("{}")` 从字面空对象 `{}` 构造一个空的 `json::Object`。如果解析失败（理论上不可能，因为 `{}` 是合法的 JSON 文本，但防御性处理了异常路径），则调用 `unexpected_json_error(context, parsed.error())` 返回 `LLMError`，其中 `context` 参数用于描述出错位置。成功时直接返回解析得到的空对象。这个函数是一个底层工具，用于在需要空 JSON 对象的场合（例如初始化某些字段或构造默认值）提供安全的工厂方法，其依赖仅限于 JSON 解析库和错误上报函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- string literal `"{}"`
- parameter `context` (`std::string_view`)
- internal state of `json::parse`

#### Usage Patterns

- create an empty JSON object for initialization
- used in net module to produce placeholder objects

### `clore::net::detail::normalize_utf8`

Declaration: `network/protocol.cppm:209`

Definition: `network/protocol.cppm:289`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数利用 `clore::support::ensure_utf8` 将输入 `text` 规范化为有效的 UTF-8 编码。若规范化后的结果与原始 `text` 不相同，说明原字符串包含无效的 UTF-8 字节序列，此时通过 `logging::warn` 记录一条警告信息，指明出错的字段名（由 `field_name` 指定）。最终返回规范化后的 `std::string`。该实现的主要依赖是 `clore::support::ensure_utf8`（负责实际的字节级修复）以及日志系统，无其他内部控制流分支。

#### Side Effects

- Logs a warning message when invalid UTF-8 sequences are replaced

#### Reads From

- text parameter
- `field_name` parameter

#### Writes To

- logging system via `logging::warn`

#### Usage Patterns

- Standardizing user-supplied or LLM-generated text before JSON serialization
- Ensuring UTF-8 validity in network request/response processing

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:231`

Definition: `network/protocol.cppm:353`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数是一个泛型辅助函数，用于将给定的 JSON 值（`json::Value`）反序列化为模板参数 `T` 指定的类型。其内部流程很简单：调用 `json::from_json<T>` 执行反序列化；若成功，则直接返回解包后的值；若失败，则构造一个 `LLMError` 实例，其中包含来自调用点传入的 `context` 字符串（用于标识解析发生的上下文）以及 `json::from_json` 返回的错误描述，并以 `std::unexpected` 的形式返回。该函数依赖于 `json::from_json` 模板的具体实现以及 `LLMError` 错误类型。它被设计为在整个代码库中用于统一处理 JSON 值向所需类型的转换，并在解析失败时提供有意义的上下文错误信息。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `raw`
- parameter `context`

#### Usage Patterns

- Deserialize JSON configuration or response into a typed struct or value
- Used in error-handling paths that need contextual failure messages

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:234`

Definition: `network/protocol.cppm:364`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该模板重载首先调用 `json::to_string` 将输入的 `json::Value` 参数序列化为字符串 `raw`。若序列化失败（`raw` 不含值），则借助 `unexpected_json_error` 返回一个 `std::expected<T, LLMError>` 错误，错误消息通过 `std::format` 包含上下文标识 `context` 以及序列化底层错误。若成功，则递归委托给接受 `std::string_view` 的另一重载 `parse_json_value<T>`，传入解引用后的 `*raw` 和相同的 `context` 以完成后续的字符串解析逻辑。该重载本质上充当从 JSON 值到字符串表示的适配器，将解析的序列化步骤和错误处理分离。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (const `json::Value &`)
- `context` (`std::string_view`)

#### Usage Patterns

- Wraps the string parsing overload to accept a `json::Value`
- Used where a JSON value needs to be parsed into a specific type

### `clore::net::detail::request_text_once_async`

Declaration: `network/protocol.cppm:634`

Definition: `network/protocol.cppm:676`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数首先通过 `clore::net::detail::infer_output_contract` 推断输出合约，若失败则提前退出。随后根据 `request` 的 `clore::net::PromptRequest::output_contract` 和可选的 `response_format`，构建一个 `clore::net::CompletionRequest`，其中包含系统消息、用户消息、响应格式、工具列表及 `tool_choice`，并清空了 `parallel_tool_calls`。接着调用传入的 `request_completion` 协程（类型为 `CompletionRequester`）发起异步请求，等待响应。成功获取 `clore::net::CompletionResponse` 后，通过 `clore::net::protocol::text_from_response` 提取文本内容，再调用 `clore::net::detail::validate_prompt_output` 对该文本与合约进行验证。验证通过后返回最终的文本字符串。整个流程依赖上述内部工具函数和 `clore::net::CompletionRequest`、`clore::net::PromptRequest` 等数据结构，并在 `kota::event_loop` 上执行异步操作。

#### Side Effects

- Performs an asynchronous HTTP request via the `CompletionRequester` callable
- May propagate errors through `kota::fail` and potentially log or handle error states

#### Reads From

- `model`
- `system_prompt`
- `PromptRequest` fields: `prompt`, `output_contract`, `response_format`, `tool_choice`
- `kota::event_loop &loop`
- State captured by `CompletionRequester` (e.g., credentials, endpoint URL)

#### Writes To

- Local variable `completion_request` of type `CompletionRequest`
- Return value via coroutine (eventual `std::string`)
- Moves `request.response_format` and `request.tool_choice` into the completion request

#### Usage Patterns

- Used to initiate an async text completion request to an LLM endpoint
- Typically used within an async context with `co_await`
- Relies on a `CompletionRequester` to encapsulate the HTTP interaction and response parsing

### `clore::net::detail::run_task_sync`

Declaration: `network/protocol.cppm:222`

Definition: `network/protocol.cppm:318`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该实现在内部创建了一个 `kota::event_loop` 局部实例，并通过调用传入的 `make_task` 生成一个 `operation` 对象。随后将 `operation` 调度到事件循环中，然后阻塞执行 `loop.run()` 直到事件循环完成。完成之后，从 `operation.result()` 获取执行结果；若结果包含错误，则构造 `std::unexpected` 并返回。对于 `T` 为 `void` 的情况，直接返回一个空的 `std::expected`；否则将结果中的值通过移动语义返回。整个函数依赖于 `kota::event_loop` 的事件驱动模型以及 `operation` 类型提供的异步结果接口。

#### Side Effects

- Creates and runs an event loop, potentially performing network I/O and other asynchronous operations through the scheduled task.
- Calls `make_task` which may have its own side effects.

#### Reads From

- Parameter `make_task` (callable)
- Local event loop `loop`
- Operation's result via `operation.result()`
- Template parameter `T` for `std::is_void_v` check

#### Writes To

- Local event loop `loop` (via `schedule` and `run`)
- Return value as `std::expected<T, LLMError>`
- Potentially external state through the task's side effects

#### Usage Patterns

- Used to execute an asynchronous task synchronously in a temporary event loop.
- Often used with `make_task` lambdas that encapsulate a sequence of async calls.
- Suitable for test environments or simple synchronous callers.

### `clore::net::detail::serialize_value_to_string`

Declaration: `network/protocol.cppm:237`

Definition: `network/protocol.cppm:374`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数首先调用 `json::to_string` 将传入的 `json::Value` 序列化为原始字符串，得到的 `encoded` 是一个 `std::expected` 类型。如果序列化失败（`encoded` 不含值），则委托 `unexpected_json_error` 生成格式化的错误消息——该消息中包含通过 `std::format` 嵌入的 `context` 参数以及底层 `json::error` 对象——并直接返回构造的 `LLMError`。成功时则直接解引用 `encoded`，将内部存储的 `std::string` 返回。该实现的核心依赖是两个外部组件：`json::to_string` 提供与 `kota::codec::json` 库的衔接，而 `unexpected_json_error` 负责将 JSON 层面的错误转换为调用者期待的 `LLMError` 类型，从而保持函数返回路径的简洁。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter
- `context` parameter

#### Usage Patterns

- serialize JSON for network requests
- convert JSON to string with error context

### `clore::net::detail::to_llm_unexpected`

Declaration: `network/protocol.cppm:217`

Definition: `network/protocol.cppm:308`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数将一组状态和上下文字符串转化为一个 `LLMError`，并将其包装在 `std::unexpected` 中返回。实现首先通过调用 `error.to_string()` 获取底层状态对象的错误描述，然后使用 `std::format` 将 `context` 和该描述拼接为统一的错误消息，用该消息构造一个 `LLMError` 实例。最后直接返回 `std::unexpected<LLMError>`，不涉及分支或循环。其依赖包括 `Status::to_string()`、`std::format` 以及 `LLMError` 的构造函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error` parameter
- `context` parameter

#### Usage Patterns

- Returning unexpected `LLMError` from functions that fail with a Status error

### `clore::net::detail::unexpected_json_error`

Declaration: `network/protocol.cppm:206`

Definition: `network/protocol.cppm:284`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数负责将底层 JSON 解析过程中抛出的 `json::error` 转换为库内的标准错误类型 `LLMError`，并以 `std::unexpected` 的形式返回。调用者传入一个描述上下文的位置字符串 `context` 和原始的 `json::error` 对象 `err`，函数内部利用 `std::format` 将二者拼接为一条完整错误消息，然后构造 `LLMError` 实例并由 `std::unexpected` 包装后直接返回。整个过程不涉及条件分支或循环，仅依赖 `json::error::to_string()` 获取错误描述、`std::format` 合成消息以及 `LLMError` 的构造函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `context` parameter of type `std::string_view`
- `err` parameter of type `const json::error &`

#### Usage Patterns

- converting json errors to `LLMError`
- returning unexpected results from JSON parsing

### `clore::net::detail::validate_prompt_output`

Declaration: `network/protocol.cppm:630`

Definition: `network/protocol.cppm:662`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数的核心控制流基于 `PromptOutputContract` 枚举值进行分支选择。在 `switch` 语句中，对于 `PromptOutputContract::Json` 分支，它将 `content` 转发给 `protocol::validate_json_output` 执行严格 JSON 格式校验；对于 `PromptOutputContract::Markdown` 分支，则调用 `protocol::validate_markdown_fragment_output` 检查 Markdown 片段的合法性。若 `contract` 为 `PromptOutputContract::Unspecified` 或任何未匹配值，则立即构造一个包含错误描述的 `LLMError` 并包装为 `std::unexpected` 返回，确保调用方必须显式指定输出合同类型。该函数不直接处理 JSON 或 Markdown 的解析细节，完全依赖 `clore::net::protocol` 命名空间下的两个验证函数，并将错误类型统一为 `std::expected<void, LLMError>`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content
- contract

#### Usage Patterns

- validates prompt output format
- ensures output matches contract type

### `clore::net::get_probed_capabilities`

Declaration: `network/protocol.cppm:126`

Definition: `network/protocol.cppm:725`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::get_probed_capabilities` 的实现使用一个静态的 `std::unordered_map<std::string, std::unique_ptr<clore::net::ProbedCapabilities>>`（名为 `cache`）和一个静态 `std::mutex`（名为 `mutex`）来提供线程安全的缓存查找。当被调用时，它首先通过 `std::lock_guard` 获取互斥锁，然后在 `cache` 中搜索给定的 `provider` 字符串。如果找到已缓存的条目，它返回对该条目所指向的 `clore::net::ProbedCapabilities` 对象的引用。否则，它会通过 `emplace` 插入一个由 `std::make_unique<clore::net::ProbedCapabilities>()` 新创建的默认构造对象，并返回对刚插入的对象的引用。

该函数不执行任何实际的探测操作；它只负责管理缓存的生命周期，确保每个 `provider` 只产生一个 `clore::net::ProbedCapabilities` 实例。因此，调用者必须在该函数返回后通过其他方式（如直接修改返回的 `clore::net::ProbedCapabilities` 对象）填充探测结果。

#### Side Effects

- locks a static mutex for thread synchronization
- inserts a new entry into the static cache if the provider is not already cached
- allocates a new `ProbedCapabilities` object on the heap

#### Reads From

- the `provider` parameter
- the static cache map `cache`

#### Writes To

- the static cache map `cache`
- the newly allocated `ProbedCapabilities` object via construction

#### Usage Patterns

- used to lazily initialize and retrieve per-provider capability probes
- called before constructing requests that depend on provider capabilities

### `clore::net::icontains`

Declaration: `network/protocol.cppm:758`

Definition: `network/protocol.cppm:758`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

Implementation: [Implementation](functions/icontains.md)

函数 `clore::net::icontains` 是一种不区分大小写的子串搜索，通过逐字符比较 `haystack` 和 `needle` 实现。实现首先检查 `needle.size() > haystack.size()`，若成立则直接返回 `false`，以避免不必要的循环。随后外层循环遍历 `haystack` 中所有可能的起始位置（`i` 从 `0` 到 `haystack.size() - needle.size()`），内层循环对每个字符对进行不区分大小写的匹配：使用 `std::tolower` 将两个字符均转为小写后比较。一旦内层循环完全匹配，立即返回 `true`；否则继续扫描；若所有起始位置都检查完毕未匹配，返回 `false`。该算法不依赖项目内部工具，仅依赖标准库函数 `std::tolower` 和 `static_cast<unsigned char>` 保证字符类型安全。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `haystack`
- `needle`

#### Usage Patterns

- Used by `is_feature_rejection_error` to check if an error message contains a substring case-insensitively

### `clore::net::is_feature_rejection_error`

Declaration: `network/protocol.cppm:131`

Definition: `network/protocol.cppm:778`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::is_feature_rejection_error` 的实现采用一种简单的模式匹配算法来判定给定的错误消息是否表达了模型对某个请求特征（例如字段或参数）的拒绝。内部维护一个静态的常量字符串数组 `patterns`，其中包含诸如 `"unsupported parameter"`、`"unknown field"`、`"unrecognized field"` 等八条典型的拒绝模式。函数遍历该数组，对每个模式调用依赖函数 `clore::net::icontains`，执行不区分大小写的子串包含检查。一旦发现 `error_message` 中包含任一模式，立即返回 `true`；若遍历全部模式均不匹配，则返回 `false`。该实现不依赖任何外部状态或动态配置，完全基于硬编码的启发式规则。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `error_message`
- static const array `patterns`

#### Usage Patterns

- Determines if an LLM error response corresponds to a feature rejection (e.g., unsupported parameter)

### `clore::net::make_markdown_fragment_request`

Declaration: `network/protocol.cppm:99`

Definition: `network/protocol.cppm:140`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::make_markdown_fragment_request` 通过直接构造一个 `PromptRequest` 实例并返回来工作。它将传入的 `prompt` 字符串通过 `std::move` 移入 `.prompt` 字段，将 `.response_format` 显式置为 `std::nullopt`，并将 `.output_contract` 设置为 `PromptOutputContract::Markdown`。这使得调用方无需手动配置输出合约即可快速发起一个期望 Markdown 片段的请求。

该实现完全依赖 `PromptRequest` 聚合初始化的语法，因此其行为由 `PromptRequest` 和 `PromptOutputContract` 的定义决定。没有额外的算法或条件分支，整个函数体仅是一个构造表达式，返回值直接通过聚合初始化生成。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- Used to create a request for markdown-only responses from LLM
- Called when the caller expects the response to be formatted as markdown

### `clore::net::parse_rejected_feature_from_error`

Declaration: `network/protocol.cppm:133`

Definition: `network/protocol.cppm:797`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::parse_rejected_feature_from_error` 通过一个静态的 `field_patterns` 数组实现关键字到标准化功能名称的映射。该数组包含多对 `keyword` 与 `field`，例如 `"response_format"` 映射到 `"response_format"`，`"json_schema"` 也映射到 `"response_format"`，`"tools"` 和 `"functions"` 均映射到 `"tools"` 等。算法遍历该数组，对每个 `keyword` 调用 `clore::net::icontains` 以在输入的 `error_message` 中执行不区分大小写的子串匹配。一旦某一 `keyword` 被匹配，函数立即返回对应的 `field` 字符串（通过 `std::string` 构造）。

若遍历结束后无任何 `keyword` 匹配，则返回 `std::nullopt`，表示无法从错误消息中识别出被拒绝的特定功能。该实现完全依赖 `clore::net::icontains` 这一工具函数进行字符串包含判断，无额外的 I/O 或复杂控制流。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error_message` parameter
- static `field_patterns` array

#### Usage Patterns

- Used in error handling to identify rejected features from LLM provider responses

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:540`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数首先验证`response.message.tool_calls`非空，然后遍历`outputs`构建一个从`tool_call_id`到对应`output`的`output_by_id`无序映射，期间检查空ID和重复ID的合法性。接着将`history`中的消息整体复制到`merged`向量，并追加一条`AssistantToolCallMessage`，其`content`取自`response.message.text`、`tool_calls`取自`response.message.tool_calls`。随后按顺序遍历`response.message.tool_calls`中的每个工具调用，从`output_by_id`中查找匹配的输出，若缺失则立即返回错误，否则为每个调用创建一条`ToolResultMessage`（填充`tool_call_id`和`content`）加入`merged`。最后，若`output_by_id`的大小与工具调用数量不一致（表明存在未知的调用ID）则返回错误，否则返回构造完成的`merged`消息序列。

该实现依赖`std::unordered_map`进行ID到输出的快速查找，利用`std::span`和`std::expected`处理输入和错误传播，并借助`AssistantToolCallMessage`与`ToolResultMessage`两种消息类型维护对话结构中的工具调用与结果配对。

#### Side Effects

- allocates new Message objects
- moves ownership of Message objects

#### Reads From

- history parameter (span of Message)
- response parameter (`CompletionResponse`)
- outputs parameter (span of `ToolOutput`)

#### Writes To

- returned vector of Message

#### Usage Patterns

- appending tool outputs to conversation history
- constructing updated message list for LLM API

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:475`

Definition: `network/protocol.cppm:588`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

`parse_response_text` 的实现首先委托给 `text_from_response` 以从 `response` 中提取原始的文本负载。如果该调用失败（返回无值），函数立即将错误中的 `LLMError` 传播到 `std::unexpected` 中并返回。成功获取文本后，使用 `kota::codec::json::from_json<T>` 将文本解析为模板类型 `T`；该解析操作是函数核心反序列化能力的唯一依赖。如果 JSON 解析失败，则构造一个 `LLMError`，其中包含格式化的解析错误描述（通过 `parsed.error().to_string()`），并将其包装在 `std::unexpected` 中返回。只有解析成功时，才将解析后的 `*parsed` 值作为成功的 `std::expected` 返回。

该函数的内部控制流因此是一个简单的两阶段顺序管道：首先提取并验证文本存在，然后将其反序列化为目标类型。它不涉及任何循环或分支，依赖 `text_from_response` 负责处理 `CompletionResponse` 内部结构（如消息字段、工具调用等）并生成最终的文本内容。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `response` parameter
- `text_from_response` result
- global JSON decoder `kota::codec::json::from_json`

#### Writes To

- return value of type `std::expected<T, LLMError>`

#### Usage Patterns

- extract and parse structured data from a LLM response
- deserialize JSON from `CompletionResponse`

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:478`

Definition: `network/protocol.cppm:603`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `clore::net::protocol::parse_tool_arguments` 是一个模板，它接受 `const ToolCall& call` 并返回 `std::expected<T, LLMError>`。实现分两个阶段：首先，将 `call.arguments`（一个 `kota::codec::json::Value`）通过 `kota::codec::json::to_string` 序列化为 `std::string`，若序列化失败则返回包含格式化错误消息的 `std::unexpected<LLMError>`；然后，将序列化后的字符串（`*encoded`）通过 `kota::codec::json::from_json<T>` 反序列化为 `T` 类型，若解析失败也返回 `std::unexpected<LLMError>`，错误消息中包含工具名称 `call.name` 和解析错误描述。成功时直接返回 `*parsed`。该函数依赖 `kota::codec::json` 的序列化与反序列化能力，以及 `LLMError` 构造与 `std::format`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `call.arguments`
- `call.name`

#### Writes To

- return value (ownership of `T` or `LLMError`)

#### Usage Patterns

- Convert tool call arguments to a typed object for further processing

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:524`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `clore::net::protocol::text_from_response` 从给定的 `clore::net::CompletionResponse` 中提取纯文本结果。其算法首先检查 `response.message.refusal` 是否存在；若存在，则立即返回一个 `LLMError`，说明模型拒绝了结构化或文本响应。接着检查 `response.message.tool_calls` 是否非空；若存在工具调用，则返回一个 `LLMError`，表示响应包含工具调用而非文本完成。随后检查 `response.message.text` 是否缺失；若缺失，则返回一个 `LLMError`，指示响应无文本载荷。若以上条件均通过，则返回 `response.message.text` 的值。

该函数的控制流完全基于 `clore::net::AssistantOutput` 的三个可选/容器成员——`refusal`、`tool_calls`、`text`，依赖于 `clore::net::CompletionResponse` 的结构以及 `LLMError` 用于错误包装。它不涉及更复杂的 JSON 解析或工具输出合并逻辑，仅作快速的守卫检查并返回文本。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- response`.message``.refusal`
- response`.message``.tool_calls`
- response`.message``.text`

#### Usage Patterns

- used to obtain a plain text response from an LLM completion
- called after ensuring the response is not a tool call or refusal

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:463`

Definition: `network/protocol.cppm:484`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数的核心任务是验证给定的输入是否构成一个合法的 JSON 值。它首先调用 `kota::codec::json::parse` 将 `content` 解析为 `kota::codec::json::Value`。若解析失败，则构造一个包含错误信息的 `LLMError` 并以 `std::unexpected` 返回；否则返回一个空的 `std::expected<void, LLMError>`。整个过程仅依赖于 JSON 解析器，未涉及任何字段或工具调用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content

#### Usage Patterns

- Used to validate LLM output before further processing.

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:465`

Definition: `network/protocol.cppm:493`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `clore::net::protocol::validate_markdown_fragment_output` 首先对输入字符串 `content` 进行一系列快速失败检查。它排除空字符串以及仅由空白字符组成的输出（利用 `std::isspace` 遍历 `content` 判断 `has_non_whitespace`）。接着检测是否存在一级标题语法（`# ` 出现在行首或换行后），以及是否包含代码围栏符号（三个反引号 `` ` ``），匹配到任一情况即返回带有具体错误信息的 `std::unexpected<LLMError>`。

若上述检查均通过，函数调用 `clore::net::protocol::validate_json_output` 验证 `content` 是否是一个合法的 JSON 值。如果 `validate_json_output` 成功返回（即 `as_json` 持有值），则说明模型生成了 JSON 而非所需的 Markdown 片段，此时同样返回错误。当所有验证通过后，函数返回一个空期望值 `{}`，表示输出有效。该实现依赖 `validate_json_output` 进行 JSON 检测，并依赖 `LLMError` 类型传递结构化错误消息。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `content` of type `std::string_view`

#### Usage Patterns

- validating LLM output before further processing
- ensuring response matches expected markdown fragment format

### `clore::net::sanitize_request_for_capabilities`

Declaration: `network/protocol.cppm:128`

Definition: `network/protocol.cppm:739`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

实现由一系列基于 `ProbedCapabilities` 中标志的守卫条件构成，每个条件以 `std::memory_order_relaxed` 加载对应原子字段。若 `supports_json_schema` 为假且请求的 `response_format` 包含 `schema`，则将其置为 `std::nullopt`；若 `supports_tool_choice` 为假，则将 `tool_choice` 置为 `std::nullopt`；若 `supports_parallel_tool_calls` 为假，则将 `parallel_tool_calls` 置为 `std::nullopt`；若 `supports_tools` 为假，则清空 `tools` 容器。最后返回修改后的 `CompletionRequest`。

该函数不依赖外部数据源，仅对请求对象进行原地降级处理，其控制流呈现线性单次扫描模式，每个检查分支相互独立且无嵌套依赖。整个算法通过移除或清空非能力对应的字段来保证请求不会包含未知提供方不支持的配置项。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `caps.supports_json_schema` (atomic load relaxed)
- `caps.supports_tool_choice` (atomic load relaxed)
- `caps.supports_parallel_tool_calls` (atomic load relaxed)
- `caps.supports_tools` (atomic load relaxed)
- `request.response_format`
- `request.tool_choice`
- `request.parallel_tool_calls`
- `request.tools`

#### Writes To

- `request.response_format->schema` (set to `std::nullopt`)
- `request.tool_choice` (set to `std::nullopt`)
- `request.parallel_tool_calls` (set to `std::nullopt`)
- `request.tools` (cleared)

#### Usage Patterns

- Called before sending a completion request to ensure compatibility with provider capabilities
- Used to prune unsupported features like JSON schema, tool choice, parallel tool calls, and tools

## Internal Structure

`protocol` 模块位于 `network/` 下，属于 `clore::net` 命名空间，是 LLM HTTP 通信协议的核心封装。它导入 `http`、`std` 和 `support` 模块，并分为三层：最外层提供面向调用的数据结构（如 `CompletionRequest`、`CompletionResponse`、`SystemMessage` 等）和工具函数（如 `sanitize_request_for_capabilities`、`is_feature_rejection_error`）；`protocol` 子命名空间封装协议组合逻辑（如 `parse_tool_arguments`、`text_from_response` 等）；`detail` 子命名空间则包含 JSON 验证、克隆和错误处理的实用工具（如 `expect_object`、`clone_array`、`serialize_value_to_string`）。内部依赖 `kotacodec::json` 进行类型安全的 JSON 操作，并通过 `ObjectView`/`ArrayView` 提供零拷贝包装，避免暴露底层库。整体结构清晰地将数据定义、协议编排与底层工具分离，便于独立测试和替换 JSON 实现。

## Related Pages

- [Module http](../http/index.md)
- [Module support](../support/index.md)

