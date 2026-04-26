---
title: 'Module protocol'
description: 'The protocol module defines the core data structures and utilities for communicating with large language model (LLM) endpoints. It owns the public‑facing types that model a complete request‑response cycle, including message variants (SystemMessage, UserMessage, AssistantMessage, AssistantToolCallMessage, ToolResultMessage), composite request/response structures (CompletionRequest, CompletionResponse, PromptRequest, ResponseFormat), tool‑related types (ToolCall, ToolOutput, FunctionToolDefinition, ToolChoice variants), and capability probes (ProbedCapabilities). These serve as the primary interface for constructing, sending, and processing LLM interactions over the network.'
layout: doc
template: doc
---

# Module `protocol`

## Summary

The `protocol` module defines the core data structures and utilities for communicating with large language model (LLM) endpoints. It owns the public‑facing types that model a complete request‑response cycle, including message variants (`SystemMessage`, `UserMessage`, `AssistantMessage`, `AssistantToolCallMessage`, `ToolResultMessage`), composite request/response structures (`CompletionRequest`, `CompletionResponse`, `PromptRequest`, `ResponseFormat`), tool‑related types (`ToolCall`, `ToolOutput`, `FunctionToolDefinition`, `ToolChoice` variants), and capability probes (`ProbedCapabilities`). These serve as the primary interface for constructing, sending, and processing LLM interactions over the network.

In addition to the data types, the module provides a collection of parsing, validation, and transformation functions that operate on JSON representations of the protocol. These include helpers for deep‑cloning JSON values, arrays, and objects; type‑expectation functions (`expect_array`, `expect_object`, `expect_string`); output‑contract validators (`validate_json_output`, `validate_markdown_fragment_output`); and higher‑level routines such as `parse_tool_arguments`, `append_tool_outputs`, `text_from_response`, and `sanitize_request_for_capabilities`. It also implements detail utilities (`ObjectView`, `ArrayView`, `normalize_utf8`, `insert_string_field`, `serialize_value_to_string`, etc.) that support safe, context‑aware JSON manipulation and error reporting throughout the networking layer.

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

The struct `clore::net::AssistantMessage` is a simple data‑holder that represents a message originating from an assistant component within the networking layer. Its only member, `content`, is a `std::string` intended to store the textual payload of the message. The struct imposes no special invariants beyond those already guaranteed by `std::string` — namely, `content` always holds a valid, mutable string. As a trivially constructible aggregate, `AssistantMessage` supports default, copy, and move semantics without explicit user‑defined special members, and is designed for lightweight value‑oriented usage in protocol buffers or message passing.

#### Invariants

- A simple aggregate type with no invariants beyond standard struct initialization
- The `content` member is freely assignable and modifiable
- No constraints are implied by the definition

#### Key Members

- `content`

#### Usage Patterns

- Used as a message type in the `clore::net` namespace for network communication
- Likely serialized or passed as part of a protocol message

### `clore::net::AssistantOutput`

Declaration: `network/protocol.cppm:101`

Definition: `network/protocol.cppm:101`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The `clore::net::AssistantOutput` structure aggregates three members: `text` and `refusal`, both of type `std::optional<std::string>`, and `tool_calls`, a `std::vector<ToolCall>`. The implementation provides no user‑defined constructors, assignment `operator`s, or destructor, so the compiler implicitly declares memberwise copy, move, and destruction semantics, making the type a regular aggregate. Default initialization leaves both optional strings disengaged (`std::nullopt`) and the vector empty. The struct enforces no internal invariant among its fields; any combination of `text`, `refusal`, and `tool_calls` is permitted at the implementation level, leaving semantic consistency checks to the caller. The fields are laid out in declaration order, with the optional strings occupying a space‑optimized representation and the vector storing its contiguous buffer separately.

#### Invariants

- No explicit invariants are provided in the evidence; any combination of member values is possible.

#### Key Members

- text
- refusal
- `tool_calls`

#### Usage Patterns

- No specific usage patterns are indicated in the evidence; the struct is expected to be used to capture assistant responses.

### `clore::net::AssistantToolCallMessage`

Declaration: `network/protocol.cppm:35`

Definition: `network/protocol.cppm:35`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct aggregates an optional text response in `content` and a contiguous sequence of `ToolCall` objects in `tool_calls`. This design captures the two possible outcomes of an assistant message: it may contain natural‑language text, or it may consist of one or more tool invocations. The `std::optional<std::string>` for `content` explicitly supports the absence of a text part when the message is purely a tool call request. The `std::vector<ToolCall>` for `tool_calls` can be empty when the assistant provides only text, making the two fields mutually exclusive in intent. There are no user‑defined constructors, destructors, or assignment `operator`s; the compiler‑generated special members correctly value‑initialize both members, with `content` defaulting to `std::nullopt` and `tool_calls` to an empty vector.

### `clore::net::CompletionRequest`

Declaration: `network/protocol.cppm:77`

Definition: `network/protocol.cppm:77`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::CompletionRequest` is a plain data aggregate that holds all parameters for an LLM completion request. Its fields are carefully ordered to reflect common API usage: `model` and `messages` are required, while `response_format`, `tools`, `tool_choice`, and `parallel_tool_calls` are all wrapped in `std::optional` to allow omission. The presence of `tools` as a `std::vector` combined with `tool_choice` and `parallel_tool_calls` enables flexible tool-use configuration; an invariant is that if `tools` is empty then `tool_choice` and `parallel_tool_calls` should typically remain unset. The `model` string holds the deployment identifier and `messages` stores the conversation history; these two fields together constitute the minimal request.

#### Invariants

- `model` should typically be non-empty for a valid request
- `messages` is expected to contain at least one message for a valid request
- optional fields may be omitted or set to `std::nullopt`
- `tools` can be empty if no function calling is used

#### Key Members

- `model`
- `messages`
- `response_format`
- `tools`
- `tool_choice`
- `parallel_tool_calls`

#### Usage Patterns

- Constructed with all necessary fields before sending to an API endpoint
- Passed to request serialization or network functions
- Used to configure the behavior of a completion call

### `clore::net::CompletionResponse`

Declaration: `network/protocol.cppm:107`

Definition: `network/protocol.cppm:107`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::CompletionResponse` serves as a flat data container for the deserialized result of a completion API call. Its fields—`id`, `model`, `message` (of type `AssistantOutput`), and `raw_json`—directly correspond to the response fields from the upstream service. A key structural invariant is that `raw_json` preserves the original JSON payload exactly as received; while the other fields provide structured access, they are derived from and should be consistent with `raw_json`. No member functions are defined, so construction and assignment rely on default compiler-generated operations, and the struct is intended to be aggregate-initializable. This design keeps the type lightweight and suitable for move semantics, with all fields publicly accessible for direct inspection or serialisation.

#### Invariants

- All members are default-constructible and copyable.
- `raw_json` is expected to contain the original server response as a JSON string.
- `message` is of type `AssistantOutput`, which encapsulates the assistant's reply.

#### Key Members

- `id`
- `model`
- `message`
- `raw_json`

#### Usage Patterns

- Returned by completion client methods after a successful API call.
- Consumed by callers to extract the assistant's message or inspect the raw response.
- Stored or serialized for logging or audit purposes.

### `clore::net::ForcedFunctionToolChoice`

Declaration: `network/protocol.cppm:70`

