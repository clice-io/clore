---
title: 'Namespace clore::net::openai::protocol::detail'
description: 'The clore::net::openai::protocol::detail namespace contains internal implementation helpers for the OpenAI protocol layer. Its declarations are primarily serialization and parsing functions, such as serialize_tool_choice, serialize_tool_definition, serialize_message, serialize_response_format, parse_tool_calls, parse_content_parts, and validate_request. Each function operates on JSON types (e.g., json::Object, json::Array) and returns an integer status code to indicate success or failure, enabling consistent error handling within the protocol.'
layout: doc
template: doc
---

# Namespace `clore::net::openai::protocol::detail`

## Summary

The `clore::net::openai::protocol::detail` namespace contains internal implementation helpers for the `OpenAI` protocol layer. Its declarations are primarily serialization and parsing functions, such as `serialize_tool_choice`, `serialize_tool_definition`, `serialize_message`, `serialize_response_format`, `parse_tool_calls`, `parse_content_parts`, and `validate_request`. Each function operates on JSON types (e.g., `json::Object`, `json::Array`) and returns an integer status code to indicate success or failure, enabling consistent error handling within the protocol.

Architecturally, this namespace serves as a low-level implementation bridge between internal data models and the JSON wire format expected by the `OpenAI` API. It is not part of the public API; instead, it is used by request-building and response‑parsing logic to convert between C++ representations and JSON objects or arrays. The `validate_request` function checks request integrity before serialization, while the serialization and parsing routines handle specific protocol elements such as tool calls, content parts, and response formats, ensuring correct formatting and extraction of data.

## Functions

### `clore::net::openai::protocol::detail::parse_content_parts`

Declaration: `src/network/openai.cppm:298`

Definition: `src/network/openai.cppm:298`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::parse_content_parts` accepts a `const json::Array &` and returns an `int`. It is responsible for parsing the content parts of a message from the given JSON array, as part of the `OpenAI` protocol implementation. Callers can rely on this function to interpret the array and produce an integer result that indicates the success or the number of parts processed, forming a foundational step in message deserialization.

#### Usage Patterns

- Used internally to convert API response content parts
- Called when processing assistant messages

### `clore::net::openai::protocol::detail::parse_tool_calls`

Declaration: `src/network/openai.cppm:379`

Definition: `src/network/openai.cppm:379`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

This function parses a JSON array representing tool calls from an `OpenAI` response. It is the caller’s responsibility to ensure the JSON array is well-formed and semantically valid according to the `OpenAI` protocol specification. The function processes the array and returns an integer result that indicates the outcome of the parse—typically a success or error code. The caller must interpret the return value to determine whether the tool calls were successfully recognized and to possibly retrieve the parsed data through subsequent protocol steps.

#### Usage Patterns

- called to convert raw tool call JSON from API responses into structured data

### `clore::net::openai::protocol::detail::serialize_message`

Declaration: `src/network/openai.cppm:37`

Definition: `src/network/openai.cppm:37`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::serialize_message` is a caller-facing serialization helper that writes a representation of a message into the supplied `json::Array` output container. It accepts a `const int &` parameter, which may denote a message index, type, or other identifier, and returns an `int` that signals success or a status code. This function belongs to the internal protocol serialization layer and is intended to be used by request-building logic to convert a message into the JSON array format required by the `OpenAI` API.

#### Usage Patterns

- Called when building the JSON payload for `OpenAI` chat completion requests
- Used within serialization routines for different message types

### `clore::net::openai::protocol::detail::serialize_response_format`

Declaration: `src/network/openai.cppm:219`

Definition: `src/network/openai.cppm:219`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

Serializes a response format into the provided JSON object. The caller must supply a mutable `json::Object &` and a response format identifier as a `const int &`. Returns an integer status indicating success or failure of the serialization operation.

#### Usage Patterns

- Called when constructing a request body for `OpenAI` API calls
- Part of serialization pipeline for protocol parameters

### `clore::net::openai::protocol::detail::serialize_tool_choice`

Declaration: `src/network/openai.cppm:177`

Definition: `src/network/openai.cppm:177`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::serialize_tool_choice` is responsible for serializing a tool‑choice value into a JSON object. It accepts a mutable `json::Object &` to write into and a `const int &` that identifies the tool choice to serialize. The return value is an integer status indicating whether the operation succeeded. Callers must provide a valid JSON object and a recognized tool‑choice identifier; the function will populate the object with the corresponding representation used in `OpenAI` protocol requests.

#### Usage Patterns

- Called during serialization of an `OpenAI` chat completion request to set the `tool_choice` field.
- Part of `detail` namespace serialization utilities for protocol handling.

### `clore::net::openai::protocol::detail::serialize_tool_definition`

Declaration: `src/network/openai.cppm:258`

Definition: `src/network/openai.cppm:258`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

This serialization function accepts a `json::Array` reference and an integer identifier for a tool definition. It writes the serialized representation of the specified tool definition into the provided array. The function returns an integer status code so that the caller can determine whether the serialization succeeded or failed. The caller is responsible for passing a valid array and a tool definition identifier that exists within the protocol context.

#### Usage Patterns

- Called when constructing the JSON payload for an `OpenAI` chat request that includes tool definitions
- Used in serialization pipelines for `OpenAI` protocol

### `clore::net::openai::protocol::detail::validate_request`

Declaration: `src/network/openai.cppm:33`

Definition: `src/network/openai.cppm:33`

Implementation: [`Module openai`](../../../../../../modules/openai/index.md)

The function `clore::net::openai::protocol::detail::validate_request` validates a request object (provided as a reference to an integer) and returns an integer status code. Callers must check the return value to determine whether the request is acceptable: a value of zero indicates success, while a non‑zero value signals a specific validation failure. This function is intended for use within the protocol implementation layer and is not part of the public API.

#### Usage Patterns

- called to validate a completion request before further processing in the `OpenAI` protocol layer

## Related Pages

- [Namespace clore::net::openai::protocol](../index.md)

