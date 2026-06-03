---
title: 'Module protocol'
description: 'The protocol module defines the core data structures and functions for constructing, validating, and processing requests and responses in the LLM completion protocol. It owns public types such as CompletionRequest, CompletionResponse, PromptRequest, ToolCall, ToolOutput, FunctionToolDefinition, ResponseFormat, ProbedCapabilities, and a family of message structs (SystemMessage, UserMessage, AssistantMessage, etc.), along with the PromptOutputContract enumeration. These types form a complete abstraction for LLM interactions, enabling callers to specify model, messages, tool definitions, response formatting, and output contracts.'
layout: doc
template: doc
---

# Module `protocol`

## Summary

The `protocol` module defines the core data structures and functions for constructing, validating, and processing requests and responses in the LLM completion protocol. It owns public types such as `CompletionRequest`, `CompletionResponse`, `PromptRequest`, `ToolCall`, `ToolOutput`, `FunctionToolDefinition`, `ResponseFormat`, `ProbedCapabilities`, and a family of message structs (`SystemMessage`, `UserMessage`, `AssistantMessage`, etc.), along with the `PromptOutputContract` enumeration. These types form a complete abstraction for LLM interactions, enabling callers to specify model, messages, tool definitions, response formatting, and output contracts.

The module also provides a public API layer that includes functions for constructing prompt requests (`make_markdown_fragment_request`), probing and sanitizing model capabilities (`get_probed_capabilities`, `sanitize_request_for_capabilities`), detecting and parsing feature rejection errors (`is_feature_rejection_error`, `parse_rejected_feature_from_error`), and a `protocol` sub‑namespace with utilities for validating JSON and markdown outputs, extracting text from responses, and handling tool calls and their results (`validate_json_output`, `validate_markdown_fragment_output`, `text_from_response`, `parse_tool_arguments`, `parse_response_text`, `append_tool_outputs`). Internal detail helpers manage JSON parsing, cloning, serialization, and synchronous task execution, all underpinning the protocol’s message exchange with LLM endpoints.

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

The struct `clore::net::AssistantMessage` is implemented as a trivial aggregate type containing a single public data member `content` of type `std::string`. There are no user-declared constructors, destructors, assignment `operator`s, or other member functions, so the implementation relies entirely on default special member functions. The only invariant is that the `content` string is left in whatever state the default or aggregate-initialization produces; no additional validation or transformation is performed. Because the struct has no internal logic or member implementations beyond the compiler‑generated ones, its implementation is effectively the storage of the string itself. This design makes `AssistantMessage` a lightweight, value‑oriented data carrier intended for direct field access.

#### Invariants

- content can be any valid `std::string`

#### Key Members

- content

#### Usage Patterns

- Used to transmit assistant message content in network protocol

### `clore::net::AssistantOutput`

Declaration: `src/network/protocol.cppm:113`

Definition: `src/network/protocol.cppm:113`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::AssistantOutput` is an aggregate type that packages the three possible outputs from an assistant response. Its members are `text`, `refusal`, and `tool_calls`. The first two are `std::optional<std::string>` and are empty by default, while `tool_calls` is a `std::vector<ToolCall>` that starts empty. No special member functions are declared, so the struct relies on compiler‑generated defaults, and the invariant is that at most one informational payload (`text` or `refusal`) is expected to be set at a time, though the data representation imposes no runtime check.

#### Invariants

- At most one of `text` and `refusal` may be engaged (implied by optional semantics)
- `tool_calls` can be empty indicating no tool calls

#### Key Members

- `text`
- `refusal`
- `tool_calls`

#### Usage Patterns

- Returned by assistant processing functions
- Consumed by handling code to extract text, refusal, or tool calls

### `clore::net::AssistantToolCallMessage`

Declaration: `src/network/protocol.cppm:47`

Definition: `src/network/protocol.cppm:47`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct is a data-only aggregate that bundles an optional text response with a list of invoked tool calls. Internally, `content` is a `std::optional<std::string>` and defaults to `std::nullopt`, while `tool_calls` is a `std::vector<ToolCall>` that defaults to an empty vector. The type imposes no invariants beyond those of its members: the optional may be empty, and the vector may contain zero or more elements. Because the struct defines no custom constructors, assignment `operator`s, or destructor, it relies on compiler‑generated default special member functions; all members are direct‑initialized via aggregate initialization.

#### Invariants

- `content` is optional; it may be `std::nullopt`
- `tool_calls` may be empty
- No other invariants are documented

#### Key Members

- `content`
- `tool_calls`

#### Usage Patterns

- Used as part of assistant messages in network protocol
- Objects are likely constructed from parsed AI responses
- May be stored in conversation histories

### `clore::net::CompletionRequest`

Declaration: `src/network/protocol.cppm:89`

Definition: `src/network/protocol.cppm:89`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::CompletionRequest` aggregates the data fields required to represent a single completion request. Internally, it holds a `std::string model` and a `std::vector<Message> messages`, both defaulting to empty; an `std::optional<ResponseFormat> response_format` and an `std::optional<ToolChoice> tool_choice` that begin as `std::nullopt`; a `std::vector<FunctionToolDefinition> tools` also empty; and an `std::optional<bool> parallel_tool_calls` initially absent. These default member initializers ensure that every freshly constructed instance is in a valid, minimal state without any optional features enabled.

No custom constructors, assignment `operator`s, or invariants are enforced by the struct itself; it acts as a plain aggregate. The important invariant for callers is that the `model` field should be populated before use, and that the `messages` vector must contain at least one entry for a meaningful request. The implementation does not perform validation—it simply stores the provided values verbatim. This design keeps the type lightweight and focused on data transport, leaving semantic checks to the layers that consume the struct.

#### Invariants

- `model` is a non-empty string identifying the AI model
- `messages` contains at least one message to prompt the model
- `tools` list may be empty when no function definitions are provided
- `tool_choice` and `response_format` are absent unless explicitly set
- `parallel_tool_calls` is absent unless explicitly set

#### Key Members

- `model`
- `messages`
- `response_format`
- `tools`
- `tool_choice`
- `parallel_tool_calls`

#### Usage Patterns

- Aggregate-initialized with request parameters
- Passed to networking client to perform API call
- Serialized to JSON for transmission to a completion endpoint

### `clore::net::CompletionResponse`

Declaration: `src/network/protocol.cppm:119`

Definition: `src/network/protocol.cppm:119`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::CompletionResponse` aggregates the essential fields produced by a language model completion endpoint. It holds the response identifier (`id`), the model name that generated the completion (`model`), the textual result (`message` as an `AssistantOutput` object), and a copy of the full JSON payload (`raw_json`). The `raw_json` member acts as an immutable record of the original server response, enabling downstream consumers to inspect extra fields not captured in the structured fields or to re-parse the reply without an additional network call.

Internally, all fields are set during construction (typically from a JSON deserialization routine). No post-construction mutation is performed, so the struct maintains a snapshot of a single response. The consistency invariant is that `id`, `model`, and `message` are always derived from the same raw payload, and `raw_json` corresponds exactly to that payload—no partial or contradictory state is allowed.

#### Invariants

- All fields are populated after a successful response
- `raw_json` contains the original serialized response

#### Key Members

- id
- model
- message
- `raw_json`

#### Usage Patterns

- Returned by completion endpoints
- Consumed by callers to extract response details

### `clore::net::ForcedFunctionToolChoice`

Declaration: `src/network/protocol.cppm:82`

Definition: `src/network/protocol.cppm:82`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ForcedFunctionToolChoice` is implemented as a thin value type containing a single data member, `name` of type `std::string`. There are no additional constructors, assignment `operator`s, or invariant checks defined; the struct relies entirely on the default behavior of `std::string` for its internal state. The absence of custom copy, move, or comparison logic means the struct is a plain data container, with no enforced constraints on the content of `name` beyond whatever the caller provides. Any validation (such as requiring a non‑empty string) must be performed externally, as the struct itself does not impose or verify invariants during construction or assignment.

