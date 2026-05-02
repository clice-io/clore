---
title: 'Module protocol'
description: 'protocol 模块是 clore::net 库中负责定义 LLM 交互协议核心数据模型与处理逻辑的组件。它公开了所有主要消息结构（如 SystemMessage、UserMessage、AssistantMessage、ToolCall、ResponseFormat、CompletionRequest 与 CompletionResponse 等），以及工具调用相关的类型（ToolChoice 变体、FunctionToolDefinition、ToolOutput）和能力探测结构（ProbedCapabilities）。模块通过 detail 命名空间提供 JSON 解析、类型断言、序列化和错误处理的内部基础设施，同时暴露诸如 make_capability_probe_key、sanitize_request_for_capabilities、parse_tool_arguments、append_tool_outputs、text_from_response、parse_response_text 以及输出验证（validate_json_output、validate_markdown_fragment_output）等函数，构成了完整的请求构建、响应解析和工具交互的公开实现范围。'
layout: doc
template: doc
---

# Module `protocol`

## Summary

`protocol` 模块是 `clore::net` 库中负责定义 LLM 交互协议核心数据模型与处理逻辑的组件。它公开了所有主要消息结构（如 `SystemMessage`、`UserMessage`、`AssistantMessage`、`ToolCall`、`ResponseFormat`、`CompletionRequest` 与 `CompletionResponse` 等），以及工具调用相关的类型（`ToolChoice` 变体、`FunctionToolDefinition`、`ToolOutput`）和能力探测结构（`ProbedCapabilities`）。模块通过 `detail` 命名空间提供 JSON 解析、类型断言、序列化和错误处理的内部基础设施，同时暴露诸如 `make_capability_probe_key`、`sanitize_request_for_capabilities`、`parse_tool_arguments`、`append_tool_outputs`、`text_from_response`、`parse_response_text` 以及输出验证（`validate_json_output`、`validate_markdown_fragment_output`）等函数，构成了完整的请求构建、响应解析和工具交互的公开实现范围。

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

结构体 `clore::net::AssistantMessage` 是一个简单的数据聚合体，其内部仅包含一个 `std::string` 类型的公有成员 `content`。该结构体没有定义任何自定义构造函数、析构函数或特殊成员函数，因此完全依赖编译器合成的默认实现。这保证了其内存布局紧凑且可平凡复制，适用于作为网络协议中表示助手消息的轻量级传输单元。不变量方面，`content` 字段可存储任意 UTF‑8 编码的字符串，且不附加任何校验或格式化约束。

#### Invariants

- The struct has no invariants beyond those implied by its members.

#### Key Members

- `clore::net::AssistantMessage::content`

#### Usage Patterns

- Other code creates instances and accesses the `content` member.
- Used as a data carrier in the network protocol.

### `clore::net::AssistantOutput`

Declaration: `network/protocol.cppm:101`

Definition: `network/protocol.cppm:101`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::AssistantOutput` 作为助手的统一输出载体，内部由三个独立的数据成员构成：`text` 和 `refusal` 是 `std::optional<std::string>` 类型，分别保存文本回复和拒绝信息；`tool_calls` 是 `std::vector<ToolCall>`，存储模型请求的工具调用。这种设计使得一个实例可以同时携带多种类型的输出（例如，既包含文本，又附带工具调用），或者仅包含其中一种。由于成员均采用值语义，结构体可以通过默认的移动操作高效传递，避免额外内存分配。该结构没有自定义的构造函数或赋值运算符，完全依赖编译器生成的默认函数，因此其生命周期管理和异常安全特性由成员类型自身的保证提供。

#### Invariants

- None explicitly stated; fields are independent and may be empty or populated per usage.

#### Key Members

- `text` - optional string for plain text output
- `refusal` - optional string for refusal reason
- `tool_calls` - vector of `ToolCall` for tool invocations

#### Usage Patterns

- Assigned by code that processes assistant responses
- Read by consumers to extract textual or tool-based output
- Used as a return type or message field in network-related data transfer

### `clore::net::AssistantToolCallMessage`

Declaration: `network/protocol.cppm:35`

Definition: `network/protocol.cppm:35`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::AssistantToolCallMessage` 是一个聚合类型，其内部结构由两个数据成员直接构成：`std::optional<std::string> content` 和 `std::vector<ToolCall> tool_calls`。编译器为这个结构隐式生成了默认构造函数、析构函数以及拷贝和移动操作，这些实现分别将 `content` 默认初始化为不含值的 `std::optional`，将 `tool_calls` 默认初始化为空的 `std::vector`，从而保证了成员各自的零初始化或空状态不变性；用户也可以使用聚合初始化直接赋值任意成员。除隐式成员外，该结构未定义任何特殊的构造函数或成员函数，因此所有初值设定均依赖编译器生成的行为以及用户通过直接成员访问进行赋值。

### `clore::net::CompletionRequest`

Declaration: `network/protocol.cppm:77`

Definition: `network/protocol.cppm:77`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::CompletionRequest` 是一个简单的聚合（aggregate）结构体，其成员直接映射到一个补全 API 请求的各个组成部分。内部布局中，必填字段 `model`（类型 `std::string`）和 `message`（类型 `std::vector<Message>`）分别存储模型标识符和消息序列；可选字段 `response_format`、`tool_choice` 和 `parallel_tool_calls` 均使用 `std::optional` 包装来区分缺省值与主动设置的 `null`，而 `tools`（类型 `std::vector<FunctionToolDefinition>`）的空向量语义则代表未提供工具列表。将 `tools` 与 `tool_choice` 作为独立字段而非嵌套结构，保持了与 API 模式的一一对应。该结构未定义任何用户提供的构造函数、析构函数或赋值运算符，因此保持平凡可聚合的特性，允许直接使用花括号初始化语法构造实例。

#### Invariants

- `model` is default-initialized to empty string
- `messages` may be empty
- optional fields may be absent

#### Key Members

- `model`
- `messages`
- `tools`
- `tool_choice`
- `response_format`
- `parallel_tool_calls`

#### Usage Patterns

- Constructed with a model name and a list of messages
- Optionally configured with tool definitions and tool calling behavior

### `clore::net::CompletionResponse`

Declaration: `network/protocol.cppm:107`

Definition: `network/protocol.cppm:107`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::CompletionResponse` 是一个纯聚合体，用于容纳一次完成响应的各组成部分。其字段分别存储响应标识符 `id`、使用的模型名称 `model`、生成的助手输出 `message`（类型为 `AssistantOutput`），以及原始序列化响应的 `raw_json`。`raw_json` 成员保留了完整的 JSON 载荷，供下游处理或调试；其他字段则以类型安全的方式提供对主要响应数据的直接访问。结构体本身不施加额外的不变量，所有字段的值均完全由各自的基础类型决定。

#### Invariants

- All members are default-constructible and may hold empty strings or default-constructed `AssistantOutput`.
- The `message` member is always an `AssistantOutput` instance, regardless of the underlying response content.
- `raw_json` is expected to contain the complete JSON string from which the other fields were derived, but no consistency is enforced.

#### Key Members

- `clore::net::CompletionResponse::id`
- `clore::net::CompletionResponse::model`
- `clore::net::CompletionResponse::message`
- `clore::net::CompletionResponse::raw_json`

#### Usage Patterns

- Constructed via aggregate initialization, typically after deserializing a JSON response from the network layer.
- Read by callers to access the textual assistant reply through `message`, or to retrieve the raw response for logging or error analysis.
- Passed by value or const reference to handlers that process completion results.

### `clore::net::ForcedFunctionToolChoice`

Declaration: `network/protocol.cppm:70`

