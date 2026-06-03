---
title: 'Network Module Deep Dive'
description: 'Guide: Network Module Deep Dive'
layout: doc
template: doc
---

# Network Module Deep Dive

**Namespace**: `clore::net`  
**Source files**: `src/network/*.cppm`  
**Purpose**: Abstract LLM provider interface, HTTP transport, rate limiting, protocol handling, and schema validation.

## Architecture

The network module is the "AI backend" of Clore. It provides a clean abstraction over different LLM providers (Anthropic Claude, `OpenAI`) while handling the complex details of HTTP communication, capability probing, and response parsing.

```
PromptRequest (from generate)
    ↓
┌──────────────────────────┐
│  network.cppm            │  Top-level API: validate environment,
│  call_llm_async()        │  detect provider, dispatch
│  call_completion_async() │
└──────────────────────────┘
    ↓
┌──────────────────────────┐
│  client.cppm             │  Provider-agnostic client: capability probing,
│  call_completion_async() │  request sanitization, retry logic
│  <Protocol>              │
└──────────────────────────┘
    ↓
┌──────────────────────────┐
│  protocol.cppm           │  Request/response protocol, data structures,
│                          │  output contract inference, validation
└──────────────────────────┘
    ↓
┌──────────────────────────┐
│  http.cppm               │  HTTP transport layer, rate limiting
│  perform_http_request()  │  via token-bucket semaphore
└──────────────────────────┘
    ↓
┌─────────────────────┐  ┌─────────────────────┐
│  anthropic.cppm     │  │  openai.cppm         │  Provider implementations
│  Protocol concept   │  │  Protocol concept    │
└─────────────────────┘  └─────────────────────┘
    ↓                          ↓
┌──────────────────────────┐
│  schema.cppm             │  JSON schema validation for
│                          │  response format and tools
└──────────────────────────┘
```

## Key Components

### `network.cppm` — High-Level API

**Key functions:**
- `validate_llm_provider_environment()` → Checks that required environment variables are set (e.g., `ANTHROPIC_API_KEY`, `OPENAI_API_KEY`)
- `call_llm_async(system_prompt, prompt, model, loop)` → Top-level LLM call. Detects the active provider from environment, then dispatches to the correct implementation
- `call_completion_async(request, loop)` → Completion request dispatch

Provider detection logic in `provider.cppm`:
- Checks for `ANTHROPIC_API_KEY` → uses Anthropic provider
- Checks for `OPENAI_API_KEY` → uses `OpenAI` provider
- Falls back with an error if neither is configured

### `client.cppm` — Provider-Agnostic Client

This is a template function that implements the core LLM calling loop for any provider:

```cpp
template <typename Protocol>
auto call_completion_async(CompletionRequest, event_loop*)
    -> task<CompletionResponse, LLMError>;
```

**Key behaviors:**
1. **Capability probing**: Reads/creates `ProbedCapabilities` for the provider+model combination
2. **Request sanitization**: Removes features (tools, structured output, etc.) that the provider doesn't support
3. **Request building**: Delegates JSON construction to `Protocol::build_request_json()`
4. **HTTP call**: Delegates to `perform_http_request_async()`
5. **Error handling**: Detects feature rejection errors (40x responses) and downgrades capabilities
6. **Retry logic**: Retries up to 4 times with progressively reduced features on feature rejection

The capability probe cache (`get_probed_capabilities()`) stores per-provider, per-model capability flags in a thread-safe map, using atomic booleans for lock-free reads.

### `protocol.cppm` — Data Structures and Protocol

**Key types:**

**Messages:**
- `SystemMessage` — System prompt content
- `UserMessage` — User prompt content
- `AssistantMessage` — Assistant text response
- `AssistantToolCallMessage` — Assistant response with tool calls
- `ToolResultMessage` — Tool execution result
- `Message` — Variant of all message types

**Requests and Responses:**
- `PromptRequest` — High-level prompt (prompt text, response format, tool choice, output contract)
- `CompletionRequest` — Full API request (model, messages, tools, response format, parallel tool calls)
- `CompletionResponse` — Full API response (id, model, message, raw JSON)
- `AssistantOutput` — Extracted assistant output (text, refusal, tool calls)
- `ToolCall` — Tool invocation (id, name, arguments)

**Configuration:**
- `ToolChoiceAuto`, `ToolChoiceNone`, `ToolChoiceRequired`, `ForcedFunctionToolChoice` — Tool choice modes
- `FunctionToolDefinition` — Tool definition (name, description, parameters, strict mode)
- `ResponseFormat` — Structured output format (name, schema, strict flag)
- `ProbedCapabilities` — Feature flags per provider/model (supports JSON schema, tool choice, parallel tool calls, tools)