Definition: `network/protocol.cppm:70`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ForcedFunctionToolChoice` is a simple aggregate value type that holds a single `std::string` member `name`. No special member functions are declared, so the compiler implicitly generates default constructors, destructor, and copy/move operations. The invariant of the type is that `name` should be a valid, non‑empty identifier representing a forced function name; however, no runtime checks enforce this within the struct itself. The implementation is essentially a thin wrapper around a string, used to mark a tool‑choice mode where the model must call a specific function.

#### Invariants

- No invariants imposed by the struct.

#### Key Members

- name

#### Usage Patterns

- Used as part of an API where a specific function tool must be forced.

### `clore::net::FunctionToolDefinition`

Declaration: `network/protocol.cppm:57`

Definition: `network/protocol.cppm:57`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::FunctionToolDefinition` aggregates the metadata needed to describe a callable tool in a network protocol. Its internal structure comprises four fields: `name` and `description` as plain strings, `parameters` as a `kota::codec::json::Object` that holds the JSON Schema defining the expected arguments, and a `strict` boolean that defaults to `true`. The non‑zero default for `strict` ensures that, unless explicitly overridden, the tool definition enforces strict schema validation, preventing extra or malformed parameters at the call site. No member functions are defined; the struct acts solely as a value‑type data carrier, so its invariants are maintained entirely by the callers that populate these fields before serialization or transport.

#### Invariants

- `name` and `description` should be non-empty for a valid definition.
- `strict` defaults to `true` when not explicitly set.
- `parameters` must be a valid JSON object representing the tool's parameter schema.

#### Key Members

- `std::string name`
- `std::string description`
- `kota::codec::json::Object parameters`
- `bool strict = true`

#### Usage Patterns

- Populated by client code to describe a tool function.
- Serialized or transmitted as part of a tool definition in network messages.
- Read by remote peers to understand available functions and their expected input.

### `clore::net::Message`

Declaration: `network/protocol.cppm:45`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::Message` is a type alias for `std::variant` over five distinct message types: `SystemMessage`, `UserMessage`, `AssistantMessage`, `AssistantToolCallMessage`, and `ToolResultMessage`. By using a variant, the alias enforces that any message instance contains exactly one of these types at a time, eliminating the need for inheritance or manual type tagging. The internal structure relies on the variant’s storage, which typically uses a union-like layout, ensuring type‑safe access and visitation. Important member‑like operations are provided by the variant’s standard interface, such as `std::visit` and type‑index queries, which enable pattern‑matched handling of each concrete message type without exposing the underlying storage. This design guarantees that all messages in the protocol conform to a fixed set of types while remaining fully value‑semantic.

### `clore::net::ProbedCapabilities`

Declaration: `network/protocol.cppm:119`

Definition: `network/protocol.cppm:119`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ProbedCapabilities` is a plain aggregate that bundles four `std::atomic<bool>` members — `supports_json_schema`, `supports_tool_choice`, `supports_parallel_tool_calls`, and `supports_tools` — each default‑initialized to `true`. Its primary invariant is that each flag starts in the optimistic state and may be relaxed to `false` as probing reveals the remote endpoint does not support the corresponding feature; no mechanism ever resets a flag to `true`. The use of `std::atomic<bool>` ensures that concurrent updates (e.g., from different probing operations or events) are safe without external synchronization, making the struct suitable for shared, mutable capability tracking in multi‑threaded network components.

#### Invariants

- Each capability flag defaults to `true`
- Flags can only transition from `true` to `false` as probing reveals lack of support

#### Key Members

- `supports_tools`
- `supports_tool_choice`
- `supports_parallel_tool_calls`
- `supports_json_schema`

#### Usage Patterns

- Initialized with optimistic defaults before probing
- Updated to `false` after discovering the API does not support a feature
- Read by request construction code to decide which parameters to include

### `clore::net::PromptOutputContract`

Declaration: `network/protocol.cppm:86`

Definition: `network/protocol.cppm:86`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation uses an underlying type of `std::uint8_t` to minimize storage footprint and ensure predictable binary layout. The enumerators are ordered as `Unspecified`, `Json`, `Markdown`, which yields default zero‑initialization for `Unspecified`. This ordering ensures that a default‑constructed or zero‑memory‑set instance will assume the `Unspecified` contract, often serving as a sentinel or uninitialized state within internal protocol dispatch logic.

#### Invariants

- Each enumerator corresponds to a distinct output contract.
- Values are limited to the three explicitly defined enumerators.

#### Key Members

- `clore::net::PromptOutputContract::Unspecified`
- `clore::net::PromptOutputContract::Json`
- `clore::net::PromptOutputContract::Markdown`

#### Usage Patterns

- Used to specify the expected output format when requesting a prompt response.

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

The implementation of `clore::net::PromptRequest` aggregates the core components of a client-generated prompt request into a single plain-data structure. Its internal state consists of a required `prompt` string, default-constructed to empty, and three optional configuration fields: `response_format`, `tool_choice`, and `output_contract`, each controlling a distinct aspect of the request. The `output_contract` field is never empty, defaulting to `PromptOutputContract::Unspecified`, while the two `std::optional` members may be left unset to indicate that no explicit value was provided by the caller. No invariants beyond these defaults are enforced by the struct itself; validation and composition logic reside in surrounding request-building code.

#### Invariants

- `prompt` is always initialized, even if empty
- `output_contract` defaults to `PromptOutputContract::Unspecified`
- `response_format` and `tool_choice` are optional and can be absent

#### Key Members

- `prompt`
- `response_format`
- `tool_choice`
- `output_contract`

#### Usage Patterns

- used to pass prompt text along with optional formatting options to a network service
- constructed and filled before being sent over the network

### `clore::net::ResponseFormat`

Declaration: `network/protocol.cppm:51`

Definition: `network/protocol.cppm:51`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ResponseFormat` is a plain data aggregate with three fields. The member `name` holds a string identifier for the response format. The optional `schema` field may contain a `kota::codec::json::Object` representing a JSON Schema that defines the expected structure of the response payload; when absent, no schema validation is applied. The `strict` flag defaults to `true` and indicates whether adherence to the schema (when present) is enforced or merely advisory. These fields collectively define the format, optional schema, and strictness policy for a network response. No invariants beyond the default value of `strict` are imposed by the struct itself; callers are responsible for ensuring that an existing `schema` object is well-formed.

#### Invariants

- `strict` defaults to `true`
- `schema` is optional and may be empty
- `name` is a non-empty string (implied by typical usage)

#### Key Members

- `name`
- `schema`
- `strict`

#### Usage Patterns

- Used as a parameter to specify response format in network requests
- Constructed directly as an aggregate

### `clore::net::SystemMessage`

Declaration: `network/protocol.cppm:16`

Definition: `network/protocol.cppm:16`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::SystemMessage` consists of a single data member `content` of type `std::string`. No custom constructors, destructors, or assignment `operator`s are declared, so the struct relies entirely on the compiler‑generated special members, which in turn delegate to `std::string`’s move and copy semantics. The absence of additional fields or logic means the object’s invariant is simply that `content` holds any valid string (including the empty string); memory ownership and lifetime are managed automatically by the string implementation.

#### Invariants

- `content` is a valid `std::string` and may be empty.

#### Key Members

- `content` (`std::string`)

#### Usage Patterns

- Direct access to the `content` field for reading or setting the system message text.
- Used as a data carrier in networking protocols for system-level messages.

### `clore::net::ToolCall`

Declaration: `network/protocol.cppm:24`

