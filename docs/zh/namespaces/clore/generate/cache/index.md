---
title: 'Namespace clore::generate::cache'
description: 'clore::generate::cache 命名空间实现了生成管线中的缓存子系统，其核心职责是存储和检索针对相同提示‑响应对的生成结果，以避免重复计算。它定义了 CacheIndex 与 CacheError 类型，并提供了一系列同步与异步的操作函数：load_cache_index 和 load_cache_index_async 从持久化存储加载索引；find_cached_response 在索引中快速查找已有响应；save_cache_entry 与 save_cache_entry_async 保存新条目；make_prompt_response_cache_key 和 normalize_text_for_hashing 则负责生成一致且可复现的缓存键，确保逻辑上等价的输入映射到相同的键。该命名空间作为数据流的中介层，将上层生成请求与底层缓存存储解耦，通过本地文件或数据库等方式实现持久化，从而显著提升重复请求的响应速度。'
layout: doc
template: doc
---

# Namespace `clore::generate::cache`

## Summary

`clore::generate::cache` 命名空间实现了生成管线中的缓存子系统，其核心职责是存储和检索针对相同提示‑响应对的生成结果，以避免重复计算。它定义了 `CacheIndex` 与 `CacheError` 类型，并提供了一系列同步与异步的操作函数：`load_cache_index` 和 `load_cache_index_async` 从持久化存储加载索引；`find_cached_response` 在索引中快速查找已有响应；`save_cache_entry` 与 `save_cache_entry_async` 保存新条目；`make_prompt_response_cache_key` 和 `normalize_text_for_hashing` 则负责生成一致且可复现的缓存键，确保逻辑上等价的输入映射到相同的键。该命名空间作为数据流的中介层，将上层生成请求与底层缓存存储解耦，通过本地文件或数据库等方式实现持久化，从而显著提升重复请求的响应速度。

## Types

### `clore::generate::cache::CacheError`

Declaration: `generate/cache.cppm:16`

Definition: `generate/cache.cppm:16`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `message` 可以是任意字符串，包括空字符串
- 没有额外的不变约束

#### Key Members

- `message`：错误消息字符串

#### Usage Patterns

- 可能作为异常对象或函数返回结果中的错误信息载体
- 被其他代码直接读取 `message` 以获取错误描述

### `clore::generate::cache::CacheIndex`

Declaration: `generate/cache.cppm:20`

Definition: `generate/cache.cppm:20`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- `entries` 映射中的键和值均为字符串类型，无其他约束。
- 映射的键唯一，符合 `std::unordered_map` 的语义。

#### Key Members

- `entries`

#### Usage Patterns

- 其他代码通过访问 `entries` 成员来查询或更新缓存映射。
- 可能用于缓存查找、插入或删除操作。

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `generate/cache.cppm:35`

Definition: `generate/cache.cppm:347`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

函数 `clore::generate::cache::find_cached_response` 在给定的 `CacheIndex` 中查找与指定 `std::string_view` 键匹配的缓存响应。调用者提供一个 `const CacheIndex &` 引用和键；如果存在响应，返回包含内容的 `std::optional<std::string_view>`，否则返回 `std::nullopt`。该函数不会修改缓存索引，且调用方应保证键是有效的缓存键（例如通过 `make_prompt_response_cache_key` 生成）。

#### Usage Patterns

- Check if a cached response exists for a given key
- Retrieve a cached response string by its cache key

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::load_cache_index` 同步地从指定的缓存存储路径加载整个缓存索引。调用者传入一个 `std::string_view` 作为缓存数据源的标识符（例如文件路径或数据库键），函数返回一个 `std::expected<CacheIndex, CacheError>`。若加载成功，返回的 `CacheIndex` 表示当前缓存中的所有条目，可后续传递给 `find_cached_response` 等函数进行检索；若失败，则返回描述失败原因的 `CacheError`。调用者负责确保在首次执行任何缓存查询或写入之前正确调用此函数以初始化索引状态。

#### Usage Patterns

- 用于同步加载缓存索引，通常与其他缓存操作（如 `find_cached_response`）配合使用
- 在需要一次性加载所有缓存条目以进行后续查找的场景中调用

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

函数 `clore::generate::cache::load_cache_index_async` 异步加载缓存索引，允许调用者在等待完成的同时继续执行其他任务。它接受一个表示索引文件路径的 `std::string` 和一个引用类型的 `kota::event_loop &`，该事件循环负责驱动异步操作。函数立即返回一个 `int`，调用者应检查此值以确定操作是否成功（通常 0 表示成功，非零表示错误）。成功加载后，索引将可用，后续可以通过 `clore::generate::cache::find_cached_response` 等函数进行查找。调用者有责任确保提供的 `event_loop` 在操作完成前保持有效且正在运行。

#### Usage Patterns

- Called asynchronously to obtain the prompt response cache index
- Used by higher‑level async cache operations

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

调用者必须提供提示文本、响应文本以及一个整数标识符（可能表示提示‑响应对的版本或类型）。`clore::generate::cache::make_prompt_response_cache_key` 根据这些输入计算并返回一个唯一缓存键字符串；若无法生成键则返回 `CacheError`。返回的键可直接用于 `find_cached_response`、`save_cache_entry` 等缓存操作。调用者应确保输入字符串不为空，整数参数合理有效，否则函数可能返回错误。

#### Usage Patterns

- Generate a cache key for storing or retrieving prompt responses
- Used internally by `save_cache_entry` and `find_cached_response` like functions

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Declaration: [Declaration](functions/normalize-text-for-hashing.md)

函数 `clore::generate::cache::normalize_text_for_hashing` 将输入的文本转换为适合哈希运算的规范化形式。它接受一个 `std::string_view` 并返回一个 `std::string`；调用者应传入需要保证哈希一致性的文本，例如在构造缓存键之前对提示或响应文本进行预处理。规范化后的输出是确定性的，对于逻辑上等价的不同输入（例如忽略大小写或空白差异），会生成相同的字符串，从而确保缓存查找的正确性。该函数不涉及外部状态，也不抛出异常，适用于性能敏感的路径。

#### Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize input strings before combining into a cache key.

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::save_cache_entry` 将一条缓存条目持久化到存储中。调用者必须提供三个 `std::string_view` 参数，它们共同标识并构成该缓存条目。如果写入成功，函数返回一个空的 `std::expected<void, CacheError>`；否则返回一个描述失败原因的 `CacheError` 值。

#### Usage Patterns

- Synchronous caching of prompt-response pairs
- Called when immediate persistence is required
- Complemented by `save_cache_entry_async` for non-blocking usage

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

函数 `clore::generate::cache::save_cache_entry_async` 是同步函数 `clore::generate::cache::save_cache_entry` 的非阻塞异步对应物。它接受三个 `std::string` 参数（分别代表缓存键和关联数据）以及一个 `kota::event_loop &` 引用，在给定的事件循环上调度缓存条目的写入操作。调用者必须确保所提供的字符串和事件循环在操作完成前保持有效。该函数返回一个 `int`，表示异步操作标识符，可用于追踪操作状态或执行后续查询。

#### Usage Patterns

- called after generating a response to asynchronously persist the cache entry
- used in coroutine-based asynchronous workflows with `kota::event_loop`

## Related Pages

- [Namespace clore::generate](../index.md)

