---
title: 'Module protocol'
description: '该模块封装了与 LLM（大语言模型）进行交互所需的全部协议定义和数据处理逻辑。它公开了结构化请求/响应类型（如 CompletionRequest、CompletionResponse、PromptRequest、ResponseFormat、ToolCall 等）、多种消息变体（系统、用户、助手、工具结果）以及工具选择策略（ToolChoice 别名及其具体模式）。通过 ProbedCapabilities 和能力探测函数（get_probed_capabilities、sanitize_request_for_capabilities），模块允许调用方根据端点支持的动态特性（如 JSON Schema、工具、并行工具调用）调整请求内容。'
layout: doc
template: doc
---

# Module `protocol`

## Summary

该模块封装了与 LLM（大语言模型）进行交互所需的全部协议定义和数据处理逻辑。它公开了结构化请求/响应类型（如 `CompletionRequest`、`CompletionResponse`、`PromptRequest`、`ResponseFormat`、`ToolCall` 等）、多种消息变体（系统、用户、助手、工具结果）以及工具选择策略（`ToolChoice` 别名及其具体模式）。通过 `ProbedCapabilities` 和能力探测函数（`get_probed_capabilities`、`sanitize_request_for_capabilities`），模块允许调用方根据端点支持的动态特性（如 JSON Schema、工具、并行工具调用）调整请求内容。

在公开实现范围内，模块还提供了一系列协议级辅助函数，包括从响应中提取文本（`protocol::text_from_response`、`protocol::parse_response_text`）、验证输出格式（`protocol::validate_json_output`、`protocol::validate_markdown_fragment_output`）、解析工具参数（`protocol::parse_tool_arguments`）以及将工具输出追加到对话历史（`protocol::append_tool_outputs`）。内部 `detail` 命名空间则提供了 JSON 对象/数组的视图、克隆、序列化、验证等底层工具，这些工具被上层函数广泛使用，但对外接口保持私有。模块依赖 `http`（HTTP 通信）和 `support`（通用工具与缓存）模块，自身专注于协议数据的定义与转换。

## Imports

- [`http`](../http/index.md)
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

Declaration: `src/network/protocol.cppm:43`

Definition: `src/network/protocol.cppm:43`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::AssistantMessage` 是一个简单的聚合体，其唯一成员 `content` 为 `std::string` 类型，用于存储该辅助消息的文本载荷。内部没有额外的构造函数或成员函数，因此 `content` 的构造与析构完全交由 `std::string` 管理，不强制执行任何运行时不变量；实例的默认初始化会得到一个空字符串。该类作为网络协议中消息分发体系的一部分，承担着承载纯文本辅助信息的角色，其实现仅依赖于字符串的常规生命周期与值语义。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::AssistantOutput` 的实现包含三个数据成员：`std::optional<std::string> text`、`std::optional<std::string> refusal` 以及 `std::vector<ToolCall> tool_calls`。这些成员在默认构造时会被正确初始化为空状态——两个可选字段均为 `std::nullopt`，向量为空。所有成员均为公有，外部可直接修改其内容。

该结构体没有自定义的构造、析构、拷贝或移动操作，编译器生成的默认实现会逐一处理各成员。因此拷贝和移动语义完全由各成员的标准行为决定：`std::optional` 的拷贝会复制底层字符串（若存在），`std::vector` 的拷贝则复制整个元素序列并在堆上新分配内存。`tool_calls` 的堆内存由 `std::vector` 内部管理，而可选字符串的存储空间位于 `std::optional` 内部（可能包含小对象优化），整个类型无需额外的资源回收代码。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::AssistantToolCallMessage` 是一个聚合结构体，其内部状态仅由两个数据成员定义：`content`（`std::optional<std::string>`）用于携带选项性的文本回复，以及 `tool_calls`（`std::vector<ToolCall>`）用于记录待执行的工具调用序列。这两个成员共同表示一个助手消息的有效载荷，但结构体本身并未对它们之间的存在关系施加任何不变约束——调用方需根据消息的语义自行决定哪个成员被填充；通常二者之一为有效状态。重要的成员实现全部由编译器隐式生成，包括默认构造函数、析构函数及复制/移动操作，结构体不包含自定义的资源管理或验证逻辑，其行为等同于一个普通的公有字段容器。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::CompletionRequest` 是一个聚合体，其数据成员全部通过默认成员初始化器建立安全的初始状态：`model` 默认为空字符串，`messages` 默认为空 `std::vector`，其余 `std::optional` 字段均默认为空值。这种设计保证了无论以何种方式构造，该结构体都至少处于一个可接受的默认基线状态，而无需在调用方显式初始化。字段 `tools`、`tool_choice` 与 `parallel_tool_calls` 之间的逻辑联系——例如，当 `tool_choice` 非空时要求 `tools` 也非空——并未在结构体内部施加编译器强制约束，而是在协议层的处理函数中进行校验与协调。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构 `clore::net::CompletionResponse` 是一个纯数据聚合，包含四个公开数据成员：`id` 和 `model` 均为 `std::string` 类型，`message` 为 `AssistantOutput` 类型，`raw_json` 为 `std::string` 类型。该结构体的实现不引入自定义构造函数、析构函数或赋值操作，因此所有成员的初始化与赋值均遵循默认的逐成员语义。内部不变量上，`raw_json` 预期保持与 `id`、`model` 及 `message` 在逻辑上的一致性，但这一约束由构造点或赋值点上的调用方负责，而非结构体本身实施。

### `clore::net::ForcedFunctionToolChoice`

Declaration: `src/network/protocol.cppm:82`

Definition: `src/network/protocol.cppm:82`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体包含一个 `std::string` 成员 `name`，用于存储强制选择的工具名称。`name` 是唯一的数据成员，在整个对象生命周期内保持其值不变，没有其他约束或不变式。该类型没有自定义构造函数、析构函数或拷贝/移动操作，因此完全依赖于编译器生成的默认实现，所有成员的默认初始化将导致 `name` 为空字符串。任何对该结构体的操作均通过直接访问 `name` 完成，不涉及额外的内部状态或资源管理。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体 `clore::net::FunctionToolDefinition` 是一个聚合体，通过四个成员字段定义工具函数的元数据：`name` 和 `description` 分别为 `std::string` 类型，存储函数名称与描述；`parameters` 使用 `kota::codec::json::Object` 类型，以 JSON 对象的形式描述工具参数结构；`strict` 为 `bool` 类型，默认值为 `true`，用于指示是否启用严格模式。设计上，`strict` 的默认值构成一个重要的不变量，确保工具默认采用严格模式，无需显式初始化。所有成员均直接公开，便于外部填充。

### `clore::net::Message`

Declaration: `src/network/protocol.cppm:57`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::Message` 是一个 `std::variant` 别名，用于在单次存储中表示网络协议中所有合法的消息变体。其内部由五种具体消息类型组成：`SystemMessage`、`UserMessage`、`AssistantMessage`、`AssistantToolCallMessage` 和 `ToolResultMessage`。采用 `std::variant` 而非类继承，保证了值语义与栈上分配，避免了虚函数开销，同时通过编译期类型擦除提供类型安全的访问。核心不变性在于 `Message` 实例始终持有且仅持有上述类型之一，且各类型的设计保证了消息字段的合法性（例如时间戳非空、角色字段匹配）。重要的成员实现继承自 `std::variant` 的隐式默认构造、析构与移动语义，关键交互通过 `std::visit` 模式完成，在消息分发时由编译器生成跳转表，无需运行时 `if-else` 链。

### `clore::net::ProbedCapabilities`

Declaration: `src/network/protocol.cppm:131`

Definition: `src/network/protocol.cppm:131`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该结构体通过四个 `std::atomic<bool>` 字段以线程安全的方式记录远程端点对特定功能的支持情况。所有字段（`supports_json_schema`、`supports_tool_choice`、`supports_parallel_tool_calls` 和 `supports_tools`）均默认初始化为 `true`，体现了乐观假设：在完成实际探测前，假定端点支持所有能力。每个原子字段的设计保证了在不引入锁的情况下，可由探测线程安全写入，并由消费线程无竞争读取，从而维护了内部状态的一致性。值得注意的是，该结构体不提供任何成员函数来封装更新或读取逻辑，所有操作均直接通过公开字段上的原子加载/存储完成，这要求调用方自行保证探测与查询之间的操作顺序。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该枚举是一个强类型枚举，底层类型固定为 `std::uint8_t`，其三个枚举项按声明顺序隐式对应整数值 `0`、`1`、`2`。内部使用时，通常通过 `switch` 语句对 `clore::net::PromptOutputContract` 进行穷举匹配，以确保所有合约类型均被处理；当需要序列化或网络传输时，则安全地将枚举值转换为底层整数类型。枚举值仅表示状态，不附带额外数据，因此其逻辑不变式即确保枚举值始终落在 `Unspecified`、`Json` 或 `Markdown` 的范围内，任何超出此范围的值均视为非法。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
Json
```