Definition: `network/protocol.cppm:24`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolCall` represents a tool invocation within the network protocol layer. Its internal state is split between a raw JSON string (`arguments_json`) and a parsed JSON value (`arguments`), storing the same payload in two forms. The invariant requires that `arguments_json` and `arguments` remain consistent with each other; any modification to one should be reflected in the other. The member `id` provides a unique identifier for the call, and `name` specifies the tool to be invoked. Important member implementations include constructors and assignment `operator`s that ensure the dual representation is synchronized, and a serialization method that outputs the `arguments` as JSON text into `arguments_json` when needed, or conversely parses `arguments_json` into `arguments` after deserialization.

#### Invariants

- No explicit invariants provided in evidence; members are independent strings and a JSON value.

#### Key Members

- `id`
- `name`
- `arguments_json`
- `arguments`

#### Usage Patterns

- Used in network protocol messages to encode tool call requests with a unique ID, tool name, and arguments in both serialized and deserialized forms.

### `clore::net::ToolChoice`

Declaration: `network/protocol.cppm:74`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The alias `clore::net::ToolChoice` defines a `std::variant` over four alternatives: `ToolChoiceAuto`, `ToolChoiceRequired`, `ToolChoiceNone`, and `ForcedFunctionToolChoice`. This discriminated union captures the possible modes for selecting a tool in the network layer, where exactly one alternative is active at any time. The variant’s internal representation is inherited from `std::variant`, meaning it carries a small integer index alongside an aligned storage buffer for the active type; size and alignment are determined by the largest alternative. Since `ToolChoice` is a direct alias, all member functions (`index()`, `valueless_by_exception()`, constructors, assignment `operator`s, `emplace()`, `get_if()`, visitation via `std::visit`, etc.) are those of `std::variant`. No custom invariants or special member implementations are added at the alias level; correctness depends on the semantics enforced by the four named alternative types, each of which models a specific tool‑selection policy.

#### Invariants

- The variant always holds exactly one of the four defined alternatives.
- The alternatives are mutually exclusive tool choice modes.

#### Key Members

- `ToolChoiceAuto`
- `ToolChoiceRequired`
- `ToolChoiceNone`
- `ForcedFunctionToolChoice`

#### Usage Patterns

- Used as a parameter type in function signatures to specify tool selection behavior.
- Consumed by visitor patterns or `std::visit` to dispatch based on tool choice mode.

### `clore::net::ToolChoiceAuto`

Declaration: `network/protocol.cppm:64`

Definition: `network/protocol.cppm:64`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolChoiceAuto` is an empty tag type with no data members or non‑defaulted special member functions. Its sole purpose is to serve as a distinct type discriminator within a variant or as a compile‑time flag representing automatic tool selection. Because it contains no state, the implicit default constructor, destructor, and copy/move operations are all trivial, and the type is suitable for use with empty base optimization. No invariants or member function implementations exist; the type is a pure marker with zero overhead.

#### Invariants

- The struct is always empty and trivially constructible.
- No state or data members exist.

#### Usage Patterns

- Used as an argument type for function overloading.
- Likely employed as a tag in template metaprogramming or policy-based design.
- May appear in variant or optional type parameters.

### `clore::net::ToolChoiceNone`

Declaration: `network/protocol.cppm:68`

Definition: `network/protocol.cppm:68`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolChoiceNone` is an empty type with no data members, base classes, or member functions. Being a complete trivial class, it has no internal state and therefore maintains no invariants. Its sole purpose is to serve as a type tag, enabling compile-time discrimination when the tool choice is explicitly `none` within the context of the chat completion API. The implementation is minimal and deliberate: the empty definition ensures zero overhead at runtime while providing a distinct type for template metaprogramming or function overloading.

#### Usage Patterns

- Used as a type tag in template or variant contexts to represent the absence of a tool choice.

### `clore::net::ToolChoiceRequired`

Declaration: `network/protocol.cppm:66`

Definition: `network/protocol.cppm:66`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolChoiceRequired` is an empty marker type with no data members or member functions. As an empty class, it imposes no invariants and requires no construction or destruction logic. Its sole purpose is to serve as a compile-time tag for type discrimination or to enable a policy-based design pattern within the networking protocol layer. The trivial implementation allows instances to be created with zero overhead and used in template specializations or function overloading.

#### Invariants

- Trivially default constructible
- Trivially destructible
- No members or base classes

#### Usage Patterns

- Used as a type tag for function overloading or template specialization
- May serve as a sentinel or enum alternative in network protocol handling

### `clore::net::ToolOutput`

Declaration: `network/protocol.cppm:114`

Definition: `network/protocol.cppm:114`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolOutput` is a simple aggregate that bundles two `std::string` members: `tool_call_id` and `output`. Its purpose is to represent the result of a tool execution, pairing the identifier of the invoked tool with the text output it produced. No invariants or explicit member implementations are required beyond the default behavior of its string members; the struct is trivially copyable and movable.

### `clore::net::ToolResultMessage`

Declaration: `network/protocol.cppm:40`

Definition: `network/protocol.cppm:40`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolResultMessage` is a simple aggregate type with two public `std::string` members: `tool_call_id` and `content`. It serves as a data carrier for the outcome of a tool invocation within the networking layer. No special invariants are enforced; both fields hold arbitrary strings and are intended to be populated before transmission. There are no user‑defined constructors, destructors, or member functions; the struct relies on default aggregate initialization and assignment.

#### Invariants

- No invariants beyond the default string state.

#### Key Members

- `tool_call_id`: identifier of the tool call.
- `content`: result content.

#### Usage Patterns

- Used to pass tool execution results through the network layer.
- Constructed with `{tool_call_id, content}` syntax.

### `clore::net::UserMessage`

Declaration: `network/protocol.cppm:20`

Definition: `network/protocol.cppm:20`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::UserMessage` is implemented as a thin value type with a single `std::string content` member. There are no custom constructors, assignment `operator`s, or destructors; the compiler-generated special member functions handle copying, moving, and destruction. The invariants are those of a `std::string` – `content` is always a valid string object (default-constructed to empty) and its lifetime is managed automatically. Because the struct contains only this one member, its size and layout are identical to `std::string` (subject to alignment). No additional internal state, sentinels, or validation logic exists; the struct is a straightforward wrapper intended to carry raw textual payload without any extra processing or constraints beyond those of the `std::string` type.

### `clore::net::detail::ArrayView`

Declaration: `network/protocol.cppm:174`

Definition: `network/protocol.cppm:174`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The struct `clore::net::detail::ArrayView` is a non-owning view over a `const kota::codec::json::Array`. Its only data member, `value`, is a raw pointer default-initialized to `nullptr`. The invariant is that `value` must point to a valid, fully constructed `kota::codec::json::Array` before any operation is invoked; calling any member when `value` is `nullptr` results in undefined behavior.  

Every member function is a thin delegation to the underlying array: `empty`, `size`, `begin`, and `end` forward directly; `operator[]` performs an indexed access via `(*value)[index]`; `operator->` returns the raw pointer itself; and `operator*` returns a const reference to the pointed-to array. There is no runtime checking of the pointer’s validity, consistent with the design goal of a minimal, zero-overhead view. Copy and move operations are compiler-generated, and the struct does not manage resource lifetime—it exists solely as a lightweight accessor.

#### Invariants

- `value` must not be null for any method call.
- The lifetime of the pointed-to array must outlive the `ArrayView`.

#### Key Members

- `value` field
- `operator[]`
- `operator*`
- `begin()` / `end()`
- `size()` / `empty()`

#### Usage Patterns

- Used to pass read-only access to a JSON array without copying.
- Provides a standard container-like interface for range-based for loops.
- Access elements by index via `operator[]`.

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

Implementation of `clore::net::detail::ObjectView` is a lightweight, non‑owning wrapper over a single raw pointer to `const kota::codec::json::Object`. The sole data member `value` is default‑initialised to `nullptr`; users of the class must ensure it points to a valid object before any member function is called. All operations are simple forwarding calls: `begin()` and `end()` delegate directly to `value->begin()` and `value->end()`, while `operator->()` and `operator*()` return the pointer and reference respectively. The `get()` member performs a key query on the underlying object and returns the result as `std::optional<kota::codec::json::Cursor>`. Because the class holds only a pointer and all methods are inline, it remains trivially copyable and imposes no overhead, serving as a zero‑cost abstraction for read‑only traversal and lookup of a JSON object.

#### Invariants

- `value` must point to a valid `kota::codec::json::Object` before any member function is called that dereferences it

#### Key Members

- `value`
- `get(std::string_view)`
- `begin()`
- `end()`
- `operator->`
- `operator*`

#### Usage Patterns

- Iterating over key-value pairs of a JSON object.
- Looking up a specific key using `get()`.
- Passing the object view to functions expecting a `const kota::codec::json::Object&` via dereference `operator`s.

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

The function `clore::net::detail::clone_array` provides a trivial but essential cloning mechanism for JSON arrays. Internally, it dereferences the `ArrayView` object’s `value` member (a pointer to a `kota::codec::json::Array`) and returns a new `json::Array` constructed as a copy of that underlying array. No iteration or conditional logic is required; the entire operation relies on the copy semantics of `json::Array`. The function does not perform any validation or error handling—its only dependency is the type of the array content itself, and it assumes the `ArrayView` is valid.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source` (`ArrayView`)
- `source.value` which points to a `json::Array`

