---
title: 'Module protocol'
description: 'The protocol module defines the core data structures and operations for interacting with LLM APIs over the network. It owns the public types that model the full request–response lifecycle: conversation messages (SystemMessage, UserMessage, AssistantMessage, ToolResultMessage, AssistantToolCallMessage), tool definitions and invocations (ToolCall, ToolOutput, FunctionToolDefinition, ToolChoice variants), structured completion requests and responses (CompletionRequest, CompletionResponse, PromptRequest, ResponseFormat), and output contract specifications (PromptOutputContract). The module also provides a capability-probing subsystem (ProbedCapabilities, get_probed_capabilities) that detects which features a given endpoint supports, together with a sanitize_request_for_capabilities function to adjust requests accordingly.'
layout: doc
template: doc
---

# Module `protocol`

## Summary

The `protocol` module defines the core data structures and operations for interacting with LLM `APIs` over the network. It owns the public types that model the full request–response lifecycle: conversation messages (`SystemMessage`, `UserMessage`, `AssistantMessage`, `ToolResultMessage`, `AssistantToolCallMessage`), tool definitions and invocations (`ToolCall`, `ToolOutput`, `FunctionToolDefinition`, `ToolChoice` variants), structured completion requests and responses (`CompletionRequest`, `CompletionResponse`, `PromptRequest`, `ResponseFormat`), and output contract specifications (`PromptOutputContract`). The module also provides a capability-probing subsystem (`ProbedCapabilities`, `get_probed_capabilities`) that detects which features a given endpoint supports, together with a `sanitize_request_for_capabilities` function to adjust requests accordingly.

In addition to data types, the module exposes a set of public utility functions for JSON parsing, validation, and serialization (e.g., `expect_array`, `expect_object`, `parse_json_value`, `clone_object`, `serialize_value_to_string`), as well as higher‑level protocol helpers for constructing markdown or JSON requests, extracting text from responses, validating output format, and managing tool call flows. These elements form the caller‑facing contract of the networking layer, built on top of the `http` module for HTTP transport and the `support` module for foundational infrastructure.

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

The struct `clore::net::AssistantMessage` is implemented as a lightweight aggregate type with a single public data member `content` of type `std::string`. No constructors, destructors, or member functions are user-declared; the struct relies on default compiler‑generated special members. The sole invariant is that `content` holds the assistant’s message payload as a contiguous sequence of characters, and the struct imposes no additional constraints beyond those inherent to `std::string`. Consequently, all operations—construction, copy, move, assignment, and comparison (if any)—are trivial and deferred to the underlying string type. No extra validation or transformation is performed at the struct level, making it a plain data object suitable for serialization and network transport.

#### Invariants

- `content` holds a textual message from an assistant
- No explicit invariants are declared or implied by the evidence

#### Key Members

- `content`

#### Usage Patterns

- No usage patterns are documented in the evidence

### `clore::net::AssistantOutput`

Declaration: `network/protocol.cppm:101`

Definition: `network/protocol.cppm:101`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::AssistantOutput` is an aggregate type that holds the possible outputs from an assistant invocation. It contains three independent fields: `text` and `refusal`, both of type `std::optional<std::string>`, and `tool_calls` of type `std::vector<ToolCall>`. No constructors, destructors, or member functions are defined, so the struct is a plain data container. The fields are not constrained by any runtime invariants enforced by the struct itself; any combination of set or unset optionals and an empty or non‑empty vector is legal.

Despite the lack of explicit invariants, the design suggests that typically only one kind of output is meaningful at a time: either a textual response (`text`), a refusal (`refusal`), or a set of tool invocation requests (`tool_calls`). Implementation code that populates or reads from `AssistantOutput` is expected to enforce consistency at the call site, for example by ensuring that `text` and `refusal` are not both engaged simultaneously. The struct simply serves as a lightweight, type‑safe union for these mutually exclusive (in practice) response categories.

#### Invariants

- text is optional and may be nullopt
- refusal is optional and may be nullopt
- `tool_calls` may be an empty vector

#### Key Members

- text
- refusal
- `tool_calls`

#### Usage Patterns

- Used to capture the result of an assistant interaction
- May be serialized or deserialized in network communication
- Consumed by code expecting either text, refusal, or a list of tool calls

### `clore::net::AssistantToolCallMessage`

Declaration: `network/protocol.cppm:35`

Definition: `network/protocol.cppm:35`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::AssistantToolCallMessage` aggregates the two possible components of an assistant response after a tool‑invocation round: an optional `content` string (set when the assistant also produces a text reply) and a required `tool_calls` vector that holds the sequence of `ToolCall` objects. The internal invariant is that the message always carries at least one meaningful piece of data – either `content` has a value, or `tool_calls` is non‑empty, or both – and the representation supports straightforward serialization and deserialization by relying on the natural emptiness of `std::optional` and `std::vector`. The implementation simply exposes these two fields; no additional logic or validation is performed within the struct itself.

#### Invariants

- `content` is optional and may be `std::nullopt`.
- `tool_calls` is default-constructible to an empty vector.
- The struct has no user-defined constructors or special member functions, so it is an aggregate.

#### Key Members

- `content`
- `tool_calls`

#### Usage Patterns

- Used as a message format in the `clore::net` protocol to represent assistant responses with optional text and tool invocations.
- Aggregate initialization is likely used to construct instances.
- Consumed by serialization or network functions that process assistant messages.

### `clore::net::CompletionRequest`

Declaration: `network/protocol.cppm:77`

Definition: `network/protocol.cppm:77`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::CompletionRequest` is defined as a plain aggregate with all public fields. Default member initializers set `model` to an empty string and `messages` to an empty vector; every `std::optional` field (`response_format`, `tool_choice`, `parallel_tool_calls`) defaults to `std::nullopt`, and `tools` is an empty vector. There are no user-declared constructors, destructors, or member functions — the struct relies on compiler-generated defaults. Because the fields are independent, no invariant is enforced at the struct level; a valid request logically requires `model` and `messages` to be non-empty, and if `tools` is non-empty then `tool_choice` and `parallel_tool_calls` modify tool-calling behavior. The `response_format` field, when present, specifies a structured output schema.

#### Invariants

- All fields have default initializers
- `model` is an empty `std::string` by default
- `messages` and `tools` are empty vectors by default
- Optional fields (`response_format`, `tool_choice`, `parallel_tool_calls`) are `std::nullopt` by default

#### Key Members

- `model`
- `messages`
- `response_format`
- `tools`
- `tool_choice`
- `parallel_tool_calls`

#### Usage Patterns

- Constructed with aggregate initialization for each request
- Likely serialized to JSON for network transmission to a completion endpoint
- Populated programmatically from user input or higher‑level abstractions before sending

### `clore::net::CompletionResponse`

Declaration: `network/protocol.cppm:107`

Definition: `network/protocol.cppm:107`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::CompletionResponse` is a plain aggregate that bundles the core fields of a completion API response into a single object. Its members—`id`, `model`, `message`, and `raw_json`—are all public and directly accessible, with no special access control or invariants enforced by the struct itself. The `message` field holds the parsed assistant output (of type `AssistantOutput`), while `raw_json` stores the original JSON payload, preserving the full response for debugging or re‑serialization. No constructors, assignment `operator`s, or member functions are defined, so the struct relies on default memberwise initialization and copy/move semantics; the responsibility for populating these fields consistently (e.g., ensuring `raw_json` and the parsed members remain in sync) lies solely with the code that constructs a `CompletionResponse`.

#### Invariants

- Each field is populated after construction as a complete representation of a completion response.
- `raw_json` holds the original JSON response string.

#### Key Members

- `id`
- `model`
- `message`
- `raw_json`

#### Usage Patterns

- Used as the return type of completion API calls, allowing callers to access structured response data.
- Fields are read directly after a request completes.

### `clore::net::ForcedFunctionToolChoice`

Declaration: `network/protocol.cppm:70`