##### `clore::net::PromptOutputContract::Markdown`

Declaration: `src/network/protocol.cppm:101`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
Markdown
```

##### `clore::net::PromptOutputContract::Unspecified`

Declaration: `src/network/protocol.cppm:99`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

###### Implementation

```cpp
Unspecified
```

### `clore::net::PromptRequest`

Declaration: `src/network/protocol.cppm:104`

Definition: `src/network/protocol.cppm:104`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::PromptRequest` 是一个聚合体，用于封装一次提示请求所需的全部数据。其成员包括：必填的 `prompt` 字符串，以及三个可选或默认的配置字段——`response_format` 和 `tool_choice` 通过 `std::optional` 表示可选性，`output_contract` 则直接以 `PromptOutputContract::Unspecified` 作为默认值。这种设计使得构造请求时只需提供核心提示内容，其余扩展选项可根据需要单独设置，而 `output_contract` 的默认值保证了在未显式指定时的预期行为。

在内部布局上，该结构未定义任何构造函数、析构函数或赋值运算符，完全依赖编译器生成的默认实现，因此所有成员按声明顺序连续存储。不变量的维护主要依赖于成员自身的类型约束和外部代码在赋值前的校验，例如 `response_format` 和 `tool_choice` 的可选性避免了无效状态，而 `output_contract` 的枚举类型限制了取值范围。这种简单的点聚合形式适合作为网络协议层的数据载体，便于序列化与反序列化。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ResponseFormat` 是一个简单的聚合类型，其内部状态由三个数据成员构成：`name` 是一个 `std::string`，用于标识响应格式的名称；`schema` 是一个 `std::optional<kota::codec::json::Object>`，可选地携带描述格式结构的 JSON 对象；`strict` 是一个 `bool`，默认值为 `true`，指示格式的严格性。成员之间没有额外的约束或不变量 —— 默认构造将产生 `name` 为空字符串、`schema` 为 `std::nullopt` 且 `strict` 为 `true` 的实例，所有成员均可通过直接初始化或聚合初始化自由设定。这种设计使 `ResponseFormat` 能够作为轻量级的数据载体，在请求处理链中传递格式信息而不引入复杂的生命周期或校验逻辑。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::SystemMessage` 的内部结构仅包含一个 `std::string` 类型的 `content` 成员，用于直接承载消息的有效载荷。该结构体没有自定义构造函数、析构函数或赋值运算符，所有特殊成员函数均由编译器隐式生成，因此其不变性等价于 `std::string` 的不变性：`content` 可以包含任意有效的 UTF‑8 字节序列（或任意二进制数据，取决于调用方解释）。成员 `content` 的生命周期与 `SystemMessage` 对象完全同步，拷贝或移动该结构体时将对应地拷贝或移动底层的 `std::string` 数据。

#### Invariants

- `content` 成员可以包含任意有效的 `std::string` 值

#### Key Members

- `content`：存储消息文本的字符串

#### Usage Patterns

- 其他代码创建 `SystemMessage` 对象并设置 `content` 后，通过网络发送或接收

### `clore::net::ToolCall`

Declaration: `src/network/protocol.cppm:36`

Definition: `src/network/protocol.cppm:36`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolCall` 内部以四个字段承载一个工具调用的完整描述。`id` 和 `name` 分别以 `std::string` 保存调用的唯一标识符和函数名。参数以两种冗余形式存储：原始 JSON 字符串 `arguments_json` 和解析后的结构化对象 `arguments`（类型 `kota::codec::json::Value`）。在实现中，这两个参数表示保持同步——每当通过公共接口写入其中一个时，另一个会通过序列化或反序列化自动更新，从而保证 `arguments` 始终是 `arguments_json` 的准确结构化视图，反之亦然。这种设计在需要直接操作 JSON 树与需要原始字符串传输的场景之间提供了无缝转换。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoice` 是基于 `std::variant` 的类型别名，将四种互斥的工具选择模式组合为一个 sum type：`ToolChoiceAuto`、`ToolChoiceRequired`、`ToolChoiceNone` 与 `ForcedFunctionToolChoice`。该实现利用 `std::variant` 的运行时类型安全机制，保证任一时刻仅存储一个备选值，并通过标准库提供的 `std::visit`、`std::get` 等工具进行类型安全的分发与访问。variant 的内部布局由实现定义，但通常为各备选类型提供紧凑的存储和高效的索引切换，从而在不引入继承虚表开销的前提下支持工具选择策略的多态性。

### `clore::net::ToolChoiceAuto`

Declaration: `src/network/protocol.cppm:76`

Definition: `src/network/protocol.cppm:76`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoiceAuto` 是一个完全空的结构体类型，不含任何数据成员、基类或成员函数。其内部结构仅由编译器隐式生成的默认构造函数、析构函数和拷贝/移动操作构成，无任何自定义实现或额外的不变性约束。该类型作为纯粹的标记类型（tag type）在编译期用于类型分派，通过重载或特化机制区分不同的工具选择策略，不携带运行时状态。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ToolChoiceNone` 是一个空类类型，没有任何数据成员或基类。其内部结构仅包含编译器生成的默认构造函数、析构函数、拷贝和移动操作。由于没有成员变量，该类型不维护任何运行时不变量，纯粹作为编译期标记使用，用于在 `clore::net` 命名空间下表示“不进行工具选择”的策略。该实现不定义任何用户提供的成员函数，所有操作均为隐式声明，因而不会产生额外的运行时开销或状态。

#### Invariants

- 不包含任何数据成员
- 不可包含状态

#### Usage Patterns

- 证据中未展示具体使用模式

### `clore::net::ToolChoiceRequired`

Declaration: `src/network/protocol.cppm:78`

Definition: `src/network/protocol.cppm:78`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoiceRequired` 是一个空结构体，不包含任何数据成员或基类。其实现仅依赖于编译器隐式生成的默认构造函数、析构函数以及复制/移动操作，不引入任何额外的运行时开销或不变式约束。该类型在代码库中主要作为标记或占位符使用，用于标识特定的策略或配置选择，其空状态使得实例化成本为零。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolOutput` 是一个仅包含两个 `std::string` 成员的聚合结构体：`tool_call_id` 和 `output`。其内部结构直接映射到网络协议中工具调用的返回结果，其中 `tool_call_id` 唯一标识生成该输出的工具调用实例，`output` 携带调用产生的文本内容。该结构体没有自定义构造函数、析构函数或成员函数，依赖编译器生成的默认实现；其不变量由调用方保证——`tool_call_id` 必须与先前发出的工具调用请求中的标识符匹配，且 `output` 不应为 `nullptr` 或未初始化状态（但在当前 C++ 标准下以默认构造的 `std::string` 存在时行为定义）。该类型作为网络消息载荷的一部分，在序列化/反序列化过程中直接传递这两个字符串。

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

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