Definition: `network/protocol.cppm:70`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ForcedFunctionToolChoice` 是一个聚合类型，其唯一的非静态数据成员是 `name`，类型为 `std::string`。由于没有用户声明的构造函数、析构函数或赋值运算符，所有成员均为公开，该结构体完全依赖编译器生成的默认特殊成员函数。`name` 字段存储由调用方指定的工具函数名称；结构体本身不实施任何校验或预处理，其有效性（如非空、可解析等）由调用方保证。内部结构仅此一个数据成员，无额外虚函数或继承，使得对象布局紧凑，适合作为简单值类型使用。

#### Invariants

- The `name` member identifies a target tool function

#### Key Members

- `name`

#### Usage Patterns

- Used to pass a forced tool choice in API calls or protocol messages
- Consumed by logic that selects and dispatches to the named tool function

### `clore::net::FunctionToolDefinition`

Declaration: `network/protocol.cppm:57`

Definition: `network/protocol.cppm:57`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体是一个简单的聚合类型，内部直接存储四个值对象：`name` 和 `description` 均为 `std::string`，`parameters` 为 `kota::codec::json::Object`，`strict` 为 `bool`。所有成员均无额外包装或智能指针，因此整个对象的内存布局就是四个子对象的连续排列，拷贝与移动操作由编译器合成的特殊成员函数自动完成。默认初始化时，`strict` 被显式设置为 `true`，其余三个成员则被默认构造为空字符串或空 JSON 对象。

保持的不变量依赖于成员自身的类型约束：`parameters` 必须始终是合法的 JSON 对象（由 `kota::codec::json::Object` 的不变量保证），`name` 理论上可以为空，但在实际使用中通常期望非空。`strict` 的默认值 `true` 意味着大部分情况下该工具定义会启用严格模式，除非显示赋值为 `false`。由于所有成员都是公开的，结构本身不执行任何运行时校验，不变量完全由外部使用者或对应的序列化/校验逻辑维护。

### `clore::net::Message`

Declaration: `network/protocol.cppm:45`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

内部实现中，`clore::net::Message` 是一个 `std::variant` 类型别名，其备选类型包括 `SystemMessage`、`UserMessage`、`AssistantMessage`、`AssistantToolCallMessage` 和 `ToolResultMessage`。该变体通过其 `index()` 成员在编译期跟踪当前存储的消息类型，并在运行时通过 `std::visit` 或 `std::get` 进行安全访问。由于 `std::variant` 是值类型，`Message` 对象的大小由最大的备选类型决定，且所有备选类型的对齐要求均由实现自动满足，从而避免了动态内存分配。

每个备选类型在协议中代表一类具有固定字段的消息载荷，变体的每次状态变化均通过赋值操作完成，并在赋值时保证异常安全（若备选类型的移动构造或复制构造不抛出异常）。`Message` 的存储布局直接映射至底层字节流，但其本身不提供序列化逻辑；序列化与反序列化由外部函数通过提取变体内存储的具体类型完成。变体的默认构造（若存在）会初始化第一个备选类型，但此处各消息类型通常具有默认构造函数，因此 `Message` 默认构造为 `SystemMessage`。

### `clore::net::ProbedCapabilities`

Declaration: `network/protocol.cppm:119`

Definition: `network/protocol.cppm:119`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ProbedCapabilities` 内部只用四个 `std::atomic<bool>` 成员记录联网功能的探测结果：`supports_json_schema`、`supports_tool_choice`、`supports_parallel_tool_calls` 和 `supports_tools`。每个字段默认初始化为 `true`，表示乐观假定端点支持对应特性；探测完成后会被安全地写回原子值，使其他线程无需同步即可读取这些标志。该结构体本身不持有任何锁定原语，所有状态仅由原子变量的内存序保证一致性。

#### Invariants

- All capability flags start as `true`.
- Flags are only ever set to `false` after probing; they are never reset to `true`.
- Atomic operations guarantee consistent visibility across threads.

#### Key Members

- `supports_json_schema`
- `supports_tool_choice`
- `supports_parallel_tool_calls`
- `supports_tools`

#### Usage Patterns

- Other code checks these flags before using JSON schema constraints, tool choice options, or parallel tool invocations.
- Probing logic writes to these flags after receiving capability responses from the endpoint.
- The struct is typically accessed concurrently by network and application threads.

### `clore::net::PromptOutputContract`

Declaration: `network/protocol.cppm:86`

Definition: `network/protocol.cppm:86`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

枚举 `clore::net::PromptOutputContract` 以 `std::uint8_t` 作为底层类型，成员线性排列为 `Unspecified`、`Json`、`Markdown`，分别对应数值 0、1、2。这种顺序确保了默认初始化的值为 `Unspecified`（零值），作为未指定状态的自然哨兵。由于枚举值直接映射到网络字节流中的唯一标识符，其紧凑的底层类型和固定的数值顺序是序列化不变性的核心：任何接收方都能根据该数值精确还原出对应的输出格式，且新增成员必须追加在末尾以避免破坏现有协议解析逻辑。

#### Invariants

- 每个枚举成员唯一，值从 0 开始连续递增
- 枚举值可作为 `std::uint8_t` 类型使用

#### Key Members

- `clore::net::PromptOutputContract::Unspecified`
- `clore::net::PromptOutputContract::Json`
- `clore::net::PromptOutputContract::Markdown`

#### Usage Patterns

- 作为函数参数或数据结构成员，用于控制输出内容的格式
- 在网络协议中指定客户端期望或服务器返回的输出格式合同

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

`clore::net::PromptRequest` 的实现依赖四个数据成员：`prompt` 通过默认成员初始化器置为空字符串，`response_format` 与 `tool_choice` 均为 `std::optional` 且默认不含值，`output_contract` 默认为 `PromptOutputContract::Unspecified`。此类设计仅通过聚合初始化或默认构造构造，确保所有字段在构造后处于明确初始化的状态，无需动态资源管理或自定义析构函数。`prompt` 是唯一非可选字段，作为请求的核心文本负载；其余可选字段提供可独立覆盖的扩展配置，其缺失通过 `std::nullopt` 表示，与 `output_contract` 的枚举默认值形成语义上的互补——前者表示“未指定”，后者表示“未指定行为”。

#### Invariants

- `prompt` is always present as a `std::string`, may be empty but never null
- `response_format` and `tool_choice` may be `std::nullopt` when not specified
- `output_contract` always has a value due to default initializer
- The struct is trivially copyable and movable via defaulted operations

#### Key Members

- `prompt`
- `response_format`
- `tool_choice`
- `output_contract`

#### Usage Patterns

- Used as an input argument to a request-sending function in the `clore::net` namespace
- Callers populate fields before passing the struct to a network call
- Library code inspects the fields to construct a corresponding protocol message

### `clore::net::ResponseFormat`

Declaration: `network/protocol.cppm:51`

Definition: `network/protocol.cppm:51`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ResponseFormat` 内部由三个数据成员组成：`name`（一个 `std::string`）、`schema`（一个 `std::optional<kota::codec::json::Object>`）和 `strict`（一个 `bool`，默认初始化为 `true`）。该结构体作为一个简单的聚合类型，没有定义特殊的构造函数或成员函数；其不变量主要依赖于各字段的类型约束：`strict` 的默认值保证了在未显式指定时采用严格模式，而 `schema` 的可选性允许表示无模式约束或一个具体的 JSON 对象模式。所有字段的公开访问性意味着外部代码可以直接读取和修改它们，结构体本身不维护额外的内部一致性检查。

#### Invariants

- `name` is always a valid `std::string`
- `schema` is either absent or a valid `kota::codec::json::Object`
- `strict` defaults to `true` when not explicitly set

#### Key Members

- `name`
- `schema`
- `strict`

#### Usage Patterns

- Used to describe expected response schemas in network protocols
- May be passed to API endpoint definitions or validation logic

### `clore::net::SystemMessage`

Declaration: `network/protocol.cppm:16`

Definition: `network/protocol.cppm:16`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::SystemMessage` 的内部结构仅由一个 `std::string` 成员 `content` 构成；该成员直接持有消息的文本载荷，没有额外的元数据或对齐要求。该类型的实现假定 `content` 的初始化和销毁完全委托给 `std::string` 的 RAII 语义，因此不引入自定义构造、析构或拷贝操作，也不维护任何跨域不变量。这种单一字段的设计保持了最小的拷贝开销，并允许上层协议自由决定 `content` 的序列化格式与生命周期管理策略。

#### Invariants

- `content` 可以存放任意字符串，无长度或格式约束

#### Key Members

- `content`：存储消息内容的字符串成员

#### Usage Patterns

- 作为网络协议中的消息体传递数据
- 在发送与接收消息时填充或读取 `content` 字段

### `clore::net::ToolCall`

Declaration: `network/protocol.cppm:24`