Definition: `network/protocol.cppm:70`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ForcedFunctionToolChoice` is implemented as a simple aggregate containing a single data member `name` of type `std::string`. No user‑defined constructors, assignment `operator`s, or destructors are provided; the compiler‑generated special member functions are used, making the type trivially copyable and movable. There are no internal invariants enforced by the struct — any string value, including empty, is accepted. The implementation does not perform validation or normalization of the tool name, leaving all semantic constraints to the code that constructs or uses an instance. This minimal design keeps the struct lightweight and purely a carrier of the forced function identifier.

#### Invariants

- The struct imposes no constraints on the content of `name` beyond those of `std::string`.

#### Key Members

- name

#### Usage Patterns

- Used to represent a forced tool choice by specifying the tool's name.

### `clore::net::FunctionToolDefinition`

Declaration: `network/protocol.cppm:57`

Definition: `network/protocol.cppm:57`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::FunctionToolDefinition` is an aggregate composed of four direct data members: `name` and `description` of type `std::string`, `parameters` of type `kota::codec::json::Object`, and a `bool` named `strict`. The member `strict` is default-initialized to `true`, while the remaining members are default-initialized by their respective types (empty strings and an empty JSON object). The design intentionally provides no user‑defined constructors, assignment `operator`s, or virtual functions, making it a plain‑old‑data (POD‑like) type suitable for direct aggregate initialization and trivial copy/move semantics.

The structural invariant is that `parameters` must always contain a valid JSON object once the definition is used; no other invariants are enforced by the struct itself. All member accesses are direct and unguarded, relying on external validation to ensure `name` and `description` are non‑empty and `parameters` conforms to the expected schema before the definition is passed to lower layers. The default `strict = true` reflects the typical requirement that the tool’s parameter schema be enforced exactly, but this can be overridden per instance.

#### Invariants

- `strict` defaults to `true`
- `name` and `description` are `std::string`
- `parameters` is a `kota::codec::json::Object`

#### Key Members

- `name`
- `description`
- `strict`
- `parameters`

#### Usage Patterns

- Used to specify a function tool definition in network protocol contexts
- `strict` flag controls enforcement of parameter validation

### `clore::net::Message`

Declaration: `network/protocol.cppm:45`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::Message` is a `std::variant` discriminated union that serves as the central message type for the network protocol layer. It aggregates five distinct message variants—`SystemMessage`, `UserMessage`, `AssistantMessage`, `AssistantToolCallMessage`, and `ToolResultMessage`—into a single homogeneous type that can be passed through serialization pipelines and dispatched via `std::visit`. The ordering of alternatives follows the typical conversational flow: system context, user input, assistant responses (both plain and tool-augmented), and finally tool execution results.

The type imposes no additional invariants beyond those guaranteed by `std::variant` itself; validity is determined entirely by the active alternative's own internal constraints. This design choice delegates responsibility for semantic correctness to the constituent message types while allowing the protocol implementation to treat all messages uniformly. Member implementations are implicitly generated by the compiler, relying on `std::variant`'s value semantics and the rule of zero for the underlying alternatives.

#### Invariants

- holds exactly one of the listed alternative types
- alternative set is fixed to the five message classes
- value-semantic via `std::variant`

#### Key Members

- alternative `SystemMessage`
- alternative `UserMessage`
- alternative `AssistantMessage`
- alternative `AssistantToolCallMessage`
- alternative `ToolResultMessage`
- underlying `std::variant`

#### Usage Patterns

- dispatched via `std::visit` or alternative inspection
- used as a unified message type in protocol handling
- passed across the `clore::net` networking layer

### `clore::net::ProbedCapabilities`

Declaration: `network/protocol.cppm:119`

Definition: `network/protocol.cppm:119`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::ProbedCapabilities` is a plain aggregate of four `std::atomic<bool>` members, each defaulting to `true`. This design provides lock‑free, thread‑safe access to each probed capability flag without introducing synchronisation overhead. Because the struct is trivially constructible and has no custom constructors, assignment, or other special member functions, its invariants are minimal: all members begin optimistically enabled, and external code may flip any flag to `false` (or back) using atomic operations. No additional validation or coordination is required at the struct level; the atomicity guarantees that concurrent reads and writes from different probing or monitoring threads observe a consistent per‑flag state, making this type suitable for accumulating capabilities discovered during network handshake or capability‑negotiation phases.

#### Invariants

- All members are `std::atomic<bool>` and safe for concurrent reads and writes.
- All members start with a value of `true` by default, implying assumed support until explicitly changed.

#### Key Members

- `clore::net::ProbedCapabilities::supports_json_schema`
- `clore::net::ProbedCapabilities::supports_tool_choice`
- `clore::net::ProbedCapabilities::supports_parallel_tool_calls`
- `clore::net::ProbedCapabilities::supports_tools`

### `clore::net::PromptOutputContract`

Declaration: `network/protocol.cppm:86`

Definition: `network/protocol.cppm:86`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The enumeration `clore::net::PromptOutputContract` is implemented as a scoped enum with an underlying type of `std::uint8_t`. This compact representation ensures that values occupy a single byte, suitable for network or serialization contexts. The three enumerators—`Unspecified`, `Json`, and `Markdown`—are defined in that order, establishing an implicit ordering that can be relied upon internally. A key invariant is that `Unspecified` always occupies the zero value, serving as a default or unknown state, while the remaining values are positive. No additional methods or `operator`s are defined; the enum is used directly as a discriminant for selecting output formatting logic. The base type choice also guarantees that the enum can be safely stored in protocol message fields requiring fixed-width integers.

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

The struct `clore::net::PromptRequest` is a plain aggregate with default member initializers, designed to hold the parameters for a prompt request. Its invariants are established entirely through these defaults: `prompt` is an empty string, `response_format` and `tool_choice` are `std::nullopt`, and `output_contract` is `PromptOutputContract::Unspecified`. No custom constructors, assignment `operator`s, or destructors are provided, so the struct relies on compiler-generated special member functions. The absence of validation logic means that any invariant enforcement must occur at the point of use or via higher‑level protocol handling, not within the struct itself.

#### Invariants

- `prompt` may be empty
- `output_contract` always has a value
- `response_format` and `tool_choice` are optional and may be absent

#### Key Members

- `prompt`
- `response_format`
- `tool_choice`
- `output_contract`

#### Usage Patterns

- Used as a request payload in network communication for submitting a prompt to a service
- Default values simplify creation when optional fields are not needed

### `clore::net::ResponseFormat`

Declaration: `network/protocol.cppm:51`

Definition: `network/protocol.cppm:51`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ResponseFormat` is a plain data aggregate that bundles three public fields: `name` of type `std::string`, `schema` of type `std::optional<kota::codec::json::Object>`, and `strict` of type `bool` defaulting to `true`. No constructors, assignment `operator`s, or member functions are defined; the struct relies entirely on the compiler-generated defaults. The `strict` flag, when `true`, indicates that the *schema* descriptor (if present) must be fully enforced; the optionality of `schema` itself allows callers to specify either a concrete JSON object schema or leave the format description absent. The struct does not enforce any cross-field invariants—the relationship between the three members is left to the client code, making `ResponseFormat` a simple carrier of configuration values.

#### Invariants

- `strict` defaults to `true`
- `schema` is optional and may be `std::nullopt`
- All fields are public and freely assignable

#### Key Members

- `name`
- `schema`
- `strict`

#### Usage Patterns

- Used to configure response format expectations in network protocol definitions
- Typically initialized with a name, optional schema, and strictness setting before being passed to request functions

### `clore::net::SystemMessage`

Declaration: `network/protocol.cppm:16`

Definition: `network/protocol.cppm:16`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::SystemMessage` is an implementation‑level data carrier, defined with a single `std::string` member `content`. No user‑defined constructors, assignment `operator`s, or destructors are provided, so the compiler synthesizes the default, copy, and move operations, making the struct trivially copyable and movable. The struct imposes no invariants beyond those inherent to `std::string` — `content` may be empty or hold any valid UTF‑8 (or arbitrary) byte sequence. Because it serves as a plain aggregate, construction and assignment require only a `std::string` argument or direct member initialization, and the sole storage is a dynamically‑managed string buffer. No additional member functions or metadata are introduced, keeping the footprint minimal.

#### Invariants

- The `content` member always holds a valid `std::string` (default-constructible or assigned).
- No additional invariants or constraints are enforced by the struct definition.

#### Key Members

- `content` (`std::string`): the message payload

#### Usage Patterns

- Used to encapsulate a string payload for system-level messages within the networking protocol.
- Likely instantiated and passed through network send/receive functions or protocol handlers.

### `clore::net::ToolCall`

Declaration: `network/protocol.cppm:24`