结构体 `clore::net::ToolResultMessage` 是一个简单的聚合体，内部仅包含两个公开的 `std::string` 成员：`tool_call_id` 和 `content`。它未声明任何构造函数、析构函数或成员函数，因此所有字段均可直接读写，不变量完全由使用者保障——通常要求 `tool_call_id` 标识一次具体的工具调用，而 `content` 承载其结果。在实现中，该结构体常作为轻量级数据传输对象，依赖外部序列化/反序列化逻辑在网络上传递，自身不包含任何处理或验证逻辑。

### `clore::net::UserMessage`

Declaration: `src/network/protocol.cppm:32`

Definition: `src/network/protocol.cppm:32`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

实现基于 `std::string` 成员 `clore::net::UserMessage::content` 的存储与生命周期管理，整个结构体作为字符串的简单包装器。内部不变量仅限 `content` 遵循 `std::string` 标准的有效状态（包括空字符串），未引入额外约束。所有特殊成员函数（默认构造、拷贝、移动、析构）由编译器隐式生成，并委托给 `std::string` 的对应操作，因此资源管理、异常安全及其它行为完全由标准库保证。

#### Invariants

- `content` 可以是任意合法的 `std::string` 值，包括空字符串

#### Key Members

- `content`

#### Usage Patterns

- 其他代码通过直接读写 `content` 成员来设置或获取用户消息内容
- 可能被序列化或直接通过网络发送

### `clore::net::detail::ArrayView`

Declaration: `src/network/protocol.cppm:190`

Definition: `src/network/protocol.cppm:190`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

结构体 `clore::net::detail::ArrayView` 仅包含一个指向 `const kota::codec::json::Array` 的指针成员 `value`，默认初始化为 `nullptr`。核心不变条件是 `value` 必须始终指向一个有效的数组对象，否则调用任何成员函数会导致解引用空指针的未定义行为。通过这一指针间接访问底层数组，实现零开销的只读视图。

所有成员函数直接委托给底层 `Array` 对象：`operator*` 和 `operator->` 分别返回数组的引用与指针；`begin()` 与 `end()` 返回常量迭代器；`size()` 与 `empty()` 转发底层调用；`operator[]` 根据索引返回常量值引用。全部方法均声明为 `const` 且多为 `noexcept`，体现纯粹的转发语义，不进行任何额外校验或复制。

#### Invariants

- `value` is expected to point to a valid, non-null `kota::codec::json::Array` when any member function is called
- The view does not own the underlying `Array` object; the referenced object must outlive the view

#### Key Members

- `value` (the underlying pointer)
- `begin` / `end` (iteration support)
- `size` / `empty` (size queries)
- `operator[]` (indexed access)
- `operator->` / `operator*` (direct access to the underlying `Array`)

#### Usage Patterns

- Passed by value to functions that need read-only access to a JSON array without copying
- Returned from functions that expose a reference to a JSON array stored elsewhere
- Used to iterate over array elements via range-based for loops

#### Member Functions

##### `clore::net::detail::ArrayView::begin`

Declaration: `src/network/protocol.cppm:201`

Definition: `src/network/protocol.cppm:201`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto begin() const noexcept {
        return value->begin();
    }
```

##### `clore::net::detail::ArrayView::empty`

Declaration: `src/network/protocol.cppm:193`

Definition: `src/network/protocol.cppm:193`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto empty() const noexcept -> bool {
        return value->empty();
    }
```

##### `clore::net::detail::ArrayView::end`

Declaration: `src/network/protocol.cppm:205`

Definition: `src/network/protocol.cppm:205`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto end() const noexcept {
        return value->end();
    }
```

##### `clore::net::detail::ArrayView::operator*`

Declaration: `src/network/protocol.cppm:217`

Definition: `src/network/protocol.cppm:217`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator*() const noexcept -> const kota::codec::json::Array& {
        return *value;
    }
```

##### `clore::net::detail::ArrayView::operator->`

Declaration: `src/network/protocol.cppm:213`

Definition: `src/network/protocol.cppm:213`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator->() const noexcept -> const kota::codec::json::Array* {
        return value;
    }
```

##### `clore::net::detail::ArrayView::operator[]`

Declaration: `src/network/protocol.cppm:209`

Definition: `src/network/protocol.cppm:209`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator[](std::size_t index) const -> const kota::codec::json::Value& {
        return (*value)[index];
    }
```

##### `clore::net::detail::ArrayView::size`

Declaration: `src/network/protocol.cppm:197`

Definition: `src/network/protocol.cppm:197`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto size() const noexcept -> std::size_t {
        return value->size();
    }
```

### `clore::net::detail::ObjectView`

Declaration: `src/network/protocol.cppm:168`

Definition: `src/network/protocol.cppm:168`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::ObjectView` 是一个轻量级的非拥有视图，其内部仅包含一个指向 `const kota::codec::json::Object` 的指针成员 `value`，默认初始化为 `nullptr`。所有对底层对象的访问（迭代器、解引用、成员访问）均直接通过该指针转发，因此调用方必须保证 `value` 指向一个有效对象以避免未定义行为。`begin` 和 `end` 成员简单地返回 `value->begin()` 和 `value->end()`，从而直接暴露底层 JSON 对象的迭代器范围。`get` 成员方法接收 `std::string_view` 键名并返回 `std::optional<kota::codec::json::Cursor>`，其实现依赖于底层对象的查找逻辑；由于没有在视图内部进行空指针检查，使用时需确保 `value` 非空。

#### Invariants

- The pointer `value` may be null.
- If `value` is not null, it points to a valid, fully constructed JSON object.
- The view does not own or extend the lifetime of the pointed-to object.

#### Key Members

- `value` field
- `get(std::string_view)` method
- `begin()` / `end()` methods
- `operator->()` and `operator*()`

#### Usage Patterns

- Passed by value as a safe, non-owning handle to a JSON object.
- Used to iterate over JSON object members without copying.
- Employed in network protocol code for parsing and inspecting JSON data.

#### Member Functions

##### `clore::net::detail::ObjectView::begin`

Declaration: `src/network/protocol.cppm:173`

Definition: `src/network/protocol.cppm:173`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto begin() const noexcept {
        return value->begin();
    }
```

##### `clore::net::detail::ObjectView::end`

Declaration: `src/network/protocol.cppm:177`

Definition: `src/network/protocol.cppm:177`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto end() const noexcept {
        return value->end();
    }
```

##### `clore::net::detail::ObjectView::get`

Declaration: `src/network/protocol.cppm:171`

Definition: `src/network/protocol.cppm:292`

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

Declaration: `src/network/protocol.cppm:185`

Definition: `src/network/protocol.cppm:185`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator*() const noexcept -> const kota::codec::json::Object& {
        return *value;
    }
```

##### `clore::net::detail::ObjectView::operator->`

Declaration: `src/network/protocol.cppm:181`

Definition: `src/network/protocol.cppm:181`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

###### Implementation

```cpp
auto operator->() const noexcept -> const kota::codec::json::Object* {
        return value;
    }
