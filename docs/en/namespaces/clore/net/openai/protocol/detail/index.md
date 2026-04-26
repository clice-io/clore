---
title: 'Namespace clore::net::openai::protocol::detail'
description: 'The clore::net::openai::protocol::detail namespace contains internal implementation helpers that handle low-level serialization, parsing, and validation of OpenAI protocol elements. Notable declarations include functions for parsing tool calls and content parts, as well as serializing response formats, messages, tool choices, tool definitions, and validating requests. All functions operate on JSON objects and arrays, returning integer status codes to indicate success or specific error conditions.'
layout: doc
template: doc
---

# Namespace `clore::net::openai::protocol::detail`

## Summary

The `clore::net::openai::protocol::detail` namespace contains internal implementation helpers that handle low-level serialization, parsing, and validation of `OpenAI` protocol elements. Notable declarations include functions for parsing tool calls and content parts, as well as serializing response formats, messages, tool choices, tool definitions, and validating requests. All functions operate on JSON objects and arrays, returning integer status codes to indicate success or specific error conditions.

Architecturally, this namespace serves as a private implementation layer within the `OpenAI` protocol module. It encapsulates the JSON‑focused logic required to translate between the protocol’s abstract identifiers and the concrete JSON structures used in API communication. By isolating these details, the namespace prevents external code from depending directly on serialization internals and allows the higher‑level protocol interface to remain clean and focused on business logic.

## Functions

### `clore::net::openai::protocol::detail::parse_content_parts`

Declaration: `network/openai.cppm:288`

Definition: `network/openai.cppm:288`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::parse_content_parts` accepts a reference to a `const json::Array` and returns an `int`. It expects the caller to provide a JSON array containing content parts as defined by the `OpenAI` response schema. Successful parsing of the array is indicated by a non‑negative return value; a negative value signals an error or an unrecognized structure. This function is intended for internal use within the `OpenAI` protocol implementation and should not be called directly by external code. The caller is responsible for ensuring the input array is well‑formed according to the expected format.

#### Usage Patterns

- Called to parse the `content` array of an assistant message in the `OpenAI` protocol
- Used within message deserialization to convert raw JSON into domain types

### `clore::net::openai::protocol::detail::parse_tool_calls`

Declaration: `network/openai.cppm:369`

Definition: `network/openai.cppm:369`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::parse_tool_calls` accepts a constant reference to a `json::Array` and returns an `int`. It is responsible for parsing tool call representations from the provided JSON array according to the `OpenAI` protocol. The return value indicates the outcome of the parsing operation—typically the number of successfully parsed tool calls or an error code on failure. The caller must ensure the array is well-formed and that all required tool call fields are present; otherwise the behavior is unspecified.

#### Usage Patterns

- parse tool calls from `OpenAI` API response
- deserialize tool call array in protocol layer

### `clore::net::openai::protocol::detail::serialize_message`

Declaration: `network/openai.cppm:27`

Definition: `network/openai.cppm:27`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

Serializes an `OpenAI`‑protocol message into the provided JSON array. The caller passes a mutable `json::Array &` to receive the serialized representation and a `const int &` argument that identifies the message to be serialized. On success the function returns an integer indicating the outcome; a non‑zero value typically signals an error. The exact meaning of the return value is defined by the protocol layer and must be checked by the caller.

#### Usage Patterns

- called during request serialization to convert a `Message` variant to JSON
- used in constructing the messages array for `OpenAI` chat completions API

### `clore::net::openai::protocol::detail::serialize_response_format`

Declaration: `network/openai.cppm:209`

Definition: `network/openai.cppm:209`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::serialize_response_format` populates a given `json::Object` with the serialized representation of a response format identified by the provided integer. It is part of a family of detail‑level serialization helpers that prepare JSON request bodies for the `OpenAI` API. The caller is responsible for supplying a valid, mutable `json::Object` reference and a response format identifier that corresponds to one of the protocol‑defined formats. The function returns an integer whose value indicates success or an error condition; a non‑zero result typically signals that serialization could not be completed.

#### Usage Patterns

- Used in request serialization for `OpenAI` API calls.
- Called alongside `serialize_message`, `serialize_tool_choice`, etc. to build a full request body.

### `clore::net::openai::protocol::detail::serialize_tool_choice`

Declaration: `network/openai.cppm:167`

Definition: `network/openai.cppm:167`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::serialize_tool_choice` serializes a tool choice option into a JSON object. It accepts a reference to a `json::Object` representing a request payload and a `const int &` that identifies the tool choice selection (typically an enum or index). The caller must ensure that the provided integer corresponds to a valid tool choice and that the output object is mutable. The function returns an `int` indicating success or error status, enabling the caller to detect serialization failures.

#### Usage Patterns

- Called during `OpenAI` API request serialization to set the `tool_choice` field.
- Part of the `clore::net::openai::protocol::detail` namespace for protocol implementation.

### `clore::net::openai::protocol::detail::serialize_tool_definition`

Declaration: `network/openai.cppm:248`

Definition: `network/openai.cppm:248`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::serialize_tool_definition` serializes a tool definition, identified by a caller-supplied integer, into a JSON array. It expects a mutable `json::Array &` as the target container and a `const int &` that uniquely specifies which tool definition to encode. The return value of `int` indicates whether the serialization succeeded or failed, typically zero for success and a non‑zero error code otherwise. The caller is responsible for ensuring that the provided integer corresponds to a valid, previously registered tool definition and that the output array is ready to receive the serialized representation. This function is an internal detail of the `OpenAI` protocol layer and should not be invoked directly outside of the serialization pipeline.

#### Usage Patterns

- Serializes a single tool definition for inclusion in an `OpenAI` API request
- Called by higher-level serialization functions that build the tools array

### `clore::net::openai::protocol::detail::validate_request`

Declaration: `network/openai.cppm:23`

Definition: `network/openai.cppm:23`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::validate_request` validates a request object represented by its `const int &` parameter. It returns an `int` indicating the success or failure of the validation: a value of zero signifies a valid request, while a non-zero result encodes a specific validation error. The caller is responsible for providing a reference to an existing request; the function does not modify the input. The contract guarantees that the returned status code follows the error conventions defined by the protocol.

#### Usage Patterns

- Used to validate a `CompletionRequest` before processing

## Related Pages

- [Namespace clore::net::openai::protocol](../index.md)