#### Writes To

- newly allocated `json::Array` returned to caller

#### Usage Patterns

- Cloning array data for further manipulation
- Creating an independent copy of a JSON array
- Used in serialization and validation pipelines

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:258`

Definition: `network/protocol.cppm:442`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::clone_object` performs a deep copy of the provided `json::Object` by invoking its copy constructor and wrapping the result in a `std::expected<json::Object, LLMError>` success value. The second parameter, a `std::string_view` intended for context labeling in error messages, is not used within the body because the copy operation cannot fail; it is retained for interface consistency with other cloning functions in the same module that may need to report the operation site on failure. The implementation has no branching or loops and depends solely on the copy semantics of the underlying `json::Object` type.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- source parameter (const `json::Object`&)

#### Usage Patterns

- Copy-construct a `json::Object` from an existing one

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:261`

Definition: `network/protocol.cppm:447`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::clone_object` performs a single, straightforward operation: it constructs a new `json::Object` by copying the underlying JSON object referenced by the `ObjectView` parameter. It accesses the pointer stored in `ObjectView::value`, dereferences it with the `*` `operator`, and passes the result to `json::Object`’s copy constructor. The function then returns this newly created object wrapped in a `std::expected` with `LLMError` as the error type. There is no branching, error checking, or iteration; the entire algorithm is the copy operation itself. The second parameter (a `std::string_view`) is unused in the body, though it may serve a contextual role at call sites. The only external dependencies are the `kota::codec::json::Object` copy semantics and the `ObjectView` wrapper type.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `json::Object` & referred to by `source.value`

#### Writes To

- allocates a new `json::Object` via copy construction

#### Usage Patterns

- duplicating a JSON object for independent mutation or serialization

### `clore::net::detail::clone_value`

Declaration: `network/protocol.cppm:267`

Definition: `network/protocol.cppm:451`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::clone_value` is a straightforward pass-through that delegates entirely to the copy constructor of `json::Value`. It discards the `context` parameter (cast to void) and constructs a new `json::Value` by copying the `source` argument. The function always succeeds, returning the copy wrapped in a `std::expected` with no error path. This simple copy operation serves as the base case within a family of clone functions that operate recursively on JSON structures, providing a default behavior for leaf or primitive values where no structural transformation is needed.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source` parameter (const `json::Value`&)

#### Usage Patterns

- Used to duplicate a JSON value while ignoring the context string.

### `clore::net::detail::excerpt_for_error`

Declaration: `network/protocol.cppm:219`

Definition: `network/protocol.cppm:312`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::excerpt_for_error` is straightforward. It accepts a `std::string_view` parameter `body` and returns a `std::string` containing an excerpt of the input suitable for embedding in error messages. To limit the length, the function defines a local constant `kMaxBytes` with value `200` and delegates the actual truncation to `clore::support::truncate_utf8`, passing the same `body` and the byte limit. This utility function performs a UTF‑8‑aware truncation to ensure that the returned string does not exceed `kMaxBytes` bytes while preserving the validity of the last multi‑byte character. The control flow is entirely linear: the result of the delegating call is returned directly with no additional branching, loops, or state management.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `body` parameter (`std::string_view`)
- constant `kMaxBytes` (200)

#### Usage Patterns

- truncating long strings for error messages
- excerpting response bodies for logging or diagnostics

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:249`

Definition: `network/protocol.cppm:411`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_array` converts a `json::Cursor` into a `clore::net::detail::ArrayView` if the underlying JSON value is an array. It retrieves the raw array pointer by calling `value.get_array()`. If the pointer is `nullptr`, it returns a `std::unexpected` carrying an `LLMError` whose message is formatted via `std::format` using the provided `context` string. Otherwise, it constructs and returns a `clore::net::detail::ArrayView` initialized with the obtained array pointer. The function relies on the `json::Cursor` interface from `kota::codec::json` and on the `clore::net::detail::ArrayView` aggregate type.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` (a `json::Cursor`)
- `context` (a `std::string_view`)

#### Usage Patterns

- Validating that a JSON value is an array before further operations
- Extracting an `ArrayView` from a `json::Cursor`

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:246`

Definition: `network/protocol.cppm:402`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function validates that the incoming JSON value is an array by calling `get_array()` on the `json::Value`. If the pointer is `nullptr`, it returns `std::unexpected` with a `LLMError` whose message is constructed via `std::format` using the provided `context` to indicate that the value is not a JSON array. On success, it returns an `ArrayView` initialized with the non-null array pointer. This lightweight conversion is the sole responsibility of the function, relying on `json::Value` for type inspection and `ArrayView` as the return type used throughout the detail namespace for array-safe operations.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (via `get_array()`)
- `context` parameter

#### Usage Patterns

- Used in JSON validation and parsing contexts
- Called by overload `expect_array(json::Cursor, std::string_view)`
- Often paired with `expect_object`, `expect_string`, and `clone_array`

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:240`

Definition: `network/protocol.cppm:384`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_object` validates that a given `json::Value` is a JSON object, returning an `ObjectView` on success or an `LLMError` on failure. It calls `get_object()` on the input `value`; if the result is a null pointer, the function constructs an `LLMError` by formatting the `context` string into the message “{} is not a JSON object”. Otherwise, it returns an `ObjectView` initialized with the pointer obtained from `get_object()`. The helper relies on the `ObjectView` structure, which holds a pointer to the underlying `kota::codec::json::Object`, and on `LLMError` for error reporting. This function is one of several type‑expectation utilities used internally to perform safe, context‑aware extraction of JSON values during protocol parsing and request building.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- value (const `json::Value` &)
- context (`std::string_view`)

#### Usage Patterns

- Validate that a JSON value is an object before accessing its fields
- Provide an `ObjectView` to functions that require an object reference

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:243`