Definition: `network/protocol.cppm:24`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ToolCall` 内部由四个成员组成：`id`、`name`、`arguments_json` 和 `arguments`。其中 `arguments_json` 以 `std::string` 保存参数的 JSON 文本表示，而 `arguments` 是 `kota::codec::json::Value` 类型的解析后值。这两个字段共同描述工具调用的参数信息，设计上要求它们始终保持一致：每当 `arguments_json` 被修改时，对应的 `arguments` 必须重新解析；反之，对 `arguments` 的变更也应同步回 `arguments_json`。该结构体本身不提供自动同步逻辑，调用方在直接操作这两个成员时需自行维护这一不变量。

#### Invariants

- id and name are expected to be non-empty
- `arguments_json` should be valid JSON
- arguments corresponds to the parsed content of `arguments_json`

#### Key Members

- id
- name
- `arguments_json`
- arguments

#### Usage Patterns

- Serialized and deserialized in network protocol messages
- Passed between components to invoke external tool calls

### `clore::net::ToolChoice`

Declaration: `network/protocol.cppm:74`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoice` 被实现为 `std::variant`，其中包含四个备选类型：`ToolChoiceAuto`、`ToolChoiceRequired`、`ToolChoiceNone` 和 `ForcedFunctionToolChoice`。这个变体类型本身不维护额外的状态或不变性，而是依赖标准库 `std::variant` 的语义来保证在任何时刻只有一个备选值处于活动状态。关键成员实现（如构造、赋值、类型查询和值访问）完全由 `std::variant` 提供，通过 `std::holds_alternative`、`std::get`、`std::get_if` 以及 `valueless_by_exception` 等机制完成。由于没有自定义构造函数、析构函数或赋值操作符，该别名仅作为底层变体的透明封装。

### `clore::net::ToolChoiceAuto`

Declaration: `network/protocol.cppm:64`

Definition: `network/protocol.cppm:64`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoiceAuto` 是一个空结构体，不包含任何数据成员或成员函数。它的唯一目的是作为一个标记类型，用于在编译期区分不同的工具选择策略。由于没有内部状态，该类型不维持任何不变量，且其空实现符合零开销抽象原则——实例化 `clore::net::ToolChoiceAuto` 不会产生任何运行时代价。

#### Invariants

- Empty struct with no members
- Trivially constructible and destructible
- Used as a type tag

#### Usage Patterns

- Likely used as a template argument for `std::variant` or `std::optional` to indicate an automatic tool selection
- May appear in interfaces that require a type to distinguish between manual and automatic tool choice

### `clore::net::ToolChoiceNone`

Declaration: `network/protocol.cppm:68`

Definition: `network/protocol.cppm:68`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoiceNone` 是一个空标记结构体，不包含任何数据成员或成员函数。其实现仅为空体，因此它占用零存储开销（符合空基类优化），且复制、移动、默认构造均为平凡操作。该类型在设计上作为零成本的标签使用，通常用于模板特化或策略选择机制中表示“不提供工具选项”的语义，而不引入运行时状态。

#### Invariants

- 结构体不包含任何成员，因此无运行时状态
- 所有实例在语义上等价

#### Usage Patterns

- 作为模板参数或函数重载的区分类型
- 用于表示工具选择策略的默认值或禁用状态
- 与其他工具选择类型（如`ToolChoiceAuto`）组成变体或联合体

### `clore::net::ToolChoiceRequired`

Declaration: `network/protocol.cppm:66`

Definition: `network/protocol.cppm:66`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体是一个空标记类型，没有声明任何数据成员或成员函数。因此，它不维护内部状态，也不存在需要保持不变量的成员。所有特殊成员函数均为编译器隐式生成，使得该类型平凡可复制且标准布局。在实现层面，`clore::net::ToolChoiceRequired`的设计意图是作为一个类型标签，用于在模板或重载决议中区分不同的工具选择策略，但其内部实现本身不包含任何逻辑或数据。

#### Invariants

- No data members exist.
- The type is default-constructible and trivially destructible.

#### Usage Patterns

- No explicit usage is documented; it may be used as a type discriminator or sentinel.

### `clore::net::ToolOutput`

Declaration: `network/protocol.cppm:114`

Definition: `network/protocol.cppm:114`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolOutput` 是一个简单的聚合类型，用于将工具调用的标识符与其输出结果配对。其内部结构由两个公开的 `std::string` 成员构成：`tool_call_id` 用于唯一标识一次工具调用，`output` 用于存储该次调用的文本结果。该结构体不维护额外的类不变量，所有成员均可直接访问和修改，因此它本质上充当了一个轻量的数据传输对象，核心作用是建立 `tool_call_id` 与 `output` 之间的对应关系，以便在网络协议处理流程中传递工具调用的结果。

#### Key Members

- `tool_call_id`: 工具调用的唯一标识符
- `output`: 工具执行后的输出字符串

#### Usage Patterns

- 用于在 `clore::net` 命名空间中传递工具调用的结果
- 通常作为网络协议消息的一部分进行序列化或反序列化

### `clore::net::ToolResultMessage`

Declaration: `network/protocol.cppm:40`

Definition: `network/protocol.cppm:40`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ToolResultMessage` 的实现仅包含两个公有 `std::string` 数据成员：`tool_call_id` 和 `content`。它被定义为聚合类型，没有自定义构造函数、析构函数或任何成员函数，因此所有特殊成员函数均由编译器隐式生成。成员在默认构造时初始化为空字符串，且直接通过成员初始化列表或赋值进行修改，不添加任何运行时检查或约束。

由于不存在虚函数或继承，该类型属于标准布局且可平凡析构，但 `std::string` 成员阻止其成为平凡可复制类型。内部不维护任何额外的状态或不变量，调用方需自行确保 `tool_call_id` 与 `content` 符合高层协议语义。这一简单设计使得结构体的复制、移动和赋值操作均依赖 `std::string` 的默认实现，性能开销与字符串操作一致。

#### Invariants

- Both members are default-constructible and copyable strings

#### Key Members

- `tool_call_id`
- `content`

#### Usage Patterns

- Serialized and deserialized as part of the network protocol
- Likely used in messages exchanged between nodes

### `clore::net::UserMessage`

Declaration: `network/protocol.cppm:20`

Definition: `network/protocol.cppm:20`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::UserMessage` 是一个极简的数据容器，内部唯一持有 `std::string content` 成员。该结构体未提供任何用户定义的构造函数、析构函数或拷贝/移动特殊成员，完全依赖编译器生成的默认实现，因此 `content` 的生存期管理遵循 `std::string` 自身的 RAII 语义。不变量仅包含：`content` 的有效性完全由调用方在构造和使用时保证，结构体本身不施加任何内容长度、编码或非空限制。这种设计使其作为轻量级消息体，不引入额外的运行时包装或验证开销。

#### Invariants

- The `content` member stores the message as a `std::string`
- No invariants beyond standard string usage are documented

#### Key Members

- `content` (`std::string`) – the actual message content

#### Usage Patterns

- Acts as a carrier for user message text in networking operations
- Likely serialized or transmitted over socket connections

### `clore::net::detail::ArrayView`

Declaration: `network/protocol.cppm:178`

Definition: `network/protocol.cppm:178`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::ArrayView` 是一个轻量级、非拥有所有权的只读视图，包裹一个指向 `const kota::codec::json::Array` 的指针成员 `value`。其核心不变量是 `value` 必须指向一个有效的、已构造的 `Array` 对象（默认初始化为 `nullptr` 但使用前应确保非空），所有成员函数均假定此指针有效，直接委托给底层数组对应操作，不进行生存期管理或拷贝。实现上，`empty`、`size`、`begin`、`end`、`operator->` 和 `operator*` 均声明为 `noexcept`，而 `operator[]` 因可能触发下标检查异常而未标记 `noexcept`。该类旨在公开 `Array` 的常用只读接口子集（如范围循环、随机访问、大小查询），避免不必要的间接层次和拷贝开销，常用于协议序列化上下文中的临时视图传递。

#### Invariants

- The `value` pointer is expected to point to a valid `kota::codec::json::Array` instance before any member function is called.
- `ArrayView` does not own the pointed-to array; it is the caller's responsibility to ensure the array outlives the view.

#### Key Members

- `value` field: the underlying pointer to `kota::codec::json::Array`
- `operator[]`: element access by index
- `operator*` and `operator->`: dereference to underlying array
- `begin`/`end`: iterator access
- `size`/`empty`: query container size

#### Usage Patterns

- Used as an internal helper in `clore::net` namespace to provide a familiar array interface over a `kota::codec::json::Array`.
- Typically constructed by initializing the `value` pointer with the address of an existing array, then calling its member functions to inspect elements.

#### Member Functions