Definition: `network/protocol.cppm:24`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolCall` is a simple aggregate struct that holds the parsed and raw representations of a single tool invocation. It stores the call's `id`, `name`, and `arguments_json` as `std::string` values, alongside a pre-parsed `kota::codec::json::Value` in `arguments`. The invariant maintained by this design is that `arguments_json` and `arguments` represent the same underlying data: `arguments` is the deserialized form of the `arguments_json` string, and both fields must be kept in sync by any code that constructs or modifies a `ToolCall` instance. No custom constructors, accessors, or validation logic are present; the struct relies on direct member access and external code to uphold this consistency.

#### Invariants

- `id` is a unique identifier for the tool call
- `name` identifies the tool to be invoked
- `arguments_json` is a JSON-encoded string of the arguments
- `arguments` is a parsed JSON value corresponding to `arguments_json`

#### Key Members

- `id`
- `name`
- `arguments_json`
- `arguments`

#### Usage Patterns

- Used in network protocol messages to convey tool invocation requests
- The `arguments_json` field may be used for serialization, while `arguments` provides structured access

### `clore::net::ToolChoice`

Declaration: `network/protocol.cppm:74`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoice` is implemented as a `std::variant` over four alternative tag types: `ToolChoiceAuto`, `ToolChoiceRequired`, `ToolChoiceNone`, and `ForcedFunctionToolChoice`. The variant ordering places the discriminated, value-carrying alternative `ForcedFunctionToolChoice` last, while the leading three serve as lightweight sentinel modes; this guarantees that a default-constructed `ToolChoice` holds `ToolChoiceAuto` (index `0`).

Because it is a pure type alias, no member implementations exist—dispatch on the active alternative is expected to go through `std::visit`, `std::get`, or `std::holds_alternative` at the call site, and the invariant that exactly one of the four states is active is maintained entirely by `std::variant` itself.

#### Invariants

- holds exactly one of the four alternative types
- alternatives limited to `ToolChoiceAuto`, `ToolChoiceRequired`, `ToolChoiceNone`, `ForcedFunctionToolChoice`

#### Key Members

- alternative `ToolChoiceAuto`
- alternative `ToolChoiceRequired`
- alternative `ToolChoiceNone`
- alternative `ForcedFunctionToolChoice`
- underlying `std::variant`

#### Usage Patterns

- used to express a tool selection mode in protocol messages
- inspected via `std::variant` visitation or alternative access

### `clore::net::ToolChoiceAuto`

Declaration: `network/protocol.cppm:64`

Definition: `network/protocol.cppm:64`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The structure `clore::net::ToolChoiceAuto` is defined as an empty class, containing no data members, constructors, or member functions. It serves as a type‑level tag or marker to represent the concept of automatic tool choice selection within the networking layer. Because it holds no internal state, the struct imposes no invariants and requires no special construction or destruction logic. Its value lies solely in its type identity, enabling overload resolution or template dispatch based on the presence of `clore::net::ToolChoiceAuto` as a distinct type.

#### Invariants

- The struct has no invariants because it contains no data.

#### Usage Patterns

- No usage patterns are evident from the provided source snippets.

### `clore::net::ToolChoiceNone`

Declaration: `network/protocol.cppm:68`

Definition: `network/protocol.cppm:68`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolChoiceNone` is implemented as an empty class type with no data members, member functions, or base classes. This minimal definition means that instances require no storage and carry no runtime state, so the default constructor, destructor, and copy/move operations are trivial. There are no invariants to maintain, and the type imposes no overhead beyond its own type identity. The emptiness of the structure is its defining characteristic, providing a degenerate value that can be used in type-based dispatch or variant alternatives without additional complexity.

#### Invariants

- Empty struct with no state.
- Trivially default constructible and destructible.
- No runtime overhead.

#### Key Members

- None (no members or nested types).

#### Usage Patterns

- Used as a tag or discriminator in variants or type‑dispatching.
- Likely passed as a template argument to indicate a disabled or absent tool choice.
- Comparison or default construction of `clore::net::ToolChoiceNone` is used to represent the 'none' case.

### `clore::net::ToolChoiceRequired`

Declaration: `network/protocol.cppm:66`

Definition: `network/protocol.cppm:66`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

`clore::net::ToolChoiceRequired` is an empty tag type with no data members or member functions. Its internal structure is trivial, consisting only of the declared struct body. The type imposes no invariants, as there is no state to maintain. This marker type is likely used for type-based dispatch, policy selection, or compile-time differentiation where the presence of the type itself conveys semantic intent rather than behavior or data.

#### Invariants

- Empty struct, no state
- Trivially copyable
- Default-constructible

#### Usage Patterns

- Used as a tag type for overloading or template specialization
- Passed as a parameter to indicate a required tool choice
- May appear as a default template argument or function parameter type

### `clore::net::ToolOutput`

Declaration: `network/protocol.cppm:114`

Definition: `network/protocol.cppm:114`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolOutput` is a plain aggregate type that encapsulates the result of a tool call interaction. Its two `std::string` members, `tool_call_id` and `output`, store the identifier of the originating tool call and the text output produced by the tool, respectively. No special invariants or custom member implementations are present; the type relies solely on the compiler-generated default constructor and copy/move operations. The struct serves as a lightweight data transfer object, intended to be constructed inline or assigned directly.

#### Invariants

- aggregate of two `std::string` members
- resides in the `clore::net` namespace
- both members are publicly accessible

#### Key Members

- `tool_call_id` identifying the originating tool call
- `output` holding the tool's textual result

#### Usage Patterns

- used as a payload in network protocol communication
- constructed and passed by value as a simple data record

### `clore::net::ToolResultMessage`

Declaration: `network/protocol.cppm:40`

Definition: `network/protocol.cppm:40`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The struct `clore::net::ToolResultMessage` is a straightforward aggregate type holding two `std::string` members: `tool_call_id` and `content`. Its internal structure relies on the default copy, move, and assignment operations provided by the compiler, and no custom constructors, destructors, or member functions are declared. The invariants are those guaranteed by the string type itself – each field must remain in a valid state (e.g., non‑null pointer, valid size) – but no further constraints are enforced by the struct. As a plain data container, its implementation is trivial; all member functions are implicitly defaulted, and the struct serves solely to bundle the identifier of the tool call with the resulting content for transmission over the network.

#### Invariants

- No documented invariants beyond the standard behavior of `std::string`.
- Members are independent; no constraints between `tool_call_id` and `content` are specified.

#### Key Members

- `tool_call_id`
- content

#### Usage Patterns

- No usage patterns are documented in the provided evidence; the struct is defined but its specific usage in the codebase is not shown.

### `clore::net::UserMessage`

Declaration: `network/protocol.cppm:20`

Definition: `network/protocol.cppm:20`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The implementation of `clore::net::UserMessage` consists of a single data member: `content`, an `std::string` that stores the textual payload of the message. The struct imposes no additional invariants beyond those inherent to `std::string` itself; the `content` field can be empty or hold any valid UTF-8 or binary data. There are no custom constructors, assignment `operator`s, or member functions, so the class relies entirely on the implicitly generated special member functions from `std::string`. This minimal design keeps the message lightweight and straightforward, with the `content` string serving as the sole carrier of user‑supplied data throughout the networking layer.

#### Invariants

- `content` is a valid `std::string`
- No implicit constraints on length or content

#### Key Members

- `std::string content`

#### Usage Patterns

- Instances are constructed with a string to represent a user message
- Likely serialized or transmitted over the network
- May be parsed from incoming network data

### `clore::net::detail::ArrayView`

Declaration: `network/protocol.cppm:178`

Definition: `network/protocol.cppm:178`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The struct `clore::net::detail::ArrayView` is a lightweight non-owning view over a `const kota::codec::json::Array`. Its internal structure consists of a single pointer data member `value` initialized to `nullptr`. The critical invariant is that `value` must point to a valid `kota::codec::json::Array` before any member is called; using a default-constructed view leads to undefined behavior because all forwarding members directly dereference `value` without a null check.

All accessors delegate to the underlying array: `empty()`, `size()`, `begin()`, and `end()` forward to the corresponding `kota::codec::json::Array` members. The subscript `operator[]` returns a reference to the element at the given index, relying on the array’s bounds checking. The dereference `operator`s `operator->` and `operator*` return the raw pointer or a reference to the array respectively, enabling natural usage as a wrapped pointer.

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

