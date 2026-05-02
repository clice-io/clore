---
title: 'Namespace clore::net::anthropic::protocol::detail'
description: 'The clore::net::anthropic::protocol::detail namespace provides low‑level, internal utilities for constructing and validating Anthropic protocol messages. It exposes functions such as make_text_block, make_role_message, make_tool_use_block, make_tool_result_block, parse_json_text, format_schema_instruction, append_text_with_gap, and validate_request, which operate on opaque integer handles or status codes. A constant kDefaultMaxTokens is also defined here. These components are used exclusively within the protocol implementation layer to assemble structured content, enforce request correctness, and manage text formatting; they are not intended for direct use by external callers.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol::detail`

## Summary

The `clore::net::anthropic::protocol::detail` namespace provides low‑level, internal utilities for constructing and validating Anthropic protocol messages. It exposes functions such as `make_text_block`, `make_role_message`, `make_tool_use_block`, `make_tool_result_block`, `parse_json_text`, `format_schema_instruction`, `append_text_with_gap`, and `validate_request`, which operate on opaque integer handles or status codes. A constant `kDefaultMaxTokens` is also defined here. These components are used exclusively within the protocol implementation layer to assemble structured content, enforce request correctness, and manage text formatting; they are not intended for direct use by external callers.

## Variables

### `clore::net::anthropic::protocol::detail::kDefaultMaxTokens`

Declaration: `network/anthropic.cppm:23`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

`clore::net::anthropic::protocol::detail::kDefaultMaxTokens` is a `constexpr std::uint32_t` declared at line 23 of `network/anthropic.cppm`. It holds the default maximum token count for Anthropic API requests.

#### Usage Patterns

- Referenced in `build_request_json` function as default value for `max_tokens`

## Functions

### `clore::net::anthropic::protocol::detail::append_text_with_gap`

Declaration: `network/anthropic.cppm:25`

Definition: `network/anthropic.cppm:25`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/append-text-with-gap.md)

This function appends the content of a `std::string_view` to a `std::string`, inserting a separator or “gap” as required by the protocol’s text‑block formatting rules. It is designed for internally composing multi‑segment text content (for example, within a message or content block) where a structural break must be placed between adjacent pieces of text. The caller supplies the target string as the first argument, which is mutated in place, and the incoming text as the second argument.

#### Usage Patterns

- Used by `build_request_json` to accumulate JSON text blocks with gap separation.

### `clore::net::anthropic::protocol::detail::format_schema_instruction`

Declaration: `network/anthropic.cppm:176`

Definition: `network/anthropic.cppm:176`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

This function is part of the internal protocol formatting utilities and is responsible for converting a schema reference into a numeric instruction. It accepts a schema identifier as a `const int &` parameter and returns an `int` that represents the formatted instruction, which is subsequently used by other functions in the `clore::net::anthropic::protocol::detail` namespace to construct protocol messages. The caller must supply a valid schema identifier; the returned value is expected to be a positive instruction code or length.

#### Usage Patterns

- Called to format the JSON schema instruction for LLM prompts
- Used in constructing conversation messages with tool-use or structured output

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:154`

Definition: `network/anthropic.cppm:154`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Constructs a protocol message object representing a single role‑scoped turn in a conversation. The caller provides a role label (e.g. `"user"` or `"assistant"`) as a `std::string_view` and an array of content blocks as a `json::Array`. The function returns an opaque integer handle suitable for use with other functions in the `detail` namespace, such as `make_tool_use_block` or `validate_request`.

The caller is responsible for ensuring the role string is one of the values recognised by the Anthropic protocol and that the content array contains only valid block types (text, tool use, tool result, etc.). The returned handle is valid only within the current request‑building context; no ownership transfer or lifetime extension is implied beyond the immediate construction operation.

#### Usage Patterns

- Constructing a message object for Anthropic API requests
- Associating a role string with a list of content blocks

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `network/anthropic.cppm:130`

Definition: `network/anthropic.cppm:130`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_role_message` constructs a role message object used in the Anthropic API protocol. It accepts a role (as a `std::string_view`) and either a plain text content (also a `std::string_view`) or a structured content array (a `json::Array`), and returns an opaque integer handle. The caller is responsible for supplying a valid role string (e.g. `"user"` or `"assistant"`) and appropriately formatted content; the returned handle can be passed to other detail‑level protocol functions. No ownership or lifetime management of the handle is required by the caller—it is valid only within the current protocol assembly context.