##### `clore::net::detail::ArrayView::begin`

Declaration: `network/protocol.cppm:189`

Definition: `network/protocol.cppm:189`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto begin() const noexcept {
        return value->begin();
    }
```

##### `clore::net::detail::ArrayView::empty`

Declaration: `network/protocol.cppm:181`

Definition: `network/protocol.cppm:181`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto empty() const noexcept -> bool {
        return value->empty();
    }
```

##### `clore::net::detail::ArrayView::end`

Declaration: `network/protocol.cppm:193`

Definition: `network/protocol.cppm:193`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto end() const noexcept {
        return value->end();
    }
```

##### `clore::net::detail::ArrayView::operator*`

Declaration: `network/protocol.cppm:205`

Definition: `network/protocol.cppm:205`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator*() const noexcept -> const kota::codec::json::Array& {
        return *value;
    }
```

##### `clore::net::detail::ArrayView::operator->`

Declaration: `network/protocol.cppm:201`

Definition: `network/protocol.cppm:201`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator->() const noexcept -> const kota::codec::json::Array* {
        return value;
    }
```

##### `clore::net::detail::ArrayView::operator[]`

Declaration: `network/protocol.cppm:197`

Definition: `network/protocol.cppm:197`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator[](std::size_t index) const -> const kota::codec::json::Value& {
        return (*value)[index];
    }
```

##### `clore::net::detail::ArrayView::size`

Declaration: `network/protocol.cppm:185`

Definition: `network/protocol.cppm:185`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto size() const noexcept -> std::size_t {
        return value->size();
    }
```

### `clore::net::detail::ObjectView`

Declaration: `network/protocol.cppm:156`

Definition: `network/protocol.cppm:156`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该结构体通过单个指针成员 `value` 持有对 `const kota::codec::json::Object` 的非拥有引用，默认初始化为 `nullptr`。所有成员函数的实现均直接委托给底层对象：`begin` 和 `end` 返回底层 `Object` 的常量迭代器（通过 `value->begin()` 和 `value->end()` 实现），`operator->` 和 `operator*` 分别返回指针和引用。`get` 方法按键查找并返回 `std::optional<kota::codec::json::Cursor>`，其实现隐式假定 `value` 始终有效——调用者必须在使用前确保指针非空，否则委托调用会导致未定义行为。该视图不管理资源，作为可复制的轻量封装在设计上零开销，适用于参数传递或临时访问。

#### Invariants

- `value` 指向的 `Object` 必须在 `ObjectView` 使用期间保持有效
- `value` 可以为 `nullptr`，此时调用 `begin`/`end`/`operator->`/`operator*` 是未定义行为
- `get` 返回的 `optional` 在键不存在时为空

#### Key Members

- `value` 指针成员
- `get` 键查找方法
- `begin` / `end` 迭代器方法
- `operator->` / `operator*` 解引用运算符

#### Usage Patterns

- 作为函数参数或返回值以非拥有方式传递 JSON 对象视图
- 代替 `const kota::codec::json::Object&` 以支持可空性
- 与 `std::optional<kota::codec::json::Cursor>` 结合使用以安全访问嵌套字段
- 在协议解析代码中用作临时对象以避免不必要的拷贝

#### Member Functions

##### `clore::net::detail::ObjectView::begin`

Declaration: `network/protocol.cppm:161`

Definition: `network/protocol.cppm:161`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto begin() const noexcept {
        return value->begin();
    }
```

##### `clore::net::detail::ObjectView::end`

Declaration: `network/protocol.cppm:165`

Definition: `network/protocol.cppm:165`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto end() const noexcept {
        return value->end();
    }
```

##### `clore::net::detail::ObjectView::get`

Declaration: `network/protocol.cppm:159`

Definition: `network/protocol.cppm:280`

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

Declaration: `network/protocol.cppm:173`

Definition: `network/protocol.cppm:173`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator*() const noexcept -> const kota::codec::json::Object& {
        return *value;
    }
```

##### `clore::net::detail::ObjectView::operator->`

Declaration: `network/protocol.cppm:169`

Definition: `network/protocol.cppm:169`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator->() const noexcept -> const kota::codec::json::Object* {
        return value;
    }
```

## Functions

### `clore::net::detail::clone_array`

Declaration: `network/protocol.cppm:268`

Definition: `network/protocol.cppm:442`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数的行为是直接的：它通过解引用 `ArrayView` 内部的 `value` 指针以获取底层的 `json::Array` 对象，然后调用其复制构造函数来构造一个新的 `json::Array`。此过程不涉及任何条件分支或循环；整个算法仅由单个表达式 `json::Array(*source.value)` 组成。函数的唯一依赖是 `ArrayView` 的 `value` 成员（类型应为 `const json::Array*`）以及 `json::Array` 的复制语义。返回类型 `std::expected<json::Array, LLMError>` 表明调用者期望可能出错，但此实现总是成功，因为复制操作假定源数组有效。

#### Side Effects

- 分配一个新的 `json::Array` 对象（拷贝构造）

#### Reads From

- `source.value`（指向原 JSON 数组的指针）
- 原数组元素（经由拷贝构造函数读取）

#### Writes To

- 返回的 `json::Array` 对象

#### Usage Patterns

- 为后续独立操作克隆一个 JSON 数组
- 在序列化或验证流程中创建数组的独立副本

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:262`

Definition: `network/protocol.cppm:446`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clone_object` 通过直接调用 `json::Object` 的拷贝构造函数来创建传入 JSON 对象的独立副本，从而递归复制所有嵌套的值。未使用的 `std::string_view` 参数仅用于消歧或类型匹配；该函数始终成功，不会进入错误路径。依赖项仅限于 `json::Object` 的拷贝语义，由底层 `kota::codec::json` 库提供。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source` parameter of type `const json::Object&`

#### Writes To

- returns a new `json::Object` (the copy)

#### Usage Patterns

- deep copy a JSON object
- prepare object for serialization or manipulation

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:265`

Definition: `network/protocol.cppm:451`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::clone_object` 的实现极为简洁：它直接通过解引用 `source.value` 并调用 `json::Object` 的复制构造函数，返回一个新的 `json::Object` 副本。由于 `source` 的类型 `ObjectView` 内部封装了一个指向常量 JSON 对象的指针，该操作等同于深拷贝整个对象树。函数签名中的 `std::string_view` 形参在实现中未被使用，通常用于传递调用上下文或错误报告，但当前代码路径下被完全忽略。该算法的依赖链仅包括 `kota::codec::json::Object` 的复制语义，不涉及其他控制流或外部函数调用。

#### Side Effects

- Heap memory allocation for the cloned JSON object
- Copy construction of `json::Object`, duplicating all key-value pairs

#### Reads From

- source `ObjectView`
- source`.value` pointer
- Underlying `json::Object` data

#### Writes To

- Newly allocated `json::Object` returned via `std::expected`

#### Usage Patterns

- Cloning a JSON object to obtain an independent mutable copy
- Creating a deep copy of an `ObjectView`'s content for storage or modification

### `clore::net::detail::clone_value`

Declaration: `network/protocol.cppm:271`

Definition: `network/protocol.cppm:455`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::clone_value` 的实现直接利用 `json::Value` 的复制构造函数生成输入值的一份完整副本。它忽略 `context` 参数（该参数仅用于错误消息上下文），仅通过 `return json::Value(source);` 完成拷贝。该函数不含分支或循环，其关键依赖是 `json::Value` 所支持的深拷贝语义，确保返回独立于原始值的对象。

#### Side Effects

- Allocates a new `json::Value` object via copy construction, which may involve dynamic memory allocation for the underlying JSON data.

#### Reads From

- source (the input `json::Value`)

#### Writes To

- Returns a new `json::Value` (the copy) owned by the caller

#### Usage Patterns

- Used when a caller needs an independent copy of a JSON value, for example before modification or to avoid aliasing.

### `clore::net::detail::excerpt_for_error`

Declaration: `network/protocol.cppm:223`

Definition: `network/protocol.cppm:316`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数实现委托给 `clore::support::truncate_utf8`，将 `body` 截断至 `kMaxBytes`（200）字节，直接返回截断后的 `std::string`。无分支或错误处理，依赖 `truncate_utf8` 确保结果始终是有效的 UTF-8。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `body`

#### Writes To

- returned `std::string`

#### Usage Patterns

- used to create a safe excerpt of a server response for inclusion in error diagnostics
- called when constructing error messages that need a portion of the body

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:253`