#### Key Members

- name

#### Usage Patterns

- Likely passed as an argument to configure tool choice in network protocols

### `clore::net::FunctionToolDefinition`

Declaration: `src/network/protocol.cppm:69`

Definition: `src/network/protocol.cppm:69`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The `clore::net::FunctionToolDefinition` struct is an aggregate type that bundles the core attributes of a function tool definition for use within the network layer. It holds a `name` and `description` as `std::string` values, a `parameters` member of type `kota::codec::json::Object` representing the function’s parameter schema, and a `strict` flag that defaults to `true` to enforce strict parameter matching. As a plain aggregate, the struct has no custom constructors or user‑defined member functions; its invariants are limited to the default value of `strict`, which ensures that, unless explicitly overridden, a definition is assumed to require exact adherence to the provided parameter schema.

#### Invariants

- `strict` has a default value of `true`.
- `parameters` is a `kota::codec::json::Object`.
- The struct is a simple aggregate with no custom constructors, destructors, or assignment `operator`s.

#### Key Members

- `name`
- `description`
- `strict`
- `parameters`

#### Usage Patterns

- Used within the `clore::net` namespace, likely in the context of network protocols or function tool definitions.
- Filled with appropriate values when defining tools for function calling in a network-based system.

### `clore::net::Message`

Declaration: `src/network/protocol.cppm:57`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::Message` is a type alias for a `std::variant` over five specific message types: `SystemMessage`, `UserMessage`, `AssistantMessage`, `AssistantToolCallMessage`, and `ToolResultMessage`. Its purpose is to provide a single, statically-typed discriminated union that represents every possible message category exchanged within the network protocol. The invariant of the variant ensures that exactly one alternative is active at any time, with no internal shared state or dynamic allocation beyond what the variant itself may use for the stored type. As a pure alias, `clore::net::Message` introduces no runtime indirection or member implementations; all behavior is delegated to the `std::variant` utilities, making it a compile-time convenience for pattern-matching and storage.

#### Invariants

- Holds exactly one alternative type at a time
- Each alternative is a distinct message type in the protocol

#### Key Members

- `SystemMessage`
- `UserMessage`
- `AssistantMessage`
- `AssistantToolCallMessage`
- `ToolResultMessage`

#### Usage Patterns

- Used as the common message type for communication across the network layer
- Typically accessed via `std::visit` or pattern matching to handle specific message types

### `clore::net::ProbedCapabilities`

Declaration: `src/network/protocol.cppm:131`

Definition: `src/network/protocol.cppm:131`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ProbedCapabilities` holds four `std::atomic<bool>` fields that record the inferred capabilities of a remote endpoint after probing. Each field is initialized to `true`, reflecting an optimistic assumption that the endpoint supports the corresponding feature until probing proves otherwise. The atomicity ensures safe concurrent access when multiple threads update or read the probed results, which is critical during the asynchronous handshake or capability negotiation phase. The fields—`supports_tool_choice`, `supports_parallel_tool_calls`, `supports_tools`, and `supports_json_schema`—are independent flags with no interdependencies; any combination of them may be set to `false` as probing progresses, but they are never reset to `true` after being cleared. This monotonic‑to‑false invariant simplifies the probing logic and guarantees that once a capability is determined to be unsupported, the decision is final.

#### Invariants

- All members are initialized to `true`.
- Members are atomic and can be safely modified from multiple threads.

#### Key Members

- `supports_json_schema`
- `supports_tool_choice`
- `supports_parallel_tool_calls`
- `supports_tools`

#### Usage Patterns

- Read after probing to determine enabled features.
- Set to `false` when a capability is found unsupported.

### `clore::net::PromptOutputContract`

Declaration: `src/network/protocol.cppm:98`

Definition: `src/network/protocol.cppm:98`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The enum `clore::net::PromptOutputContract` is implemented with an underlying type of `std::uint8_t`, allowing compact storage and efficient serialization. It defines three enumerators: `Unspecified`, `Json`, and `Markdown`. The ordinal order is `Unspecified` (0), `Json` (1), `Markdown` (2); this ordering is relied upon for serialization and relational comparisons. Being an `enum class`, it prevents implicit conversions from or to integer types, enforcing type safety. No member functions or additional invariants exist beyond the fixed set of possible values; the enumeration serves as a discriminant in protocol messages to indicate the expected output format, with `Unspecified` acting as a default or unset state.

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

The struct `clore::net::PromptRequest` aggregates the core fields for a prompt-based request. The `prompt` member is a `std::string` default-initialized to empty; its content is expected to be set before use. The `response_format` and `tool_choice` are wrapped in `std::optional`, allowing them to be absent unless explicitly configured. The `output_contract` field uses the enumeration `PromptOutputContract` with a default value of `Unspecified`, ensuring a defined starting state. The default constructors and aggregate initialization order follow the declaration sequence, with `prompt` first, then the optionals, and finally the contract field. No invariants are enforced by the struct itself; validation of field combinations is left to the surrounding protocol logic.

#### Invariants

- `output_contract` always has a value (default `PromptOutputContract::Unspecified`)
- `prompt` is default-constructed as empty string

#### Key Members

- `prompt`
- `response_format`
- `tool_choice`
- `output_contract`

#### Usage Patterns

- Used as input for LLM inference, carrying the prompt and optional response constraints
- Default `output_contract` allows unspecified contract behavior

### `clore::net::ResponseFormat`

Declaration: `src/network/protocol.cppm:63`

Definition: `src/network/protocol.cppm:63`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ResponseFormat` aggregates three public members that characterize a response format in the networking layer: a `name` string which identifies the format, an optional `schema` of type `kota::codec::json::Object` that may carry a JSON schema definition, and a `strict` boolean flag defaulting to `true`. The `strict` flag governs whether the format should be interpreted with strict validation; its default value establishes strictness as the standard behavior unless explicitly overridden. The `schema` being optional allows formats to be referenced by name alone—for instance, well-known types whose structure is enforced elsewhere. The struct uses plain public fields without encapsulation or invariant enforcement beyond the type system; in typical usage the `name` field is expected to be non-empty, though this is not checked at the structure level.

#### Invariants

- `strict` is always initialized, defaulting to `true`
- `schema` may be empty (`std::nullopt`)

#### Key Members

- `name`
- `schema`
- `strict`

#### Usage Patterns

- Used to specify response format when calling `APIs`
- The `schema` can be provided to constrain output
- The `strict` flag controls schema enforcement

### `clore::net::SystemMessage`

Declaration: `src/network/protocol.cppm:28`

Definition: `src/network/protocol.cppm:28`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::SystemMessage` is an aggregate type that contains a single `std::string` member `content`. It relies entirely on implicitly generated constructors, destructor, and assignment `operator`s, with no custom invariants beyond those enforced by `std::string` itself. The implementation is minimal — the struct acts as a lightweight, movable container for a system‑level message payload within the network protocol layer.

#### Invariants

- No invariants specified; the struct is a plain aggregate.

#### Key Members

- `content` (`std::string`)

#### Usage Patterns

- Used as a lightweight container for a system message string in network-related code.

### `clore::net::ToolCall`

Declaration: `src/network/protocol.cppm:36`

