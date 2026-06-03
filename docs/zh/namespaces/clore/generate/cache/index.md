---
title: 'Namespace clore::generate::cache'
description: '该命名空间负责为生成式响应提供缓存功能，旨在避免重复计算并加速相同输入下的结果获取。其核心声明包括缓存键生成函数 make_prompt_response_cache_key、文本规范化函数 normalize_text_for_hashing、缓存索引的同步/异步加载（load_cache_index、load_cache_index_async）、缓存条目的同步/异步保存（save_cache_entry、save_cache_entry_async）以及基于索引的查找函数 find_cached_response。架构上，这些函数协同工作：通过规范化用户提示和系统提示生成稳定唯一键，将响应结果以 CacheIndex 结构组织并持久化，并借助 kota::event_loop 支持非阻塞操作，同时使用自定义 CacheError 类型统一错误处理。该命名空间作为生成管道的中间层，为上层调用者屏蔽了缓存的具体实现细节，确保了缓存操作的可靠性、一致性和异步支持。'
layout: doc
template: doc
---

# Namespace `clore::generate::cache`

## Summary

该命名空间负责为生成式响应提供缓存功能，旨在避免重复计算并加速相同输入下的结果获取。其核心声明包括缓存键生成函数 `make_prompt_response_cache_key`、文本规范化函数 `normalize_text_for_hashing`、缓存索引的同步/异步加载（`load_cache_index`、`load_cache_index_async`）、缓存条目的同步/异步保存（`save_cache_entry`、`save_cache_entry_async`）以及基于索引的查找函数 `find_cached_response`。架构上，这些函数协同工作：通过规范化用户提示和系统提示生成稳定唯一键，将响应结果以 `CacheIndex` 结构组织并持久化，并借助 `kota::event_loop` 支持非阻塞操作，同时使用自定义 `CacheError` 类型统一错误处理。该命名空间作为生成管道的中间层，为上层调用者屏蔽了缓存的具体实现细节，确保了缓存操作的可靠性、一致性和异步支持。

## Types

### `clore::generate::cache::CacheError`

Declaration: `src/generate/cache.cppm:35`

Definition: `src/generate/cache.cppm:35`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Key Members

- 成员 `message`：存储错误描述文本。

### `clore::generate::cache::CacheIndex`

Declaration: `src/generate/cache.cppm:39`

Definition: `src/generate/cache.cppm:39`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Insufficient evidence to summarize; provide more EVIDENCE.

#### Invariants

- Keys in `entries` are unique per instance.
- No ordering of elements is guaranteed.
- Both keys and values are mutable after construction.

#### Key Members

- `entries` member of type `std::unordered_map<std::string, std::string>`

#### Usage Patterns

- Accessed and modified via the `entries` member.
- Likely used as a cache container for generated output or intermediate data.
- No special methods or inheritance observed.

## Functions

### `clore::generate::cache::find_cached_response`

Declaration: `src/generate/cache.cppm:54`

Definition: `src/generate/cache.cppm:366`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

函数 `clore::generate::cache::find_cached_response` 接受一个 `const CacheIndex &` 和一个 `std::string_view` 作为键，在缓存索引中查找匹配的记录。若找到，返回对应的缓存响应内容，类型为 `std::optional<std::string_view>`；否则返回 `std::nullopt`。调用者必须确保传入的 `CacheIndex` 已通过 `clore::generate::cache::load_cache_index` 正确加载且有效，该函数不修改索引内容，属于只读查询操作。

#### Usage Patterns

- lookup cached response by key
- check if response exists in cache
- used before generation to avoid redundant computation

### `clore::generate::cache::load_cache_index`

Declaration: `src/generate/cache.cppm:48`

Definition: `src/generate/cache.cppm:271`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

该函数负责从指定的缓存文件路径加载并反序列化缓存索引。接受一个 `std::string_view` 参数，代表缓存文件的路径；返回一个 `std::expected<CacheIndex, CacheError>`，成功时包含一个 `CacheIndex` 对象，失败时包含一个描述原因的 `CacheError`。

如果文件不存在或格式无效，`CacheError` 会报告具体错误。调用者应始终检查返回值并处理可能的失败情况，以确保后续的缓存操作（如 `find_cached_response`）基于有效的索引进行。

#### Usage Patterns

