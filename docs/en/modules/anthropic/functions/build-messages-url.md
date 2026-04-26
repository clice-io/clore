---
title: 'clore::net::anthropic::protocol::buildmessagesurl'
description: 'The function first normalizes the input api_base by removing any trailing forward slash characters. It then checks whether the resulting string already ends with the path segment /v1. If it does, the function delegates to clore::net::detail::append_url_path to directly append "messages" to the base URL. Otherwise, it appends the full path "v1/messages" using the same utility, ensuring the final URL correctly points to the Anthropic Messages API endpoint. The algorithm relies on clore::net::detail::append_url_path to handle path concatenation, which manages proper slash insertion between path components.'
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

The function first normalizes the input `api_base` by removing any trailing forward slash characters. It then checks whether the resulting string already ends with the path segment `/v1`. If it does, the function delegates to `clore::net::detail::append_url_path` to directly append `"messages"` to the base URL. Otherwise, it appends the full path `"v1/messages"` using the same utility, ensuring the final URL correctly points to the Anthropic Messages API endpoint. The algorithm relies on `clore::net::detail::append_url_path` to handle path concatenation, which manages proper slash insertion between path components.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `api_base` parameter

## Writes To

- returned `std::string`

## Usage Patterns

- Called by `clore::net::anthropic::detail::Protocol::build_url`

## Called By

- method `clore::net::anthropic::detail::Protocol::build_url`