Definition: `src/network/protocol.cppm:36`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolCall` bundles four fields representing a remote function invocation identified by `id` and `name`. The arguments are stored in two complementary forms: `arguments_json` holds the raw JSON string as received from the network, while `arguments` caches the parsed `kota::codec::json::Value` to avoid repeated deserialization. An invariant is that `arguments` must remain consistent with `arguments_json`; typically the parsed value is derived from the string during construction or assignment. No fields are `const`, allowing mutation of the JSON representation while preserving the raw text.

### `clore::net::ToolChoice`

Declaration: `src/network/protocol.cppm:86`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The type alias `clore::net::ToolChoice` is defined as a `std::variant` over four mutually exclusive alternatives: `ToolChoiceAuto`, `ToolChoiceRequired`, `ToolChoiceNone`, and `ForcedFunctionToolChoice`. Each alternative represents a distinct category of tool selection behavior in the networking protocol. The variant ensures that exactly one of these choices is active at any time, enforcing a type-safe sum type pattern. As a variant, it provides standard access through `std::visit` or `std::get`, but the alias itself does not add any additional members or invariants beyond those inherent to the underlying `std::variant` type. The four concrete types are defined elsewhere in the module and are expected to be small, lightweight value types suitable for inclusion in a variant.

#### Invariants

- holds exactly one of the four variant alternatives
- each alternative is a distinct type for a specific tool choice mode

#### Key Members

- `ToolChoiceAuto`
- `ToolChoiceRequired`
- `ToolChoiceNone`
- `ForcedFunctionToolChoice`

#### Usage Patterns

- constructed from one of the four tool choice types
- inspected via `std::visit` to dispatch on the active alternative
- passed as parameter to specify tool selection behavior

### `clore::net::ToolChoiceAuto`

Declaration: `src/network/protocol.cppm:76`

Definition: `src/network/protocol.cppm:76`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolChoiceAuto` is an empty tag type with no data members, base classes, or member functions. It carries no invariants and its default constructor, destructor, copy, and move operations are all trivial. The absence of any stored state means every instance of `clore::net::ToolChoiceAuto` is functionally identical; the type exists solely to differentiate a compile-time selection or to act as a sentinel for API overloads. Its implementation is therefore a minimal definition that introduces a distinct name without imposing any runtime overhead.

#### Invariants

- It is a trivially constructible and destructible type.
- It provides no runtime state.

#### Key Members

- None; the struct has no members.

#### Usage Patterns

- Other code might use `clore::net::ToolChoiceAuto` as a template parameter or overload resolution tag.
- Its emptiness ensures no overhead when used as a discriminator.

### `clore::net::ToolChoiceNone`

Declaration: `src/network/protocol.cppm:80`

Definition: `src/network/protocol.cppm:80`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolChoiceNone` is an empty aggregate type with no data members, base classes, or virtual functions. As a result, it has no invariants to maintain and no member functions beyond the compiler-generated default constructor, copy, move, and destructor. The type exists solely as a lightweight tag or sentinel to represent the concept of “no tool selected” in the tool‑choice mechanism, enabling compile‑time dispatch or template specializations without any runtime overhead.

#### Invariants

- Empty struct, no state
- Trivially copyable
- Trivially destructible

#### Usage Patterns

- Used as a type tag to indicate absence of tool selection
- Likely used in variant or template contexts to disambiguate tool choices

### `clore::net::ToolChoiceRequired`

Declaration: `src/network/protocol.cppm:78`

Definition: `src/network/protocol.cppm:78`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The `clore::net::ToolChoiceRequired` struct is an empty, stateless type with no data members or member functions. It serves purely as a tag or marker type, likely used to discriminate between overloads or to represent a variant alternative in a `std::variant` or similar discriminated union. Its sole purpose is to carry type information at compile time; no runtime state is stored or maintained. The struct does not define any constructors, destructors, assignment `operator`s, or comparison operations beyond those implicitly generated by the compiler. Consequently, no invariants or special member implementations exist beyond the trivial defaults.

### `clore::net::ToolOutput`

Declaration: `src/network/protocol.cppm:126`

Definition: `src/network/protocol.cppm:126`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The `clore::net::ToolOutput` struct is a plain aggregate that associates a tool call’s textual result with its unique identifier for routing responses back to the caller. It contains two `std::string` members: `tool_call_id`, which stores the identifier matching a prior tool invocation, and `output`, which holds the generated output string. There are no additional invariants enforced by the struct itself beyond the natural semantics of the contained strings; users of the type are responsible for ensuring the `tool_call_id` corresponds to an active or known tool call. The struct has no custom constructors, destructors, or member functions, relying entirely on default compiler‑generated special member functions to behave as a trivial value type.

#### Invariants

- No explicit invariants; fields are independent strings.

#### Key Members

- `tool_call_id`
- `output`

#### Usage Patterns

- Used to communicate tool execution results back to the caller.
- Likely serialized or stored as a pair associating an identifier with its output.

### `clore::net::ToolResultMessage`

Declaration: `src/network/protocol.cppm:52`

Definition: `src/network/protocol.cppm:52`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolResultMessage` packages the result of a single tool execution. It aggregates a `tool_call_id` that links the result back to the original request and a `content` string that holds the tool’s output (e.g., JSON, plain text). Both fields are default-constructed `std::string` values, so no preconditions are required: an empty `content` is a valid representation of a null or unavailable result. No custom constructors, assignment `operator`s, or invariants are enforced; the struct remains a passive data container whose lifecycle is managed entirely by its context within the protocol layer.

#### Invariants

- Both `tool_call_id` and `content` are default-constructible and copyable strings.
- No special invariants beyond standard string validity.

#### Key Members

- `tool_call_id`: identifier for the corresponding tool invocation.
- `content`: payload of the tool result.

#### Usage Patterns

- Constructed and sent by the tool execution backend.
- Parsed by the protocol handler to match results with pending calls.
- Serialized and deserialized as part of a larger network message.

### `clore::net::UserMessage`

Declaration: `src/network/protocol.cppm:32`

Definition: `src/network/protocol.cppm:32`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The `clore::net::UserMessage` struct is a plain data container that holds a single `std::string content` member. It carries no internal invariants beyond those naturally enforced by `std::string` (e.g., valid UTF-8 or any other encoding is left to the caller). All special member functions (construction, assignment, destruction) are implicitly defined as trivial or defaulted, making the type trivially copyable and movable. The implementation serves exclusively as a lightweight wrapper for a text payload in the networking layer, with no additional state or logic.

#### Invariants

- content is a valid `std::string`

#### Key Members

- content

#### Usage Patterns

- used as a simple data carrier in network communication

### `clore::net::detail::ArrayView`

Declaration: `src/network/protocol.cppm:190`

Definition: `src/network/protocol.cppm:190`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The struct `clore::net::detail::ArrayView` is a thin, non-owning view over a `kota::codec::json::Array`. Its sole data member, `value`, is a pointer to `const kota::codec::json::Array` default‑initialized to `nullptr`. All member functions delegate directly to the underlying array object through this pointer: `empty()` and `size()` forward to the corresponding `value` methods; `begin()` and `end()` return `value`’s iterators; `operator[]` performs indexed access via `(*value)[index]`; and `operator->()` and `operator*()` expose the pointer and reference respectively. The implementation does not perform null‑checks on `value`, relying on the caller to ensure it points to a valid array before any member is invoked. This structure provides a const‑qualified, read‑only interface that mirrors the JSON array’s API without copying or owning the data.

#### Invariants

- The `value` pointer must remain valid for the lifetime of the view.
- All member functions assume `value` is non-null and point to a valid `Array`.

#### Key Members

- `value` pointer
- `begin` / `end` for iteration
- `operator[]` for indexed access
- `operator*` and `operator->` for direct array access

#### Usage Patterns

- Used to pass read-only references to JSON arrays without copying.
- Frequently employed in network protocol handling where constant array data is accessed.

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

The struct `clore::net::detail::ObjectView` is a lightweight, non-owning view over a `const kota::codec::json::Object`. Its sole data member is a raw pointer `value`, default‑initialised to `nullptr`. All accessor methods—`begin()`, `end()`, `operator->`, and `operator*`—delegate directly to the underlying object through this pointer, assuming that `value` is non‑null at the point of use; no runtime validation is performed. The class is trivially copyable and provides a restricted interface that forwards iteration and dereference operations without exposing the full `Object` type.

The `get` method returns `std::optional<kota::codec::json::Cursor>` and performs a key lookup on the referenced object; its implementation likely returns `std::nullopt` if `value` is `nullptr` or if the key is absent, but the exact behaviour is not shown. The `value` pointer is the only internal state; there are no additional invariants beyond the expectation that callers ensure the pointer remains valid for the lifetime of the view.