The implementation of `clore::net::detail::ObjectView` is a lightweight, non-owning wrapper centered on a single raw pointer member `value` to a `const kota::codec::json::Object`. The core invariant is that `value` is expected to point to a valid object when any member function is called; default initialization to `nullptr` provides no safety against null dereference, so callers must ensure a valid object is assigned before use.  

All member implementations delegate directly to the underlying JSON object. The `get` method performs a key lookup and returns `std::optional<kota::codec::json::Cursor>`; `begin` and `end` simply forward to the object’s iteration interface; and the dereference `operator`s `operator->` and `operator*` expose the raw pointer and reference respectively, enabling idiomatic access without additional abstraction.

#### Invariants

- The `value` pointer may be null by default.
- When `value` is non‑null, the pointed‑to object must outlive the view.
- No lifetime management is performed by the view.

#### Key Members

- `value` – pointer to the underlying JSON object.
- `get(std::string_view)` – retrieves an optional cursor for a given key.
- `begin()` / `end()` – iterator access to the object’s key‑value pairs.
- `operator->()` / `operator*()` – dereference to the underlying object.

#### Usage Patterns

- Returned from functions that provide read‑only access to a JSON object without transferring ownership.
- Used in protocol‑level parsing to read structured message fields.
- Often passed by value or const reference to functions that need a temporary, non‑owning handle.

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

The function `clore::net::detail::clone_array` performs a shallow copy of the underlying JSON array held by an `ArrayView` instance. It accesses the `source.value` member (a pointer to a `kota::codec::json::Array`) and directly constructs a new `json::Array` from the dereferenced object via copy construction. The second parameter, a `std::string_view` intended for error context, is unused. The control flow has no branching; it simply returns the copy wrapped in a `std::expected` success value. This implementation depends on the copy semantics of `kota::codec::json::Array` and assumes the `ArrayView` is non‑null—no validation or error handling is applied.

#### Side Effects

- Allocates a new `json::Array` object via copy construction

#### Reads From

- `ArrayView source` (specifically `source.value`)
- the underlying `json::Array` object

#### Writes To

- The dynamically allocated `json::Array` returned by the function

#### Usage Patterns

- Used to clone JSON array data structures
- Likely called during deep‑copy operations on parsed JSON objects

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:262`

Definition: `network/protocol.cppm:446`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::clone_object` performs a straightforward deep copy of a `json::Object`. It constructs a new `json::Object` by passing the `source` object to the copy constructor, which duplicates all internal key‑value pairs. The second parameter (`std::string_view`) is provided for consistency with other cloning functions but is unused in this implementation; error reporting is not needed because the copy operation is infallible. The function depends solely on the `json::Object` type and its copy semantics, requiring no iteration, validation, or transformation of the contained data.

#### Side Effects

- allocates a new `json::Object` via copy construction

#### Reads From

- `source` parameter (const `json::Object&`)

#### Writes To

- returned `std::expected<json::Object, LLMError>` value (new `json::Object`)

#### Usage Patterns

- deep copy of a JSON object
- cloning before mutation
- preserving original in serialization pipelines

### `clore::net::detail::clone_object`

Declaration: `network/protocol.cppm:265`

Definition: `network/protocol.cppm:451`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::clone_object` accepts a `clore::net::detail::ObjectView` and an unused `std::string_view` (likely a context for error messages), and returns a copy of the underlying JSON object. Its implementation is trivial: it dereferences the `ObjectView` to obtain a const reference to a `kota::codec::json::Object` and invokes that object’s copy constructor. There is no iteration, validation, or error handling; the function directly relies on the copy semantics of `kota::codec::json::Object`. This simple forwarding is used wherever a standalone, mutable copy of an object viewed through `ObjectView` is needed, decoupling the caller from the original source’s lifetime.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `source.value` which points to the `json::Object` being copied

#### Usage Patterns

- Copying a JSON object view for further processing
- Cloning an object when deep copy is required in protocol serialization or validation

### `clore::net::detail::clone_value`

Declaration: `network/protocol.cppm:271`

Definition: `network/protocol.cppm:455`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::clone_value` accepts a const reference to a `json::Value` (`source`) and a `std::string_view` (`context`). Its internal control flow is minimal: it silences the unused `context` parameter via `static_cast` and then returns a new `json::Value` constructed as a copy of `source` using the copy constructor. The algorithm relies entirely on the copy semantics of the `json::Value` type, which performs a deep clone of the underlying JSON data. No additional validation, transformation, or error handling is performed; the function delegates the cloning operation to the value’s copy mechanism, making it a trivial passthrough for duplicating JSON trees.

#### Side Effects

- Allocates memory for the copy of the JSON value

#### Reads From

- source parameter (`const json::Value& source`)

#### Writes To

- The returned `json::Value` (copy constructed from `source`)

#### Usage Patterns

- Creates an independent copy of a JSON value
- Used when a deep, mutable clone of a JSON value is required

### `clore::net::detail::excerpt_for_error`

Declaration: `network/protocol.cppm:223`

Definition: `network/protocol.cppm:316`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::excerpt_for_error` takes a `std::string_view body` and returns a `std::string` containing a truncated version of the input. Internally it defines a constant `kMaxBytes` set to 200 and delegates entirely to `clore::support::truncate_utf8`, passing `body` and the byte limit. This single-step implementation ensures the returned excerpt is no longer than 200 bytes while respecting UTF-8 character boundaries, avoiding malformed sequences. The function depends solely on the `truncate_utf8` utility from the `clore::support` module.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `body` (parameter)

#### Usage Patterns

- Creating a safe excerpt of a response body for inclusion in error messages

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:253`

Definition: `network/protocol.cppm:415`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function converts a `json::Cursor` into an `ArrayView` by calling `get_array()` on the cursor. If the result is a null pointer, it immediately returns an `std::unexpected` containing a `LLMError` whose message is constructed via `std::format` using the supplied `context` string. On success, it returns an `ArrayView` initialized with the pointer as its `value` member. This check ensures the caller receives a meaningful error location when the JSON value at the cursor is not an array, and the returned `ArrayView` provides a typed, non‑owning interface over the underlying `kota::codec::json::Array`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor` parameter `value` via its `get_array()` method
- `std::string_view` parameter `context`

#### Usage Patterns

- Converting a JSON cursor to an `ArrayView` for array traversal
- Validating that a JSON value is an array before further processing
- Used in conjunction with `clone_array` and other array operations

### `clore::net::detail::expect_array`

Declaration: `network/protocol.cppm:250`

Definition: `network/protocol.cppm:406`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_array` attempts to extract an array from a `json::Value` by calling `get_array()`. If the pointer is `nullptr`, it returns an unexpected `LLMError` containing a formatted message that includes the `context` string, indicating the expected array was not present. On success, it constructs and returns an `ArrayView` value initialized with the retrieved array pointer. This straightforward validation-and-wrap pattern depends on the `json::Value::get_array()` method and the `ArrayView` structure, which stores a pointer to the underlying JSON array and is used throughout the parsing layer for safe, nullable array access.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (`const json::Value&`)
- `context` parameter (`std::string_view`)

#### Usage Patterns

- Checking if a JSON value is an array
- Validating JSON type and returning an `ArrayView`
- Error reporting with a descriptive context string

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:244`

Definition: `network/protocol.cppm:388`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_object` validates that a given `json::Value` is a JSON object and, if so, returns an `ObjectView` that provides a safe, structured view over the underlying `kota::codec::json::Object`. Internally, it calls `value.get_object()` to retrieve a pointer to the object; if the pointer is null (i.e., the value is not an object), it returns an `std::unexpected` containing an `LLMError` with a formatted message that includes the `context` string for error context. Otherwise, it constructs an `ObjectView` by initializing its `.value` member with the non‑null object pointer. This function depends on the `json::Value` API, the `ObjectView` struct (defined in `clore::net::detail`), and the `LLMError` type for error reporting.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter
- `context` parameter
- `value.get_object()` method

#### Usage Patterns

- validating JSON object type
- converting JSON value to `ObjectView`
- reporting error for non-object values

### `clore::net::detail::expect_object`

Declaration: `network/protocol.cppm:247`

Definition: `network/protocol.cppm:397`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function extracts the underlying JSON object from a `json::Cursor` by calling `value.get_object()`. If the cursor does not point to a JSON object—that is, `get_object` returns a null pointer—it returns `std::unexpected` containing an `LLMError` with a descriptive message formed from the `context` string. On success, it returns an `ObjectView` initialized with the obtained object pointer. This helper depends on `kota::codec::json` types and the locally defined `ObjectView` struct, and its error path consistently uses the caller-provided `context` to identify the location of the malformed input.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `value` parameter (`json::Cursor`)
- `context` parameter (`std::string_view`)

