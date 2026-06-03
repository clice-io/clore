---
title: 'clore::net::anthropic::protocol::buildmessagesurl'
description: 'The function first copies the input api_base into a local std::string and then strips any trailing forward slashes by repeatedly calling pop_back() as long as the string is non‑empty and ends with ''/''. After normalisation, it checks whether the resulting URL ends with the literal path "/v1" using ends_with. If it does, it delegates to clore::net::detail::append_url_path, appending only "messages" to the base. Otherwise it appends the full path "v1/messages" via the same helper. This ensures that the caller‑supplied base URL is correctly normalised before the Anthropic Messages API endpoint path is appended, avoiding duplicate "v1" segments when the base already includes the API version.'
layout: doc
template: doc
---

# `clore::net::anthropic::protocol::build_messages_url`

Owner: [Module anthropic](../index.md)

Declaration: `src/network/anthropic.cppm:210`

Definition: `src/network/anthropic.cppm:233`

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

The function first copies the input `api_base` into a local `std::string` and then strips any trailing forward slashes by repeatedly calling `pop_back()` as long as the string is non‑empty and ends with `'/'`. After normalisation, it checks whether the resulting URL ends with the literal path `"/v1"` using `ends_with`. If it does, it delegates to `clore::net::detail::append_url_path`, appending only `"messages"` to the base. Otherwise it appends the full path `"v1/messages"` via the same helper. This ensures that the caller‑supplied base URL is correctly normalised before the Anthropic Messages API endpoint path is appended, avoiding duplicate `"v1"` segments when the base already includes the API version.

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- param `api_base`

## Usage Patterns

- called by `Protocol::build_url` to generate the final URL endpoint

## Called By

- method `clore::net::anthropic::detail::Protocol::build_url`