#### Invariants

- `value` may be `nullptr` if default-constructed or not properly initialized
- `begin()` and `end()` assume `value` is non-null (undefined behavior otherwise)
- `get` performs a lookup in the underlying object; result is empty if key not found

#### Key Members

- `value`
- `get`
- `begin`
- `end`
- `operator->`
- `operator*`

#### Usage Patterns

- Callers iterate over key-value pairs using range-based for loops via `begin`/`end`
- Callers retrieve a specific value by key with `get`, handling the optional return
- Used as a parameter or return type in `clore::net::detail` to avoid copying JSON objects

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

`clore::net::detail::clone_array` constructs a new `json::Array` by copy‑constructing it from the array pointed to by the incoming `ArrayView` parameter. The function dereferences `ArrayView::value` (which holds a pointer to a `kota::codec::json::Array`) and passes it directly into the `json::Array` copy constructor, producing a deep copy of the array elements. The second parameter (a `std::string_view` intended for error‑message context) is not consulted during execution, as the copy operation either succeeds or propagates an exception from the underlying JSON library. Dependencies are limited to the `ArrayView` abstraction and the `kota::codec::json::Array` type; no additional control flow or branching is present.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source.value` (pointer to `json::Array`)
- the contents of the `json::Array` pointed to by `source.value`

#### Usage Patterns

- Create a deep copy of a JSON array for independent use
- Duplicate an array when ownership transfer is needed

### `clore::net::detail::clone_object`

Declaration: `src/network/protocol.cppm:277`

Definition: `src/network/protocol.cppm:463`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::clone_object` performs a direct copy of the underlying JSON object held by the provided `ObjectView`. It dereferences `source.value` and constructs a new `json::Object` from it, relying on the copy constructor of that type. The second `std::string_view` parameter is not used in the body; it is present for consistent error‑reporting signatures across similar cloning utilities.

Internally, the function delegates entirely to the `json::Object` copy constructor, making the operation O(n) in the size of the object. No explicit error‑handling logic is present in the snippet, and the return type `std::expected<json::Object, LLMError>` implies that any failure would need to be propagated from upstream (e.g., null or invalid `value` pointer), though the provided code assumes a valid `ObjectView`. Dependencies are limited to `ObjectView` and the `json::Object` class from the underlying JSON library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- source`.value` (pointed-to `kota::codec::json::Object`)

#### Usage Patterns

- creating independent copies of JSON objects from views

### `clore::net::detail::clone_object`

Declaration: `src/network/protocol.cppm:274`

Definition: `src/network/protocol.cppm:458`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::clone_object` is a straightforward copy operation. It accepts a `const json::Object &` named `source` and a `std::string_view` (used as a context label for error messages) and returns a `std::expected<json::Object, LLMError>`. Internally, the function constructs a new `json::Object` by copy‑constructing from `source`, relying entirely on the deep‑copy semantics of the underlying JSON library (`kota::codec::json`). The copy is then returned directly as a success value inside the `std::expected` wrapper, with no further transformations, branching, or error handling beyond possible exceptions from the JSON copy constructor. The second parameter (`std::string_view`) is not used in the body shown but is consistent with the interface pattern used by other detail functions for associating error context. The function’s dependencies are limited to the `json::Object` copy constructor and the `std::expected` type.

#### Side Effects

- Allocates a new `json::Object` via copy construction.

#### Reads From

- source

#### Usage Patterns

- Called from `clone_value` when the encountered value is an object.
- Used to obtain a mutable copy of an immutable object view.

### `clore::net::detail::clone_value`

Declaration: `src/network/protocol.cppm:283`

Definition: `src/network/protocol.cppm:467`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::clone_value` implements a straightforward deep copy of a `json::Value` by invoking its copy constructor. The `context` parameter, intended to provide error-context strings in similar utility functions, is explicitly discarded via `static_cast<void>`. The return is a copy‑constructed `json::Value` wrapped in `std::expected`; because `json::Value` is a value type that owns its internal representation, the copy reproduces the entire JSON subtree. No parsing, validation, or mutation occurs—the sole purpose is to produce an independent duplicate of the node. This function depends directly on the `json::Value` copy semantics and on the `std::expected` error‑handling machinery. It is used internally by higher‑level cloning routines (such as `clone_object` and `clone_array`) that require an identical, disconnected copy of a value for safe manipulation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source` parameter (const `json::Value` reference)

#### Writes To

- return value (new `json::Value`)

#### Usage Patterns

- Cloning a JSON value for local mutation without affecting the original
- Defensive copying before validation or transformation
- Used in JSON processing pipelines to preserve the input

### `clore::net::detail::excerpt_for_error`

Declaration: `src/network/protocol.cppm:235`

Definition: `src/network/protocol.cppm:328`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::excerpt_for_error` accepts a `std::string_view body` and returns a `std::string`. It defines a local constant `kMaxBytes` of value 200 as the maximum byte length for the excerpt. The sole operation is a call to `clore::support::truncate_utf8(body, kMaxBytes)`, which truncates the input to at most 200 bytes while ensuring the resulting string ends at a valid UTF-8 character boundary. No branching, error handling, or additional logic is present; the function acts as a thin convenience wrapper for producing a safe, size‑limited fragment of an error‑related payload for diagnostic or logging purposes. Its only external dependency is the UTF‑8‑aware truncation utility from the `clore::support` module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `body`

#### Usage Patterns

- used to produce a short excerpt of an error response body for display or logging

### `clore::net::detail::expect_array`

Declaration: `src/network/protocol.cppm:265`