Definition: `network/protocol.cppm:393`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::expect_object` performs a single structural validation and conversion step. It calls `value.get_object()` to retrieve a raw pointer to the underlying `json::Object`. If that pointer is `nullptr`, the function returns `std::unexpected` containing an `LLMError` with a message derived from the `context` parameter, indicating that the current JSON value is not an object. On success, it constructs and returns a `clore::net::detail::ObjectView` struct initialized with the obtained object pointer.

This function depends on the JSON cursor’s `get_object` method to access the object type, and on the `ObjectView` struct and `LLMError` class. The control flow is a simple conditional branch with no loops or additional parsing, making it a straightforward type-safe accessor for expected JSON objects.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (`json::Cursor`)
- `context` parameter (`std::string_view`)
- The underlying JSON data via `value.get_object()`

#### Usage Patterns

- Validating that a JSON cursor represents an object during parsing or deserialization
- Extracting an `ObjectView` for subsequent field access or iteration
- Used internally by functions that require a JSON object input

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:252`

Definition: `network/protocol.cppm:420`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_string` implements a safe extraction of a string from a `json::Value`. It calls `value.get_string()` and checks the resulting `std::optional`. If the optional does not contain a value, it immediately returns `std::unexpected` with an `LLMError` that includes the `context` parameter in a formatted message stating the input is not a JSON string. On success, it returns the dereferenced string view. This function serves as a primitive validation helper, relying on the JSON library’s `get_string` method and the project’s error type `LLMError`; its control flow is a single conditional branch with no loops or side effects.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter of type `const json::Value &`
- `context` parameter of type `std::string_view`

#### Usage Patterns

- Used to validate and extract a string from a JSON value during JSON parsing
- Called when processing JSON fields that are expected to be strings

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:255`

Definition: `network/protocol.cppm:429`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function attempts to extract a JSON string from the given `json::Cursor` by calling `value.get_string()`. If the cursor does not hold a string (i.e., `get_string()` returns `std::nullopt`), it immediately returns `std::unexpected` containing an `LLMError` whose message is constructed via `std::format` using the `context` parameter to indicate which field or location failed validation. Otherwise, it dereferences the result and returns the underlying `std::string_view`. This helper is a thin wrapper over the cursor’s string accessor, relying solely on the Cursor API and the project’s error type.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor value`
- `std::string_view context`

#### Usage Patterns

- Used to validate and extract a JSON string value, returning an error on failure

### `clore::net::detail::infer_output_contract`

Declaration: `network/protocol.cppm:627`

Definition: `network/protocol.cppm:644`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function resolves the effective <code>`PromptOutputContract`</code> for a given <code>`PromptRequest`</code> by examining the <code>`response_format`</code> and <code>`output_contract`</code> fields. If a <code>`response_format`</code> is present, the contract is forced to <code>`PromptOutputContract::Json`</code>; a conflicting <code>`PromptOutputContract::Markdown`</code> setting causes an immediate error. When no <code>`response_format`</code> is provided, the function requires <code>`output_contract`</code> to be explicitly set to either <code>Json</code> or <code>Markdown</code> — leaving it as <code>Unspecified</code> yields an error. Otherwise the supplied <code>`output_contract`</code> is returned. This logic relies on <code>`std::expected`</code> to carry either the resolved contract or an <code>`LLMError`</code> describing the conflict.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.response_format`
- `request.output_contract`

#### Usage Patterns

- resolve output contract from a `PromptRequest`
- validate consistency between `response_format` and `output_contract`

### `clore::net::detail::insert_string_field`

Declaration: `network/protocol.cppm:211`

Definition: `network/protocol.cppm:299`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::insert_string_field` is a straightforward utility for adding a string-valued field to a JSON object during serialization. Its implementation consists of a single call to `kota::codec::json::Object::insert`, converting the two `std::string_view` parameters (`key` and `value`) to `std::string` instances. The `context` parameter is not used in the body but serves as a diagnostic label for error messages in other functions within the same module. The return type `std::expected<void, LLMError>` indicates the potential for an error, yet the function always returns an engaged `expected` with a default-constructed `void` value, as the insertion operation is infallible. This function is a low-level building block employed by higher-level serialization routines when constructing the JSON payload for API requests, relying on `kota::codec::json::Object` as its sole external dependency.

#### Side Effects

- Modifies the `json::Object` passed by reference by inserting a new key-value pair

#### Reads From

- parameters `key` and `value`

#### Writes To

- `object` (the `json::Object&` parameter)

#### Usage Patterns

- Insert a string field into a JSON object
- Used in JSON construction utilities

### `clore::net::detail::make_empty_array`

Declaration: `network/protocol.cppm:227`

Definition: `network/protocol.cppm:344`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `make_empty_array` is a utility that produces an empty JSON array by parsing the static string `"[]"`. It delegates to `json::parse<json::Array>`, which is expected to succeed for a well‑formed empty array. On failure—which would indicate an internal inconsistency or a change in the JSON parser—it invokes `unexpected_json_error` to convert the parse error into an `LLMError` using the caller‑supplied `context` string. Otherwise, it returns the successfully parsed `json::Array` value. This design centralizes the creation of an empty array without hardcoding an array constructor, relying on the parser’s validation for robustness. The function depends on `json::parse<json::Array>` and `unexpected_json_error` for error reporting; the algorithm is straightforward—attempt to parse, check success, and either forward the parse error or return the array.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `context` parameter (used for error reporting)
- the string literal `"[]"` (parsed as JSON)

#### Usage Patterns

- create an empty JSON array value
- obtain a default empty array
- initialize array fields with no elements

### `clore::net::detail::make_empty_object`

Declaration: `network/protocol.cppm:224`

Definition: `network/protocol.cppm:336`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::make_empty_object` creates a JSON object by parsing the literal string `"{}"` using `json::parse<json::Object>`. If the parse fails — which is not expected for an empty object but is handled defensively — the error is converted into an `LLMError` via `unexpected_json_error`, and the function returns `std::unexpected(error)`. Otherwise it returns the successfully parsed `json::Object`. The only internal control flow is the branch checking `parsed.has_value()`, and the sole dependency on the JSON library is the parse operation and the error conversion helper `unexpected_json_error`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- string literal `"{}"`
- parameter `context`
- parse result from `json::parse`
- error from parse failure

#### Usage Patterns

- Creating an empty object for default or placeholder values
- Used to represent an empty JSON object in LLM-related code

### `clore::net::detail::normalize_utf8`

Declaration: `network/protocol.cppm:209`

Definition: `network/protocol.cppm:289`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation of `clore::net::detail::normalize_utf8` delegates the core normalization to `clore::support::ensure_utf8`, which replaces invalid UTF-8 byte sequences in the input `text`. After normalization, it compares the result to the original `text`. If they differ, indicating that invalid sequences were present, it logs a warning via `logging::warn`, describing the issue with the provided `field_name`. The normalized string is then returned.

The function has no branching beyond the equality check and no iteration; its control flow is a single conditional. Its only external dependencies are `clore::support::ensure_utf8` for the actual normalization and the logging infrastructure. It does not perform validation or error recovery beyond what `ensure_utf8` provides.

#### Side Effects

- Logs a warning via `logging::warn` when invalid UTF-8 is detected.

#### Reads From

- `text` parameter (`std::string_view`)
- `field_name` parameter (`std::string_view`)

#### Writes To

- Return value (`std::string`) containing normalized UTF-8
- Log output via `logging::warn`

#### Usage Patterns