- Called to load the cache index before performing cache lookups
- Typically invoked once at startup or when a cache reload is needed
- Used in conjunction with `find_cached_response` to query cached results

### `clore::generate::cache::load_cache_index_async`

Declaration: `src/generate/cache.cppm:57`

Definition: `src/generate/cache.cppm:375`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::load_cache_index_async` 启动异步加载缓存索引的操作。它接受一个指定缓存索引文件路径的 `std::string` 和一个 `kota::event_loop &` 引用，用于调度异步任务。返回一个 `int` 值，通常为零表示操作已成功启动，非零值表示发生了错误。该函数不直接返回缓存索引内容；加载完成后，索引可在内部使用，并通过 `clore::generate::cache::find_cached_response` 等查询函数访问。

#### Usage Patterns

- 在需要异步获取缓存索引的场景中调用，例如初始化或需要索引时
- 调用者通常使用 `co_await` 等待返回的 `kota::task` 以获取 `CacheIndex` 或处理 `CacheError`

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `src/generate/cache.cppm:43`

Definition: `src/generate/cache.cppm:238`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

函数 `clore::generate::cache::make_prompt_response_cache_key` 接受两个提示和响应文本（均为 `std::string_view`）以及一个整数引用，返回一个 `std::expected<std::string, CacheError>`。调用方负责提供规范化前的原始文本和一个用于区分缓存版本的整数参数；成功时得到一个可用于后续缓存查找或存储的稳定字符串键，失败时返回一个错误类型 `CacheError`。该键的生成规则（包括文本归一化处理）对调用方透明，仅保证相同输入产生相同键值。

#### Usage Patterns

- Called to generate a composite cache key for prompt-response pairs before caching or retrieval

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `src/generate/cache.cppm:211`

Definition: `src/generate/cache.cppm:211`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

Declaration: [Declaration](functions/normalize-text-for-hashing.md)

`clore::generate::cache::normalize_text_for_hashing` 函数接受一个 `std::string_view` 并返回一个 `std::string`。它负责在文本用作散列输入之前对其进行规范化，从而确保语义上相同的文本在缓存键生成过程中产生一致的规范化表示。此函数由 `clore::generate::cache::make_prompt_response_cache_key` 调用，用于规范化系统提示和用户请求提示，作为构建复合缓存键的一部分。调用者可以依赖该函数生成的规范化结果来保证缓存查找的确定性。

#### Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize text before hashing.

### `clore::generate::cache::save_cache_entry`

Declaration: `src/generate/cache.cppm:50`

Definition: `src/generate/cache.cppm:322`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::save_cache_entry` 同步地将一条缓存条目写入持久化缓存存储。调用者需提供三个 `std::string_view` 参数，它们共同标识要保存的条目及其内容（具体含义可参考 `make_prompt_response_cache_key` 与 `find_cached_response` 等配套函数）。该函数会处理必要的序列化与写入操作。

成功时返回一个空 `std::expected<void, CacheError>`；失败时则返回一个 `CacheError` 枚举值，表明错误类型（如文件系统错误或数据格式错误）。调用者应检查返回值以确保条目被正确持久化。

#### Usage Patterns

- Called by synchronous cache-saving code paths after a response has been generated.
- Used to persist a cache entry keyed by `cache_key` for later retrieval via cached response lookup.

### `clore::generate::cache::save_cache_entry_async`

Declaration: `src/generate/cache.cppm:60`

Definition: `src/generate/cache.cppm:395`

Implementation: [`Module generate:cache`](../../../../modules/generate/cache.md)

`clore::generate::cache::save_cache_entry_async` 异步地将一个缓存条目写入缓存。调用者提供三个 `std::string` 参数（其含义由具体的缓存策略定义）以及一个 `kota::event_loop &`，该函数会调度保存操作在给定的事件循环上异步执行。返回值 `int` 表示操作的状态或标识符；调用者应检查该值以确认操作是否成功启动。注意，调用者有责任确保传入的事件循环在操作完成前保持有效并处于运行状态。

#### Usage Patterns

- Called when a new prompt‑response pair needs to be stored in the cache asynchronously
- Typically awaited by the caller to receive completion or error
- Used in conjunction with `load_cache_index_async` for full async cache workflows

## Related Pages

- [Namespace clore::generate](../index.md)