Definition: `src/network/protocol.cppm:427`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function first attempts to obtain the underlying JSON array from the provided `json::Cursor` by calling `value.get_array()`. If the cursor does not represent a JSON array — i.e., `get_array()` returns `nullptr` — the function returns `std::unexpected` containing an `LLMError` whose message is constructed with `std::format` and the supplied `context` string to indicate the expected array was missing. Otherwise, it constructs and returns a `clore::net::detail::ArrayView` object, initializing its `.value` member with the yielded `const kota::codec::json::Array*`. This control flow performs a single type-check at the cursor level and wraps the validated array pointer. The function depends on `json::Cursor` and its `get_array()` method, the `ArrayView` helper type, and `LLMError` for error reporting.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor` value
- `std::string_view` context

#### Usage Patterns

- validating JSON array in context
- extracting `ArrayView` from cursor
- error reporting for non-array values

### `clore::net::detail::expect_array`

Declaration: `src/network/protocol.cppm:262`

Definition: `src/network/protocol.cppm:418`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function attempts to extract a JSON array from the provided `json::Value` by calling its `get_array` method. If the result is a null pointer, it returns an error using `LLMError` with a descriptive message that incorporates the `context` parameter, formatted via `std::format`. On success, it constructs and returns an `ArrayView` instance initialized directly from the underlying `kota::codec::json::Array` pointer.

The implementation has no loops or complex branching beyond the null check. Its dependencies are limited to the `json::Value` accessor, the `ArrayView` utility type (which holds the non‑owning pointer and provides iteration and size queries), and the `std::expected` infrastructure for error propagation. The `context` string is used solely for error reporting, making the call‑site responsible for supplying meaningful location or field‑name information.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter
- `context` parameter
- `value.get_array()` (through the `json::Value` object)

#### Usage Patterns

- Used in JSON parsing helpers to extract arrays from parsed values
- Called by higher-level validation functions when an array is expected

### `clore::net::detail::expect_object`

Declaration: `src/network/protocol.cppm:256`

Definition: `src/network/protocol.cppm:400`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_object` first invokes `value.get_object()` to obtain a pointer to the underlying `kota::codec::json::Object`. If the pointer is null (meaning the value is not a JSON object), it returns `std::unexpected` containing a `LLMError` with a descriptive message that incorporates the `context` string. On success, it constructs and returns a `clore::net::detail::ObjectView` by initializing its `value` member with the retrieved object pointer. This function serves as a typed accessor that converts a generic `json::Value` into a lightweight view over a JSON object, propagating a structured error when the expected type does not match.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const json::Value& value` parameter
- `std::string_view context` parameter

#### Usage Patterns

- Used to validate and extract an object view from a JSON value
- Called in parsing or validation contexts where a JSON object is expected

### `clore::net::detail::expect_object`

Declaration: `src/network/protocol.cppm:259`

Definition: `src/network/protocol.cppm:409`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function attempts to obtain a raw JSON object pointer from the provided `json::Cursor` by calling `get_object()`. If the result is null — indicating the cursor does not currently reference a JSON object — it returns an `LLMError` with a message composed via `std::format` using the supplied `context` string. On success, it constructs and returns an `ObjectView` by initializing its `value` member with the obtained pointer. This routine serves as a low-level validation step, delegating the actual object extraction to the underlying JSON cursor and wrapping the result in a type‑safe view or an error.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `value` (`json::Cursor`)
- parameter `context` (`std::string_view`)

#### Usage Patterns

- validate that a JSON cursor points to an object
- convert a JSON cursor to an `ObjectView` for structured access
- used in JSON parsing and validation pipelines

### `clore::net::detail::expect_string`

Declaration: `src/network/protocol.cppm:268`

Definition: `src/network/protocol.cppm:436`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function begins by calling `value.get_string()`, which attempts to interpret the `json::Value` as a JSON string. If the result is absent (i.e., the value is not a string), it constructs an `std::unexpected` containing an `LLMError` with a message formed via `std::format`, embedding the given `context` to identify where the failure arose. Otherwise, it dereferences the returned optional and yields the resulting `std::string_view`. The only dependency is the `json::Value::get_string()` method and the `LLMError` type for error propagation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- value (`json::Value`)
- context (`std::string_view`)

#### Usage Patterns

- Extract string from parsed JSON
- Validate JSON field type

### `clore::net::detail::expect_string`

Declaration: `src/network/protocol.cppm:271`

Definition: `src/network/protocol.cppm:445`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_string` is a low-level validation helper that attempts to interpret the JSON value at a given `json::Cursor` as a string. Internally it calls `get_string()` on the cursor, which returns an `std::optional<std::string_view>`. If the optional is empty (the JSON value is not a string), the function returns a `std::unexpected` containing an `LLMError` whose message is constructed via `std::format` using the `context` parameter to describe the expected location (e.g., “`field_name` is not a JSON string”). On success, it returns the contained `std::string_view` by dereferencing the optional. This function has no external dependencies beyond the JSON cursor type (`kota::codec::json::Cursor`), and its sole purpose is to perform a type assertion while providing a descriptive error context; it is used by higher-level parsing routines in the `clore::net::detail` namespace to unpack string fields from JSON objects or arrays.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor` value
- `std::string_view` context

#### Usage Patterns

- Extracting a required string from a JSON value in parsing routines
- Validating that a JSON value is a string before further processing
- Providing context-aware error messages when validation fails

### `clore::net::detail::infer_output_contract`

Declaration: `src/network/protocol.cppm:643`

Definition: `src/network/protocol.cppm:660`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function first checks whether `request.response_format` contains a value. If it does, the result is unconditionally `PromptOutputContract::Json`. However, if the caller also specified `request.output_contract` as `PromptOutputContract::Markdown`, the function returns an `LLMError` indicating a conflict between the `response_format` and the explicit output contract. If no `response_format` is present, the function next examines `request.output_contract`: if it is `PromptOutputContract::Unspecified`, an error is returned mandating that the contract be explicitly set to `Json` or `Markdown`. Otherwise the function simply returns the provided `PromptOutputContract`. This logic depends only on the fields of `PromptRequest` and the `PromptOutputContract` enumeration, using no external infrastructure or asynchronous operations.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.response_format`
- `request.output_contract`

#### Usage Patterns

- Infer output contract before processing a `PromptRequest`
- Validate consistency between `output_contract` and `response_format`

### `clore::net::detail::insert_string_field`

Declaration: `src/network/protocol.cppm:227`

Definition: `src/network/protocol.cppm:315`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function directly inserts a string field into the target `json::Object` by copying the provided `key` and `value` parameters into `std::string` and calling `object.insert`. The `context` parameter is not used in the body; it likely exists for consistency with other functions in the same detail namespace or for diagnostic purposes. No error handling or validation is performed—the function always returns a success `std::expected<void, LLMError>`. Its minimal control flow consists solely of the insertion operation and an implicit return of an empty expected. The only external dependency is the `json::Object::insert` method, which expects heap-allocated string keys and values.

#### Side Effects

- mutates the `json::Object` by inserting a key-value pair
- allocates dynamic memory for the key and value `std::string` objects

#### Reads From

- the `key` parameter
- the `value` parameter
- the `context` parameter (unused)
- the `json::Object` parameter (to insert into)

#### Writes To

- the `json::Object` parameter (inserts a field)

#### Usage Patterns

- used to set string fields in JSON objects during request construction
- called by higher-level serialization functions

### `clore::net::detail::make_empty_array`

Declaration: `src/network/protocol.cppm:243`

Definition: `src/network/protocol.cppm:360`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::make_empty_array` constructs an empty JSON array by parsing the string literal `"[]"` using `json::parse<json::Array>`. If the parse operation fails—which should not happen for a static well‑formed input but is handled defensively—the error is forwarded via `unexpected_json_error` together with the provided `context` string to produce a `std::expected` containing an `LLMError`. On success, the parsed `json::Array` is moved out. The function serves as a lightweight factory that centralises the creation of an empty array, relying on the same error‑reporting infrastructure used elsewhere in the JSON‑parsing utilities.

#### Side Effects

- Allocates a new `json::Array` and transfers ownership to the caller.

#### Reads From

- the `context` parameter (only when an error occurs)

#### Writes To

- the returned `std::expected<json::Array, LLMError>` object

#### Usage Patterns

- Create an empty JSON array in a given context
- Handle JSON parse errors with context
- Used where an empty array is needed with error reporting

### `clore::net::detail::make_empty_object`

Declaration: `src/network/protocol.cppm:240`

Definition: `src/network/protocol.cppm:352`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::make_empty_object` begins by invoking `json::parse<json::Object>` on the string literal `"{}"`. If parsing fails (i.e., the returned `expected` does not have a value), it delegates to `unexpected_json_error` forwarding both the caller-provided `context` string and the parse error, producing an error result. On success, it unwraps and returns the parsed `json::Object`. The function thus serves as a safe factory for an empty JSON object, relying on the JSON parser’s ability to handle the trivial literal and propagating any unexpected failure through the dedicated error helper.

#### Side Effects

- Allocates memory for the returned `json::Object` via `json::parse`
- May call `unexpected_json_error` which could perform logging or error reporting

#### Reads From

- Parameter `context` (only in error path)
- String literal `"{}"`

#### Writes To

- Returned `json::Object` via move
- Potentially error logging via `unexpected_json_error`

#### Usage Patterns

- Creating a base empty object for incremental construction
- Serving as a default or placeholder JSON object

### `clore::net::detail::normalize_utf8`

Declaration: `src/network/protocol.cppm:225`