- Normalizing input text before JSON serialization
- Ensuring valid UTF-8 for LLM responses or prompts

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:231`

Definition: `network/protocol.cppm:353`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::parse_json_value` is a template helper that deserializes a JSON string into the requested type `T`. It takes two `std::string_view` parameters: `raw` (the JSON text to parse) and `context` (a human‑readable label used in error messages). Internally it calls `json::from_json<T>(raw)` from the `kota::codec::json` library; if this returns an error, the function constructs an `LLMError` containing a formatted message that includes the `context` and the textual error from the JSON parser, and returns that as an unexpected result. On success it returns the parsed value of type `T`. The function is a small wrapper that converts JSON parsing failures into the project’s `std::expected<T, LLMError>` error handling idiom, with no additional validation or transformation of the parsed data.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `raw` (input JSON string)
- `context` (error context label)
- `json::from_json<T>` (internal parsing)

#### Usage Patterns

- Used to deserialize JSON responses into strongly-typed objects
- Provides contextual error messages on parse failure

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:234`

Definition: `network/protocol.cppm:364`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function first serializes the input `json::Value` to a string via `json::to_string`. If serialization fails, it immediately returns an `unexpected_json_error` with a formatted message that includes the supplied `context`. On success, it delegates to the string-based overload `parse_json_value<T>(std::string_view, std::string_view)`, passing the serialized string and the same `context`. This design centralizes the actual parsing logic in the string-based overload while providing a uniform serialization‑and‑error‑handling step for callers that already hold a `json::Value`.

#### Side Effects

- allocates a temporary string via `json::to_string`

#### Reads From

- value (`json::Value`)
- context (`std::string_view`)

#### Usage Patterns

- used to parse a `json::Value` into a typed result
- bridge between `json::Value` and the string-based parsing path

### `clore::net::detail::request_text_once_async`

Declaration: `network/protocol.cppm:634`

Definition: `network/protocol.cppm:676`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function first infers the output contract using `infer_output_contract` and fails early if inference fails. It then prepares an optional `ResponseFormat` based on the request's own format and the output contract (for `Json` contracts without an explicit schema, a format with no schema is created). A `CompletionRequest` is assembled with the provided `model`, a system message from `system_prompt`, a user message from the request's prompt, the response format, an empty tools list, the request's tool choice, and no parallel tool calls. The external `request_completion` functor is awaited to produce a `CompletionResponse`, with failure propagated via `kota::fail`. From the response, `protocol::text_from_response` extracts the text content; extraction failure is also propagated. Finally, `validate_prompt_output` checks the extracted text against the contract, and on success the validated text is returned. Key internal dependencies are `infer_output_contract`, `validate_prompt_output` (both in `detail`), and `protocol::text_from_response`, as well as the `CompletionRequest` and `ResponseFormat` structures and the `kota` async infrastructure.

#### Side Effects

- performs an asynchronous HTTP request via the `request_completion` callback

#### Reads From

- parameters: `model`, `system_prompt`, `request`, `loop`
- result of `request_completion` (HTTP response)
- `request.response_format`
- `request.output_contract`
- `request.prompt`
- `request.tool_choice`

#### Usage Patterns

- used to asynchronously obtain a single text completion from an LLM
- called from higher-level async functions that need a textual response

### `clore::net::detail::run_task_sync`

Declaration: `network/protocol.cppm:222`

Definition: `network/protocol.cppm:318`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The implementation first creates a `kota::event_loop` and passes it to the callable `make_task`, which returns an asynchronous operation object. That operation is scheduled on the loop, and the loop is then run synchronously. After the loop finishes, `operation.result()` is inspected. If the result contains an error, it is wrapped in `std::unexpected` and returned. Otherwise, if the template parameter `T` is a non-void type, the contained value is moved out; if `T` is `void`, an empty `std::expected` is returned. The function depends on `kota::event_loop` for event‑driven concurrency and on the task operation providing a `.result()` method that returns a `std::expected<T, LLMError>`.

#### Side Effects

- Creates and runs a local event loop
- Schedules and executes the supplied async operation to completion
- Moves and returns the operation's result or error

#### Reads From

- `make_task` parameter (callable returning an async operation)
- operation`.result()` (to check for error and retrieve value)

#### Writes To

- Local event loop object (loop)
- Local operation object (operation)
- The async operation's internal state (via execution)
- Return value (`std::expected` or `std::unexpected`)

#### Usage Patterns

- Synchronously execute an async task defined by a factory callable
- Bridge async operations to a blocking interface
- Obtain the result of a task that runs on a temporary event loop

### `clore::net::detail::serialize_value_to_string`

Declaration: `network/protocol.cppm:237`

Definition: `network/protocol.cppm:374`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function calls `json::to_string` on the input `json::Value`. If the serialization succeeds, the resulting `std::string` is returned directly by dereferencing the `std::expected` result. On failure, it formats an error message that includes the caller‑supplied `context` string (via `std::format`) and delegates to `unexpected_json_error` to construct and return the appropriate `LLMError`. No additional validation or transformation is performed; the function solely converts a JSON value into its text representation, relying on the JSON codec library for the actual serialization and on the helper `unexpected_json_error` for error reporting.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (`json::Value`)
- `context` parameter (`std::string_view`)

#### Usage Patterns

- serializing JSON values for HTTP request body
- converting JSON to string for logging or error messages

### `clore::net::detail::to_llm_unexpected`

Declaration: `network/protocol.cppm:217`

Definition: `network/protocol.cppm:308`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::to_llm_unexpected` is a thin factory that converts an internal `Status` error into a `std::unexpected<LLMError>`. It takes a `Status error` and a `std::string_view context` describing where the error occurred. The algorithm is trivial: it calls `error.to_string()` to obtain a human-readable message from the custom error type, combines it with the `context` string using `std::format` in the pattern `"{}: {}"`, and passes the resulting string to the `LLMError` constructor. The constructed `LLMError` is then wrapped in `std::unexpected` to produce the return value. This helper is used throughout the protocol layer to produce uniform error objects from disparate error sources, and its only dependencies are the `LLMError` class and the `to_string()` member function of the `Status` template parameter. No branching or iteration occurs.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error` parameter
- `context` parameter
- `error.to_string()`

#### Usage Patterns

- Propagate `Status` errors as `LLMError` in `std::unexpected` return

### `clore::net::detail::unexpected_json_error`

Declaration: `network/protocol.cppm:206`

Definition: `network/protocol.cppm:284`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::unexpected_json_error` is a small helper that converts a JSON library error into an `LLMError` wrapped in `std::unexpected` for use in error‑returning contexts. It accepts a `std::string_view context` (describing where the error occurred) and a `const json::error& err`. The implementation calls `std::format` to produce a message string combining the `context` and the result of `err.to_string()`, then constructs an `LLMError` from that message and returns `std::unexpected<LLMError>`. There is no branching or iteration; the control flow is purely linear. The function depends on `std::format`, the `json::error::to_string()` method, the `LLMError` type (an error type defined elsewhere in the codebase), and the standard library’s `std::unexpected` wrapper. This utility is used throughout the implementation to uniformly wrap JSON parsing or validation failures into a common error representation.

#### Side Effects

- allocates memory for the formatted error string
- constructs a new `LLMError` object

#### Reads From

- context parameter
- err parameter
- err`.to_string()`

#### Usage Patterns

- converting `json::error` to `LLMError` for unexpected results
- used in JSON validation and parsing error handling

### `clore::net::detail::validate_prompt_output`

Declaration: `network/protocol.cppm:630`

