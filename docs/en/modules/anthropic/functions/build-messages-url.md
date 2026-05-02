---
title: 'clore::net::anthropic::protocol::buildmessagesurl'
description: 'clore::net::anthropic::protocol::build_messages_url normalises the provided api_base string by stripping trailing forward slashes, then determines the correct path to append for the Anthropic messages endpoint. If the cleaned base already ends with "/v1", it appends the literal "messages" via clore::net::detail::append_url_path; otherwise it appends "v1/messages". This logic ensures the resulting URL always points to the standard Anthropic messages API path regardless of whether the caller supplies a base URL that includes the version segment. The function depends solely on clore::net::detail::append_url_path (a generic path‑appending utility) and performs no network or I/O operations itself.'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::build_messages_url`

Owner: [Module anthropic](../index.md)

Declaration: `network/anthropic.cppm:201`

Definition: `network/anthropic.cppm:224`

Declaration: [`Namespace clore::net::anthropic::protocol`](../../../namespaces/clore/net/anthropic/protocol/index.md)

## Implementation

```cpp
auto build_messages_url(std::string_view api_base) -> std::string {
    std::string url(api_base);
    while(!url.empty() && url.back() == '/') {
        url.pop_back();
    }
    if(url.ends_with("/v1")) {
        return clore::net::detail::append_url_path(url, "messages");
    }
    return clore::net::detail::append_url_path(url, "v1/messages");
}
```

`clore::net::anthropic::protocol::build_messages_url` normalises the provided `api_base` string by stripping trailing forward slashes, then determines the correct path to append for the Anthropic messages endpoint. If the cleaned base already ends with `"/v1"`, it appends the literal `"messages"` via `clore::net::detail::append_url_path`; otherwise it appends `"v1/messages"`. This logic ensures the resulting URL always points to the standard Anthropic `messages` API path regardless of whether the caller supplies a base URL that includes the version segment. The function depends solely on `clore::net::detail::append_url_path` (a generic path‑appending utility) and performs no network or I/O operations itself.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- parameter `api_base`

## Usage Patterns

- called by `clore::net::anthropic::detail::Protocol::build_url` to produce the messages endpoint URL

## Called By

- method `clore::net::anthropic::detail::Protocol::build_url`