```

## Functions

### `clore::net::detail::clone_array`

Declaration: `src/network/protocol.cppm:280`

Definition: `src/network/protocol.cppm:454`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::clone_array` 通过解引用 `ArrayView` 的 `value` 成员获取底层 `json::Array` 的常量引用，并直接返回其副本，从而创建一个完整的数组深拷贝。该函数忽略第二个 `std::string_view` 参数（通常用于错误上下文），其控制流仅包含一个返回表达式，不涉及迭代或条件分支。实现依赖于 `json::Array` 的拷贝构造函数以及 `ArrayView` 的 `operator*` 解引用操作，是系列中用于将视图对象安全地转换为独立所有权 JSON 值的简洁辅助方法。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source` parameter, specifically `source.value`

#### Usage Patterns

- Cloning JSON arrays during validation or transformation stages.
- Producing independent copies of array data for further processing.

### `clore::net::detail::clone_object`

Declaration: `src/network/protocol.cppm:277`

Definition: `src/network/protocol.cppm:463`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数直接将源 `clore::net::detail::ObjectView` 的底层 `json::Object` 值通过复制构造到返回值中。它没有遍历或修改内容，而是依赖 `json::Object` 的拷贝语义来完成深层复制。第二个参数 `std::string_view` 当前未被使用，通常用于为可能的错误提供上下文，但在此实现中路径始终成功。实现与外部错误处理或日志记录之间无交互，仅通过 `std::expected<json::Object, LLMError>` 类型传递结果。

#### Side Effects

- Allocates a new `json::Object` via its copy constructor, involving heap memory allocation

#### Reads From

- The `json::Object` pointed to by `source.value`
- The second `std::string_view` parameter (unused in the body)

#### Writes To

- Heap memory for the newly allocated `json::Object`
- The return value of type `std::expected<json::Object, LLMError>`

#### Usage Patterns

- Used to obtain an independent copy of a JSON object from an `ObjectView`
- Employed in JSON processing functions that require a mutable or owned `json::Object` without aliasing the source

### `clore::net::detail::clone_object`

Declaration: `src/network/protocol.cppm:274`

Definition: `src/network/protocol.cppm:458`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::clone_object` 的实现在内部直接依赖 `kota::codec::json::Object` 的复制构造函数，通过从源对象 `source` 复制构造来创建一个新的 `json::Object` 实例。算法本身不涉及循环或分支，只进行一次复制操作。第二个参数 `std::string_view` 在当前实现中未被使用，可能预留用于错误上下文。函数返回 `std::expected<json::Object, LLMError>`，因此复制失败时（通常不会）可通过 `LLMError` 报告错误，但典型的复制构造函数不抛出异常，所以此函数基本不影响控制流。整个流程线性且无状态依赖，仅依赖于 json 库的内部复制机制。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source` parameter (const `json::Object&`)

#### Writes To

- returned `std::expected<json::Object, LLMError>` containing the new `json::Object`

#### Usage Patterns

- creating an independent copy of a JSON object to avoid aliasing or shared ownership

### `clore::net::detail::clone_value`

Declaration: `src/network/protocol.cppm:283`

Definition: `src/network/protocol.cppm:467`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::clone_value` 通过复制给定的 `json::Value` 来创建一个独立且完全独立的副本。它接收一个 `std::string_view context` 参数，该参数仅在内部用于抑制编译器警告，不参与实际逻辑。函数直接返回通过调用 `json::Value(value)` 构造的新值，本质上是标准复制语义。该实现没有额外的控制流、错误处理或对输入值的结构化遍历，完全依赖 `json::Value` 的复制构造函数来确保数据的深度复制。

#### Side Effects

- Allocates memory for a deep copy of the JSON value.

#### Reads From

- source parameter (`const json::Value &`)

#### Writes To

- Returned `std::expected<json::Value, LLMError>` object containing the cloned value

#### Usage Patterns

- Deep-copy a `json::Value` while discarding the context

### `clore::net::detail::excerpt_for_error`

Declaration: `src/network/protocol.cppm:235`

Definition: `src/network/protocol.cppm:328`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::excerpt_for_error` 实现为对一个固定长度 UTF-8 截断工具的简单包装。它定义了一个局部常量 `kMaxBytes`，值为 `200`，并将输入字符串 `body` 委托给 `clore::support::truncate_utf8` 进行处理，返回截断后的结果。

该实现不包含分支或循环，其核心依赖是 `clore::support::truncate_utf8`，它负责确保截断边界不会破坏 UTF-8 字符的完整性。整个函数旨在生成一个长度上限为 `kMaxBytes` 的错误信息摘要，避免在错误报告中暴露过长的原始内容。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `body` parameter

#### Usage Patterns

- truncating long response bodies for error messages
- used in error-handling code paths to limit output length

### `clore::net::detail::expect_array`

Declaration: `src/network/protocol.cppm:265`

Definition: `src/network/protocol.cppm:427`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::expect_array` 通过调用 `json::Cursor::get_array` 尝试从输入的 `value` 中提取底层 JSON 数组指针。若指针为 `nullptr`，表示当前 JSON 值不是数组，则使用 `context` 字符串构造一条错误信息并返回 `std::unexpected<LLMError>`。若成功，则直接通过聚合初始化构造一个 `clore::net::detail::ArrayView` 对象，将其 `.value` 成员设为该数组指针，并作为 `std::expected<ArrayView, LLMError>` 的合法值返回。该函数不执行深度复制或元素校验，仅完成类型检查与视图封装，依赖 `ArrayView` 的聚合初始化特性和 `json::Cursor` 的数组访问接口。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (a `json::Cursor`)
- `context` parameter (a `std::string_view`)

#### Usage Patterns

- Used in parsing JSON arrays from network responses
- Called when validating tool arguments or metadata
- Often combined with `expect_string`, `expect_object`, and `clone_array`

### `clore::net::detail::expect_array`

Declaration: `src/network/protocol.cppm:262`

Definition: `src/network/protocol.cppm:418`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::expect_array` 验证给定的 `json::Value` 是否为 JSON 数组。它调用 `value.get_array()` 获取底层数组指针；若指针为空则返回 `std::unexpected`，附带格式化的错误消息（形如 `"{context} is not a JSON array"`）；否则构造并返回一个 `ArrayView` 实例，该实例包装了数组指针。此函数依赖于 `ArrayView` 结构体和 `LLMError` 类型，用于统一错误处理和数组视图抽象。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (`const json::Value&`)
- `context` parameter (`std::string_view`)

#### Usage Patterns

- Called to safely retrieve a JSON array from a value during parsing or validation
- Used in contexts where a JSON array is expected and an error should be reported otherwise

### `clore::net::detail::expect_object`

Declaration: `src/network/protocol.cppm:256`

Definition: `src/network/protocol.cppm:400`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数对传入的 `json::Value` 调用 `get_object()` 以尝试提取底层 JSON 对象。若返回空指针，则通过 `std::format` 构造一条包含给定 `context` 标识的错误信息，并作为 `std::unexpected` 包裹的 `LLMError` 返回。否则，用获取到的对象指针初始化一个 `ObjectView` 并返回该值。整个实现仅依赖 `json::Value` 的成员函数、`ObjectView` 的聚合初始化以及 `LLMError` 的构造函数，控制流为单一的分支判断。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (const `json::Value&`)
- `context` (`std::string_view`)
- internal state of `value` (via `get_object()`)

#### Usage Patterns

- validating that a JSON value is an object
- extracting an `ObjectView` from a parsed JSON value
- used in serialization and validation pipelines

### `clore::net::detail::expect_object`

Declaration: `src/network/protocol.cppm:259`

Definition: `src/network/protocol.cppm:409`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数接收一个 `json::Cursor value` 和一个 `std::string_view context`，并尝试将游标解析为一个 JSON 对象。内部调用 `value.get_object()`，若返回空指针，则构造一个 `LLMError` 对象，错误消息由 `std::format` 生成，格式为 `"{} is not a JSON object"` 并填入 `context`，然后通过 `std::unexpected` 返回。若成功，则返回一个 `ObjectView` 值，其 `value` 字段直接指向获取到的底层 JSON 对象指针。

此函数依赖于 `ObjectView` 结构体（包含一个指向 `kota::codec::json::Object` 的指针）作为成功结果的载体，以及 `LLMError` 类型表示转换失败。错误消息的生成依赖 `std::format`，接收方可根据 `context` 定位具体失败的 JSON 值来源。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor` value
- `std::string_view` context
- the underlying JSON object data

#### Usage Patterns

- JSON parsing validation
- error reporting with context
- converting Cursor to `ObjectView`

### `clore::net::detail::expect_string`

Declaration: `src/network/protocol.cppm:268`

