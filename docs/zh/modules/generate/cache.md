---
title: 'Module generate:cache'
description: 'generate:cache 模块提供 LLM 生成响应的持久化缓存功能，负责缓存键的构造、存储与检索。它公开了索引加载（load_cache_index 及其异步版本）、条目查找（find_cached_response）、条目保存（save_cache_entry 及其异步版本），以及缓存键生成（make_prompt_response_cache_key）和文本规范化（normalize_text_for_hashing）等核心接口。模块内部通过 JSON Lines 文件实现缓存的物理存储，并依赖 CacheIndex 和 CacheError 等自定义类型来表示索引状态与错误信息。'
layout: doc
template: doc
---

# Module `generate:cache`

## Summary

`generate:cache` 模块提供 LLM 生成响应的持久化缓存功能，负责缓存键的构造、存储与检索。它公开了索引加载（`load_cache_index` 及其异步版本）、条目查找（`find_cached_response`）、条目保存（`save_cache_entry` 及其异步版本），以及缓存键生成（`make_prompt_response_cache_key`）和文本规范化（`normalize_text_for_hashing`）等核心接口。模块内部通过 JSON Lines 文件实现缓存的物理存储，并依赖 `CacheIndex` 和 `CacheError` 等自定义类型来表示索引状态与错误信息。

## Imports