#### Usage Patterns

- validating that a JSON cursor is an object before further processing
- obtaining an `ObjectView` from a cursor

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:256`

Definition: `network/protocol.cppm:424`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::expect_string` attempts to extract a JSON string from the given `json::Value`. It delegates to `value.get_string()`; if that call returns `std::nullopt` (meaning the value is not a string), the function constructs an `LLMError` with a descriptive message prefixed by the `context` parameter and returns it as an unexpected result. Otherwise it dereferences the returned `std::string_view` and returns that value. This is a straightforward validation pattern: a single condition check followed by either an early error return or a successful extraction.

The function relies on the `get_string` method of `kota::codec::json::Value` to determine the type of the underlying JSON node. It also depends on `std::format` to build the error string and uses the `LLMError` type (likely a `std::unexpected<LLMError>`) for error reporting. It follows the same structural idiom as sibling helpers `clore::net::detail::expect_array` and `clore::net::detail::expect_object`, all of which verify that a JSON element matches an expected type and produce a uniform error format given a context label.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const json::Value &value`
- `std::string_view context`

#### Usage Patterns

- validate that a JSON value is a string
- extract string content from a JSON node
- produce informative error messages for non-string JSON values

### `clore::net::detail::expect_string`

Declaration: `network/protocol.cppm:259`

Definition: `network/protocol.cppm:433`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function retrieves a string value from a `json::Cursor` by calling `value.get_string()`. If the cursor does not contain a JSON string, the optional returned by `get_string` lacks a value, and the function returns `std::unexpected` with an `LLMError` whose message is formatted using the provided `context` parameter (e.g., "`is not a JSON string`"). On success, it dereferences the optional to obtain the `std::string_view` and returns it. This routine relies on the `kota::codec::json` library’s cursor abstraction and the standard `std::expected` error‑handling pattern; all other identifiers in the surrounding code are unrelated to this function’s internal logic.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `json::Cursor value` parameter
- `std::string_view context` parameter

#### Usage Patterns

- Extract a required string field from a JSON cursor
- Generate descriptive error when value is not a string

### `clore::net::detail::infer_output_contract`

Declaration: `network/protocol.cppm:631`

Definition: `network/protocol.cppm:648`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::infer_output_contract` resolves the output contract for a prompt request by examining a combination of the `PromptRequest::response_format` and `PromptRequest::output_contract` fields. The algorithm first checks whether `request.response_format` has a value (indicating a JSON schema was provided). If so, it validates that `request.output_contract` is not `PromptOutputContract::Markdown`—a conflict that produces an error via `std::unexpected` with an `LLMError` message—and then returns `PromptOutputContract::Json`. If no `response_format` is present, the function examines `request.output_contract`: if it equals `PromptOutputContract::Unspecified`, it returns an error requiring the contract to be explicitly set to either `Markdown` or `Json`; otherwise it returns the stored contract value. This control flow ensures a deterministic, non‑ambiguous contract for downstream output validation. Dependencies include `PromptRequest`, `PromptOutputContract`, and `LLMError`; no external services are invoked.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request.response_format`
- `request.output_contract`
- `PromptOutputContract::Markdown`
- `PromptOutputContract::Json`
- `PromptOutputContract::Unspecified`

#### Usage Patterns

- called to validate or infer the output contract before serializing a prompt
- used in request processing to ensure a consistent `PromptOutputContract`
- invoked during `PromptRequest` validation pipelines

### `clore::net::detail::insert_string_field`

Declaration: `network/protocol.cppm:215`

Definition: `network/protocol.cppm:303`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::insert_string_field` inserts a string field into a `json::Object` by converting the provided `key` and `value` parameters from `std::string_view` to `std::string` and calling `object.insert`. The `context` parameter is accepted but unused in the implementation; it is likely reserved for diagnostic or error-reporting purposes in a broader serialization framework. The return type is `std::expected<void, LLMError>`, and the function unconditionally returns a success value, making it an inline utility that never produces a failure. No branching or error handling occurs inside the function—its entire logic is the single insert operation. This simplicity indicates that `insert_string_field` is intended as a low-level building block for composing larger JSON payloads in the protocol layer.

#### Side Effects

- Mutates the `json::Object` argument by inserting a string field with the given key and value.

#### Reads From

- `key` (parameter)
- `value` (parameter)

#### Writes To

- `object` (reference parameter)

#### Usage Patterns

- Building JSON objects for LLM requests or responses
- Inserting string values into structured data

### `clore::net::detail::make_empty_array`

Declaration: `network/protocol.cppm:231`

Definition: `network/protocol.cppm:348`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The `clore::net::detail::make_empty_array` function constructs a valid, empty JSON array by parsing the literal string `"[]"` using `json::parse<json::Array>`. If the parse fails—which is theoretically impossible given a hardcoded constant—it invokes `unexpected_json_error` with the supplied `context` string and the error from `json::parse`, returning a `std::expected<json::Array, LLMError>` containing the error. On success, it moves the parsed value out and returns it. The function depends solely on `json::parse`, `json::Array`, `LLMError`, and `unexpected_json_error`; there is no branching or iteration beyond the single parse‑and‑check step.

#### Side Effects

- Allocates a new `json::Array` object
- May allocate an `LLMError` object on parse failure

#### Reads From

- context parameter

#### Writes To

- Returned value of type `std::expected<json::Array, LLMError>`

#### Usage Patterns

- Creating an empty JSON array for constructing API request bodies
- Providing a default array value in data structures
- Building placeholder arrays in validation or serialization routines

### `clore::net::detail::make_empty_object`

Declaration: `network/protocol.cppm:228`

Definition: `network/protocol.cppm:340`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::make_empty_object` constructs a minimal JSON object by parsing the literal string `{}`. It delegates to `json::parse<json::Object>("{}")`; if the parse fails (which is unexpected because the input is a constant valid JSON object), the error is forwarded via `clore::net::detail::unexpected_json_error` using the provided `context` string for diagnostics. On success, the parsed `json::Object` is returned. The function thus serves as a reliable factory that never produces a non‑empty object and centralizes the error‑handling point for this trivial case.

#### Side Effects

- Allocates a new `json::Object` on the heap
- May invoke error logging or error reporting via `unexpected_json_error` if `json::parse` fails

#### Reads From

- `context` parameter for error messages

#### Writes To

- Returned `std::expected<json::Object, LLMError>` containing a new `json::Object`

#### Usage Patterns

- Used to obtain an empty JSON object for initialization or default values
- Typically called when no JSON object is provided but a placeholder is needed

### `clore::net::detail::normalize_utf8`

Declaration: `network/protocol.cppm:213`

Definition: `network/protocol.cppm:293`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function invokes `clore::support::ensure_utf8` to produce a normalized copy of `text` that contains only valid UTF‑8 byte sequences. After normalization, it compares the normalized result with the original `text`. If they differ—indicating the original contained invalid UTF‑8 sequences—a warning is emitted via `logging::warn`, using the `field_name` parameter to identify which field caused the issue. The normalized string is then returned, ensuring downstream processing (e.g., JSON serialization) operates on well‑formed UTF‑8 data.

#### Side Effects

- Logs a warning message via `logging::warn` if the input contained invalid UTF-8

#### Reads From

- parameter `text`
- parameter `field_name`

#### Writes To

- Logging system via `logging::warn`

#### Usage Patterns

- Sanitizing LLM output or user input before JSON serialization
- Ensuring UTF-8 validity for strings in network protocol handling

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:235`