Definition: `src/network/protocol.cppm:436`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::expect_string` 实现了一个简单的 JSON 类型守卫：它调用 `json::Value::get_string()` 从输入值中提取 `std::string_view`。如果该操作失败（即值不是 JSON 字符串），则使用给定的 `context` 描述符构造一个 `LLMError` 并包装在 `std::unexpected` 中返回；否则直接返回成功提取的字符串视图。该函数不涉及循环或复杂控制流，唯一的分支依赖于 `std::optional` 的 `has_value()` 检查。其核心依赖是 `json::Value` 提供的字符串提取 API 和 `LLMError` 错误类型，`std::format` 则用于生成包含上下文的错误信息。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (const `json::Value`&)
- `context` (`std::string_view`)
- the result of `value.get_string()`

#### Usage Patterns

- validating that a JSON value is a string
- extracting string content from a JSON value
- reporting an error with context when the value is not a string

### `clore::net::detail::expect_string`

Declaration: `src/network/protocol.cppm:271`

Definition: `src/network/protocol.cppm:445`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数从给定的 `json::Cursor` 中提取字符串值。它调用 `value.get_string()` 将 JSON 节点解析为字符串，如果该节点不是字符串类型则返回 `std::nullopt`。如果返回为空，函数通过 `std::unexpected` 返回一个 `LLMError`，错误消息使用 `std::format` 将 `context` 参数嵌入其中，指出预期的 JSON 字符串缺失。如果成功，则解引用 `get_string()` 的返回值并包装在 `std::expected` 中返回。

内部控制流是简单的线性路径：一次检查，成功则返回字符串，失败则返回错误。该函数依赖于 `json::Cursor` 的 `get_string` 方法以及 `LLMError` 的构造和 `std::format` 库。它被设计为在解析期望为字符串的 JSON 字段时提供一致的错误报告，其中 `context` 参数用于标识是哪个字段或位置出了问题。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor value` parameter
- `std::string_view context` parameter
- internal JSON data accessed via `get_string()`

#### Usage Patterns

- validating JSON string fields in responses
- ensuring a JSON value is a string before processing
- providing context for error messages in JSON parsing

### `clore::net::detail::infer_output_contract`

Declaration: `src/network/protocol.cppm:643`

Definition: `src/network/protocol.cppm:660`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::infer_output_contract` 依据 `PromptRequest` 的字段 `response_format` 与 `output_contract` 决定最终的 `PromptOutputContract`。其控制流首先检查 `response_format` 是否存在可选值；若存在，则强制推断合约应为 `PromptOutputContract::Json`，但若此时 `output_contract` 已被显式设为 `PromptOutputContract::Markdown`，则返回一个描述冲突的错误。若 `response_format` 不存在，则检查 `output_contract` 是否为 `PromptOutputContract::Unspecified`；若是，则要求调用方必须显式指定合约，否则返回错误。当以上条件均不触发时，直接返回 `request.output_contract` 的值。该函数依赖 `PromptRequest` 的字段和枚举 `PromptOutputContract`，并通过 `std::expected` 表达成功或失败结果。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.response_format`
- `request.output_contract`

#### Usage Patterns

- 在验证 `PromptRequest` 时调用以确定输出合同
- 在请求处理流程中用于决策输出格式

### `clore::net::detail::insert_string_field`

Declaration: `src/network/protocol.cppm:227`

Definition: `src/network/protocol.cppm:315`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::insert_string_field` 的核心逻辑是直接向传入的 `json::Object` 中插入一个键值对。它从 `std::string_view` 参数 `key` 和 `value` 构造等值的 `std::string` 对象，并调用 `object.insert` 完成存储。该实现没有任何条件分支或异常处理路径——它总是返回一个空的 `std::expected<void, LLMError>`，表示成功。唯一的依赖是 `json::Object` 的 `insert` 成员函数；第四个参数 `context` 在所示代码中未被使用，但它在接口中保留，可能用于错误消息上下文（此实现忽略了它）。由于没有验证、转换或复杂算法，该函数在控制流上是极简的。

#### Side Effects

- Modifies the `json::Object` by inserting a new key-value pair
- Allocates memory for the two `std::string` copies of `key` and `value`

#### Reads From

- `key` (`std::string_view`)
- `value` (`std::string_view`)
- `context` (`std::string_view`, unused)
- `object` (`json::Object`&, read via insertion internal checks)

#### Writes To

- `object` (`json::Object`&, modified by insertion)

#### Usage Patterns

- Adding a string field to a JSON object during network message construction
- Inserting string data into request or response JSON payloads

### `clore::net::detail::make_empty_array`

Declaration: `src/network/protocol.cppm:243`

Definition: `src/network/protocol.cppm:360`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::make_empty_array` 通过将空数组字面量 `"[]"` 传递给 `json::parse<json::Array>` 来构造一个空的 JSON 数组对象。解析成功则直接返回该数组；若解析失败，则调用 `clore::net::detail::unexpected_json_error` 将底层的 `json::error` 转换为 `LLMError` 并返回。该函数不涉及任何状态或循环，仅依赖 `json::parse` 的模板特化和 `clore::net::detail::unexpected_json_error` 的错误包装逻辑，用于在请求构建或 JSON 操作中快速获取一个可靠的空数组实例。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Parameter `context` (`std::string_view`)
- String literal `"[]"`

#### Usage Patterns

- Creating an empty JSON array as a default or fallback value
- Initializing JSON containers in parsing or serialization utilities

### `clore::net::detail::make_empty_object`

Declaration: `src/network/protocol.cppm:240`

Definition: `src/network/protocol.cppm:352`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::make_empty_object` 将硬编码的字符串 `"{}"` 传递给 `json::parse<json::Object>` 以构造一个空的 JSON 对象。解析成功时直接返回结果；若失败则调用 `unexpected_json_error` 生成 `LLMError`。依赖 `json::parse` 模板特化、`json::Object` 类型以及错误报告辅助函数。整个流程无分支逻辑，仅作为创建一个空对象的便捷包装。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `context` parameter of type `std::string_view`
- string literal `"{}"` (constant)

#### Usage Patterns

- create an empty JSON object for default or error-case initialization
- used in conjunction with similar functions like `make_empty_array`

### `clore::net::detail::normalize_utf8`

Declaration: `src/network/protocol.cppm:225`

Definition: `src/network/protocol.cppm:305`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::normalize_utf8` 的核心算法是对输入的 `text` 调用 `clore::support::ensure_utf8` 进行 UTF-8 净化，并通过比较返回的 `normalized` 与原始 `text` 来检测是否发生了替换。若两者不同，则通过 `logging::warn` 记录一条包含 `field_name` 的警告，以指明是哪个字段含有无效的 UTF-8 序列。该函数最终返回 `normalized` 字符串，使其后续用于 JSON 序列化时保证编码有效性。

依赖方面，该函数仅依赖于 `clore::support::ensure_utf8` 这个底层编码修复工具和通用的日志设施，自身不直接操作 JSON 或处理其他数据结构，仅作为字符串预处理步骤。

#### Side Effects

- Writes a warning message to the log via `logging::warn` when the input contains invalid UTF-8 byte sequences

#### Reads From

- The parameter `text` (input string view)
- The parameter `field_name` (used in log message)

#### Writes To

- The returned `std::string` (normalized UTF-8 string)
- The logging subsystem via `logging::warn`

#### Usage Patterns

- Used when preparing strings for JSON serialization to ensure valid UTF-8, with logging for diagnostic purposes

### `clore::net::detail::parse_json_value`

Declaration: `src/network/protocol.cppm:247`

