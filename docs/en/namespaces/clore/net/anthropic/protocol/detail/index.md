---
title: 'Namespace clore::net::anthropic::protocol::detail'
description: 'The clore::net::anthropic::protocol::detail namespace encapsulates the internal implementation helpers for the Anthropic protocol layer. Its primary responsibility is to provide low‑level building blocks for constructing and validating protocol messages, including functions to create text blocks (make_text_block), role messages (make_role_message), tool‑use and tool‑result blocks (make_tool_use_block, make_tool_result_block), JSON text parsing (parse_json_text), incremental string assembly with gaps (append_text_with_gap), schema instruction formatting (format_schema_instruction), and request validation (validate_request). A constant kDefaultMaxTokens (value 2048) is also defined here.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol::detail`

## Summary

The `clore::net::anthropic::protocol::detail` namespace encapsulates the internal implementation helpers for the Anthropic protocol layer. Its primary responsibility is to provide low‑level building blocks for constructing and validating protocol messages, including functions to create text blocks (`make_text_block`), role messages (`make_role_message`), tool‑use and tool‑result blocks (`make_tool_use_block`, `make_tool_result_block`), JSON text parsing (`parse_json_text`), incremental string assembly with gaps (`append_text_with_gap`), schema instruction formatting (`format_schema_instruction`), and request validation (`validate_request`). A constant `kDefaultMaxTokens` (value `2048`) is also defined here.

Architecturally, this namespace isolates protocol‑specific logic from the rest of the codebase. The functions and constants within are intended solely for use by other protocol internals and are not part of the public API. By centralizing these details, the higher‑level protocol and network layers can rely on concise, consistent operations without exposing their implementation.

## Variables

### `clore::net::anthropic::protocol::detail::kDefaultMaxTokens`

Declaration: `network/anthropic.cppm:23`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

A constant of type `std::uint32_t` with value `2048`, defined in namespace `clore::net::anthropic::protocol::detail`.

#### Usage Patterns

- used as default argument in `build_request_json`

## Functions

### `clore::net::anthropic::protocol::detail::append_text_with_gap`

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/append-text-with-gap.md)

The function `clore::net::anthropic::protocol::detail::append_text_with_gap` is a helper used during request construction to incrementally build a string containing text content. It accepts a target string (by reference) and a text fragment to append. If the target string already holds content, the function inserts a gap—typically a newline or delimiter—before the new fragment, ensuring that separate text pieces are clearly separated in the final output. Callers provide the accumulating string and the next text fragment; after the call, the target string is extended with the fragment (and a preceding gap when needed), preserving correct formatting for the larger JSON request body.

#### Usage Patterns

- Used in `build_request_json` to assemble text blocks with gaps

### `clore::net::anthropic::protocol::detail::format_schema_instruction`

Declaration: `network/anthropic.cppm:176`

Definition: `network/anthropic.cppm:176`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::format_schema_instruction` accepts a reference to an integer representing a schema and returns an integer. It is responsible for converting a given schema into a formatted instruction suitable for inclusion in tool use blocks or similar protocol elements. Callers can expect the return value to indicate the result of the formatting operation, typically an identifier or status code.

#### Usage Patterns

- Generates instruction string for structured output in Anthropic API requests
- Called when constructing messages that require a JSON schema response format

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:154`

Definition: `network/anthropic.cppm:154`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_role_message` constructs a protocol message object for a given role. It accepts a `std::string_view` identifying the role (e.g., `"user"` or `"assistant"`) and either a `json::Array` of content blocks or a plain text `std::string_view`. This overloaded interface allows callers to supply either structured content (such as text and tool‑use blocks) or a simple text string. The return type is `int`, which indicates the result of the construction (typically a status code or an identifier for the created message). Callers must ensure the role string is valid and the content is correctly formatted according to the Anthropic protocol requirements.

#### Usage Patterns

- Used to construct role-based message objects for the Anthropic protocol
- Called when building a request message with a role and content blocks

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:130`

Definition: `network/anthropic.cppm:130`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_role_message` constructs a protocol‑level message with a specified role. It is the caller’s responsibility to provide the role as a `std::string_view` and the message content either as a plain text `std::string_view` or as a `json::Array` of content blocks. The function returns an integer identifier for the created message, which can be used in subsequent protocol operations. The contract does not specify whether the role value is validated; callers should supply a role string that is valid per the Anthropic message protocol (such as `"user"` or `"assistant"`).

#### Usage Patterns

- called to create a role message object for Anthropic protocol messages

### `clore::net::anthropic::protocol::detail::make_text_block`

Declaration: `network/anthropic.cppm:35`

Definition: `network/anthropic.cppm:35`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_text_block` accepts a `std::string_view` and returns an `int`. It constructs a text block representation suitable for use within an Anthropic protocol message. The return value indicates success or failure, or may serve as an identifier for the created block; callers should check the returned `int` to determine whether the operation completed successfully.

#### Usage Patterns

- Constructing a text content block for an Anthropic API message array

### `clore::net::anthropic::protocol::detail::make_tool_result_block`

Declaration: `network/anthropic.cppm:98`

Definition: `network/anthropic.cppm:98`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_tool_result_block` constructs a tool result block for use in Anthropic protocol messages. It accepts a reference to an integer identifier for the tool use instance and returns an integer status indicating successful creation or an error condition. Callers rely on this function to produce a properly formatted block that represents the outcome of a tool invocation, which can then be embedded into a larger response structure.

#### Usage Patterns

- Used when constructing a tool result message to be sent to the Anthropic API.
- Expected to be called from higher-level protocol message building functions.

### `clore::net::anthropic::protocol::detail::make_tool_use_block`

Declaration: `network/anthropic.cppm:58`

Definition: `network/anthropic.cppm:58`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_tool_use_block` constructs a tool‑use content block for inclusion in an Anthropic protocol message. It accepts a tool invocation identifier (provided as a `const int &`) and returns an opaque value of type `int` that represents the newly created block.  

The caller is responsible for supplying a valid identifier that corresponds to a specific tool call as defined by the Anthropic API. The returned block must be inserted into the appropriate message’s content array, and its lifetime is managed by the caller according to the protocol’s requirements.

#### Usage Patterns

- used to create a `tool_use` block for Anthropic protocol requests

### `clore::net::anthropic::protocol::detail::parse_json_text`

Declaration: `network/anthropic.cppm:171`

Definition: `network/anthropic.cppm:171`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::parse_json_text` is an internal helper within the Anthropic protocol implementation. It accepts two `std::string_view` arguments—the first being a JSON text string and the second a related context or identifier—and returns an `int` result, typically indicating success or an error code. Its caller‑facing responsibility is to parse the provided JSON content and return a status that downstream detail functions, such as `make_text_block` or `make_role_message`, can use to proceed or handle failure. This function is not part of the public API and is intended solely for use by other protocol internals.

#### Usage Patterns

- Used internally to parse JSON text in Anthropic protocol implementation

### `clore::net::anthropic::protocol::detail::validate_request`

Declaration: `network/anthropic.cppm:193`

Definition: `network/anthropic.cppm:193`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::validate_request` validates an API request identified by the provided integer reference. It examines whether the request represented by `const int &` conforms to expected protocol constraints and returns an integer status code that signals the outcome of the validation (typically indicating success or a specific error condition). Callers must supply a valid reference to a request identifier; the function does not modify the argument and relies solely on the input to determine validity.

#### Usage Patterns

- Called before sending a completion request to Anthropic API
- Used in request construction and validation pipelines

## Related Pages

- [Namespace clore::net::anthropic::protocol](../index.md)