Definition: `network/protocol.cppm:662`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::validate_prompt_output` implements a straightforward dispatch based on the `contract` parameter. It takes a `std::string_view content` and a `PromptOutputContract` value, then uses a `switch` statement to select the appropriate validation routine. For `PromptOutputContract::Json`, it delegates to `protocol::validate_json_output`; for `PromptOutputContract::Markdown`, it delegates to `protocol::validate_markdown_fragment_output`. When the contract is `PromptOutputContract::Unspecified`, the function immediately returns an unexpected `LLMError` indicating that the contract must be explicitly set. A fallthrough `default` case also returns an unexpected error for any unsupported contract value. No other validation or transformation occurs; the sole dependency is on the two protocol-level validation functions and the `LLMError` type.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content parameter
- contract parameter

#### Usage Patterns

- validating prompt output content according to a contract
- ensuring output matches expected format (JSON or Markdown)

### `clore::net::get_probed_capabilities`

Declaration: `network/protocol.cppm:126`

Definition: `network/protocol.cppm:725`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::get_probed_capabilities` implements a thread‑safe, lazily‑populated cache of `ProbedCapabilities` objects keyed by provider identifier. On invocation, it acquires a `static std::mutex`, then performs a lookup in a `static std::unordered_map<std::string, std::unique_ptr<ProbedCapabilities>>`. If an entry for the given `provider` string already exists, the function returns a reference to the stored `ProbedCapabilities`. Otherwise, it default‑constructs a new `ProbedCapabilities` via `std::make_unique`, inserts it into the map, and returns a reference to the newly created object. This design ensures that each provider is probed exactly once (the first request triggers a cache miss and populates the entry), and that subsequent calls, possibly from concurrent threads, safely retrieve the same mutable `ProbedCapabilities` reference. The dependency on `std::mutex` and `std::unordered_map` is internal; the caller receives a modifiable `ProbedCapabilities&` and is expected to update its fields (e.g., `supports_tools`, `supports_parallel_tool_calls`, `supports_json_schema`) after performing actual capability probing logic elsewhere.

#### Side Effects

- acquires a static mutex lock
- inserts a new entry into the static cache if the provider is not already cached

#### Reads From

- `provider` parameter

#### Writes To

- static cache map `cache` (via insertion or no-op if found)

#### Usage Patterns

- called to obtain a cached `ProbedCapabilities` reference for a given provider
- used to initialize or retrieve probed capabilities without repeating probes

### `clore::net::icontains`

Declaration: `network/protocol.cppm:758`

Definition: `network/protocol.cppm:758`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

Implementation: [Implementation](functions/icontains.md)

The function `clore::net::icontains` implements a case‑insensitive substring search using a straightforward brute‑force algorithm. It first performs an early‑exit check: if the length of `needle` exceeds the length of `haystack`, it returns `false` immediately. Otherwise, it iterates over each possible starting index `i` from `0` to `haystack.size() - needle.size()` inclusive. For each `i`, it sets a `match` flag to `true` and enters an inner loop over index `j` from `0` to `needle.size() - 1`. Inside the inner loop it compares `haystack[i + j]` and `needle[j]` after converting both to lowercase via `std::tolower(static_cast<unsigned char>(…))`. If any pair differs, `match` is set to `false` and the inner loop is broken. After the inner loop, if `match` remains `true` the function returns `true` immediately. If no starting position yields a full match, the function returns `false`. The only external dependency is `std::tolower` from the C++ standard library; no project‑specific utilities are used.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `haystack` parameter
- `needle` parameter

#### Usage Patterns

- Used by `is_feature_rejection_error` to check for substring patterns in error messages.

### `clore::net::is_feature_rejection_error`

Declaration: `network/protocol.cppm:131`

Definition: `network/protocol.cppm:778`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::is_feature_rejection_error` performs a simple substring‑matching heuristic to determine whether an error message from an API provider indicates that a requested feature (tool, response format, parameter, etc.) was rejected. Internally, it iterates over a static array of common rejection phrases — such as `"unsupported parameter"`, `"unknown field"`, `"does not support"`, and others — and checks each against the provided `error_message` using `clore::net::icontains`, which performs a case‑insensitive search. If any pattern is found, the function returns `true`; otherwise it returns `false`. This function has no dependencies on external state or complex data structures, relying only on the pattern list and the case‑insensitive containment check.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error_message` parameter
- static local array `patterns`

#### Usage Patterns

- Called to classify LLM provider errors as feature rejections during capability probing
- Used in error handling logic to decide whether to retry without certain features

### `clore::net::make_markdown_fragment_request`

Declaration: `network/protocol.cppm:99`

Definition: `network/protocol.cppm:140`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::make_markdown_fragment_request` constructs and returns a `PromptRequest` with the caller-supplied prompt text, a `response_format` of `std::nullopt`, and an `output_contract` set to `PromptOutputContract::Markdown`. The implementation is a straightforward initialisation of the `PromptRequest` aggregate — no branching, iteration, or error handling is performed. It simply packs the provided `std::string` into the `.prompt` field and defaults the output contract to a markdown fragment, leaving the response format unspecified so that the downstream completion pipeline will later infer or enforce the markdown requirement via `PromptOutputContract::Markdown`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `prompt` parameter (`std::string`)

#### Writes To

- returned `PromptRequest` object

#### Usage Patterns

- constructing a request for markdown fragment responses from an LLM

### `clore::net::parse_rejected_feature_from_error`

Declaration: `network/protocol.cppm:133`

Definition: `network/protocol.cppm:797`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::parse_rejected_feature_from_error` iterates over a hard‑coded array of pattern pairs, `field_patterns`, where each entry maps a keyword to a feature field name such as `"response_format"`, `"tool_choice"`, or `"tools"`. For each entry, it calls `clore::net::icontains` to perform a case‑insensitive substring search within the supplied `error_message`. The first match triggers an early return of the corresponding field name as a `std::string`; if no keyword is found, the function returns `std::nullopt`.

The control flow is purely linear and depends on the static `field_patterns` lookup table and the helper `clore::net::icontains`. The function’s sole purpose in the implementation is to scan provider‑returned error strings for known feature rejection patterns and report which specific feature was rejected, enabling the caller to adjust subsequent requests accordingly (e.g., by falling back to a simpler parameter set).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error_message` parameter
- static local array `field_patterns`
- function `clore::net::icontains`

#### Usage Patterns

- parsing LLM error responses to detect rejected feature
- extracting canonical feature name from error message

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:540`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function begins by validating that the response contains tool calls; if `response.message.tool_calls` is empty, it immediately returns an `LLMError`. It then builds a lookup map (`output_by_id`) from the provided `outputs` span, keyed by `ToolOutput::tool_call_id`, and enforces that no ID is empty and no duplicate `IDs` exist. After constructing the map, a merged message vector is created by copying the input `history` and appending an `AssistantToolCallMessage` whose `content` and `tool_calls` are taken directly from `response.message`.  

The core loop iterates over each `ToolCall` in `response.message.tool_calls`, looks up the corresponding output in the map by `tool_call.id`, and appends a `ToolResultMessage` containing that output. If any tool call ID is missing from the map, or if the final map size exceeds the number of tool calls (indicating unknown `IDs`), the function returns an `LLMError`. On success, it returns the merged message list. This algorithm depends on `std::unordered_map` for O(1) lookups and uses the `CompletionResponse`, `ToolOutput`, `ToolCall`, and related message types defined in the `clore::net` namespace.

#### Side Effects

- Allocates heap memory for the returned `std::vector<Message>` and an internal `std::unordered_map`
- Transfers ownership of the allocated vector to the caller

#### Reads From

- `history` (input span of const Message)
- `response` (input const `CompletionResponse`&)
- `outputs` (input span of const `ToolOutput`)

#### Writes To

