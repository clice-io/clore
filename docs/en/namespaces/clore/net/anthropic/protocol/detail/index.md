---
title: 'Namespace clore::net::anthropic::protocol::detail'
description: 'The clore::net::anthropic::protocol::detail namespace encapsulates the internal, low‑level helpers that support construction and validation of Anthropic API request payloads. It provides functions for creating content blocks (make_text_block, make_tool_use_block, make_tool_result_block), assembling messages with a specific role (make_role_message), parsing JSON content (parse_json_text), formatting schema instructions (format_schema_instruction), and validating requests (validate_request). A constant kDefaultMaxTokens (set to 2048) is also defined here.'
layout: doc
template: doc
---

# Namespace `clore::net::anthropic::protocol::detail`

## Summary

The `clore::net::anthropic::protocol::detail` namespace encapsulates the internal, low‑level helpers that support construction and validation of Anthropic API request payloads. It provides functions for creating content blocks (`make_text_block`, `make_tool_use_block`, `make_tool_result_block`), assembling messages with a specific role (`make_role_message`), parsing JSON content (`parse_json_text`), formatting schema instructions (`format_schema_instruction`), and validating requests (`validate_request`). A constant `kDefaultMaxTokens` (set to 2048) is also defined here.

Architecturally, this namespace sits beneath the public‑facing protocol layer, insulating higher‑level code from the details of block construction, JSON handling, and structural validation. By exposing only opaque integer handles and simple status codes, it decouples the protocol’s internal representation from its consumers, allowing the implementation to evolve without affecting callers. The functions and variables listed work together to transform raw input parameters (strings, arrays, or tool identifiers) into the structured elements required by the Anthropic messaging format.

## Variables

### `clore::net::anthropic::protocol::detail::kDefaultMaxTokens`

Declaration: `src/network/anthropic.cppm:32`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

A compile-time constant of type `std::uint32_t` set to `2048`, representing the default maximum number of tokens for Anthropic API requests.

#### Usage Patterns

- default parameter value in request building

## Functions

### `clore::net::anthropic::protocol::detail::append_text_with_gap`

Declaration: `src/network/anthropic.cppm:34`

Definition: `src/network/anthropic.cppm:34`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Declaration: [Declaration](functions/append-text-with-gap.md)

Appends a text segment, provided as a `std::string_view`, to the given `std::string`, ensuring a gap (typically a newline) is inserted between any existing content and the newly appended text. By managing this separation, the function helps structure the output string so that logically distinct textual parts remain visually separated, which is useful when constructing multi‑line message bodies or JSON content. Callers supply the destination string and the text to append; after the call, the destination string contains the original content, a gap, and the appended text.

#### Usage Patterns

- called by `build_request_json` to concatenate text segments with separation

### `clore::net::anthropic::protocol::detail::format_schema_instruction`

Declaration: `src/network/anthropic.cppm:185`