Definition: `src/network/protocol.cppm:369`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::parse_json_value` 将输入字符串视图 `raw` 直接传给 `json::parse<T>` 执行 JSON 解码。如果解析成功，返回解析后的值；否则，构造一个 `LLMError`，将 `context` 参数与 `parsed.error().to_string()` 合并为描述性错误消息，并通过 `std::unexpected` 返回。该函数依赖 `std::expected` 进行错误传播，依赖 `LLMError` 表示解析失败，其内部不进行二次校验或递归处理，仅封装一次 JSON 解析操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `raw` `string_view` parameter
- the `context` `string_view` parameter

#### Usage Patterns

- Used to deserialize JSON into a typed value
- Called when parsing network response payloads

### `clore::net::detail::parse_json_value`

Declaration: `src/network/protocol.cppm:250`

Definition: `src/network/protocol.cppm:380`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::parse_json_value` serves as a thin delegation wrapper: it first serializes the input `const json::Value&` to a string using `json::to_string`. If serialization fails, it returns an `unexpected_json_error` with a context‑specific message; otherwise, it forwards the resulting string to the sibling overload `parse_json_value<T>(std::string_view, std::string_view)`. The only dependency beyond the serialization utility is the string‑based parse path, which carries out the actual type‑dependent conversion and validation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (const `json::Value` &)
- `context` (`std::string_view`)
- `json::to_string` result

#### Usage Patterns

- Called when a `json::Value` must be parsed into a domain type `T`
- Used as a convenience wrapper over the string‑based `parse_json_value`

### `clore::net::detail::request_text_once_async`

Declaration: `src/network/protocol.cppm:650`

Definition: `src/network/protocol.cppm:692`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数实现一个异步文本请求流程，其核心逻辑围绕构建 `CompletionRequest`、发起协程调用、验证响应内容并返回最终文本。首先根据输入的 `PromptRequest` 通过 `infer_output_contract` 推断期望的输出合约，若推断失败则直接返回错误。随后处理可选的 `ResponseFormat`：当请求指定 `PromptOutputContract::Json` 但未提供格式时，自动创建一个不含 `schema` 的 `ResponseFormat`。接着构造 `CompletionRequest`，将模型名称、系统提示、用户提示以及（可能被调整后的）响应格式、工具选项、工具选择、并行工具调用开关填入其中。之后通过传入的 `CompletionRequester` 协程调用 `request_completion`（通常对应 HTTP API 调用），并等待其完成。从返回的 `CompletionResponse` 中调用 `protocol::text_from_response` 提取文本内容，若提取失败则抛出错误。最后使用 `validate_prompt_output` 对文本按照之前推断的合约进行校验，校验通过后返回文本。整个流程依赖 `infer_output_contract`、`protocol::text_from_response`、`validate_prompt_output` 等内部工具，以及 `PromptRequest`、`CompletionRequest`、`ResponseFormat`、`PromptOutputContract` 等数据结构。

#### Side Effects

- Initiates an asynchronous HTTP request via the provided `CompletionRequester`
- Allocates memory for constructing the `CompletionRequest` and processing the response
- May schedule work on the passed `kota::event_loop`

#### Reads From

- `model` parameter (`std::string_view`)
- `system_prompt` parameter (`std::string_view`)
- `request` parameter (`PromptRequest`) – including `prompt`, `output_contract`, `response_format`, `tool_choice`
- `loop` parameter (`kota::event_loop &`)

#### Writes To

- Completes an HTTP request via `CompletionRequester` and retrieves the response
- Modifies internal state of the event loop passed to `request_completion`

#### Usage Patterns

- Used to perform a single asynchronous text completion with output contract validation
- Integrates with event loop via a provided `CompletionRequester` callable
- Called when a text response from a language model is needed, with built-in error handling for contract and validation failures

### `clore::net::detail::run_task_sync`

Declaration: `src/network/protocol.cppm:238`

Definition: `src/network/protocol.cppm:334`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::run_task_sync` 的实现利用一个临时的 `kota::event_loop` 来同步执行异步操作。它首先通过调用 `make_task` 参数生成一个操作对象，然后将该操作调度到事件循环中，并调用 `kota::event_loop::run` 阻塞直至完成。之后，它检查操作的 `result`：若包含错误，则返回 `std::unexpected` 携带移动后的 `LLMError`；否则，根据 `T` 是否为 `void` 返回空值或移动的结果值。此实现完全依赖 `kota::event_loop` 的事件驱动模型以及操作对象提供的 `result` 接口，没有引入额外的并发或外部依赖。

#### Side Effects

- runs a blocking event loop
- executes the given task synchronously
- moves the error from the internal result

#### Reads From

- the `make_task` callable
- the `kota::event_loop` object
- the operation's result after completion

#### Writes To

- the returned `std::expected<T, LLMError>` value
- the local `kota::event_loop` state (internal)

#### Usage Patterns

- wrapping an asynchronous operation to run synchronously
- converting an async task into a synchronous expected result
- used to execute network requests or other async tasks in a blocking context

### `clore::net::detail::serialize_value_to_string`

Declaration: `src/network/protocol.cppm:253`

Definition: `src/network/protocol.cppm:390`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数通过调用 `json::to_string` 将输入的 `json::Value` 序列化为 JSON 字符串。若 `json::to_string` 返回的 `std::expected` 不含有值，则使用 `unexpected_json_error` 构造一个包含格式化错误消息（指明上下文）和底层 `json::error` 的 `LLMError`，并作为 `std::expected` 的错误分支返回；否则直接返回序列化后的 `std::string`。此函数为其他需要将 JSON 值转为字符串的上下文（如序列化工具参数或请求体）提供了基础工具。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- param `value` of type `const json::Value&`
- param `context` of type `std::string_view`

#### Usage Patterns

- Used to serialize JSON values to strings with proper error handling
- Likely used in scenarios where JSON encoding may fail and the error needs to be reported with a context string

### `clore::net::detail::to_llm_unexpected`

Declaration: `src/network/protocol.cppm:233`

Definition: `src/network/protocol.cppm:324`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

该函数是一个简单的错误转换工具，接收一个 `Status` 错误对象和一个描述性上下文字符串，然后构造一个 `LLMError` 实例，其中包含格式化的错误消息 `"{context}: {error.to_string()}"`，并将其包装在 `std::unexpected` 中返回。它依赖于 `LLMError` 的构造函数和 `std::format` 的字符串拼接能力。内部控制流仅包含一次格式化操作和返回值构造，没有分支或循环。该函数通常在其他错误处理路径中被调用，用于将特定于模块的错误类型（通过 `Status` 的 `to_string` 方法）统一转换为库级别的 `LLMError`，以便于上层错误传播和错误日志记录。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error` (the `Status` parameter)
- `context` (the `std::string_view` parameter)

#### Usage Patterns

- converting an error status to an unexpected `LLMError`
- error propagation in expected-based `APIs`

### `clore::net::detail::unexpected_json_error`

Declaration: `src/network/protocol.cppm:222`

Definition: `src/network/protocol.cppm:300`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

函数 `clore::net::detail::unexpected_json_error` 是一个轻量级工具，用于将 JSON 解析错误包装为 `std::unexpected<LLMError>` 返回类型。它接受一个描述当前操作的 `std::string_view context` 和底层 `json::error` 引用，并通过 `std::format` 拼接成 `"context: error.to_string()"` 格式的错误消息，随后构造一个 `LLMError` 对象。

内部控制流极其简单：仅进行单次格式化与对象构造。该函数依赖 `json::error::to_string()` 提取具体错误描述，并依赖 `std::format` 完成字符串拼接，最终返回的 `std::unexpected<LLMError>` 可直接与预期返回类型（如 `std::expected<T, LLMError>`）配合，在调用方需要报告 JSON 解析失败时使用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- context
- err and err`.to_string()`

#### Usage Patterns

- Convert JSON errors to unexpected `LLMError` results
- Return `LLMError` in JSON parsing or validation failure paths

### `clore::net::detail::validate_prompt_output`

Declaration: `src/network/protocol.cppm:646`

Definition: `src/network/protocol.cppm:678`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