Definition: `network/protocol.cppm:415`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数首先通过 `value.get_array()` 尝试将当前 JSON 游标指向一个数组。如果返回空指针，说明该 JSON 值不是数组，函数立即返回一个 `LLMError`，其错误消息中包含了调用者提供的 `context` 字符串以指示错误位置。若成功，则使用获取到的 `kota::codec::json::Array` 指针初始化 `clore::net::detail::ArrayView` 结构体并返回。该函数是 JSON 解析验证链中的基础检查环节，依赖 `json::Cursor` 的类型查询能力和 `ArrayView` 的构造方式，为后续遍历数组元素提供安全的包装。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `json::Cursor` parameter `value`
- the `std::string_view` parameter `context`

#### Usage Patterns

- Used to validate JSON arrays during parsing or deserialization
- Provides an `ArrayView` for indexed or range access
- Often called with a meaningful context string to aid error reporting

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:250`

Definition: `network/protocol.cppm:406`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::expect_array` 接受一个 `const json::Value &` 和一个描述性 `std::string_view context`，用于从 JSON 输入中安全提取数组。内部控制流首先尝试通过 `value.get_array()` 获取底层数组的指针；若指针为空（即 `value` 并非 JSON 数组类型），则立即构造一个包含格式化错误消息的 `std::unexpected<LLMError>`，其中 `context` 被嵌入以辅助调试。只有当确认 `value` 是一个数组时，函数才返回一个 `clore::net::detail::ArrayView`，其 `.value` 成员直接指向该数组。

依赖方面，该函数是 JSON 验证流水线的一环，依赖 `json::Value` 的类型查询接口（`get_array`）以及 `LLMError` 错误类型。返回的 `ArrayView` 是一个轻量视图类，其 `operator*`、`operator[]` 和 `size` 等方法由调用方用于进一步遍历或操作数组内容，自身不拥有数据。此函数与同命名空间的 `expect_object`、`expect_string` 等遵循相同的模式：先检查类型，若失败则生成包含上下文标识的错误，从而在复杂协议解析中提供清晰的错误定位。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (const `json::Value` &)
- `context` parameter (`std::string_view`)

#### Usage Patterns

- extracting a JSON array with error handling
- validating JSON structure in networking code
- wrapping an array pointer into an `ArrayView`

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:244`

Definition: `network/protocol.cppm:388`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数尝试从传入的 `json::Value` 中提取底层 JSON 对象指针。内部调用 `value.get_object()`，若返回空指针，则立即构造一个 `LLMError`，其中包含由 `context` 字符串说明的错误信息，并返回 `std::unexpected`。若指针非空，则使用该指针构造并返回一个 `clore::net::detail::ObjectView` 实例，将其 `value` 成员初始化为所获取的对象。此函数是内部 JSON 类型校验和视图构建的基础依赖，被其他解析流程用于安全地获取对象句柄。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter of type `const json::Value&`
- `context` parameter of type `std::string_view`

#### Usage Patterns

- used to validate that a JSON value is an object before further processing
- works alongside `expect_string`, `expect_array` for type-safe JSON extraction

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:247`

Definition: `network/protocol.cppm:397`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数首先尝试通过 `value.get_object()` 从 `json::Cursor` 中提取底层 JSON 对象的指针。如果结果为 `nullptr`，表示当前光标位置不是一个 JSON 对象，则立即构造一个 `LLMError`，将传入的 `context` 描述嵌入到错误消息中，并通过 `std::unexpected` 返回该错误。若指针有效，则直接创建一个 `clore::net::detail::ObjectView`，将其 `value` 成员初始化为该对象指针并返回。整个过程不涉及额外的循环或递归，仅依赖 `json::Cursor` 的类型查询能力和 `ObjectView` 的简单封装。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor value` parameter

#### Usage Patterns

- Used to extract an `ObjectView` from a `json::Cursor` while validating that the cursor holds an object
- Called when parsing JSON responses that must contain a top-level object

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:256`

Definition: `network/protocol.cppm:424`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::expect_string` 首先尝试通过调用输入 `json::Value` 的 `get_string()` 方法来将其解释为字符串。如果该方法返回空值（表明该值不是一个 JSON 字符串），函数立即构造一个 `LLMError` 并返回 `std::unexpected`，其中错误消息通过 `std::format` 生成，包含调用者提供的 `context` 参数以指示错误发生的上下文。如果 `get_string()` 成功返回一个 `std::string_view`，则直接返回该视图。

该函数的控制流简单且仅依赖 `json::Value::get_string()` 的可选语义。它常被作为内部验证器使用，例如在 `clore::net::detail::expect_object` 和 `clore::net::detail::expect_array` 等高层函数中，用于从 JSON 结构提取并校验字符串字段。错误消息格式中的 `context` 参数帮助调用链快速定位是哪个字段或场景的解析失败。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (via `value.get_string()`)
- `context` (for error message formatting)

#### Usage Patterns

- Validating that a JSON value is a string
- Extracting string content from JSON during parsing
- Used in conjunction with other `expect_*` functions like `expect_array` and `expect_object`

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:259`

Definition: `network/protocol.cppm:433`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::expect_string` 函数接收一个 `json::Cursor` 和一个用于错误报告的 `std::string_view context`，执行 JSON 字符串类型检查与提取。它唯一依赖的是 `json::Cursor::get_string` 方法：若该方法返回空（即游标指向的值不是 JSON 字符串），则立即通过 `std::format` 生成包含 `context` 的错误信息，并返回一个 `std::unexpected` 包装的 `LLMError`；否则返回解引用后的 `std::string_view`。该函数无循环或分支嵌套，控制流简洁：一次调用、一次条件判断、两个返回路径——成功路径直接返回提取的字符串，失败路径构造并返回错误。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (`json::Cursor`, including the underlying JSON data accessed via `get_string()`)
- `context` parameter (`std::string_view`)

#### Usage Patterns

- extracting a string from a JSON cursor
- validating JSON string values
- error propagation for non-string JSON values

### `clore::net::detail::infer_output_contract`

Declaration: `network/protocol.cppm:631`

Definition: `network/protocol.cppm:648`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数首先检查 `PromptRequest` 中的 `response_format` 是否已设置：若有值，且 `output_contract` 为 `PromptOutputContract::Markdown`，则因冲突返回 `std::unexpected`；否则直接返回 `PromptOutputContract::Json`。若 `response_format` 为空，则检查 `output_contract`：若为 `PromptOutputContract::Unspecified`，则返回错误提示必须显式指定 `json` 或 `markdown`；否则原样返回 `output_contract`。整个过程不涉及网络调用或能力探测，仅依靠请求字段的静态决策。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.response_format`
- `request.output_contract`

#### Usage Patterns

- Called to infer or validate the output contract before constructing or processing a completion request.
- Used to ensure that `response_format` and `output_contract` are consistent.

### `clore::net::detail::insert_string_field`

Declaration: `network/protocol.cppm:215`

Definition: `network/protocol.cppm:303`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数接受一个 `json::Object` 引用以及三个 `std::string_view` 参数（键、值及未使用的上下文），其核心逻辑仅是将键和值转换为 `std::string` 后调用 `object.insert` 插入目标对象。它不执行任何验证、错误分支或条件判断，完全依赖底层 `json::Object` 的实现完成字段添加。返回值始终为成功（`std::expected<void, LLMError>`），因此其作用是一个零开销的便捷包装，用于避免调用方手动构造 `std::string` 临时对象。

#### Side Effects

- Mutates the JSON object by inserting a key-value pair.

#### Reads From

- object parameter
- key parameter
- value parameter
- context parameter

#### Writes To

- object parameter

#### Usage Patterns

- Used to add a string field to a JSON object when constructing request payloads.

### `clore::net::detail::make_empty_array`

Declaration: `network/protocol.cppm:231`

Definition: `network/protocol.cppm:348`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数通过直接解析一个空 JSON 数组的字面量 `"[]"` 来创建一个空的 `kota::codec::json::Array`。解析结果用 `std::expected` 返回：若成功则返回构造好的空数组；若 `json::parse` 失败，则将上下文和解析错误转发给 `unexpected_json_error` 并返回错误。主要依赖是 `json::parse` 和错误处理辅助函数 `unexpected_json_error`，没有额外的循环或条件分支。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `context` parameter of type `std::string_view`