Definition: `src/network/anthropic.cppm:185`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::format_schema_instruction` accepts a `const int &` representing a schema descriptor and returns an `int`. It is responsible for transforming the provided schema into a formatted instruction that conforms to the Anthropic protocol. The caller supplies a schema reference, and the function provides an integer result that can be used in subsequent protocol message construction, such as for tool use blocks or other structured requests.

#### Usage Patterns

- Used to generate the schema instruction part of a system prompt for Anthropic API requests

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `src/network/anthropic.cppm:139`

Definition: `src/network/anthropic.cppm:139`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Constructs a message with a specified role and text content for the Anthropic protocol request. Callers supply the role (e.g., `"user"` or `"assistant"`) and the text body as string views. The function returns an integer indicating success (typically zero) or a non‑zero error code on failure. It is intended for internal use within the protocol detail layer and assumes valid, non‑empty inputs.

#### Usage Patterns

- Constructing user or assistant messages from plain text
- Part of message building pipeline in Anthropic protocol

### `clore::net::anthropic::protocol::detail::make_role_message`

Declaration: `src/network/anthropic.cppm:163`

Definition: `src/network/anthropic.cppm:163`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

Constructs a protocol message with the specified role and content. The function accepts a role string (such as `"user"` or `"assistant"`) and either a `json::Array` of content blocks or a plain `std::string_view` text. It returns an opaque integer handle representing the newly created message object within the protocol's internal representation. The caller is responsible for providing a valid role identifier and ensuring that the content is well-formed according to the Anthropic message format; the function does not perform schema validation.

#### Usage Patterns

- Creating complete message objects for Anthropic API requests
- Combining a role with content blocks for dialogue messages

### `clore::net::anthropic::protocol::detail::make_text_block`

Declaration: `src/network/anthropic.cppm:44`

Definition: `src/network/anthropic.cppm:44`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The caller invokes `clore::net::anthropic::protocol::detail::make_text_block` to construct a text content block for use in the Claude API request protocol. It accepts a single `std::string_view` parameter containing the textual content and returns an `int` that acts as an opaque handle to the created block.

The caller is responsible for providing a valid string view whose lifetime extends until the returned handle is consumed. The function does not take ownership of the string data; the caller must ensure the referenced content remains valid for the duration of its use. The returned `int` should be passed to other protocol utility functions, such as `make_role_message`, to assemble the full message structure.

#### Usage Patterns

- used to construct content blocks for Anthropic API requests
- called by higher-level message-building functions

### `clore::net::anthropic::protocol::detail::make_tool_result_block`

Declaration: `src/network/anthropic.cppm:107`

Definition: `src/network/anthropic.cppm:107`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

This function constructs a tool result block within the Anthropic protocol, accepting a constant reference to an integer (presumably identifying a tool call or result index) and returning an integer representing the created block. It is intended for internal use by the protocol layer and is part of the building blocks for assembling structured tool‑related content in request or response messages.

#### Usage Patterns

- Called to build a tool result block for Anthropic API requests
- Used in higher-level protocol message construction

### `clore::net::anthropic::protocol::detail::make_tool_use_block`

Declaration: `src/network/anthropic.cppm:67`

Definition: `src/network/anthropic.cppm:67`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

This function constructs a tool-use content block from the provided tool call data. It accepts a reference to an integer representing the tool call and returns an integer that can be used to reference the resulting block. This function is intended for use in building Anthropic protocol messages; callers must supply a valid tool call identifier and should not rely on the integer value beyond the immediate construction context.

#### Usage Patterns

- Called to produce a JSON `tool_use` block from a parsed or provided `ToolCall`
- Used when constructing Anthropic API request messages

### `clore::net::anthropic::protocol::detail::parse_json_text`

Declaration: `src/network/anthropic.cppm:180`

Definition: `src/network/anthropic.cppm:180`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

This function parses a JSON text and returns an integer identifier. It is part of the protocol detail layer, used to interpret JSON input for the Anthropic API. The caller supplies two `std::string_view` arguments: the first is the JSON content to parse, and the second may provide additional context (such as a schema or key). The return value is an integer that can be used to reference the parsed construct in subsequent operations. The function assumes valid JSON input and does not handle malformed data gracefully.

#### Usage Patterns

- Delegates JSON parsing to `clore::net::detail::parse_json_object`
- Used by protocol layer to parse LLM responses or request bodies

### `clore::net::anthropic::protocol::detail::validate_request`

Declaration: `src/network/anthropic.cppm:202`

Definition: `src/network/anthropic.cppm:202`

Implementation: [`Module anthropic`](../../../../../../modules/anthropic/index.md)

The function `clore::net::anthropic::protocol::detail::validate_request` validates a given request and returns an integer status. The caller provides a reference to an integer representing the request to validate. The returned integer signals the validation outcome, typically zero for a valid request and a non‑zero error code otherwise.

#### Usage Patterns

- called before sending a completion request to ensure validity
- used in request preparation pipeline

## Related Pages

- [Namespace clore::net::anthropic::protocol](../index.md)