Definition: `network/protocol.cppm:357`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::parse_json_value` implements a typed JSON deserialization wrapper that converts a `std::string_view` raw into a value of the template parameter `T`. Its internal control flow consists of a single call to `json::from_json<T>(raw)` and a check on the returned `std::expected`. If the conversion succeeds, `*parsed` is returned; otherwise, an `LLMError` is constructed using `std::format` with the context string and the error message from `parsed.error().to_string()`. This design keeps parsing logic isolated from callers and centralises error reporting, making it a dependency for any code that needs to extract structured data (e.g., `clore::net::CompletionResponse` fields) from raw JSON.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `raw` parameter
- `context` parameter

#### Usage Patterns

- used to safely parse JSON strings into domain types with descriptive error messages
- called with raw response body and a context label for error reporting

### `clore::net::detail::parse_json_value`

Declaration: `network/protocol.cppm:238`

Definition: `network/protocol.cppm:368`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::parse_json_value` serves as a serializer bridge for JSON value parsing. It first attempts to convert the input `json::Value` into a string representation by calling `json::to_string`. If this serialization fails, the function immediately returns an error using `unexpected_json_error`, providing a contextual message about the failure. On success, it delegates the actual parsing to the overloaded version of `parse_json_value` that operates on a `std::string_view`, passing the serialized string and the original context.

The control flow is linear: serialization, error check, and delegation. Its key dependencies include `json::to_string` for conversion, `unexpected_json_error` for error propagation, and the string-based `parse_json_value` overload that performs the core parsing logic. This design isolates serialization concerns from the parsing algorithm.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the parameter `value` of type `const json::Value &`
- the parameter `context` of type `std::string_view`

#### Writes To

- the returned `std::expected<T, LLMError>` object

#### Usage Patterns

- Used when a `json::Value` is already available and needs to be parsed into a specific type `T`
- Called by code that has a parsed JSON tree and requires a deserialized result with error handling

### `clore::net::detail::request_text_once_async`

Declaration: `network/protocol.cppm:638`

Definition: `network/protocol.cppm:680`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function first infers the output contract via `infer_output_contract`; failure causes an immediate `co_await kota::fail`. It then adjusts the `response_format` if the contract is `PromptOutputContract::Json` and no format was supplied. A `CompletionRequest` is built from the given `model`, `system_prompt`, `request`, and the optional `response_format`, with `tools` left empty and `parallel_tool_calls` set to `std::nullopt`. The `CompletionRequester` is invoked with this request and the provided `loop`; the result is unwrapped with `or_fail()`.

Upon success, `protocol::text_from_response` extracts the text from the `CompletionResponse`; any error triggers `co_await kota::fail`. Finally, `validate_prompt_output` verifies the text against the inferred `contract`, and on success the text is moved into the returned `kota::task`. The control flow is linear with early exits on error, relying on `infer_output_contract`, `protocol::text_from_response`, and `validate_prompt_output` as key cooperative dependencies.

#### Side Effects

- performs asynchronous I/O via the `CompletionRequester` callable
- `co_awaits` on potential failures and suspension of the coroutine
- constructs and returns a dynamically allocated `std::string`

#### Reads From

- parameter `model`
- parameter `system_prompt`
- parameter `request` (including fields `prompt`, `response_format`, `tool_choice`, `output_contract`)
- parameter `loop`
- local variable `contract`
- local variable `response`
- local variable `text`

#### Writes To

- returns a `std::string` value to the caller via `co_return`

#### Usage Patterns

- Used to perform an asynchronous text completion request with automatic output contract inference and validation

### `clore::net::detail::run_task_sync`

Declaration: `network/protocol.cppm:226`

Definition: `network/protocol.cppm:322`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::run_task_sync` executes an asynchronous task synchronously by bridging it onto a `kota::event_loop`. It first constructs a local `loop` instance, then schedules the `operation` produced by the caller-supplied `make_task` lambda on that loop. Calling `loop.run()` blocks the current thread until all scheduled work—including the operation—completes. After the loop exits, the function retrieves the operation’s result; if it contains an error, it wraps the error with `std::unexpected` and returns a failed `std::expected`. Otherwise, it uses `std::is_void_v` to decide between returning a default-constructed `expected` (for `void` `T`) or moving the contained value out of the result.

The implementation depends on `kota::event_loop` to provide a run‑until‑completion barrier and on the operation type’s `result()` method, which returns a `std::expected` (or a type with a compatible error interface). This pattern allows the remaining synchronous caller to treat any asynchronous error path uniformly through `std::unexpected`, unifying error handling with the rest of the codebase.

#### Side Effects

- Creates and runs a local event loop, blocking the calling thread until the scheduled asynchronous task completes.
- Potentially performs I/O or other side effects through the execution of the task returned by `make_task`.
- Moves the result value (if any) out of the operation, transferring ownership to the caller.

#### Reads From

- `make_task` callable
- `kota::event_loop loop` (its state after scheduling and running)
- `operation.result()` (the internal result and error state)

#### Writes To

- `loop` local variable (its internal event queue and completion state)
- The returned `std::expected<T, LLMError>` (via move construction from the operation's result)

#### Usage Patterns

- Wrapping an asynchronous task for synchronous execution in a blocking manner
- Running a task on a short-lived event loop for testing or simple call sites
- Converting a `make_task` pattern into a synchronous interface

### `clore::net::detail::serialize_value_to_string`

Declaration: `network/protocol.cppm:241`

Definition: `network/protocol.cppm:378`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::serialize_value_to_string` wraps a call to `json::to_string` with error‑handling logic. It receives a `json::Value` and a `std::string_view` identifying the *context* of the serialization (used in error messages). The control flow is minimal: `json::to_string` is invoked on the input value; if it returns an unexpected result (i.e., `encoded` has no value), the function delegates to `unexpected_json_error`, passing a formatted string that includes the *context* and the raw `json::error` from the failed serialization. On success, the encoded string is returned directly. The only dependency beyond the JSON codec is the utility function `unexpected_json_error` and `std::format` for constructing the error message.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `value` parameter of type `const json::Value&`
- the `context` parameter of type `std::string_view`

#### Writes To

- the returned `std::expected<std::string, LLMError>` object

#### Usage Patterns

- serializing a JSON value to a string with contextual error handling
- used internally whenever a JSON value must be converted to a string for further processing or reporting

### `clore::net::detail::to_llm_unexpected`

Declaration: `network/protocol.cppm:221`

Definition: `network/protocol.cppm:312`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The template function `clore::net::detail::to_llm_unexpected` constructs a `std::unexpected<LLMError>` by formatting a descriptive error message using `std::format`. It concatenates the given `context` string with the string representation of the `Status` error obtained via `error.to_string()`, separated by `": "`. The resulting string is passed to the `LLMError` constructor, and the `LLMError` object is then wrapped in `std::unexpected` and returned. The function depends on `std::format`, the `LLMError` type, and the `to_string` method of the `Status` template parameter.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `Status error` parameter (via `error.to_string()`)
- `std::string_view context` parameter
- string literal `"{}: {}"`

#### Usage Patterns

- Converting a domain-specific error and a descriptive context into an `LLMError` embedded in a `std::unexpected` for use with `std::expected` return types

### `clore::net::detail::unexpected_json_error`

Declaration: `network/protocol.cppm:210`

Definition: `network/protocol.cppm:288`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

This function constructs an `LLMError` by formatting a message that concatenates the given `context` string with the string representation of the `json::error` obtained via `err.to_string()`. It then returns the error wrapped in `std::unexpected`. The implementation is a single expression that delegates to `std::format` for message composition. This utility is used throughout the detail layer to convert JSON parsing failures into typed LLM errors while preserving contextual information about which operation or field caused the error.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Parameter `context` of type `std::string_view`
- Parameter `err` of type `const json::error&` (via call to `err.to_string()`)

#### Usage Patterns

- Wrapping a JSON error into an `LLMError` unexpected result
- Creating an error response for JSON parsing or validation failures

### `clore::net::detail::validate_prompt_output`

Declaration: `network/protocol.cppm:634`

Definition: `network/protocol.cppm:666`

Declaration: [`Namespace clore::net::detail`](../../namespaces/clore/net/detail/index.md)

The function `clore::net::detail::validate_prompt_output` accepts a `std::string_view content` and a `PromptOutputContract contract`. It dispatches based on the `contract` value using a `switch`. For `PromptOutputContract::Json`, it delegates to `protocol::validate_json_output(content)`; for `PromptOutputContract::Markdown`, it calls `protocol::validate_markdown_fragment_output(content)`. The `PromptOutputContract::Unspecified` case and the default branch both return an error via `std::unexpected(LLMError(...))` with a descriptive message indicating that the contract must be explicitly set or is unsupported. The function thus relies on the `PromptOutputContract` enum and the two protocol‑level validation functions, and serves as a thin dispatching layer.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `content` parameter (`std::string_view`)
- `contract` parameter (`PromptOutputContract`)

#### Usage Patterns