`clore::net::detail::validate_prompt_output` 通过一个简单的 `switch` 分发到专门的验证函数。当 `contract` 为 `PromptOutputContract::Json` 时，它委托给 `protocol::validate_json_output`；当为 `PromptOutputContract::Markdown` 时，委托给 `protocol::validate_markdown_fragment_output`。其他情况（包括 `Unspecified` 和未识别的枚举值）会直接返回一个包含 `LLMError` 的意外结果。该函数充当输出合同验证的调度器，没有额外的自身逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `content` parameter
- `contract` parameter

#### Usage Patterns

- Used to validate prompt output before further processing
- Called after receiving a response to ensure compliance with the expected output format

### `clore::net::get_probed_capabilities`

Declaration: `src/network/protocol.cppm:138`

Definition: `src/network/protocol.cppm:741`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::get_probed_capabilities` 通过一个由静态 `std::mutex` 保护的静态 `std::unordered_map` 来缓存 `ProbedCapabilities` 对象。函数首先对 `mutex` 加锁，然后尝试以给定的 `cache_key` 在映射中查找；若找到已有条目，则直接返回该 `ProbedCapabilities` 的引用。若未找到，则使用 `std::make_unique` 创建一个默认构造的 `ProbedCapabilities` 并将其插入到映射中，随后返回新插入条目的引用。所有操作在单个临界区内完成，确保线程安全。此函数本身不执行任何网络探测或能力检测逻辑，仅作为缓存层，通过首次访问时初始化默认状态的 `ProbedCapabilities` 并返回可修改的引用，供调用者在后续探测流程中填充实际能力数据。

#### Side Effects

- Acquires a static mutex (synchronization)
- Modifies the static cache map if the key is not already present (insertion)
- Allocates a new `ProbedCapabilities` object via `std::make_unique` when the key is missing

#### Reads From

- parameter `cache_key` (`std::string_view`)
- static cache map `cache`

#### Writes To

- static cache map `cache` (via emplace if key missing)
- static mutex `mutex` (lock state)

#### Usage Patterns

- Used to obtain a cached `ProbedCapabilities` instance without repeated probing
- Callers pass a key derived from model, provider, or endpoint to reuse capability results

### `clore::net::icontains`

Declaration: `src/network/protocol.cppm:780`

Definition: `src/network/protocol.cppm:780`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

Implementation: [Implementation](functions/icontains.md)

函数 `clore::net::icontains` 实现了一个不区分大小写的子串搜索算法。它首先快速退出：若待查找的 `needle` 长度大于被搜索的 `haystack` 长度，则直接返回 `false`。否则，外层循环遍历 `haystack` 中所有可能的起始位置（从 `0` 到 `haystack.size() - needle.size()`），内层循环逐字符比较两个字符串对应字符的 `std::tolower` 结果（字符类型通过 `static_cast<unsigned char>` 提升，以避免负值导致的未定义行为）。一旦内层循环完全匹配，函数立即返回 `true`；若所有起始位置均不匹配，则最终返回 `false`。该函数仅依赖标准库的 `<cctype>` 和 `<string_view>`，不引入外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `haystack`
- `needle`

#### Usage Patterns

- Used by `clore::net::is_feature_rejection_error` to perform case-insensitive substring matching on error strings.

### `clore::net::is_feature_rejection_error`

Declaration: `src/network/protocol.cppm:147`

Definition: `src/network/protocol.cppm:800`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

该函数通过遍历一组硬编码的特征拒绝错误模式字符串来判断给定的错误消息是否属于此类错误。内部维护一个静态常量字符串数组 `patterns`，其中包含诸如 `"unsupported parameter"`、`"unknown field"` 等常见 API 错误文本。对每个模式，调用 `clore::net::icontains` 进行不区分大小写的子串匹配，一旦任何模式匹配则立即返回 `true`。若全部遍历后无匹配则返回 `false`。整个控制流仅依赖 `clore::net::icontains` 这一个辅助函数，无其他外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `error_message` of type `std::string_view`
- constant array `patterns` of `std::string_view`

#### Usage Patterns

- Determining if an LLM API error is a feature rejection
- Filtering errors that signal unsupported parameters before retrying
- Classifying network errors for structured error handling

### `clore::net::make_capability_probe_key`

Declaration: `src/network/protocol.cppm:140`

Definition: `src/network/protocol.cppm:755`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`make_capability_probe_key` 将三个参数（`provider`、`api_base`、`model`）通过 `std::format` 直接格式化为以竖线分隔的字符串。该函数不含任何分支或循环，其行为完全由字符串格式化决定；内部控制流仅包含一次格式化调用并返回结果。它依赖 `std::format` 完成字符串连接，返回生成的 `std::string`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `provider`
- 参数 `api_base`
- 参数 `model`

#### Usage Patterns

- 用于生成 `ProbedCapabilities` 缓存的查找键
- 与其他 `std::unordered_map` 配合记录已探测的能力

### `clore::net::make_markdown_fragment_request`

Declaration: `src/network/protocol.cppm:111`

Definition: `src/network/protocol.cppm:156`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::make_markdown_fragment_request` 是一个轻量封装函数，其核心逻辑是构造并返回一个 `PromptRequest` 实例。它通过移动接收的 `std::string` 参数赋值给 `prompt` 字段，将 `response_format` 设为 `std::nullopt` 以允许自由文本生成，并将 `output_contract` 显式指定为 `PromptOutputContract::Markdown`，从而为下游管道标记预期的输出类型。该函数不涉及任何复杂算法或外部依赖，仅依赖 `PromptRequest` 数据结构及 `PromptOutputContract` 枚举的标记值。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `prompt` (moved)

#### Writes To

- returned `PromptRequest` object (constructed via aggregate initialization)

#### Usage Patterns

- create a request for markdown fragment
- initialize `PromptRequest` with markdown contract

### `clore::net::parse_rejected_feature_from_error`

Declaration: `src/network/protocol.cppm:149`

Definition: `src/network/protocol.cppm:819`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::parse_rejected_feature_from_error` 通过穷举检查错误消息中是否包含一组预定义的关键字模式，来确定导致 API 拒绝的具体被禁功能。该实现定义了一个常量数组 `field_patterns`，其中每个元素为 `std::pair<std::string_view, std::string_view>`，映射一个模糊的关键字（如 `"json_schema"`）到其规范化的字段名称（如 `"response_format"`）。算法按顺序遍历该数组，对每个模式中的关键字在 `error_message` 上调用 `clore::net::icontains`（不区分大小写的子串查找），一旦匹配，立即返回对应的规范字段名。

控制流是线性的单次扫描，不进行回溯或递归。如果遍历完所有模式均未命中，则返回 `std::nullopt`。该函数的唯一外部依赖是 `clore::net::icontains`，其实现假定为基于 `std::string_view` 的包含检查。整个逻辑旨在保持对常见错误关键词的低开销、确定性匹配，使调用方能够根据返回的特征字段名采取针对性处理（例如回退或调整请求参数）。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error_message` parameter

#### Usage Patterns

- Used to parse LLM error responses to identify which requested feature was rejected
- Called by error handling logic to determine feature-specific rejection information

### `clore::net::protocol::append_tool_outputs`

Declaration: `src/network/protocol.cppm:485`

Definition: `src/network/protocol.cppm:556`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数首先进行前置校验：若 `response.message.tool_calls` 为空则返回错误。随后遍历 `outputs`，将每个 `ToolOutput` 的 `tool_call_id` 与 `output` 存入 `std::unordered_map`，同时检查 `tool_call_id` 非空且无重复。核心算法为消息拼接：将 `history` 完整复制到结果向量，添加一个 `AssistantToolCallMessage`（包含 `response.message.text` 和 `response.message.tool_calls`），再按 `response.message.tool_calls` 中的顺序为每个 `ToolCall` 从映射中查找对应的 `content` 并生成一条 `ToolResultMessage` 追加到结果中。最后校验映射大小与 `tool_calls` 数量一致，确保没有多余或缺失的输出。该函数依赖 `AssistantToolCallMessage`、`ToolResultMessage`、`LLMError` 等类型，并利用 `std::unordered_map` 进行 O(1) 查找。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `history`
- `response`
- `outputs`