- Returned `std::vector<Message>` (allocated and populated)
- Internal local `std::unordered_map<std::string, std::string>` (temporary)

#### Usage Patterns

- Used by callers to incorporate tool execution results back into the conversation history
- Called after a tool call response is received and tool outputs have been collected

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:475`

Definition: `network/protocol.cppm:588`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function first delegates to `text_from_response` to extract the raw text payload from the `CompletionResponse`. If that step fails, the function immediately returns a `std::unexpected` containing the moved `LLMError`. Otherwise, it attempts to deserialize the extracted text into the target type `T` using `kota::codec::json::from_json<T>`. If JSON parsing fails, it constructs an `LLMError` with a descriptive message that includes the underlying parser error via `parsed.error().to_string()`. On success, the parsed value is returned directly. The algorithm is linear and relies entirely on the two external components `text_from_response` and the JSON parsing library; no iteration or additional validation is performed within this function.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `response` parameter of type `CompletionResponse`
- `text_from_response` function
- `kota::codec::json::from_json<T>` function

#### Usage Patterns

- Used to parse structured LLM responses into a specified type.
- Combines text extraction and JSON parsing with error handling.

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:478`

Definition: `network/protocol.cppm:603`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function `clore::net::protocol::parse_tool_arguments` implements a two‑stage conversion from a raw JSON value to the caller’s type `T`. First, `call.arguments` is serialized to a string via `kota::codec::json::to_string`; if serialization fails, an `std::unexpected` containing an `LLMError` (including `call.name` and the error description) is returned. Second, the resulting string is deserialized into `T` using `kota::codec::json::from_json<T>`; on failure, an analogous `LLMError` is produced. On success, the parsed value is returned directly. The control flow is strictly linear with no branching beyond the two error checks, and all error propagation is channelled through `std::expected<T, LLMError>`. The algorithm depends entirely on the `kota::codec::json` library for both serialization and deserialization, and on `LLMError` as the error‑type carrier.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- call`.arguments`
- call`.name`

#### Usage Patterns

- Used to deserialize tool call arguments into a typed structure.
- Typically called when processing a `ToolCall` to extract its arguments as a specific type.

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:524`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function `clore::net::protocol::text_from_response` implements a linear validation pipeline over the incoming `CompletionResponse`. It first checks the `AssistantOutput`’s `refusal` field; if populated, it immediately returns an `LLMError` describing the refusal. Next, it inspects `tool_calls` on the same message — any non-empty collection causes an early exit with an error indicating the response contained tool calls rather than plain text. Only then does it verify the presence of `text`; if absent, a second error is produced. On passing all three guards, the function extracts and returns the unwrapped `text` value. No external dependencies beyond the response structure and `LLMError` are required, and the entire control flow consists of simple conditionals with no loops or recursion.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- response`.message``.refusal`
- response`.message``.tool_calls`
- response`.message``.text`

#### Usage Patterns

- safely extract text content from `CompletionResponse`
- handle refusal, tool calls, or missing text errors
- convert response to text or error

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:463`

Definition: `network/protocol.cppm:484`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function `clore::net::protocol::validate_json_output` first attempts to parse the input string `content` by invoking `kota::codec::json::parse<kota::codec::json::Value>`. If this parse operation fails, the function captures the error description from the returned `kota::codec::json::error` object and constructs an `LLMError` with a diagnostic message indicating that the output is not valid JSON. It then returns this error as a `std::unexpected` result. On successful parsing, the function returns an empty `std::expected<void, LLMError>`, indicating the output is valid JSON. The core dependency is the JSON parser provided by the `kota::codec::json` library, and the function’s control flow is a simple branch based on the parse success.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `content` parameter

#### Usage Patterns

- Validate JSON output before further processing
- Check if LLM output is valid JSON

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:465`

Definition: `network/protocol.cppm:493`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The implementation of `clore::net::protocol::validate_markdown_fragment_output` performs a sequence of structural and content checks on the supplied `content` string. It first rejects empty or whitespace-only strings by iterating over characters and testing with `std::isspace`. Next, it searches for the presence of an H1 heading marker (`# `) at the start or after a newline, and for any occurrence of a code fence (` ``\` `), returning an error if either is found. Finally, it attempts to parse the `content` as JSON by calling ``validate_json_output``; if that succeeds, the output is rejected because a markdown fragment was required. Each failure returns a specific ``LLMError`` message describing the violation.

The function depends only on `validate_json_output` (declared in the same namespace) and standard library utilities for character classification and substring search. No external JSON parsing library is invoked directly; the check is delegated entirely to the existing JSON validation routine.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- content parameter

#### Usage Patterns

- validation step in LLM response processing
- safeguard before markdown fragment interpretation

### `clore::net::sanitize_request_for_capabilities`

Declaration: `network/protocol.cppm:128`

Definition: `network/protocol.cppm:739`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function sequentially inspects each of the four capability flags from the supplied `ProbedCapabilities` object, using `load(std::memory_order_relaxed)` to read the atomic booleans. For each unsupported capability, it sanitizes the corresponding field in the passed-by-value `CompletionRequest`: if `supports_json_schema` is false, the `schema` sub‑field of `response_format` is set to `std::nullopt`; if `supports_tool_choice` is false, `tool_choice` is cleared; if `supports_parallel_tool_calls` is false, `parallel_tool_calls` is cleared; and if `supports_tools` is false, the entire `tools` vector is cleared. The modified `CompletionRequest` is then returned by value. The algorithm has no looping or recursion and depends solely on the atomic flag values and the structure of `CompletionRequest` (including `ResponseFormat` and `FunctionToolDefinition`).

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `ProbedCapabilities``.supports_json_schema`
- `ProbedCapabilities``.supports_tool_choice`
- `ProbedCapabilities``.supports_parallel_tool_calls`
- `ProbedCapabilities``.supports_tools`
- `CompletionRequest``.response_format`
- `CompletionRequest``.tool_choice`
- `CompletionRequest``.parallel_tool_calls`
- `CompletionRequest``.tools`

#### Writes To

- local copy of `CompletionRequest` fields: `response_format`->schema, `tool_choice`, `parallel_tool_calls`, tools

#### Usage Patterns

- called before making a completion request to ensure requested features match provider capabilities

## Internal Structure

The `protocol` module defines the data types and functions that implement the LLM interaction protocol for the `clore::net` library. It is responsible for representing completion requests and responses, tool definitions, message variants (system, user, assistant, tool), output contracts, and capabilities probing. The module is decomposed into a public interface exposing the primary structs (`CompletionRequest`, `CompletionResponse`, `PromptRequest`, `ToolCall`, `FunctionToolDefinition`, etc.) and a `detail` namespace that contains internal JSON‑manipulation utilities such as `ObjectView`, `ArrayView`, type‑expectation helpers (`expect_array`, `expect_object`, `expect_string`), clone functions, serialization, UTF‑8 normalization, and synchronous asynchronous task execution helpers.

Internally, the module imports the `http` module (for network communication), the `support` module (for foundational utilities like file I/O and hashing), and standard library facilities. The implementation is layered: the `detail` namespace provides low‑level JSON building blocks that higher‑level public functions (e.g., `protocol::parse_tool_arguments`, `protocol::append_tool_outputs`, `sanitize_request_for_capabilities`) call to construct, parse, validate, and transform protocol messages. All protocol types heavily leverage the `kota::codec::json` library for JSON representation, and the module uses the `LLMError` type from `http` for error handling across the protocol layer.

## Related Pages

- [Module http](../http/index.md)
- [Module support](../support/index.md)