- used to check that a prompt response matches the expected format
- called after receiving a prompt output to validate against the contract
- returns an error if the contract is not set to `Json` or `Markdown`

### `clore::net::get_probed_capabilities`

Declaration: `network/protocol.cppm:126`

Definition: `network/protocol.cppm:729`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::get_probed_capabilities` implements a thread‑safe lazy‑initialization cache for `ProbedCapabilities` objects. It uses a static `std::mutex` and a static `std::unordered_map` keyed by the string form of `cache_key`. On entry, it acquires a `std::lock_guard` on the mutex and performs a lookup. If the key already exists, it returns a reference to the stored `ProbedCapabilities`. Otherwise, it creates a default‑constructed `ProbedCapabilities` via `std::make_unique`, emplaces it into the cache, and returns a reference to the newly inserted object. This design ensures that each distinct `cache_key` yields exactly one persistent `ProbedCapabilities` instance, and that concurrent calls are safely serialised. The returned reference remains valid for the lifetime of the cache; no probing logic is visible in the snippet – the returned `ProbedCapabilities` is initially empty and is presumably populated later by the caller.

#### Side Effects

- modifies the static cache map by inserting a new `ProbedCapabilities` entry when the key is not present
- acquires a static mutex lock for thread synchronization

#### Reads From

- input parameter `cache_key`
- static cache `std::unordered_map` for lookup

#### Writes To

- static cache `std::unordered_map` via insertion of new entries

#### Usage Patterns

- Retrieving or initializing probed capabilities for a given key
- Caching probe results to avoid redundant probing

### `clore::net::icontains`

Declaration: `network/protocol.cppm:768`

Definition: `network/protocol.cppm:768`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

Implementation: [Implementation](functions/icontains.md)

The function `clore::net::icontains` performs a case‑insensitive substring search using a brute‑force sliding‑window algorithm. It first rejects empty or oversized needles with an early return when `needle.size()` exceeds `haystack.size()`. The outer loop iterates over every possible starting offset `i` in `haystack`, up to `haystack.size() - needle.size()`. For each offset, an inner loop compares the `needle` character by character; both characters are lowercased via `std::tolower` with an explicit cast to `unsigned char` to avoid implementation‑defined behavior on negative char values. If all positions match, the function returns `true`; otherwise it continues searching. If no offset produces a full match, it returns `false`. The implementation relies solely on the standard library (function template `std::tolower`, class `std::string_view`); no locale or external data structures are involved, and the algorithm completes in `O(n·m)` time where `n` is the length of `haystack` and `m` the length of `needle`.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `haystack` contents and size
- parameter `needle` contents and size

#### Usage Patterns

- invoked by `clore::net::is_feature_rejection_error` to detect feature-rejection keywords inside error message text
- general case-insensitive substring matching within the `clore::net` module

### `clore::net::is_feature_rejection_error`

Declaration: `network/protocol.cppm:135`

Definition: `network/protocol.cppm:788`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::is_feature_rejection_error` determines whether a given `error_message` string likely indicates that the API rejected a request due to an unsupported or unrecognized feature. It iterates over a static array of common rejection substrings — such as `"unsupported parameter"`, `"unknown field"`, `"invalid field"`, and `"not supported"` — and uses `clore::net::icontains` to perform a case‑insensitive substring search against the `error_message`. If any pattern matches, the function immediately returns `true`; otherwise, it returns `false` after exhausting the list. This simple scan–and–match approach provides fast rejection detection without parsing the error structure, and depends only on `clore::net::icontains` for the containment check.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `error_message` parameter
- `patterns` static array

#### Usage Patterns

- Checking if an LLM error response indicates rejection due to unsupported features
- Filtering error messages for capability probing

### `clore::net::make_capability_probe_key`

Declaration: `network/protocol.cppm:128`

Definition: `network/protocol.cppm:743`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function constructs a cache key for capability probing by joining the three string views—`provider`, `api_base`, and `model`—with the pipe character (`|`) as a delimiter. The implementation uses `std::format` to produce a single `std::string` in the pattern `"provider|api_base|model"`. Dependencies are limited to the C++ standard library’s formatting facility (`std::format`) and the arguments’ types (`std::string_view`). There is no branching, error handling, or external state access; the control flow consists solely of the formatted concatenation and immediate return.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `provider` parameter (`std::string_view`)
- `api_base` parameter (`std::string_view`)
- `model` parameter (`std::string_view`)

#### Usage Patterns

- Used to generate a unique lookup key for capability probes based on provider, API base, and model.

### `clore::net::make_markdown_fragment_request`

Declaration: `network/protocol.cppm:99`