Definition: `src/network/protocol.cppm:305`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::normalize_utf8` normalizes a `std::string_view text` into a valid UTF-8 `std::string` by invoking `clore::support::ensure_utf8`. After normalization, it compares the result to the original input; if a difference is detected, it logs a warning via `logging::warn`, identifying the `field_name` that contained the invalid byte sequences. The normalized string is then returned. The implementation has no other branching or state‑dependent behavior, relying entirely on the external normalization utility for the actual byte‑level handling.

#### Side Effects

- Logs a warning message via `logging::warn` when invalid UTF-8 sequences are replaced.

#### Reads From

- `text` parameter (input string)
- `field_name` parameter (used for warning message)

#### Writes To

- Return value (normalized string)
- Logging system (warning message)

#### Usage Patterns

- Used before JSON serialization to ensure UTF-8 validity of LLM responses or inputs

### `clore::net::detail::parse_json_value`

Declaration: `src/network/protocol.cppm:247`

Definition: `src/network/protocol.cppm:369`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::parse_json_value` relies on `json::parse` to deserialize a `std::string_view` into the template type `T`. Internally, it calls `json::parse<T>(raw)` and checks whether the result holds a value. If parsing fails, the function constructs a `LLMError` with a descriptive message that includes the caller-provided `context` string and the error text from `parsed.error().to_string()`, then returns that error wrapped in `std::unexpected`. On success, it moves the parsed value out of the `std::expected` and returns it. The function has no loops or branching beyond the single error check; its entire control flow is linear. Its primary dependencies are the `json` parsing infrastructure (from the `kota` library) and the `LLMError` type used throughout the module for error reporting.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- raw
- context

#### Usage Patterns

- parse JSON into expected type
- handle JSON parsing errors with context

### `clore::net::detail::parse_json_value`

Declaration: `src/network/protocol.cppm:250`

Definition: `src/network/protocol.cppm:380`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::parse_json_value` for a `json::Value` input serves as a thin wrapper that bridges the structured JSON representation to the string‑based parsing path. The function first attempts to serialize the given `value` to a string using `json::to_string`. If serialization fails, it calls `clore::net::detail::unexpected_json_error` with a descriptive message (including the provided `context`) and the serialization error, returning the resulting `LLMError`. On success, the function delegates to the sibling overload `clore::net::detail::parse_json_value<T>(std::string_view, std::string_view)`, forwarding the serialized string and the original `context`. This design isolates serialization‑related failure handling while reusing the existing string‑based parsing logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (const `json::Value`&)
- `context` (`std::string_view`)
- internal implementation of `json::to_string`

#### Usage Patterns

- Converting a `json::Value` to a string then parsing it into a given type
- Error handling during JSON serialization
- Delegating to string-based parsing

### `clore::net::detail::request_text_once_async`

Declaration: `src/network/protocol.cppm:650`

Definition: `src/network/protocol.cppm:692`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function first infers the output contract from the request using `infer_output_contract`. If inference fails, the coroutine fails immediately. It then prepares an optional `ResponseFormat`: if the contract is `PromptOutputContract::Json` and no format was provided, a schema‑less format is set; otherwise the request’s own format is used. A single `CompletionRequest` is assembled from the supplied `model` and `system_prompt`, the user’s prompt, the resolved `response_format`, and any `tool_choice` (tools and `parallel_tool_calls` are left empty). The `CompletionRequester` is invoked with this request and the event loop, and the response is awaited. From the response, `protocol::text_from_response` extracts the text content; if extraction fails, the error is forwarded. Finally, `validate_prompt_output` checks the extracted text against the inferred contract, failing if validation fails, and the validated text is returned. No conversation loop or tool‑call handling occurs—this is a single‑turn, text‑only completion.

#### Side Effects

- sends an HTTP request via the provided `completion_requester`
- mutates the `completion_request` object
- `co_await` suspends and resumes the coroutine
- may call `kota::fail` to propagate errors

#### Reads From

- parameters: `completion_requester`, model, `system_prompt`, request, loop
- result of `infer_output_contract`
- response from `completion_requester`
- output of `protocol::text_from_response`

#### Writes To

- local variable contract
- local variable `response_format`
- local variable `completion_request`
- local variable response
- local variable text
- local variable validation
- return value of the coroutine (`std::string`)

#### Usage Patterns

- called to asynchronously obtain validated LLM text
- used in the request pipeline for LLM interactions
- typically invoked with a lambda wrapping `perform_http_request_async`

### `clore::net::detail::run_task_sync`

Declaration: `src/network/protocol.cppm:238`

Definition: `src/network/protocol.cppm:334`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::run_task_sync` implements a synchronous execution harness for an asynchronous task. It first creates a local `kota::event_loop` instance and invokes the supplied callable `make_task` with a reference to that loop to obtain an asynchronous operation. The operation is scheduled on the loop via `loop.schedule`, after which `loop.run()` is called to block the current thread until all scheduled work completes. After the loop finishes, the function retrieves the operation’s result. If the result contains an error, it is wrapped in `std::unexpected` and returned as a `std::expected<T, LLMError>`. For a `void` return type `T`, the function returns a default-constructed `expected`; otherwise it moves the value out of the operation’s result.

The primary dependencies are the `kota::event_loop` type (which provides the scheduling and blocking `run` mechanism) and the operation type produced by `make_task`, which must offer `result()`, `has_error()`, and (for non-void `T`) a dereference `operator`. The function acts as a glue layer that converts an asynchronous, callback‑based workflow into a synchronous call site, handling error propagation and value extraction in a generic, template‑driven manner.

#### Side Effects

- executes the event loop, which may perform I/O, network calls, or other asynchronous operations
- moves the result from the operation into the return value

#### Reads From

- `make_task` callable parameter
- `operation.result()`

#### Writes To

- local `kota::event_loop` object `loop`
- local operation object
- returned `std::expected<T, LLMError>`

#### Usage Patterns

- used to run an asynchronous task synchronously within a caller's scope
- wraps an async operation that returns `std::expected<T, LLMError>`

### `clore::net::detail::serialize_value_to_string`

Declaration: `src/network/protocol.cppm:253`

Definition: `src/network/protocol.cppm:390`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::serialize_value_to_string` converts a `json::Value` into its string representation by delegating to `json::to_string`. If serialization succeeds, the resulting string is returned; otherwise, the function forwards the error using `unexpected_json_error`, enriching the diagnostic message with the `context` string. This wrapper standardises error handling for JSON serialization within the protocol layer, ensuring that failures are consistently reported with contextual information.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- value
- context

#### Usage Patterns

- Serialize JSON for network transmission or error reporting

### `clore::net::detail::to_llm_unexpected`

Declaration: `src/network/protocol.cppm:233`

Definition: `src/network/protocol.cppm:324`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This function constructs an `LLMError` by combining the given `context` string with the string representation of the `Status` error using `std::format`. It then wraps the resulting `LLMError` in a `std::unexpected` object and returns it. The implementation is linear and straightforward: it relies on `Status::to_string()` to produce a human-readable error message and on `std::format` for string concatenation. No loops, conditionals, or additional error handling are involved. The function serves as a conversion utility for turning domain-specific status codes into standard LLM error results within the `clore::net` library.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- error
- context

#### Usage Patterns

- Used to return an unexpected `LLMError` from functions returning `std::expected<T, LLMError>`
- Called in error paths of other detail functions like `perform_http_request`, `read_environment`, etc.

### `clore::net::detail::unexpected_json_error`

Declaration: `src/network/protocol.cppm:222`

Definition: `src/network/protocol.cppm:300`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::unexpected_json_error` is a straightforward conversion function that transforms a JSON library error into the domain-specific error type `LLMError`. It accepts two parameters: a `std::string_view` named `context` providing contextual information about where the error occurred, and a `const json::error&` named `err` representing the underlying JSON parsing or validation failure. The function constructs a descriptive error string by concatenating `context` and the result of `err.to_string()` using `std::format`, then wraps this string in a `std::unexpected<LLMError>` object, which is the standard mechanism for returning an error from a function that uses the `std::expected` pattern. The core logic is a single expression; there is no branching, looping, or conditional logic. The function depends on two external types: `json::error` from the JSON codec library (likely `kota::codec::json`) for the error representation, and `LLMError` from the same module for the unified error type. This utility acts as a thin adapter to consistently propagate JSON-related failures through the network layer's error reporting mechanism.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `context` parameter
- `err` parameter (calls `err.to_string()`)

#### Writes To

