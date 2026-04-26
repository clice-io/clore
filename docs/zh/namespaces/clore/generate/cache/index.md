---
title: 'Namespace clore::generate::cache'
description: 'clore::generate::cache 命名空间负责实现生成过程的缓存子系统，通过文件系统持久化存储提示（prompt）与响应（response）的映射，以避免重复计算。其核心数据结构包括 CacheIndex（缓存索引）和 CacheError（错误类型），并围绕它们提供同步与异步操作：load_cache_index / load_cache_index_async 用于从指定路径加载索引，find_cached_response 在索引中查找缓存条目，save_cache_entry / save_cache_entry_async 将新条目持久化。此外，make_prompt_response_cache_key 生成唯一的缓存键，而 normalize_text_for_hashing 对文本进行规范化以保证键的一致性。这些功能共同构成了一个可复用的缓存层，降低了 clore::generate 中模型调用的开销。'
layout: doc
template: doc
---

# Namespace `clore::generate::cache`

## Summary

`clore::generate::cache` 命名空间负责实现生成过程的缓存子系统，通过文件系统持久化存储提示（prompt）与响应（response）的映射，以避免重复计算。其核心数据结构包括 `CacheIndex`（缓存索引）和 `CacheError`（错误类型），并围绕它们提供同步与异步操作：`load_cache_index` / `load_cache_index_async` 用于从指定路径加载索引，`find_cached_response` 在索引中查找缓存条目，`save_cache_entry` / `save_cache_entry_async` 将新条目持久化。此外，`make_prompt_response_cache_key` 生成唯一的缓存键，而 `normalize_text_for_hashing` 对文本进行规范化以保证键的一致性。这些功能共同构成了一个可复用的缓存层，降低了 `clore::generate` 中模型调用的开销。

## Types

### `clore::generate::cache::CacheError`

Declaration: `generate/cache.cppm:16`

Definition: `generate/cache.cppm:16`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

### `clore::generate::cache::CacheIndex`

Declaration: `generate/cache.cppm:20`

Definition: `generate/cache.cppm:20`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- The `entries` map contains string keys mapped to string values.
- Keys are expected to be unique within the map.

#### Key Members

- `entries`: the underlying `std::unordered_map<std::string, std::string>` that holds the cache index data.

#### Usage Patterns

- Used to look up cached results by key.
- Populated and queried by cache management functions.

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `generate/cache.cppm:35`

Definition: `generate/cache.cppm:347`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

函数 `clore::generate::cache::find_cached_response` 在给定的 `CacheIndex` 中查找与指定键关联的缓存响应。第一个参数是 `const CacheIndex &`，即缓存索引的引用；第二个参数是 `std::string_view`，表示要查找的键。如果找到对应的缓存条目，返回包含响应内容的 `std::optional<std::string_view>`；如果未找到，返回 `std::nullopt`。调用者必须确保传入的 `CacheIndex` 已通过 `load_cache_index` 或 `load_cache_index_async` 正确加载，且键已按照约定的格式（例如通过 `make_prompt_response_cache_key` 生成）提供。

#### Usage Patterns

- called to retrieve a previously cached generation response
- used in conjunction with `save_cache_entry` or similar caching functions

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

函数 `clore::generate::cache::load_cache_index` 负责从指定位置加载缓存索引。调用者传入一个 `std::string_view` 参数，该参数标识缓存索引的存储位置（例如文件路径）。函数返回 `std::expected<CacheIndex, CacheError>`：成功时得到一个 `CacheIndex` 对象，可用于后续的缓存查询（如 `clore::generate::cache::find_cached_response`）或更新（如 `clore::generate::cache::save_cache_entry`）；失败时则返回一个 `CacheError` 描述错误原因。调用者应确保提供的标识符有效，并处理可能的错误情形。

#### Usage Patterns

- Loading cache index for lookup
- Called before `find_cached_response`

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

该函数启动异步加载缓存索引的操作。调用者需提供一个文件路径（`std::string`）和一个 `kota::event_loop &`，该事件循环用于调度异步工作。返回值是一个 `int`，表示操作的结果状态或句柄，具体含义需结合调用上下文解读。

与同步版本 `load_cache_index` 不同，此函数不阻塞调用线程，而是将加载任务交给事件循环处理。调用者应确保在操作完成前保持 `kota::event_loop` 处于活跃状态。返回的整数可用于后续检查操作是否已成功完成，或与异步 I/O 机制对接。

#### Usage Patterns

- Called to obtain the cache index asynchronously before cache lookups or modifications
- Used in coroutine contexts that need to interact with the generation cache

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

根据提供的提示文本、响应文本和整数参数，生成一个可用于缓存查找或存储的唯一且确定的缓存键。该函数返回 `std::expected<std::string, CacheError>`，在成功时返回缓存键字符串，失败时返回 `CacheError` 以指示键生成过程中的错误。生成的键旨在与 `clore::generate::cache` 模块中的其他缓存操作（如 `find_cached_response` 或 `save_cache_entry`）配合使用。

#### Usage Patterns

- used to generate a unique cache key for storing or retrieving cached responses
- called by cache lookup and insertion functions

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Declaration: [Declaration](functions/normalize-text-for-hashing.md)

该函数接受一个 `std::string_view` 并返回一个 `std::string`，用于将输入文本规范化为可预测、可重复的格式，以便后续用于哈希运算。调用者可以期望：对于内容相同但细微差异（如空白字符、大小写或标点）的两个输入，函数能生成相同的规范化输出，从而确保哈希键的一致性。此函数是 `make_prompt_response_cache_key` 内部流程的一部分，用于生成缓存键的组件。

#### Usage Patterns

- called by `clore::generate::cache::make_prompt_response_cache_key` to normalize prompt or response text before hashing
- used to ensure consistent cache keys regardless of whitespace variation

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

调用者应当通过提供三个字符串视图参数来请求持久化一条缓存记录：第一个参数标识缓存键，第二个参数是要缓存的数据内容，第三个参数携带额外的上下文信息（例如对应的原始请求或规范化后的文本）。函数会同步执行写入操作并返回 `std::expected<void, CacheError>`：若成功则返回无值的预期，否则返回描述写入失败的 `CacheError`。调用者需要检查该返回值以确认操作是否成功，并据此决定后续流程。

#### Usage Patterns

- called after a successful generation to persist the response
- synchronous version of `save_cache_entry_async`
- invoked by higher-level generation routines

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::save_cache_entry_async` 接受三个 `std::string` 参数（分别代表缓存键、提示和响应内容）以及一个 `kota::event_loop &`，用于异步保存缓存条目。该函数启动一个异步操作，将提供的键值对写入缓存，并在事件循环上调度完成处理；返回一个 `int`，可能表示操作标识符或状态码。调用者应确保事件循环保持活动，直到操作完成，并通过异步回调或后续处理获取最终结果。

#### Usage Patterns

- Called to persist a generated response asynchronously
- Used after response generation to avoid blocking the main thread

## Related Pages

- [Namespace clore::generate](../index.md)