Definition: `network/protocol.cppm:144`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::make_markdown_fragment_request` constructs and returns a `PromptRequest` in a single expression. It assigns the given `prompt` string to the `.prompt` field, explicitly sets `.response_format` to `std::nullopt` to disable any JSON schema enforcement, and marks `.output_contract` as `PromptOutputContract::Markdown`. This eliminates the need for callers to manually configure these fields when the goal is to obtain a plain Markdown fragment from the model. The implementation depends solely on the `PromptRequest` aggregate and the `PromptOutputContract` enumeration; no control flow or additional validation is performed at construction time. The resulting request is later consumed by the completion pipeline, where the `output_contract` value influences output validation (e.g., the protocol layer invokes `protocol::validate_markdown_fragment_output` on the response) and capability probing for tool support.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the parameter `prompt` (moved into the returned `PromptRequest`)

#### Writes To

- the returned `PromptRequest` value

#### Usage Patterns

- used to create a markdown-fragment request from a plain prompt string
- callers rely on it to avoid manually setting `response_format` and `output_contract`

### `clore::net::parse_rejected_feature_from_error`

Declaration: `network/protocol.cppm:137`

Definition: `network/protocol.cppm:807`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function uses a static table of `field_patterns`: each entry pairs a keyword (a substring to search for) with a canonical field name. It performs a linear scan over these entries, invoking `icontains` to perform a case‑insensitive substring check of the input `error_message` against the keyword. On the first match it converts the corresponding field name to `std::string` and returns it. If no entry matches, it returns `std::nullopt`. The only external dependency is the utility `icontains`. No parsing or JSON interaction is involved; the logic is a pure heuristic based on textual pattern matching.

#### Side Effects

- Allocates a `std::string` if a matching field is found.

#### Reads From

- `error_message` parameter
- static local array `field_patterns` of `std::pair<std::string_view, std::string_view>`

#### Usage Patterns

- Called during error parsing to determine which LLM feature was rejected based on the error message returned by the provider.
- Used in conjunction with `clore::net::is_feature_rejection_error` to categorize errors.

### `clore::net::protocol::append_tool_outputs`

Declaration: `network/protocol.cppm:473`

Definition: `network/protocol.cppm:544`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function first validates the response and the supplied tool outputs. It checks that the `CompletionResponse` contains at least one tool call, then builds an index map from `ToolOutput::tool_call_id` to the output text, rejecting empty or duplicate identifiers. A merged vector is constructed by copying the existing history and appending an `AssistantToolCallMessage` that carries the response’s `AssistantOutput::text` and `AssistantOutput::tool_calls`. For each `ToolCall` in the response, the function looks up its `id` in the map and pushes a corresponding `ToolResultMessage` containing the matched output. Finally, it verifies that the number of supplied outputs exactly matches the number of tool calls, returning an `LLMError` if any are missing or extra. This ensures the resulting conversation state is consistent and ready for the next completion request. The implementation depends on `std::span`, `std::unordered_map`, `AssistantToolCallMessage`, `ToolResultMessage`, and the `CompletionResponse` and `ToolOutput` types, all of which are defined in the networking protocol layer.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `history`
- `response`
- `outputs`
- `response.message.tool_calls`
- `response.message.text`
- `output.tool_call_id`
- `output.output`
- `tool_call.id`

#### Usage Patterns

- Called to integrate tool outputs into a conversation history
- Used after receiving a completion response containing tool calls
- Part of protocol‑level message construction for LLM interactions

### `clore::net::protocol::parse_response_text`

Declaration: `network/protocol.cppm:479`

Definition: `network/protocol.cppm:592`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function `clore::net::protocol::parse_response_text` implements a two‑step extraction pipeline. It first delegates to `clore::net::protocol::text_from_response` to retrieve the raw text payload from the given `CompletionResponse`. If `text_from_response` fails, the resulting `LLMError` is forwarded immediately via `std::unexpected`. On success, the extracted string is parsed as JSON into the target type `T` using `kota::codec::json::from_json<T>`. A parse failure produces a descriptive `LLMError` that includes the underlying `kota::codec::json::error` message. Only when both steps succeed is the deserialized value returned. The function consequently depends on `clore::net::protocol::text_from_response` for text extraction and on the JSON library’s deserialization infrastructure.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `response` parameter
- `*text` after dereference from `text_from_response` result

#### Usage Patterns

- Used to convert a `CompletionResponse` into a structured type `T` after an LLM completion
- Typically invoked after receiving a response that contains a JSON body to be deserialized

### `clore::net::protocol::parse_tool_arguments`

Declaration: `network/protocol.cppm:482`

Definition: `network/protocol.cppm:607`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function first attempts to serialize `call.arguments` (a `kota::codec::json::Value`) to a string using `kota::codec::json::to_string`. If serialization fails, the function immediately returns `std::unexpected` with an `LLMError` that includes the tool name (`call.name`) and the serialization error. Upon success, it attempts to parse the resulting string into the template type `T` via `kota::codec::json::from_json<T>`. If parsing fails, it returns another `std::unexpected` with an `LLMError` describing the failure, again including the tool name. Only when both stages succeed does the function return the parsed value.

This implementation relies on the JSON codec library `kota::codec::json` for both serialization and deserialization, and on the error type `LLMError` to propagate failures. The control flow is a simple two-step pipeline: convert the internal `arguments` field into a string, then deserialize that string into the caller-requested type `T`. Each step checks for errors and produces a contextual error message using the tool name, ensuring traceability when tool argument processing fails.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- call`.arguments`
- call`.name`

#### Usage Patterns

- deserialize tool call arguments into the expected type T
- called during LLM response processing to extract typed tool parameters

### `clore::net::protocol::text_from_response`

Declaration: `network/protocol.cppm:471`

Definition: `network/protocol.cppm:528`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function `clore::net::protocol::text_from_response` implements a validation gate that extracts plain text from a `CompletionResponse` or returns an error. The algorithm follows a linear, early-return control flow: it first checks whether the `response.message.refusal` field holds a value; if so, it constructs an `LLMError` describing the refusal. Next, it verifies `response.message.tool_calls` is empty; a non‑empty set indicates the LLM produced a tool call rather than a textual answer, which is reported as an error. Finally, it ensures `response.message.text` is present; if absent, it returns an error stating there is no text payload. On success, the function dereferences and returns `*response.message.text`. The implementation depends on the `CompletionResponse`, its nested `AssistantOutput` message, the `LLMError` type, and `std::expected` for result propagation. No external parsing or serialization is performed—the logic purely validates and extracts the already‑available structured fields.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `response.message.refusal`
- `response.message.tool_calls`
- `response.message.text`

#### Usage Patterns

- Extracting text from a `CompletionResponse` when text is expected
- Validating that the response is not a refusal and does not contain tool calls

### `clore::net::protocol::validate_json_output`

Declaration: `network/protocol.cppm:467`

Definition: `network/protocol.cppm:488`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The implementation of `clore::net::protocol::validate_json_output` is a straightforward validation step that relies entirely on the external `kota::codec::json` library. It calls `kota::codec::json::parse<kota::codec::json::Value>` on the provided `std::string_view content`. If the parse fails (i.e., the returned `std::optional` or equivalent result does not contain a value), the function constructs a `std::unexpected` error of type `LLMError` using the formatted error message from the parser. On success, the function returns an empty `std::expected<void, LLMError>`.

The internal control flow is a simple conditional branch with no loops or additional processing. The sole dependency is the `kota::codec::json` parsing machinery; the function does not inspect the parsed JSON structure, check for specific keys, or perform any semantic validation. Its purpose is limited to confirming that the input is syntactically valid JSON before further operations use it.

#### Side Effects

- allocates a string for the error message via `std::format`
- transfers ownership of an `LLMError` object to the caller on failure

#### Reads From

- parameter `content` (a `std::string_view`)

#### Usage Patterns

- validate JSON output from an LLM response
- called before further processing of tool call outputs or responses

### `clore::net::protocol::validate_markdown_fragment_output`

Declaration: `network/protocol.cppm:469`

Definition: `network/protocol.cppm:497`

Declaration: [`Namespace clore::net::protocol`](../../namespaces/clore/net/protocol/index.md)

The function `clore::net::protocol::validate_markdown_fragment_output` applies a series of rejection checks to the supplied `std::string_view` `content`. It first tests for emptiness and then scans for any non‑whitespace character; if either test fails, it returns `std::unexpected` with a descriptive `LLMError`. An H1 heading (a line beginning with `# `) anywhere in the content or the presence of a code fence (`` ` ``) triggers an error. Finally, the function delegates to `validate_json_output` and, if that call succeeds (meaning the content is valid JSON), returns an error because a markdown fragment was required. Only when all checks pass does it return an empty result. The algorithm thus implements a linear control flow of early‑exit validations, relying on `LLMError` for error reporting and `validate_json_output` as a dependency to detect JSON content.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `content` parameter

#### Usage Patterns

- Used to validate that LLM output is a markdown fragment in the protocol pipeline
- Likely called after extracting text from a completion response

### `clore::net::sanitize_request_for_capabilities`

Declaration: `network/protocol.cppm:132`

Definition: `network/protocol.cppm:749`

Declaration: [`Namespace clore::net`](../../namespaces/clore/net/index.md)

The function `clore::net::sanitize_request_for_capabilities` performs a sequence of conditional sanitization steps on a `CompletionRequest` based on the capabilities reported in `ProbedCapabilities`. Each capability flag is read using `std::memory_order_relaxed` atomic loads, and the corresponding field in the request is either cleared or set to `std::nullopt` if the capability is absent. Specifically, when `supports_json_schema` is `false`, the nested `schema` inside a present `response_format` is removed; when `supports_tool_choice` is `false`, the `tool_choice` field is cleared; when `supports_parallel_tool_calls` is `false`, `parallel_tool_calls` is set to `std::nullopt`; and when `supports_tools` is `false`, the entire `tools` vector is cleared. The function returns the modified `CompletionRequest` by value, leaving the original copy unaffected. No external calls or complex control flow are involved, making the operation strictly a local adjustment of the request structure.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `caps.supports_json_schema`
- `caps.supports_tool_choice`
- `caps.supports_parallel_tool_calls`
- `caps.supports_tools`
- `request.response_format`
- `request.response_format->schema`
- `request.tool_choice`
- `request.parallel_tool_calls`
- `request.tools`

#### Usage Patterns

- preprocess completion request before API call based on probed capabilities
- apply capability constraints to request to avoid rejected features

## Internal Structure

The `protocol` module defines the core data types and functions for LLM communication. It models the full request-response cycle with structures like `CompletionRequest`, `CompletionResponse`, `PromptRequest`, and `ToolCall`, alongside message variants (`SystemMessage`, `UserMessage`, `AssistantMessage`, `ToolResultMessage`, `AssistantToolCallMessage`) aggregated under the `Message` type alias. It also encapsulates tool choice policies (`ToolChoiceAuto`, `ToolChoiceNone`, `ToolChoiceRequired`, `ForcedFunctionToolChoice`) and output validation contracts (`PromptOutputContract`). The module imports `std` for standard support, `http` for network access, and `support` for foundational utilities (e.g., UTF‑8 normalization, caching, logging).

Internally, the module is layered into a public API and a `detail` namespace. The public API exposes types and functions for constructing requests (`make_markdown_fragment_request`), sanitizing requests against probed capabilities (`sanitize_request_for_capabilities`), parsing responses (`text_from_response`, `parse_response_text`, `parse_tool_arguments`), and validating outputs (`validate_json_output`, `validate_markdown_fragment_output`). The detail layer provides lower‑level JSON manipulation (`expect_array`, `expect_object`, `clone_array`, `clone_object`, `parse_json_value`), synchronous task execution (`run_task_sync`), asynchronous request helpers (`request_text_once_async`), and capability probing infrastructure (`make_capability_probe_key`, `get_probed_capabilities`, `ProbedCapabilities`). This separation keeps the core protocol logic independent of HTTP specifics while enabling reuse of validation and serialization primitives.

## Related Pages

- [Module http](../http/index.md)
- [Module support](../support/index.md)