- heap memory for the `LLMError` string
- heap memory for the `std::unexpected` object

#### Usage Patterns

- used to convert JSON parsing errors into `std::unexpected<LLMError>` for error propagation
- likely called from other JSON-handling functions like `parse_json_value`, `expect_string`, etc.

### `clore::net::detail::validate_prompt_output`

Declaration: `src/network/protocol.cppm:646`

Definition: `src/network/protocol.cppm:678`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::validate_prompt_output` dispatches to the appropriate content validation routine based on the `PromptOutputContract` value. It switches on the `contract` parameter: for `PromptOutputContract::Json` it delegates to `protocol::validate_json_output`, for `PromptOutputContract::Markdown` it delegates to `protocol::validate_markdown_fragment_output`, and for `PromptOutputContract::Unspecified` it returns an error indicating that the contract must be explicitly set. Any unrecognized contract value triggers a generic unsupported contract error. The function thus acts as a thin dispatch layer that maps an enumeration to a specific validation function, depending on `protocol`-level validators that each handle a single output format.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content
- contract

#### Usage Patterns

- Called to verify that LLM output matches the expected format before further processing

### `clore::net::get_probed_capabilities`

Declaration: `src/network/protocol.cppm:138`

Definition: `src/network/protocol.cppm:741`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::get_probed_capabilities` implements a thread‑safe cache for `ProbedCapabilities` objects. Internally it uses a static `mutex` and a static `std::unordered_map` mapping `std::string` keys (derived from the `cache_key` parameter) to `std::unique_ptr<ProbedCapabilities>`. When called, the function acquires the lock, looks up the key; if found, it returns a reference to the existing object. Otherwise, it emplaces a new default‑constructed `ProbedCapabilities` into the cache and returns a reference to the newly inserted entry. The actual capability‑probing logic is performed elsewhere; this function only manages the storage and retrieval of the result.

#### Side Effects

- acquires a static mutex lock
- inserts a new `ProbedCapabilities` object into the static cache if the key is not already present

#### Reads From

- `cache_key` parameter
- static cache map entries

#### Writes To

- static cache map (insertion of new element)
- static mutex (lock state)

#### Usage Patterns

- used to lazily obtain or create cached capability probing results
- called during capability probing to ensure one probe per key

### `clore::net::icontains`

Declaration: `src/network/protocol.cppm:780`

Definition: `src/network/protocol.cppm:780`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

Implementation: [Implementation](functions/icontains.md)

The function `clore::net::icontains` performs a case‑insensitive substring search over two `std::string_view` parameters `haystack` and `needle`. The algorithm first short‑circuits when `needle.size()` exceeds `haystack.size()`, immediately returning `false`. Otherwise, it slides a window of size `needle.size()` across `haystack` using a pair of nested loops: the outer loop advances a starting index `i` up to `haystack.size() - needle.size()`, and the inner loop compares each corresponding character after normalizing both to lowercase via `std::tolower` with a cast to `unsigned char` to avoid undefined behavior for negative `char` values. If all characters in a window match, the function returns `true`; if no window completes, it returns `false`.

The implementation relies only on the C++ standard library (`std::tolower`, `std::size_t`, `std::string_view`) and contains no project‑specific dependencies. Its control flow is linear: an early size guard, a forward scan with a character‑by‑character equality check, and an immediate early exit on success.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `haystack`
- `needle`

#### Usage Patterns

- Used in `is_feature_rejection_error` to perform case-insensitive substring checks on error messages.

### `clore::net::is_feature_rejection_error`

Declaration: `src/network/protocol.cppm:147`

Definition: `src/network/protocol.cppm:800`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::is_feature_rejection_error` performs a simple heuristic substring match against a predefined set of patterns commonly found in LLM error responses when a requested feature is not supported. The function initializes a static array of `std::string_view` patterns such as `"unsupported parameter"`, `"not supported"`, and `"unrecognized field"`. It then iterates over each pattern, using `clore::net::icontains` to perform a case‑insensitive substring search within the given `error_message`. If any pattern is found, the function immediately returns `true`; otherwise, after exhausting the list, it returns `false`. The only external dependency is `clore::net::icontains`, which abstracts the case‑insensitive containment check. The control flow is linear and deterministic, with no branching beyond the early‑exit success condition.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error_message` parameter
- static `patterns` array

#### Usage Patterns

- detecting feature rejection in LLM API errors
- determining whether to retry without specific parameters

### `clore::net::make_capability_probe_key`

Declaration: `src/network/protocol.cppm:140`

Definition: `src/network/protocol.cppm:755`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function constructs a unique key string by joining the three inputs—`provider`, `api_base`, and `model`—with a `'|'` separator via `std::format`. This produces a deterministic, flat string (e.g., `"openai|https://api.openai.com/v1|gpt-4"`) that serves as a cache or lookup key. There is no conditional logic or iteration; the entire operation is a single formatted concatenation. The resulting key is used in conjunction with `clore::net::get_probed_capabilities` to retrieve or store per‑model capability metadata, ensuring that each distinct provider–base–model triple maps to its own `ProbedCapabilities` entry.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- provider
- `api_base`
- model

#### Usage Patterns

- building probe key for capability cache
- creating unique identifier for provider+model combination

### `clore::net::make_markdown_fragment_request`

Declaration: `src/network/protocol.cppm:111`

Definition: `src/network/protocol.cppm:156`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::make_markdown_fragment_request` creates a `PromptRequest` by moving the input `prompt` string into the `.prompt` field, leaving `.response_format` as `std::nullopt`, and setting `.output_contract` to `PromptOutputContract::Markdown`. It is a straightforward factory with no internal control flow or loops; its sole purpose is to construct and return the request with the expected contract for markdown output. The returned `PromptRequest` is later used by the completion pipeline, where the `output_contract` value directs downstream validation routines such as `protocol::validate_markdown_fragment_output` to enforce that the model’s response is a valid Markdown fragment.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- prompt parameter

#### Usage Patterns

- Construct a `PromptRequest` for markdown fragment generation

### `clore::net::parse_rejected_feature_from_error`

Declaration: `src/network/protocol.cppm:149`

Definition: `src/network/protocol.cppm:819`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::parse_rejected_feature_from_error` performs a linear scan over a static array of keyword-to-field mapping pairs. For each pair, it checks whether the given `error_message` contains the keyword using the helper function `clore::net::icontains`. If a match is found, the corresponding field name (e.g., `"response_format"`, `"tool_choice"`, `"tools"`) is returned as a `std::string` wrapped in a `std::optional`. If no keyword matches, `std::nullopt` is returned.

The control flow is a simple early‑return loop; the first matching keyword determines the result. This function depends on `clore::net::icontains` for case‑insensitive substring matching and relies on a pre‑defined set of feature‑related keywords (such as `"json_schema"`, `"parallel_tool_calls"`, and `"functions"`) that are known to appear in provider error messages when a requested capability is unsupported.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `error_message`

#### Usage Patterns

- Used to parse error responses from LLM `APIs` to identify which feature was rejected
- Likely called in error handling paths after calling LLM functions

### `clore::net::protocol::append_tool_outputs`

Declaration: `src/network/protocol.cppm:485`

Definition: `src/network/protocol.cppm:556`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function begins by validating that the response contains at least one tool call; if not, it returns an error. It then constructs an `unordered_map` from `tool_call_id` to `output` from the provided `ToolOutput` objects, rejecting any empty ids or duplicate keys. Next, the history is copied into a new vector and an `AssistantToolCallMessage` is appended, carrying the response’s text and tool calls. For each tool call in the response, the corresponding output is looked up by id; a `ToolResultMessage` is appended with that id and the output. Finally, the function verifies that every tool output was consumed (no extra unknown ids in the map) and returns the merged message sequence. This implementation relies on standard containers (`std::unordered_map`, `std::span`, `std::vector`), error handling via `std::expected` and `LLMError`, and direct access to fields on `CompletionResponse::message`, `ToolCall`, and `ToolOutput`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `history` span
- `response.message.tool_calls`
- `response.message.text`
- `outputs` span
- `ToolOutput.tool_call_id`
- `ToolOutput.output`