#### Usage Patterns

- creating an empty JSON array as a default or placeholder
- initializing arrays in JSON construction routines

### `clore::net::detail::make_empty_object`

Declaration: `network/protocol.cppm:228`

Definition: `network/protocol.cppm:340`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数构造一个空 JSON 对象的字符串表示 `"{}"`，并通过模板函数 `json::parse<json::Object>` 解析它。如果解析失败（理论上极少发生，因为输入是固定字面量），它将调用 `unexpected_json_error`，传入参数 `context` 和解析错误信息，返回相应的错误结果；否则返回成功解析得到的 `json::Object` 实例。整个流程非常轻量，依赖 `json::parse` 和 `unexpected_json_error`，没有复杂的分支或循环。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `context` parameter
- string literal `"{}"`

#### Usage Patterns

- used to obtain a guaranteed-valid empty JSON object
- likely called when a default object is needed
- serves as a helper for initializing empty containers

### `clore::net::detail::normalize_utf8`

Declaration: `network/protocol.cppm:213`

Definition: `network/protocol.cppm:293`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::normalize_utf8` 首先调用 `clore::support::ensure_utf8` 对输入 `text` 进行 UTF-8 标准化，将任何无效的字节序列替换为合法的替换字符。然后比较标准化后的结果与原始 `text`；若二者不同，则通过 `logging::warn` 记录一条警告，指示指定 `field_name` 对应的 LLM 输出包含无效 UTF-8，并已修正。最后返回标准化后的字符串。该函数仅依赖 `clore::support::ensure_utf8` 这一外部标准化工具，控制流简单：标准化、有条件地警告、返回结果。

#### Side Effects

- Logs a warning message via `logging::warn` if invalid UTF-8 sequences are found and replaced.

#### Reads From

- The parameter `text`
- The parameter `field_name`

#### Writes To

- Returns a new `std::string` containing the normalized UTF-8 text
- Logs a warning to the logging system

#### Usage Patterns

- Sanitize UTF-8 input before JSON serialization
- Log warnings for invalid UTF-8 sequences with a descriptive field name

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:235`

Definition: `network/protocol.cppm:357`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数接收原始 JSON 字符串 `raw` 和一个描述性的 `context` 字符串，用于构造错误信息。内部依赖 `json::from_json<T>` 尝试将 `raw` 反序列化为模板类型 `T`，并将结果存入 `std::expected<T, LLMError>` 中。如果反序列化失败，则提取 `json::error` 的错误描述，与 `context` 合并后包装为 `LLMError` 并返回 `std::unexpected`。否则直接返回解析后的值。整个控制流仅包含一个解析操作和条件错误处理，不涉及其他辅助函数或复杂的数据结构。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `raw` (input JSON string)
- `context` (error context label)

#### Usage Patterns

- Used to parse JSON strings into type `T` with a context string for error reporting

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:238`

Definition: `network/protocol.cppm:368`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该实现首先将传入的 `json::Value` 通过 `json::to_string` 序列化为 `std::string`。若序列化失败（例如包含非法的浮点值），则直接调用 `unexpected_json_error`，生成一条包含上下文描述的错误信息并返回。成功序列化后，将结果字符串转发给接收 `std::string_view` 的另一重载 `parse_json_value<T>` 进行实际的类型解析。这种分层既复用了基于字符串的解析逻辑，又将 JSON 转换与类型提取清晰分离，使得错误报告能精确定位到参与操作的上下文标识。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` 参数（需序列化的 JSON 值）
- `context` 参数（错误上下文字符串）
- `json::to_string` 内部读取的 JSON 结构

#### Usage Patterns

- 从已解析的 JSON 对象中提取字段并直接解析为类型 T
- 将 JSON 值序列化为字符串以进行后续语法解析
- 在需要同时处理 JSON 值与上下文错误的场景中简化调用

### `clore::net::detail::request_text_once_async`

Declaration: `network/protocol.cppm:638`

Definition: `network/protocol.cppm:680`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数首先通过 `infer_output_contract` 推断输出契约，若失败则提前返回错误。接着处理可选的 `ResponseFormat`：若输出契约是 `Json` 且未提供响应格式，则使用不含 `schema` 的默认格式。然后构造 `CompletionRequest`，包含模型、系统提示、用户消息、响应格式、工具（工具列表根据后续上下文构建，此处为空）、工具选择以及并行工具调用选项（设置为 `std::nullopt`）。之后调用 `request_completion` 异步完成请求，并通过 `.or_fail()` 将协程错误转换为预期结果。从 `CompletionResponse` 中提取文本内容，若失败则返回错误；最后通过 `validate_prompt_output` 验证输出是否符合推断的契约，若验证通过则返回文本。整个流程依赖 `infer_output_contract`、`ResponseFormat` 处理、异步请求框架、`protocol::text_from_response` 以及输出验证函数。

#### Side Effects

- Initiates an asynchronous HTTP request to a language model API

#### Reads From

- `model` (`std::string_view`)
- `system_prompt` (`std::string_view`)
- `request` (`PromptRequest`)
- `loop` (`kota::event_loop&`)
- `request.response_format`
- `request.prompt`
- `request.tool_choice`
- `request.output_contract`
- response from `request_completion`

#### Usage Patterns

- Used to send a single prompt to a language model and obtain its text response asynchronously
- Often called with a lambda or function object that performs the actual HTTP request
- Used in contexts where only the first completion text is needed (no streaming)

### `clore::net::detail::run_task_sync`

Declaration: `network/protocol.cppm:226`

Definition: `network/protocol.cppm:322`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该实现创建一个 `kota::event_loop` 实例 `loop`，然后调用传入的可调用对象 `make_task` 并传入 `loop`，得到一个异步操作 `operation`。接着将 `operation` 调度到 `loop` 中并启动事件循环，阻塞等待所有任务完成。循环结束后，从 `operation.result()` 获取执行结果。如果结果包含错误，则将其包装为 `std::unexpected` 并返回一个 `std::expected` 的错误状态；否则，在 `T` 为非 `void` 类型时直接移动取出值，若 `T` 为 `void` 则返回一个无值构造的 `expected`。

依赖上，该函数完全基于 `kota::event_loop` 的事件驱动机制同步地将异步任务转为阻塞式调用，并使用 `std::expected` 和 `LLMError` 类型传达错误信息，不涉及网络或 JSON 解析等具体业务逻辑。

#### Side Effects

- Creates and runs an event loop that may perform I/O operations, modify shared state, or trigger callbacks.

#### Reads From

- The `make_task` callable argument
- The `operation.result()` after completion

#### Writes To

- Returns a `std::expected<T, LLMError>` containing the result or error

#### Usage Patterns

- Used to wrap asynchronous tasks in a synchronous blocking call
- Common when integrating with synchronous code paths

### `clore::net::detail::serialize_value_to_string`

Declaration: `network/protocol.cppm:241`

Definition: `network/protocol.cppm:378`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数接收一个 `json::Value` 引用和 `std::string_view` 上下文，通过调用 `json::to_string(value)` 尝试将值序列化为 JSON 字符串。若序列化成功，直接返回得到的字符串；否则构造一条描述失败上下文的消息（使用 `std::format`），并委托 `unexpected_json_error` 组装包含原始 JSON 错误码的 `std::expected` 错误结果。整个流程只包含一次非空检查和两条返回路径，依赖 `json::to_string` 以及 `unexpected_json_error` 两个辅助函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `value`
- 参数 `context`

#### Usage Patterns

- 用于将 JSON 值序列化为字符串
- 在错误处理中提供上下文信息

### `clore::net::detail::to_llm_unexpected`

Declaration: `network/protocol.cppm:221`

Definition: `network/protocol.cppm:312`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数接收一个 `Status` 类型的 `error` 以及一个 `std::string_view` 的 `context`，其核心逻辑是使用 `std::format` 将二者拼接为 `"{}: {}"` 格式的字符串，随后将该字符串传递给 `LLMError` 的构造函数，并将结果包装在 `std::unexpected` 中返回。实现本身不涉及分支或循环，唯一的外部依赖是 `LLMError` 的类型构造函数以及 `Status::to_string` 的成员调用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `error` (its `to_string()` method)
- parameter `context` (a `std::string_view`)

#### Usage Patterns

- Converting error statuses into `std::unexpected` for expected-based error handling
- Used as a utility in functions returning `std::expected` to propagate formatted errors