- [`protocol`](../protocol/index.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::cache::CacheError`

Declaration: `generate/cache.cppm:16`

Definition: `generate/cache.cppm:16`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

`CacheError` 内部仅包含一个 `std::string message` 成员，用于存储错误描述信息。该结构体的所有构造、赋值与析构行为完全委托给 `message` 的相应操作，因此它是一个轻量且可复制的错误类型，没有额外的内部不变量或验证逻辑。

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

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

结构体 `clore::generate::cache::CacheIndex` 直接暴露一个 `std::unordered_map<std::string, std::string>` 类型的 `entries` 字段作为唯一数据成员。内部不维护任何额外的不变量或元数据；所有缓存键值对的存储和查找完全由该映射管理。由于没有封装或辅助方法，类型的正确性完全依赖于调用方对 `entries` 的直接操作，例如确保键的唯一性以及值的一致更新。该设计将缓存索引简化为一个纯数据容器，避免了间接层。

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

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

该实现直接将传入的 `cache_key`（`std::string_view`）转换为 `std::string` 后，在 `index` 的 `entries` 成员（一个关联容器，其键类型为 `std::string`）上调用 `find`。若迭代器 `it` 到达 `index.entries.end()`，则表示无匹配响应，函数返回 `std::nullopt`；否则返回 `it->second`，即缓存中存储的 `std::optional<std::string_view>`。整个算法为单次映射查找，无额外控制流或副作用。核心依赖为 `CacheIndex` 中 `entries` 的数据结构（预期为基于哈希或红黑树的关联容器）和标准库的 `std::string`、`std::optional`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `index.entries` (the underlying map)
- `cache_key` parameter

#### Usage Patterns

- Check if a cached response exists for a given key
- Retrieve a cached response string by its cache key

### `clore::generate::cache::load_cache_index`

Declaration: `generate/cache.cppm:29`

Definition: `generate/cache.cppm:252`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数 `clore::generate::cache::load_cache_index` 通过扫描工作区根目录下所有 JSONL 文件来构建内存中的缓存索引。它首先调用 `all_jsonl_files` 获取文件路径列表，若失败则直接返回 `CacheError`。随后遍历每个文件，使用 `clore::support::read_utf8_text_file` 读取其 UTF-8 文本内容（读取失败则跳过该文件）。对于每个有效文件，逐行解析为 `kota::codec::json::Object`：如果某行解析失败或缺少非空字符串字段 `"key"` 和 `"resp"`，则静默跳过该行；否则将键值对插入到 `CacheIndex::entries` 中，已存在的键会被覆盖。处理完所有文件后返回最终的索引。

该函数的控制流完全依赖 `all_jsonl_files` 的成功执行，且对文件读取和 JSON 解析错误均采取“容忍并跳过”策略，不会让单个损坏文件阻塞整个索引加载过程。底层依赖 `kota::codec::json` 的解析能力、文件 I/O 函数以及匿名命名空间中的辅助函数 `all_jsonl_files`。最终输出的 `CacheIndex` 对象仅包含 `entries` 映射，不持有外部资源。

#### Side Effects

- 读取文件系统中的 JSONL 文件
- 为 `CacheIndex` 内部的字符串和映射分配动态内存

#### Reads From

- `workspace_root` 参数
- 由 `all_jsonl_files` 查找到的 JSONL 文件内容

#### Writes To

- 返回的 `clore::generate::cache::CacheIndex` 对象

#### Usage Patterns

- 用于同步加载缓存索引，通常与其他缓存操作（如 `find_cached_response`）配合使用
- 在需要一次性加载所有缓存条目以进行后续查找的场景中调用

### `clore::generate::cache::load_cache_index_async`

Declaration: `generate/cache.cppm:38`

Definition: `generate/cache.cppm:356`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数 `clore::generate::cache::load_cache_index_async` 通过协程将同步函数 `clore::generate::cache::load_cache_index` 异步化。它接收一个 `workspace_root` 字符串和 `kota::event_loop` 的引用，返回 `kota::task<CacheIndex, CacheError>`。实现内部，调用 `kota::queue` 将一个 lambda 封闭 `load_cache_index` 调用并调度到指定的 `loop` 上执行；该 lambda 捕获移动后的 `workspace_root`，在后台线程中运行 `load_cache_index(workspace_root)` 并返回其结果。

等待 `kota::queue` 的结果后，函数检查是否因取消或其他错误导致 `queued_result` 本身含有错误（`has_error()`），若有则将错误消息包装为新的 `CacheError` 并通过 `kota::fail` 返回。若队列执行成功，但 `load_cache_index` 内部返回了 `std::expected` 的错误（`queued_result->has_value()` 为假），则将该错误值移动并传递给 `kota::fail`。最终，若两者均成功，则移动出底层的 `CacheIndex` 值并 `co_return`。该函数不重复实现缓存索引的解析逻辑，完全依赖同步版本，仅添加异常处理与异步调度。

#### Side Effects

- Reads cache index file from `workspace_root` on disk

#### Reads From

- `workspace_root` parameter
- cache index file on disk

#### Usage Patterns

- Called asynchronously to obtain the prompt response cache index
- Used by higher‑level async cache operations

### `clore::generate::cache::make_prompt_response_cache_key`

Declaration: `generate/cache.cppm:24`

Definition: `generate/cache.cppm:219`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

函数首先计算请求的响应格式指纹和工具选择指纹，分别通过`response_format_fingerprint`和`tool_choice_fingerprint`完成，这两个调用可能返回错误，如果任一失败则直接传递错误。随后对原始`prompt`和`system_prompt`分别调用`normalize_text_for_hashing`进行规范化，再用`llvm::xxh3_64bits`计算64位哈希值。缓存键由制表符分隔的五部分拼接而成：原样的`request_key`、提示词哈希、系统提示词哈希、响应格式指纹和工具选择指纹，最后附加一个表示`output_contract`的字符。此算法确保相同语义的请求（经过规范化）生成相同的键，同时支持通过`response_format`和`tool_choice`等字段进行细粒度区分。整个过程中依赖`normalize_text_for_hashing`进行文本预处理，而指纹函数则封装了对应的对象序列化逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `request_key`
- `system_prompt`
- `request.response_format`
- `request.prompt`
- `request.tool_choice`
- `request.output_contract`

#### Usage Patterns

- Generate a cache key for storing or retrieving prompt responses
- Used internally by `save_cache_entry` and `find_cached_response` like functions

### `clore::generate::cache::normalize_text_for_hashing`

Declaration: `generate/cache.cppm:192`

Definition: `generate/cache.cppm:192`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

Implementation: [Implementation](functions/normalize-text-for-hashing.md)

函数 `clore::generate::cache::normalize_text_for_hashing` 实现了对输入文本的标准化，旨在为后续哈希计算提供一致、可比较的字符串。算法通过两步完成：首先跳过所有前导空白字符，然后遍历剩余字符，使用一个 `prev_space` 标志记录是否遇到了空白；每当遇到非空白字符时，如果之前有空白且结果字符串非空，则插入一个空格，再添加该字符，从而将任意连续的空白序列（包括换行、制表等）压缩为单个标准空格，同时自动消除尾部空白。控制流基于单次线性扫描与条件判断，依赖标准库的 `std::isspace` 检测空白字符，不涉及外部系统或复杂数据结构，保证了轻量且确定性的规范化效果。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter (`std::string_view`)

#### Usage Patterns

- Called by `make_prompt_response_cache_key` to normalize input strings before combining into a cache key.

### `clore::generate::cache::save_cache_entry`

Declaration: `generate/cache.cppm:31`

Definition: `generate/cache.cppm:303`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

该函数通过一个静态的 `std::mutex` 实现线程安全的序列化写入，保护对缓存文件的并发访问。首先调用 `clore::generate::cache::(anonymous namespace)::cache_directory` 获取缓存根目录，若失败则直接返回对应的 `CacheError`；否则使用 `fs::create_directories` 确保目录存在。接着拼接当前日期对应的 JSONL 文件名（通过 `current_jsonl_filename` 获得），并借助 `clore::generate::cache::(anonymous namespace)::build_jsonl_line` 将 `cache_key` 和 `response` 格式化为一行 JSON 文本。最后以二进制追加模式打开文件，写入该行并刷新，任何 I/O 失败均返回包含描述信息的 `CacheError`。

#### Side Effects

- Acquires a static mutex for synchronization
- Creates cache directories if they do not exist
- Appends a JSONL line to the cache file
- Flushes the output stream

#### Reads From

- `workspace_root`
- `cache_key`
- `response`
- Result of `cache_directory(workspace_root)`
- Result of `current_jsonl_filename()`
- Result of `build_jsonl_line(cache_key, response)`

#### Writes To

- Cache directory structure via `fs::create_directories`
- Cache file via `std::ofstream` append write

#### Usage Patterns

- Synchronous caching of prompt-response pairs
- Called when immediate persistence is required
- Complemented by `save_cache_entry_async` for non-blocking usage

### `clore::generate::cache::save_cache_entry_async`

Declaration: `generate/cache.cppm:41`

Definition: `generate/cache.cppm:376`

Declaration: [`Namespace clore::generate::cache`](../../namespaces/clore/generate/cache/index.md)

该函数通过将同步核心 `save_cache_entry` 的调用提交至给定的 `kota::event_loop` `loop` 来实现异步缓存写入。它捕获 `workspace_root`、`cache_key` 和 `response` 的所有权到 `kota::queue` 的闭包中，等待执行完成。若队列任务因取消或内部错误而失败，则通过 `kota::fail` 构造一个带有描述性消息的 `CacheError` 并结束协程；若 `save_cache_entry` 本身返回 `std::expected` 中的错误，则直接传播该 `CacheError`。正常完成时协程无值返回。

#### Side Effects

- writes cache entry to persistent storage via `save_cache_entry`
- may propagate `CacheError` from underlying save operation

#### Reads From

- `workspace_root` (by move)
- `cache_key` (by move)
- `response` (by move)
- `loop` (reference to `kota::event_loop`)

#### Writes To

- persistent cache storage via `save_cache_entry`
- returned `kota::task<void, CacheError>` object

#### Usage Patterns

- called after generating a response to asynchronously persist the cache entry
- used in coroutine-based asynchronous workflows with `kota::event_loop`

## Internal Structure

`generate:cache` 模块为 LLM 生成请求提供持久化缓存机制，其公共接口包括 `CacheIndex` 索引结构、`CacheError` 错误类型以及一组同步/异步 API（如 `load_cache_index`、`save_cache_entry`、`find_cached_response`、`make_prompt_response_cache_key`）。内部实现按职责分解为一个匿名命名空间，封装了 JSONL 文件扫描、缓存键指纹计算（例如 `tool_choice_fingerprint`、`response_format_fingerprint`）、文本规范化（`normalize_text_for_hashing`）以及 ISO 时间戳生成等底层工具函数。模块导入 `protocol` 以复用 LLM 请求/响应的核心数据模型，并依赖 `support` 模块提供的跨切面文本处理与文件 I/O 辅助功能。

在实现结构上，同步函数直接操作文件系统与 `std::expected` 返回结果，而异步版本（`load_cache_index_async`、`save_cache_entry_async`）通过 `kota::event_loop` 调度非阻塞 I/O。缓存条目以 JSONL 格式按日期分片存储（`current_jsonl_filename` 生成每日文件名），并由 `CacheIndex` 在内存中维护所有键到位置的映射。整个模块围绕着两条核心路径组织：写入时先构建键、规范化并序列化；读取时通过索引快速定位并反序列化响应，同时利用匿名函数隔离了 JSON 转义、哈希归一化等与业务无关的细节，保持了公共接口的清晰性。

## Related Pages

- [Module protocol](../protocol/index.md)
- [Module support](../support/index.md)

