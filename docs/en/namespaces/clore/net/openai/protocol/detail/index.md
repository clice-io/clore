---
title: 'Namespace clore::net::openai::protocol::detail'
description: 'The clore::net::openai::protocol::detail namespace contains the internal, low‑level helpers for serializing and parsing OpenAI‑protocol data structures. Functions such as serialize_tool_definition, serialize_tool_choice, serialize_message, and serialize_response_format write protocol elements into caller‑supplied JSON objects or arrays, returning an integral status code to indicate success or failure. Corresponding parsing functions—parse_tool_calls, parse_content_parts, and validate_request—convert JSON input into internal representations and check request validity. These functions are not part of the public API; they form the foundation on which higher‑level protocol abstractions are built, enforcing consistent error‑reporting conventions and insulating external code from implementation details.'
layout: doc
template: doc
---

# Namespace `clore::net::openai::protocol::detail`

## Summary

The `clore::net::openai::protocol::detail` namespace contains the internal, low‑level helpers for serializing and parsing `OpenAI`‑protocol data structures. Functions such as `serialize_tool_definition`, `serialize_tool_choice`, `serialize_message`, and `serialize_response_format` write protocol elements into caller‑supplied JSON objects or arrays, returning an integral status code to indicate success or failure. Corresponding parsing functions—`parse_tool_calls`, `parse_content_parts`, and `validate_request`—convert JSON input into internal representations and check request validity. These functions are not part of the public API; they form the foundation on which higher‑level protocol abstractions are built, enforcing consistent error‑reporting conventions and insulating external code from implementation details.

## Functions

### `clore::net::openai::protocol::detail::parse_content_parts`

Declaration: `network/openai.cppm:288`

Definition: `network/openai.cppm:288`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::parse_content_parts` accepts a constant reference to a `json::Array` and returns an `int` status code. It is responsible for parsing the content parts embedded in a message within the `OpenAI` protocol, converting the JSON array into the internal representation used by the library. The caller must supply a valid JSON array that conforms to the expected schema; the return value indicates whether parsing completed successfully, with a non‑zero value signalling an error. This function is part of the internal protocol‑handling layer and is not intended for direct use by application code.

#### Usage Patterns

- Called to deserialize the content field from an `OpenAI` chat completion response
- Used in protocol layer to convert JSON content array into structured `AssistantOutput`
- Part of the `clore::net::openai::protocol::detail` parsing pipeline

### `clore::net::openai::protocol::detail::parse_tool_calls`

Declaration: `network/openai.cppm:369`

Definition: `network/openai.cppm:369`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::parse_tool_calls` accepts a constant reference to a `json::Array` that is expected to represent a sequence of tool call objects as defined by the `OpenAI` protocol. It parses this array and returns an `int` that communicates the result of the operation—typically a count of successfully parsed calls or a status code indicating success or failure. The caller is responsible for providing a valid JSON array conforming to the expected schema; the function does not throw exceptions and reports any issues solely through its return value.

#### Usage Patterns

- parse tool calls from a chat completion response
- extract and validate tool call objects from a raw JSON array
- convert JSON tool call representation to structured `ToolCall` instances

### `clore::net::openai::protocol::detail::serialize_message`

Declaration: `network/openai.cppm:27`

Definition: `network/openai.cppm:27`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::serialize_message` serializes a message identified by the given integer reference into the provided `json::Array`. The caller supplies a mutable `json::Array` that will be appended with the serialized message representation, and a `const int &` that identifies which message to serialize. The function returns an integral value indicating success (typically zero) or a non‑zero error code. This function is an internal protocol helper; it assumes that the referenced message data is valid and accessible.

#### Usage Patterns

- Used to convert a single message into JSON for inclusion in an `OpenAI` API request payload
- Called during serialization of a conversation history to a JSON array of messages

### `clore::net::openai::protocol::detail::serialize_response_format`

Declaration: `network/openai.cppm:209`

Definition: `network/openai.cppm:209`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::serialize_response_format` writes the response format configuration into a caller‑supplied `json::Object`. The caller provides an integer parameter that selects the desired response format type; the function populates the object accordingly. The return value indicates whether serialization succeeded—zero typically denotes success, while a non‑zero value signals an invalid format identifier or an internal error. The caller must ensure that the integer argument corresponds to one of the recognized response format types defined by the protocol.

#### Usage Patterns

- called during construction of an `OpenAI` chat completion request containing a response format
- invoked within the protocol serialization pipeline for request building

### `clore::net::openai::protocol::detail::serialize_tool_choice`

Declaration: `network/openai.cppm:167`

Definition: `network/openai.cppm:167`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

`clore::net::openai::protocol::detail::serialize_tool_choice` accepts a mutable `json::Object` reference and a `const int` reference representing the tool‑choice configuration. It serialises the given tool choice into the supplied JSON object, preparing the object for inclusion in an `OpenAI` protocol request. The function returns an `int` to indicate the outcome of the serialisation; a caller should treat a non‑zero result as an error. This function is intended for use within the protocol‑detail layer and is not part of the public API.

#### Usage Patterns

- Used during serialization of `OpenAI` chat completion requests to encode the `tool_choice` field.
- Called by higher-level serialization functions that build the JSON payload.

### `clore::net::openai::protocol::detail::serialize_tool_definition`

Declaration: `network/openai.cppm:248`

Definition: `network/openai.cppm:248`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The caller‑facing function `clore::net::openai::protocol::detail::serialize_tool_definition` serializes a single tool definition into the supplied `json::Array`. The `const int &` parameter identifies the specific tool definition to serialize—likely an index or a resource handle. The function appends the serialized representation (typically a JSON object) to the given array. It returns an `int` that indicates success (zero) or a non‑zero error code on failure, consistent with the error‑reporting convention used by other serialization helpers in the same module. The caller must ensure the `json::Array` reference is valid and that the integer argument corresponds to an existing, well‑formed tool definition.

#### Usage Patterns

- called during serialization of a list of tool definitions in an `OpenAI` API request
- used by functions that construct the request body before sending to the API

### `clore::net::openai::protocol::detail::validate_request`

Declaration: `network/openai.cppm:23`

Definition: `network/openai.cppm:23`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::validate_request` verifies that a request object provided as a `const int &` conforms to the expected protocol constraints. It returns an `int` status code: a value of zero signals a valid request, while non-zero values indicate specific validation failures that the caller must handle. Callers are responsible for constructing the request correctly before calling this function, and for relying on the returned status to decide whether the request can proceed through the pipeline.

#### Usage Patterns

- Called to validate a completion request before using it in a protocol operation

## Related Pages

- [Namespace clore::net::openai::protocol](../index.md)