### `clore::net::detail::unexpected_json_error`

Declaration: `network/protocol.cppm:210`

Definition: `network/protocol.cppm:288`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数将上下文描述 `context` 与 `json::error` 对象的 `to_string()` 输出通过 `std::format` 拼接成一条错误消息，并用此消息构造一个 `LLMError` 实例，最终以 `std::unexpected` 包装返回。它充当 JSON 解析错误到 `LLMError` 错误类型的适配器，使调用方可以通过统一的错误机制报告来自 JSON 编解码器的具体错误信息。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `context` parameter of type `std::string_view`
- `err` parameter of type `const json::error &`
- `err.to_string()` which reads the internal state of `err`

#### Usage Patterns

- Convert JSON error into `LLMError` for error propagation
- Used in functions that parse JSON and need to return a standardized error type

### `clore::net::detail::validate_prompt_output`

Declaration: `network/protocol.cppm:634`

Definition: `network/protocol.cppm:666`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数基于`PromptOutputContract`枚举值进行分派。当`contract`为`PromptOutputContract::Json`时，调用`protocol::validate_json_output`验证内容是否为合法JSON；当为`PromptOutputContract::Markdown`时，调用`protocol::validate_markdown_fragment_output`执行Markdown片段校验；若为`PromptOutputContract::Unspecified`或无法识别的值，则直接返回包含相应错误描述的`std::unexpected<LLMError>`，避免向下游传递未指定的约束。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `content` parameter of type `std::string_view`
- `contract` parameter of type `PromptOutputContract`

#### Usage Patterns

- Validate LLM output format against a specified contract

### `clore::net::get_probed_capabilities`

Declaration: `network/protocol.cppm:126`

Definition: `network/protocol.cppm:729`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::get_probed_capabilities` 使用一个静态的 `std::mutex` 和一个静态的 `std::unordered_map<std::string, std::unique_ptr<ProbedCapabilities>>` 来实现线程安全的缓存。它首先通过 `std::lock_guard` 获取互斥锁，然后根据 `cache_key` 在映射中查找已存在的 `ProbedCapabilities` 实例。若找到，直接返回该实例的引用；否则，使用 `std::make_unique` 创建一个新的默认构造的 `ProbedCapabilities`，并通过 `std::unordered_map::emplace` 插入映射，最后返回新插入实例的引用。该函数不执行实际的探测逻辑，仅管理缓存的生命周期和访问同步，依赖于标准库的并发容器操作和智能指针。

#### Side Effects

- Acquires static mutex for synchronization
- May allocate memory for a new `ProbedCapabilities` object
- Modifies static cache map on first insertion

#### Reads From

- parameter `cache_key`
- static `cache` map
- static `mutex`

#### Writes To

- static `cache` map (via insertion)

#### Usage Patterns

- Used to obtain a mutable reference to probed capabilities for a given key
- Provides a lazy initialization pattern for per‑key capability probes
- Likely called by functions that need to adjust or read capabilities for a specific endpoint

### `clore::net::icontains`

Declaration: `network/protocol.cppm:768`

Definition: `network/protocol.cppm:768`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

Implementation: [Implementation](functions/icontains.md)

`clore::net::icontains` 采用朴素的 **O(n·m)** 子串搜索算法来执行不区分大小写的包含检查。函数首先进行快速长度剪枝：若 `needle` 的长度大于 `haystack` 则直接返回 `false`。随后在外层循环中枚举 `haystack` 内所有可能作为起点的位置，内层循环逐字符进行比较，每次比较前将两字符通过 `std::tolower` 统一转换为小写，一旦发现完整匹配即返回 `true`。若所有位置均未匹配，则返回 `false`。该实现完全依赖标准库的 `std::tolower` 进行大小写折叠，无其他外部依赖或动态内存分配，适用于短字符串或性能要求不高的场景。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `haystack`
- `needle`

#### Usage Patterns

- Used by `clore::net::is_feature_rejection_error` to perform case-insensitive matching on error messages.

### `clore::net::is_feature_rejection_error`

Declaration: `network/protocol.cppm:135`

Definition: `network/protocol.cppm:788`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::is_feature_rejection_error` 的实现采用了一种基于模式匹配的启发式算法。它内部维护了一个静态的 `std::string_view` 数组 `patterns`，其中包含了若干常见于 API 错误响应中表示特征被拒绝的短语，例如 `"unsupported parameter"`、`"unknown field"`、`"does not support"` 等。函数通过一个范围 for 循环遍历该数组，对每个 `pattern` 调用依赖函数 `clore::net::icontains` 进行不区分大小写的子串匹配。一旦发现任意一个模式存在于输入的 `error_message` 中，函数立即返回 `true`；若所有模式均未匹配，则返回 `false`。

该实现不依赖任何外部状态或复杂的控制流，完全依赖 `icontains` 的字符串查找能力。模式列表的选取基于经验观察，覆盖了典型 API 错误消息中对未启用功能的描述性表述。整个函数的时间复杂度为 O(N·L)，其中 N 为模式数量，L 为错误消息长度，实际运行时由于模式数量固定，可视为 O(L)。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `error_message` of type `std::string_view`
- internal constant array `patterns` of `std::string_view`

#### Usage Patterns

- used in error handling after LLM API calls to determine feature rejection
- likely called before retrying or adjusting capabilities

### `clore::net::make_capability_probe_key`

Declaration: `network/protocol.cppm:128`

Definition: `network/protocol.cppm:743`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::make_capability_probe_key` 的实现极其简单：它接收三个 `std::string_view` 参数 `provider`、`api_base` 和 `model`，然后使用 `std::format` 将它们以竖线分隔符拼接成一个 `std::string`。内部不含任何条件分支、循环或错误处理，完全依赖标准库的格式化设施。该函数不调用任何 `clore::net` 内部模块的其他函数，也没有对输入进行验证或变换，直接返回格式化后的标识符字符串。此键通常用于缓存或查找已探测到的能力信息，将三个标识组件（服务商、API 基址、模型名）编码为一个唯一的可比较键。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `provider` 作为 `std::string_view` 引用的外部字符串
- `api_base` 作为 `std::string_view` 引用的外部字符串
- `model` 作为 `std::string_view` 引用的外部字符串

#### Usage Patterns

- 用于生成能力探测结果的缓存键或标识符
- 与 `get_probed_capabilities` 等函数配合查询已探测的能力

### `clore::net::make_markdown_fragment_request`

Declaration: `network/protocol.cppm:99`

Definition: `network/protocol.cppm:144`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该函数构造并返回一个 `PromptRequest` 实例，将输入字符串直接赋给 `prompt` 字段。它将 `response_format` 设为 `std::nullopt`，并将 `output_contract` 设为枚举值 `PromptOutputContract::Markdown`。这是一个纯构造逻辑，不依赖其他函数或外部状态，也不涉及任何分支或循环。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The parameter `prompt` (via move)

#### Usage Patterns

- Creating a request for Markdown fragment generation
- Constructing a `PromptRequest` with Markdown output contract

### `clore::net::parse_rejected_feature_from_error`

Declaration: `network/protocol.cppm:137`

Definition: `network/protocol.cppm:807`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::parse_rejected_feature_from_error` 内部维护一个静态的 `field_patterns` 数组，其中每个元素为一对关键字与对应的字段名。它遍历该数组，对每个 `pair` 调用 `clore::net::icontains` 检查错误消息中是否包含关键字；一旦找到匹配，立即返回对应的字段名（例如 `"response_format"`、`"tools"` 等）。若遍历完所有模式均未命中，则返回 `std::nullopt`。整个流程仅依赖字符串包含检测，不涉及 JSON 解析或外部状态。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `error_message`

#### Usage Patterns