**Enums:**
- `PromptOutputContract` — `Unspecified`, `Json`, `Markdown`

**Key functions:**
- `make_markdown_fragment_request(prompt)` → Creates a `PromptRequest` for markdown output
- `sanitize_request_for_capabilities(request, caps)` → Removes unsupported features from a request
- `get_probed_capabilities(model_key)` → Thread-safe capability lookup/creation
- `make_capability_probe_key(provider, model, api_base)` → Builds unique cache key for capabilities
- `is_feature_rejection_error(body)` → Detects if a 4xx response indicates an unsupported feature
- `parse_rejected_feature_from_error(body)` → Extracts the rejected feature name from error JSON
- `infer_output_contract(request)` → Determines what output format to expect based on the request
- `validate_prompt_output(text, contract)` → Validates that the LLM output matches the expected contract

### `http.cppm` — HTTP Transport and Rate Limiting

**Key types:**
- `LLMError` — Error type with string message, constructible from `std::string` or `kota::error`
- `EnvironmentConfig` — API key + base URL
- `RawHttpResponse` — HTTP status + body string

**Key functions:**
- `initialize_llm_rate_limit(requests_per_minute)` → Configures the token-bucket rate limiter
- `shutdown_llm_rate_limit()` → Cleans up rate limiter resources
- `perform_http_request(url, method, body)` → Synchronous HTTP request
- `perform_http_request_async(url, method, body, loop)` → Asynchronous HTTP request with rate limiting
- `configure_request(request, timeout_ms, body)` → Configures HTTP request with headers and timeout
- `read_environment(api_key_env, base_url_env)` → Reads credentials from environment variables
- `read_required_env(name)` → Reads a required environment variable

**Rate limiting:**
- Uses a global semaphore (`g_llm_semaphore`) protected by a mutex
- Configurable requests-per-minute limit
- A `SemaphoreGuard` RAII class manages semaphore acquisition/release
- Rate limit applies across all providers and models

**Connection management:**
- Configurable timeouts: connect (5s), request (120s)
- TCP keepalive: idle 60s, interval 10s, probes 3
- DNS cache: 300s TTL
- Connection max age: 120s

### `anthropic.cppm` — Anthropic Claude Provider

Implements the `Protocol` concept for Anthropic's API:
- Environment: `ANTHROPIC_API_KEY`, `ANTHROPIC_BASE_URL` (optional)
- URL: `https://api.anthropic.com/v1/messages`
- Request format: Anthropic's messages API with system prompt as a separate field
- Response parsing: Extracts text, tool calls, and refusal from Anthropic's response format
- Headers: `x-api-key`, `anthropic-version: 2023-06-01`

### `openai.cppm` — `OpenAI` Provider

Implements the `Protocol` concept for `OpenAI`'s API:
- Environment: `OPENAI_API_KEY`, `OPENAI_BASE_URL` (optional)
- URL: `https://api.openai.com/v1/chat/completions`
- Request format: `OpenAI`'s chat completions API with system message
- Response parsing: Extracts text, tool calls (function calling), and refusal from `OpenAI`'s format
- Headers: `Authorization: Bearer <key>`

### `schema.cppm` — JSON Schema Validation

**Key functions:**
- `validate_response_format(format)` → Validates a response format schema definition
- `validate_tool_definition(tool)` → Validates a tool's JSON schema parameters
- `serialize_tool_arguments(args, schema_name)` → Serializes tool arguments according to the schema

## Capability Probing Flow

```
1. First request to a provider+model combination
2. Build capability probe key: "{provider}:{model}:{base_url}"
3. Initialize ProbedCapabilities with all features enabled
4. Send request
5. If 4xx error with feature rejection:
   a. Parse the rejected feature name
   b. Disable that feature in ProbedCapabilities (atomic store)
   c. Retry with sanitized request
6. Cache the final capabilities for future requests
```

This adaptive approach ensures graceful degradation when providers don't support structured output, tool choice, or parallel tool calls.

## Provider Implementation Requirements

To add a new provider, implement these operations:

| Operation | Description |
|-----------|-------------|
| `provider_name()` | Human-readable name for logging |
| `read_environment()` | Read API key and base URL from env |
| `build_url(env)` | Construct the API endpoint URL |
| `build_headers(env)` | Construct HTTP headers |
| `build_request_json(request)` | Serialize request to JSON |
| `parse_response(raw)` | Parse JSON response to `CompletionResponse` |
| `capability_probe_key(env, request)` | Build unique key for capability caching |