#### Writes To

- 返回的新 `std::vector<Message>`

#### Usage Patterns

- 在完成响应包含工具调用时，将工具输出合并到对话历史中
- 作为构建请求或响应处理流程的一部分

### `clore::net::protocol::parse_response_text`

Declaration: `src/network/protocol.cppm:491`

Definition: `src/network/protocol.cppm:604`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数首先委托给 `clore::net::protocol::text_from_response` 从 `CompletionResponse` 中提取文本内容；若提取失败，则直接转发该错误。成功获得文本后，调用 `kota::codec::json::parse<T>` 尝试将文本解析为模板参数 `T` 所对应的 JSON 结构。一旦解析失败，将解析器返回的错误信息包装为 `LLMError` 并返回；否则返回解析成功的结果值。整个流程依赖于 `text_from_response` 对响应内容的初步抽取以及 `kota::codec::json::parse` 的泛型 JSON 反序列化能力，未涉及额外的校验或转换逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `clore::net::protocol::CompletionResponse` 对象
- `text_from_response` 返回的文本（内部数据）

#### Usage Patterns

- 将 LLM 的 JSON 结构化响应转换为特定类型 `T`
- 在协议解析链中处理结构化输出
- 与 `build_request_json`、`parse_tool_arguments` 等函数配合使用

### `clore::net::protocol::parse_tool_arguments`

Declaration: `src/network/protocol.cppm:494`

Definition: `src/network/protocol.cppm:619`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数是一个模板函数，它接受 `const ToolCall & call` 并返回 `std::expected<T, LLMError>`。其内部算法分为两个核心步骤：首先将 `call.arguments`（一个 JSON 值）通过 `kota::codec::json::to_string` 序列化为字符串；若序列化失败，则构造一个包含工具名称和序列化错误信息的 `LLMError` 并返回 `std::unexpected`。若序列化成功，则调用 `kota::codec::json::parse<T>` 将字符串反序列化为目标类型 `T`；若解析失败，则类似地返回包含工具名称和解析错误信息的 `LLMError`。只有两步都成功时才返回解析得到的值。

函数全权依赖 `ToolCall` 结构体（特别是其 `name` 和 `arguments` 字段），以及 `kota::codec::json` 命名空间下的 `to_string` 和 `parse` 函数。错误类型 `LLMError` 用于统一表示序列化或解析阶段的失败，并通过 `std::expected` 的错误路径向外传递。整个流程没有外部状态或副作用，仅对给定的工具调用参数执行格式转换与类型安全的反序列化。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `call.arguments`
- `call.name`

#### Usage Patterns

- Used to extract typed tool arguments from a `ToolCall` object
- Commonly called in tool invocation handling code

### `clore::net::protocol::text_from_response`

Declaration: `src/network/protocol.cppm:483`

Definition: `src/network/protocol.cppm:540`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数首先对 `response.message` 的成员进行顺序检查以确定文本内容的可用性。如果 `refusal` 字段存在值，说明模型拒绝生成文本，立即返回 `std::unexpected` 并携带错误信息。若 `tool_calls` 非空，则认为响应是工具调用而非纯文本，同样返回错误。最后检查 `text` 字段是否缺席，若缺席则说明响应缺少预期的文本载荷。只有全部条件通过后才解引用 `text` 字段并返回其字符串值。整个流程仅依赖 `clore::net::CompletionResponse` 中 `message` 字段（类型为 `clore::net::AssistantOutput`）所暴露的三个可选成员，属于纯粹的取值与验证逻辑，不涉及网络、解析或格式化等外部操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `response.message.refusal`
- `response.message.tool_calls`
- `response.message.text`

#### Usage Patterns

- Extracting text from a completion response after validating no refusal or tool calls

### `clore::net::protocol::validate_json_output`

Declaration: `src/network/protocol.cppm:479`

Definition: `src/network/protocol.cppm:500`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

函数 `clore::net::protocol::validate_json_output` 的实现在于对入参执行一次 JSON 语法验证。它调用 `kota::codec::json::parse<kota::codec::json::Value>` 来解析输入字符串；若解析成功，直接返回空预期（表示验证通过）。若解析失败，则提取解析器的错误信息，构造一个 `LLMError` 并包装为 `std::unexpected` 返回。整个过程不依赖任何外部状态或可变缓存，仅依赖于 JSON 解析库的字符串读取与语法检查能力，控制流形成单一的分支：成功路径直接返回，失败路径构造错误后返回。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content parameter

#### Usage Patterns

- Called after receiving an LLM response to ensure its content is valid JSON before further processing
- Likely used in conjunction with other protocol functions like `parse_response` or `validate_markdown_fragment_output`

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `src/network/protocol.cppm:481`

Definition: `src/network/protocol.cppm:509`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

该函数首先对输入 `content` 执行一系列防御性检查：检查是否为空、是否仅含空白字符，若命中则立即返回 `LLMError`。接着搜索是否包含 H1 标题（`"# "` 前缀或 `"\n# "` 模式）或三反引号代码围栏，若发现则同样报错。最后调用 `clore::net::protocol::validate_json_output` 判断内容是否为合法 JSON，若是则拒绝（因为要求 Markdown 片段）。所有检查均通过后返回 `std::expected<void, LLMError>` 的成功值。控制流是顺序短路式——任一条件失败即终止验证。依赖仅限于标准库字符串操作（`std::isspace`、`std::string_view` 成员函数）及项目内的 JSON 验证函数 `validate_json_output` 和错误类型 `LLMError`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `content` parameter

#### Usage Patterns

- Validation of markdown fragments from LLM responses
- Rejecting JSON when markdown fragment is expected

### `clore::net::sanitize_request_for_capabilities`

Declaration: `src/network/protocol.cppm:144`

Definition: `src/network/protocol.cppm:761`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

函数 `clore::net::sanitize_request_for_capabilities` 对传入的 `CompletionRequest` 执行一系列原子检查，依据 `ProbedCapabilities` 中的能力标志逐字段裁剪请求中不受支持的特性。控制流依次评估四个布尔标志：若 `supports_json_schema` 为 `false`，则清除 `response_format` 中的 `schema` 字段；若 `supports_tool_choice` 为 `false`，则将 `tool_choice` 置为 `std::nullopt`；若 `supports_parallel_tool_calls` 为 `false`，则移除 `parallel_tool_calls`；若 `supports_tools` 为 `false`，则直接调用 `clear()` 清空整个 `tools` 列表。每个判断均通过 `std::memory_order_relaxed` 加载原子标志，且修改顺序无关；最终返回仅包含能力兼容字段的 `CompletionRequest` 副本。

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

#### Usage Patterns

- 在构造 LLM 调用请求后，传递完能力探测结果后调用，以清除不支持的选项
- 作为前置处理步骤，位于 `call_completion_async` 或类似调用之前

## Internal Structure

`protocol` 模块封装了与 LLM 通信的协议层，负责定义请求/响应数据结构（如 `CompletionRequest`、`PromptRequest`、`ToolCall`、`CompletionResponse`）并管理其序列化、解析与验证。它通过细粒度的内部层次组织功能：`detail` 子命名空间提供面向 JSON 的底层原语（如 `ObjectView`、`ArrayView`、`expect_string`、`clone_object`）和流程辅助函数（如 `run_task_sync`、`request_text_once_async`）；`protocol` 子命名空间则暴露更高阶的工厂与解析函数（如 `parse_tool_arguments`、`append_tool_outputs`、`validate_json_output`），用于构建请求、解析响应及处理工具调用。模块依赖 `http` 模块完成实际网络通信，并依赖 `support` 模块获取工具函数与日志基础设施，从而将协议逻辑与底层 I/O 及通用支持解耦。

## Related Pages

- [Module http](../http/index.md)
- [Module support](../support/index.md)