#### Usage Patterns

- Processing tool call results to reconstruct full message sequence
- Appending assistant tool call and tool result messages after an API response with tool calls

### `clore::net::protocol::parse_response_text`

Declaration: `src/network/protocol.cppm:491`

Definition: `src/network/protocol.cppm:604`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The template function `clore::net::protocol::parse_response_text` first invokes `clore::net::protocol::text_from_response` on the incoming `CompletionResponse` to obtain a string representation of the response content. If this step fails, it immediately returns the resulting `LLMError` via `std::unexpected`. On success, the extracted text is passed to `kota::codec::json::parse<T>` to decode a JSON value of the target type `T`. A parse failure produces an `LLMError` constructed with `std::format` containing the error description from the JSON library. Otherwise, the parsed value is returned directly. This function depends on `text_from_response` for textual extraction and on the JSON parsing infrastructure in `kota::codec::json` for deserialization.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- response

#### Usage Patterns

- Parse structured LLM response into type T
- Used after receiving a completion response

### `clore::net::protocol::parse_tool_arguments`

Declaration: `src/network/protocol.cppm:494`

Definition: `src/network/protocol.cppm:619`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The template function `clore::net::protocol::parse_tool_arguments` implements a two‑stage serialization‑then‑parsing pipeline to convert the JSON arguments of a `ToolCall` into a caller‑specified type `T`. It first serializes `call.arguments` to a string via `kota::codec::json::to_string` ; if serialization fails, it returns a descriptive `LLMError`. Otherwise, it deserializes the resulting string into `T` using `kota::codec::json::parse<T>` and returns the value on success, or another `LLMError` on parse failure. The function leverages the `std::expected` return pattern and relies on external JSON handling from the `kota::codec::json` namespace, with `call.name` providing context in error messages.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `call.arguments`
- `call.name`

#### Usage Patterns

- used to convert tool call arguments into typed structure
- called in tool invocation handling

### `clore::net::protocol::text_from_response`

Declaration: `src/network/protocol.cppm:483`

Definition: `src/network/protocol.cppm:540`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function `clore::net::protocol::text_from_response` implements a straightforward extraction and validation pipeline. It first checks the `refusal` field of the response's `message` member; if a refusal string is present, it returns an `LLMError` containing that refusal. Next, it verifies that the `tool_calls` vector is empty—any tool calls cause an immediate error because the caller expects a plain text completion. Finally, it confirms that the `text` optional field has a value; if not, an error is returned. On successful validation, the function returns the unwrapped text string. The logic depends only on the `AssistantOutput` structure (which aggregates `refusal`, `tool_calls`, and `text`) and the `std::expected` machinery.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- response`.message``.refusal`
- response`.message``.tool_calls`
- response`.message``.text`

#### Usage Patterns

- Extract text from a completion response for further processing

### `clore::net::protocol::validate_json_output`

Declaration: `src/network/protocol.cppm:479`

Definition: `src/network/protocol.cppm:500`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function parses the content string as JSON via `kota::codec::json::parse<kota::codec::json::Value>`. If parsing fails, it wraps the parser’s error message into a `LLMError` and returns `std::unexpected`; otherwise it returns an empty expected success. The implementation depends solely on the JSON parser library and the project’s error type; no other validation or structural checks are performed on the parsed value.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content (`std::string_view` parameter)

#### Usage Patterns

- Validating JSON output from an LLM before further processing
- Called as a validation step in protocol handling

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `src/network/protocol.cppm:481`

Definition: `src/network/protocol.cppm:509`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function performs a sequence of defensive checks on the given output string, returning early with an error description if any condition is violated. It first rejects empty content and content that consists only of whitespace. It then scans for disallowed markdown patterns: a top-level H1 heading (either at the start of the string or following a newline) and the presence of a code fence (triple backticks). Finally, it delegates to `clore::net::protocol::validate_json_output` to detect whether the output is valid JSON; if so, the function returns an error because a markdown fragment was required but JSON‑structured content was provided. Only when all checks succeed does the function return an empty `std::expected<void, LLMError>`. Dependencies include the JSON‑validation routine `validate_json_output` and standard library utilities for string inspection (`std::isspace`, `std::string_view::starts_with`, `std::string_view::find`).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content parameter

#### Usage Patterns

- Validating LLM responses expected to be markdown fragments
- Called during response parsing in protocol module

### `clore::net::sanitize_request_for_capabilities`

Declaration: `src/network/protocol.cppm:144`

Definition: `src/network/protocol.cppm:761`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function operates by creating a mutable copy of the incoming `CompletionRequest` and then sequentially checking each capability flag stored in the `ProbedCapabilities` instance. All flag loads use `std::memory_order_relaxed` since this is a one‑way sanitization that does not synchronize with other operations. If `supports_json_schema` is false, the optional `schema` sub‑field inside `response_format` is reset to `std::nullopt`. If `supports_tool_choice` is false, the entire `tool_choice` optional is cleared. If `supports_parallel_tool_calls` is false, that optional is also cleared. Finally, if `supports_tools` is false, the `tools` vector is emptied. The modified copy is then returned.

The internal control flow is a straight‑line sequence of four independent conditional statements; there is no branching or looping among the capability checks. The function’s only dependency is the `ProbedCapabilities` structure (specifically its four atomic boolean fields) and the `CompletionRequest` structure with its nested types (`ResponseFormat`, `ToolChoice`, `ToolChoiceNone`, `ForcedFunctionToolChoice`, `ToolChoiceAuto`, `ToolChoiceRequired`, and `FunctionToolDefinition`). No other functions are called within `sanitize_request_for_capabilities`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `caps` (`ProbedCapabilities`) atomic fields: `supports_json_schema`, `supports_tool_choice`, `supports_parallel_tool_calls`, `supports_tools`
- `request` (`CompletionRequest`) fields: `response_format`, `tool_choice`, `parallel_tool_calls`, `tools`

#### Writes To

- the returned `CompletionRequest` object, with reduced fields

#### Usage Patterns

- called before sending a request to match known provider capabilities
- used in request preprocessing pipelines

## Internal Structure

The `protocol` module is the core abstraction for the LLM network interaction layer, decomposed into public data types and internal utilities. It defines the message shape for requests and responses (`CompletionRequest`, `PromptRequest`, `CompletionResponse`, and a union of message roles via `Message`) alongside tool‑calling constructs (`ToolCall`, `ToolOutput`, `FunctionToolDefinition`, `ToolChoice` variants) and output contracts (`PromptOutputContract`, `ResponseFormat`). The module also exposes capabilities probing (`ProbedCapabilities`, `get_probed_capabilities`, `sanitize_request_for_capabilities`) and a set of synchronous, template‑based protocol operations (`text_from_response`, `append_tool_outputs`, `parse_tool_arguments`, etc.) that bridge between raw JSON and strongly‑typed application code.

Internally, the module is layered into a `clore::net::detail` namespace that handles low‑level JSON traversal (`ObjectView`, `ArrayView`), validation (`expect_string`, `expect_object`, `expect_array`), cloning (`clone_value`, `clone_object`, `clone_array`), serialization, and error shaping (`unexpected_json_error`, `to_llm_unexpected`, `excerpt_for_error`). These utilities are used by the higher‑level functions in `clore::net::protocol` to enforce schemas, normalize UTF‑8, and implement output contract validation (`validate_json_output`, `validate_markdown_fragment_output`, `infer_output_contract`). The module imports `http` for network communication and `support` for foundational string and cache utilities, and its implementation structure relies heavily on template functions and `std::string_view`‑based error context to remain type‑safe and diagnostic‑friendly without runtime overhead.

## Related Pages

- [Module http](../http/index.md)
- [Module support](../support/index.md)