#### Usage Patterns

- Used to generate a JSON role-content message pair for Anthropic protocol, typically as part of building a request payload.

### `clore::net::anthropic::protocol::detail::make_text_block`

Declaration: `network/anthropic.cppm:35`

Definition: `network/anthropic.cppm:35`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_text_block` accepts a `std::string_view` representing the text content and returns an `int` that serves as a handle or identifier for the constructed text block. The caller provides the raw text, and the function creates the corresponding protocol-level block object, returning a value that can be used with other `detail` functions (e.g., `make_role_message`) that accept such identifiers. The exact interpretation of the returned `int` is not specified, but it uniquely identifies the block for subsequent protocol operations.

#### Usage Patterns

- creating text blocks for Anthropic API requests
- building message content parts in the protocol layer

### `clore::net::anthropic::protocol::detail::make_tool_result_block`

Declaration: `network/anthropic.cppm:98`

Definition: `network/anthropic.cppm:98`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

This function constructs a tool result content block for inclusion in a message’s content array. The caller supplies a reference to an identifier that links the result to a prior tool use; the function returns a representation of that block suitable for serialization in the Anthropic protocol format. It is one of several `detail`‑level utilities that together compose structured content within a request’s message body.

#### Usage Patterns

- called to create `tool_result` blocks for Anthropic assistant responses
- used in higher-level message construction functions

### `clore::net::anthropic::protocol::detail::make_tool_use_block`

Declaration: `network/anthropic.cppm:58`

Definition: `network/anthropic.cppm:58`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::make_tool_use_block` is an internal utility that constructs a tool use block for the Anthropic protocol. The caller provides a `const int &` identifier, which designates the tool call to be represented, and the function returns an `int` — either a handle to the created block or a status code indicating success or failure. This function is part of the detail namespace and is intended for use only within the protocol implementation layer, not by external callers.

#### Usage Patterns

- used when constructing Anthropic API requests that include tool use blocks
- called by higher-level functions building message content or request bodies

### `clore::net::anthropic::protocol::detail::parse_json_text`

Declaration: `network/anthropic.cppm:171`

Definition: `network/anthropic.cppm:171`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::parse_json_text` parses the JSON content provided via its first `std::string_view` parameter, using the second `std::string_view` parameter as an auxiliary argument (for example, a key or a parse context). It returns an `int` that represents the outcome of the parse: a value of `0` typically indicates success, while a non‑zero value signals an error condition. This function is part of the internal detail layer for building Anthropic protocol messages and should be called only by other protocol construction utilities; callers must treat all non‑zero return values as failures and avoid relying on the exact numeric mapping.

#### Usage Patterns

- parsing JSON objects from raw text strings
- delegating to the core JSON parser `clore::net::detail::parse_json_object`
- used in Anthropic protocol message construction or response processing

### `clore::net::anthropic::protocol::detail::validate_request`

Declaration: `network/anthropic.cppm:193`

Definition: `network/anthropic.cppm:193`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::validate_request` is an internal helper that performs validation of an Anthropic protocol request. The caller must supply a const reference to the request object (identified by an `int` handle or similar), and the function returns an `int` that signals the outcome of validation: a zero value typically indicates the request is valid, while a non-zero value denotes a specific error condition. The exact error semantics are defined by the implementation, but the contract guarantees that the request is not modified and that validation is based solely on the request’s current content.

#### Usage Patterns

- Called to validate a completion request before submitting to the API.

## Related Pages

- [Namespace clore::net::anthropic::protocol](../index.md)