- Parsing error responses from LLM `APIs` to identify which feature was rejected
- Used in conjunction with `clore::net::is_feature_rejection_error` to diagnose capability constraints
- Feeds into error‑handling logic that may adjust subsequent requests (e.g., removing unsupported fields)

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:473`

Definition: `network/protocol.cppm:544`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `clore::net::protocol::append_tool_outputs` 首先验证请求的有效性：如果 `response.message.tool_calls` 为空则立即返回一个 `LLMError`。接着遍历 `outputs` 序列，将每个 `ToolOutput` 的 `tool_call_id` 与对应的 `output` 存入一个 `std::unordered_map`，同时检查 `tool_call_id` 非空且无重复键；若违反则提前返回错误。

内部控制流分为三个阶段：复制历史消息，追加一条 `AssistantToolCallMessage`（携带响应中的文本和工具调用列表），然后依据响应中的每个 `tool_call.id` 从映射中取出对应的输出，依次追加 `ToolResultMessage`。最后检查映射大小与 `response.message.tool_calls` 的尺寸是否一致，以检测是否存在未匹配的 `output_by_id` 键，若不一致则返回错误。如果所有检查通过，返回合并后的完整消息向量。该函数依赖 `clore::net` 命名空间下的消息类型、`ToolOutput` 和 `LLMError`，并使用标准库容器与格式化功能。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `history` (span of `Message`)
- `response.message.text` (string)
- `response.message.tool_calls` (span of `ToolCall`)
- `outputs` (span of `ToolOutput`)
- each `output.tool_call_id`
- each `output.output`

#### Writes To

- the returned merged message list (`std::expected<std::vector<Message>, LLMError>`)

#### Usage Patterns

- Used to incorporate tool execution results back into the conversation history
- Called after a `CompletionResponse` with non‑empty `tool_calls` is received
- Replaces manual construction of assistant and tool result messages

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:479`

Definition: `network/protocol.cppm:592`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `clore::net::protocol::parse_response_text` 的实现首先调用 `clore::net::protocol::text_from_response` 从传入的 `CompletionResponse` 中提取原始文本。若提取失败（返回 `std::expected` 的无值状态），则直接转发该错误；否则将获取的字符串传递给 `kota::codec::json::from_json<T>` 以解析为模板参数 `T` 所指定的类型。如果 JSON 解析出错，构造一个 `LLMError` 并附带格式化的错误消息；若解析成功，则返回解析得到的值。该函数的核心控制流为顺序的两步操作：文本提取与 JSON 反序列化，依赖 `clore::net::protocol::text_from_response` 和 `kota::codec::json::from_json` 完成具体工作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `response` parameter
- `text_from_response` return value

#### Usage Patterns

- parse structured LLM response into type `T`
- convert `CompletionResponse` to `std::expected<T, LLMError>`

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:482`

Definition: `network/protocol.cppm:607`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `parse_tool_arguments` 将 `ToolCall` 中的 `arguments` JSON 值序列化为字符串，然后尝试将结果字符串反序列化为目标类型 `T`。若任一操作失败，它会构造一个包含上下文信息（工具名称和底层错误消息）的 `LLMError` 并返回 `std::unexpected`。该实现依赖于 `kota::codec::json::to_string` 进行序列化，以及 `kota::codec::json::from_json` 进行反序列化，所有核心逻辑均在单个顺序流程中执行，无分支或循环。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `call` parameter
- `call.arguments`
- `call.name`
- implicit reads from `kota::codec::json::to_string` and `kota::codec::json::from_json`

#### Usage Patterns

- Invoked to deserialize tool call arguments into a concrete type for further processing
- Commonly used in tool execution or validation pipelines

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:471`

Definition: `network/protocol.cppm:528`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数通过顺序检查 `CompletionResponse` 中 `message` 字段的三个条件来提取纯文本结果。首先验证 `response.message.refusal` 是否被设置，若存在则立即返回一个包含 LLM 拒绝内容的错误。接着检查 `response.message.tool_calls` 是否非空，若包含工具调用则返回错误，表明响应应包含文本而非调用。最后确认 `response.message.text` 是否持有值，若缺失则返回无文本载荷的错误。仅当所有检查通过后，才解引用 `response.message.text` 并返回其内容字符串。

整个流程不依赖复杂的算法，而是直接利用 `CompletionResponse` 的结构化成员进行守卫式条件判断。函数的实现依赖于 `CompletionResponse` 中 `message` 所嵌套的 `refusal`、`tool_calls` 和 `text` 字段，并通过 `std::expected` 返回成功结果或 `LLMError` 类型的错误。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `response.message.refusal`
- `response.message.tool_calls`
- `response.message.text`

#### Usage Patterns

- extracting text output from a `CompletionResponse`
- validating that a response is a text completion before further processing
- used in conjunction with error handling via `std::expected`

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:488`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `clore::net::protocol::validate_json_output` 的实现核心是调用 `kota::codec::json::parse` 将传入的 `std::string_view` 解析为 `kota::codec::json::Value`。如果解析失败（即 `parsed` 不包含值），则立即返回一个构造的 `std::unexpected`，其中包含 `LLMError`，错误消息使用 `std::format` 拼接解析错误文本。若解析成功，函数顺利返回 `std::expected<void, LLMError>` 的无错误状态。整个算法不涉及递归或循环，仅依赖 JSON 解析器的底层实现，控制流仅表现为一次条件分支。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `content` 指向的字符串视图

#### Usage Patterns

- 用于校验 LLM 输出格式是否为合法 JSON

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:497`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数首先对输入 `content` 执行一系列格式校验。它通过 `content.empty()` 检测空输出，然后使用 `std::isspace` 遍历字符确认是否仅含空白。接着搜索禁止的 Markdown 结构：以 `"# "` 开头或包含 `"\n# "` 的子串均视为 H1 标题，而 `"``\`"`（即三个反引号）的出现则代表代码围栏。若任一校验失败，函数立即返回包装为 ``std::unexpected`` 的 ``LLMError``。最终，它委托给 ``clore::net::protocol::validate_json_output`` 尝试将内容解析为 JSON——若成功，说明输出为 JSON 而非预期的 Markdown 片段，同样返回错误。

整个控制流为顺序短路判断，依赖标准库字符串操作和 `validate_json_output` 的 JSON 检测能力。需要注意的是，代码围栏的查找仅使用简单的 `std::string::find`，不处理嵌套或转义情况；H1 标题的判断仅针对 `# ` 后紧跟空格的行首模式，不覆盖其他可能标题形式（如 `#` 后无空格）。这些设计选择构成了该验证器的轻量级特性。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `content` parameter

#### Usage Patterns

- Called during LLM response validation to ensure output is a valid markdown fragment

### `clore::net::sanitize_request_for_capabilities`

Declaration: `network/protocol.cppm:132`

Definition: `network/protocol.cppm:749`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该函数按顺序检查 `ProbedCapabilities` 中的四个原子标志（均以 `memory_order_relaxed` 读取），并就地修改传入的 `CompletionRequest` 副本以移除不受支持的特性。它依次判断 `supports_json_schema`、`supports_tool_choice`、`supports_parallel_tool_calls` 和 `supports_tools`，若某标志为 `false`，则分别清除 `request.response_format->schema`、`request.tool_choice`、`request.parallel_tool_calls` 或清空 `request.tools` 容器。所有操作均为原子位测试和可选字段擦除，不涉及 JSON 解析或外部 I/O，最终返回修剪后的请求副本。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `caps.supports_json_schema`
- `caps.supports_tool_choice`
- `caps.supports_parallel_tool_calls`
- `caps.supports_tools`
- `request.response_format`
- `request.tool_choice`
- `request.parallel_tool_calls`
- `request.tools`

#### Writes To

- `request.response_format->schema`
- `request.tool_choice`
- `request.parallel_tool_calls`
- `request.tools`

#### Usage Patterns

- Callers use this function to strip unsupported features from a completion request before sending it to a provider that may not support them.
- Commonly used with `ProbedCapabilities` obtained from `get_probed_capabilities`.

## Internal Structure

`protocol` 模块位于 `clore::net` 命名空间，负责定义并实现与 LLM 完成 API 交互的核心数据结构与协议逻辑。它通过依赖 `http` 模块进行网络请求，并利用 `support` 模块提供字符串规范化、错误摘录等辅助功能。模块内部明确分为两层：公共接口暴露了 `CompletionRequest`、`CompletionResponse`、`Message`、`ToolCall` 等数据模型，以及 `parse_tool_arguments`、`validate_json_output`、`sanitize_request_for_capabilities` 等协议处理函数；而 `detail` 子命名空间则封装了基于 `kota::codec::json` 的 JSON 类型检查（如 `expect_array`、`expect_object`）、值克隆（`clone_value`、`clone_array`、`clone_object`）以及异步请求编排（`request_text_once_async`）等底层实现细节。这种分解使得上层使用方只需关注类型安全的请求构建与响应解析，而无需直接处理 JSON 序列化、能力探测或网络传输等复杂问题。

## Related Pages

- [Module http](../http/index.md)
- [Module support](../support/index.md)

