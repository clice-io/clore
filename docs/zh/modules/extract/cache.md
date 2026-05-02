---
title: 'Module extract:cache'
description: '该模块负责管理代码提取操作的缓存逻辑，包括生成、持久化和验证提取结果。它公开了构建编译签名（build_compile_signature）、捕获依赖状态快照（capture_dependency_snapshot）以及检测依赖是否发生变化（dependencies_changed）等关键函数，并提供对文件内容进行哈希（hash_file）的能力。缓存键的生成（build_cache_key）与解析（split_cache_key）被封装为独立接口，而 load_extract_cache 和 save_extract_cache 则负责以键值对形式读写提取缓存。此外，它还提供针对 CLICE 工作区的专用缓存数据（CliceCacheData、CliceCachePCMEntry、CliceCachePCHEntry 等）的序列化与存取操作（load_clice_cache、save_clice_cache）。模块内部包含匿名命名空间中的辅助函数，用于路径规范化、缓存文件路径推导、依赖快照的编码/解码以及多线程并行哈希等实现细节。'
layout: doc
template: doc
---

# Module `extract:cache`

## Summary

该模块负责管理代码提取操作的缓存逻辑，包括生成、持久化和验证提取结果。它公开了构建编译签名（`build_compile_signature`）、捕获依赖状态快照（`capture_dependency_snapshot`）以及检测依赖是否发生变化（`dependencies_changed`）等关键函数，并提供对文件内容进行哈希（`hash_file`）的能力。缓存键的生成（`build_cache_key`）与解析（`split_cache_key`）被封装为独立接口，而 `load_extract_cache` 和 `save_extract_cache` 则负责以键值对形式读写提取缓存。此外，它还提供针对 CLICE 工作区的专用缓存数据（`CliceCacheData`、`CliceCachePCMEntry`、`CliceCachePCHEntry` 等）的序列化与存取操作（`load_clice_cache`、`save_clice_cache`）。模块内部包含匿名命名空间中的辅助函数，用于路径规范化、缓存文件路径推导、依赖快照的编码/解码以及多线程并行哈希等实现细节。

## Imports

- [`extract:ast`](ast.md)
- [`extract:compiler`](compiler.md)
- [`extract:scan`](scan.md)
- `std`
- [`support`](../support/index.md)

## Types

### `clore::extract::cache::CacheError`

Declaration: `extract/cache.cppm:20`

Definition: `extract/cache.cppm:20`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CacheError` 是一个仅含单一 `std::string` 数据成员 `message` 的简单值类型。该结构体不维护任何额外的不变量——`message` 的内容完全由调用者控制，并且没有隐式的格式化或验证逻辑。设计上，它作为轻量级错误载体，直接暴露 `message` 供外部读取，无需虚函数或继承，仅依赖字符串的默认构造、复制和析构行为。

#### Invariants

- The `message` member may be empty or contain any string
- No additional constraints beyond standard `std::string` behavior

#### Key Members

- `message`

#### Usage Patterns

- Returned or caught as an error result from cache operations
- Constructed with a descriptive string to indicate failure cause
- Likely used with `std::expected` or `std::variant` error handling patterns

### `clore::extract::cache::CacheKeyParts`

Declaration: `extract/cache.cppm:24`

Definition: `extract/cache.cppm:24`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CacheKeyParts` 是一个聚合结构体，直接持有缓存键的两个核心组成部分。其成员 `path` 为 `std::string` 类型，存储源文件路径；`compile_signature` 为 `std::uint64_t` 类型，记录文件的编译时签名（通常是依赖树哈希或修改时间戳）。这两个字段共同唯一标识一个缓存条目：`path` 定位目标文件，`compile_signature` 区分同一文件在不同编译上下文下的版本。结构体本身无自定义构造函数或成员函数，完全依赖聚合初始化，所有成员均公有且不维护额外不变式——调用方需确保 `path` 为空时能按预期处理，并保持 `compile_signature` 与 `path` 的关联一致性。

#### Invariants

- `path` 应该是一个有效的文件系统路径
- `compile_signature` 应当唯一标识一个编译单元的特定版本

#### Key Members

- `path`：文件路径
- `compile_signature`：编译签名

#### Usage Patterns

- 用作缓存键的一部分，与完整的缓存键组合或比较
- 可能通过其成员来生成哈希或进行相等性判断

### `clore::extract::cache::CacheRecord`

Declaration: `extract/cache.cppm:36`

Definition: `extract/cache.cppm:36`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CacheRecord` 将缓存的提取结果分解为五个公有字段。其核心不变式在于 `compile_signature` 与 `source_hash` 的组合唯一标识一组输入，而 `ast_deps`、`scan` 和 `ast` 则分别存储对该输入进行依赖分析、扫描及 AST 解析后得到的结果。在缓存命中时，外部组件首先校验两个哈希值是否与当前编译上下文匹配，随后即可安全地读取其余三个字段的内容。由于该结构体仅作为纯数据聚合体，所有字段均直接暴露，验证、序列化及生命周期管理等职责由 `CacheManager` 等配套类型承担。

#### Invariants

- `compile_signature` 与 `source_hash` 的组合唯一标识一个编译单元
- 缓存记录中的 `ast_deps`、`scan`、`ast` 应与对应的 `source_hash` 和 `compile_signature` 保持一致
- 当源文件或编译配置变化时，相关字段应失效或更新

#### Key Members

- `compile_signature`
- `source_hash`
- `ast_deps`
- `scan`
- `ast`

#### Usage Patterns

- 在缓存查找时通过 `source_hash` 和 `compile_signature` 匹配记录
- 提取过程中生成新的 `CacheRecord` 实例并存储到缓存中
- 其他代码通过读取成员（如 `scan`、`ast`）获取缓存的分析结果

### `clore::extract::cache::CliceCacheData`

Declaration: `extract/cache.cppm:68`

Definition: `extract/cache.cppm:68`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

结构体 `clore::extract::cache::CliceCacheData` 是一个纯聚合类型，用于容纳缓存数据的三个独立领域：`paths` 中存储原始路径字符串，`pch` 中按序存储预编译头缓存条目（类型 `CliceCachePCHEntry`），`pcm` 中按序存储模块缓存条目（类型 `CliceCachePCMEntry`）。三个 `std::vector` 成员在逻辑上互相独立，没有隐式的同步或对齐约束；实现上将路径、PCH 与 PCM 分离主要是为了配合上层存取逻辑的差异，并简化序列化/反序列化的布局。该结构体本身不维护任何跨字段的不变性——所有成员均可独立修改，依赖外部调用方保证语义一致性。

#### Invariants

- All three member vectors may be empty; no non‑empty guarantee is implied.
- The struct provides no validation or ordering invariants beyond what `std::vector` offers.

#### Key Members

- `paths`
- `pch`
- `pcm`

#### Usage Patterns

- Used to aggregate and transfer cache data in the extraction pipeline.
- Likely populated by serialization or extraction routines and consumed by cache lookup or storage logic.

### `clore::extract::cache::CliceCacheDepEntry`

Declaration: `extract/cache.cppm:46`

Definition: `extract/cache.cppm:46`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该结构体 `clore::extract::cache::CliceCacheDepEntry` 是一个扁平的数据载体，内部仅包含两个字段：`path`（`std::uint32_t`）和 `hash`（`std::uint64_t`）。两个字段均默认初始化为零，使得默认构造的实例立即处于可判定的“空”或“无效”状态。整个结构体保持与外部 `clice/src/server/workspace.cpp` 中 `CacheData` 的 schema 兼容，因此其内存布局、成员顺序及类型均严格匹配，以便直接进行底层序列化或内存映射。由于该结构体为平凡可复制类型（trivially copyable），不依赖虚函数或自定义构造函数，其实现完全由成员默认值和非虚析构函数构成，无需额外的运行时维护逻辑。

#### Invariants

- `path` 和 `hash` 的类型固定为 `std::uint32_t` 和 `std::uint64_t`
- 所有成员初始化为零，表示空或未设置状态
- 结构的字段布局与外部 `CacheData` 保持兼容

#### Key Members

- `path`
- `hash`

#### Usage Patterns

- 作为缓存数据结构中的元素，用于存储依赖项的路径和哈希值
- 通过比较 `path` 和 `hash` 判断依赖是否发生变化
- 是 `CliceCacheEntry` 或其他缓存容器的一部分（未在证据中明确）

### `clore::extract::cache::CliceCachePCHEntry`

Declaration: `extract/cache.cppm:51`

Definition: `extract/cache.cppm:51`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CliceCachePCHEntry` 是一个内部记录类型，用于表示单个预编译头缓存项的元数据。其字段 `filename` 存储缓存文件的路径，`source_file` 记录关联的主源文件在缓存数据库中的索引，`hash` 是 PCH 内容的摘要值，用于检测内容是否已变化；`bound` 是一个引用计数，指示当前有多少编译任务已绑定到该项以确保并发安全，`build_at` 是构建完成的时间戳，用于判断缓存时效，而 `deps` 则包含一个 `CliceCacheDepEntry` 向量，列出该项的所有直接依赖项。这些成员共同维护了缓存项在本地的完整状态，使缓存系统能够高效地进行验证、重用和失效处理。

#### Invariants

- `hash` uniquely identifies the PCH content
- `deps` holds all dependency entries for the PCH
- `build_at` is a timestamp (likely Unix epoch in seconds or milliseconds)
- `bound` represents a binding count or reference counter

#### Key Members

- `filename`
- `source_file`
- `hash`
- `bound`
- `build_at`
- `deps`

#### Usage Patterns

- Used as element in a cache container (e.g., map or vector)
- Fields accessed directly for read/write by cache serialization and comparison logic
- Dependency information stored in `deps` for validity checks

### `clore::extract::cache::CliceCachePCMEntry`

Declaration: `extract/cache.cppm:60`

Definition: `extract/cache.cppm:60`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该结构体是一个聚合类型，用于持久化缓存中的已编译 PCM 条目。内部通过 `filename` 存储 PCM 文件的路径，`module_name` 记录关联的模块名称，`source_file` 引用源文件标识符（默认初始化为 0），`build_at` 记录构建时间戳（默认初始化为 0），`deps` 以 `std::vector<CliceCacheDepEntry>` 保存依赖项列表。所有成员均为公有，支持直接成员初始化；其不变性体现在：`build_at` 的非负性（语义上应为时间戳），以及 `source_file` 若大于 0 则代表有效的源文件索引，但这些约束由外部调用方保证，结构体本身不执行校验。

#### Invariants

- `source_file` and `build_at` default to zero when not explicitly initialized
- `deps` is a vector that may be empty

#### Key Members

- `filename`
- `module_name`
- `source_file`
- `build_at`
- `deps`

#### Usage Patterns

- Stored in a cache collection managed by `clore::extract::cache`
- Populated during PCM extraction and used for build system dependency tracking
- Likely serialized or persisted to disk for incremental builds

### `clore::extract::cache::DependencySnapshot`

Declaration: `extract/cache.cppm:29`

Definition: `extract/cache.cppm:29`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该结构体以三个并行向量为核心：`files` 存储依赖项的文件路径，`hashes` 存储每个文件的内容哈希，`mtimes` 存储最后的修改时间。这些向量之间的索引隐含的对应关系是核心不变性：对于任意索引 `i`，`files[i]`、`hashes[i]` 和 `mtimes[i]` 描述的是同一个文件。`build_at` 记录该快照创建的时间戳，默认值为 `0` 表示未设置。所有成员均可直接访问，不提供额外的封装或校验逻辑，因此调用方必须自行维护向量的同步性（例如在添加或删除文件时同时处理三个向量）确保快照的一致性。

#### Invariants

- No documented invariants; the vectors are not explicitly constrained to have the same length.

#### Key Members

- `files`
- `hashes`
- `mtimes`
- `build_at`

#### Usage Patterns

- Used to bundle file paths, hashes, modification times, and build timestamp for caching purposes.

## Functions

### `clore::extract::cache::build_cache_key`

Declaration: `extract/cache.cppm:76`

Definition: `extract/cache.cppm:228`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数通过字符串拼接构造缓存键。首先为 `normalized_path` 的长度加 1（分隔符占用）再加 20（`uint64_t` 最大十进制长度）预分配空间，然后依次追加 `normalized_path`、常量 `kCacheKeyDelimiter` 以及通过 `std::to_string` 转换的 `compile_signature` 十进制形式。整个过程仅涉及线性字符串操作，无分支或循环，依赖 `kCacheKeyDelimiter` 作为内部定界符。

内部逻辑的核心是生成一个确定性、可解析的唯一键，供 `split_cache_key` 反向拆解为 `CacheKeyParts`。该键直接作为缓存查找的依据，与 `cache_file_path` 及 `clice_cache_file_path` 等文件路径生成函数配合使用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `normalized_path`
- 参数 `compile_signature`
- 常量 `kCacheKeyDelimiter`

#### Usage Patterns

- 被缓存相关函数用于构建唯一缓存键

### `clore::extract::cache::build_compile_signature`

Declaration: `extract/cache.cppm:74`

Definition: `extract/cache.cppm:224`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::build_compile_signature` 接受一个 `CompileEntry` 常量引用，直接转发至 `clore::extract::build_compile_signature`，并将该调用的返回值作为结果返回。其内部不包含任何算法或控制流，仅作为接口适配层，依赖下层模块提供的签名计算实现。该实现可能基于输入的编译选项和源文件内容生成一个 `std::uint64_t` 哈希，用于缓存键的构建。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry` parameter (a `const CompileEntry&`)

#### Usage Patterns

- Used to obtain a compile signature for cache key operations by delegating to the core signature builder.

### `clore::extract::cache::capture_dependency_snapshot`

Declaration: `extract/cache.cppm:83`

Definition: `extract/cache.cppm:282`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先对输入的文件列表进行规范化、排序和去重，得到 `normalized` 路径集合，并记录当前时间戳到 `snapshot.build_at`。随后，它根据硬件并发数计算线程数 `num_threads`，将工作均匀分配给 `per_thread` 大小的块，并启动一组 `std::thread`。每个工作线程调用 `llvm::sys::fs::status` 获取文件的修改时间 `mtime`，再调用 `hash_file` 计算内容哈希；若 `hash_file` 失败（例如依赖文件已被删除），则将哈希置为 `0` 而非中止，以保证后续缓存比较能够检测到变化。所有线程通过 `std::mutex` 保护的 `first_error` 变量同步错误状态——一旦某个线程发现错误，其余线程将提前退出。

所有线程汇合后，若存在错误，则返回 `std::unexpected<CacheError>`；否则，将每个 `DependencyHashTaskResult` 中的文件路径、哈希值和修改时间分别移入 `snapshot.files`、`snapshot.hashes` 和 `snapshot.mtimes` 字段，最终返回完整的 `DependencySnapshot`。该实现通过并行化 I/O 密集型操作和容错处理，在保证性能的同时应对文件系统的不确定性。

#### Side Effects

- Reads file metadata via `llvm::sys::fs::status`
- Reads file content via `hash_file`
- Uses `std::thread` to spawn concurrent workers
- Uses `std::mutex` to synchronize error-checking between threads

#### Reads From

- Parameter `files` of type `const std::vector<std::string>&`
- File system metadata (modification time) for each file path
- File system content (via `hash_file`) for each file path

#### Writes To

- Local `DependencySnapshot` object (`snapshot`)
- Thread-shared `first_error` optional (under mutex)
- Temporary vectors `normalized` and `task_results`

#### Usage Patterns

- Capturing dependency state for incremental compilation cache
- Feeding snapshot to `dependencies_changed` for change detection
- Storing snapshot in cache via `save_clice_cache`

### `clore::extract::cache::dependencies_changed`

Declaration: `extract/cache.cppm:86`

Definition: `extract/cache.cppm:401`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先对输入参数进行快速合法性检查：若 `snapshot.build_at` 不大于零或 `snapshot.files` 为空，或 `snapshot.files` 的长度与 `snapshot.hashes` 或 `snapshot.mtimes` 不一致，则立即返回 `true`，表示依赖已变化。随后根据依赖数量决定执行路径：当文件数量不超过内部常量 `kParallelThreshold`（16）时，采用串行循环依次调用 `check_single_dependency` 检查每一项依赖，一旦发现变化立即返回 `true`；否则进入并行路径，基于 `std::thread::hardware_concurrency` 确定线程数，使用 `std::atomic<bool> changed` 作为协调标志，创建多个线程分别处理连续区间，每个线程在发现依赖变化时通过 `changed.store` 通知其他线程提前退出。所有线程汇合后返回 `changed` 的最终值。该函数的整个控制流仅依赖 `check_single_dependency` 对单个依赖项的判定逻辑，以及标准库线程原语。

#### Side Effects

- Creates and joins worker threads when the dependency set exceeds the parallel threshold
- Performs atomic loads and stores on a local `std::atomic<bool>` variable

#### Reads From

- `snapshot.build_at`
- `snapshot.files`
- `snapshot.hashes`
- `snapshot.mtimes`
- Underlying file system state accessed by `check_single_dependency`

#### Usage Patterns

- Called before deciding whether to reuse a cached extraction result
- Used in conjunction with `capture_dependency_snapshot` and cache loading/saving functions

### `clore::extract::cache::hash_file`

Declaration: `extract/cache.cppm:81`

Definition: `extract/cache.cppm:270`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数使用 LLVM 提供的 `llvm::MemoryBuffer::getFile` 读取指定路径的文件。若读取失败，函数立即返回一个 `std::unexpected<CacheError>`，其中包含格式化的错误信息，明确报告无法读取文件及其原因。若读取成功，则将文件内容传递给 `llvm::xxh3_64bits` 以计算 64 位 XXH3 哈希值，并直接返回该哈希值。整个过程不涉及并行处理或外部缓存，依赖仅限于 LLVM 核心库的文件读取与哈希计算功能。

#### Side Effects

- 读取文件系统中的文件内容

#### Reads From

- 参数 `path` 指定的文件系统上的文件

#### Usage Patterns

- 用于计算文件内容的哈希值，通常作为缓存键的一部分
- 被 `build_cache_key` 或类似缓存管理函数调用

### `clore::extract::cache::load_clice_cache`

Declaration: `extract/cache.cppm:95`

Definition: `extract/cache.cppm:670`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::load_clice_cache` 通过以下顺序步骤从磁盘加载缓存的编译数据：它首先调用 `clice_cache_file_path` 将 `workspace_root` 解析为预期的缓存文件路径；若该步骤失败，则立即返回 `CacheError`。接着使用 `fs::exists` 检查路径是否存在，若文件不存在（且无底层错误）则直接返回一个空的 `CliceCacheData`；若存在但无法读取，则通过 `clore::support::read_utf8_text_file` 读取完整内容并返回错误。读取成功后，调用 `json::from_json` 将 JSON 内容反序列化为 `CliceCacheData` 结构；若反序列化失败（例如格式错误或版本不匹配），函数会记录一条警告并返回空 `CliceCacheData` 以静默忽略损坏的缓存。最终返回反序列化后的数据。

该函数的内部流程完全依赖于外层提供的 `workspace_root` 字符串、文件系统操作（`fs::exists`、`clice_cache_file_path`）以及 JSON 解析库（`json::from_json`）。错误处理采用 `std::expected` 模式，将大部分异常情况转化为显式的 `CacheError` 或空数据回退，确保调用方不会因缓存损坏而崩溃。

#### Side Effects

- logs a warning via `logging::warn` when JSON deserialization fails

#### Reads From

- `workspace_root` parameter
- file system via `fs::exists`
- cache file content via `clore::support::read_utf8_text_file`

#### Writes To

- logging system (via `logging::warn`)

#### Usage Patterns

- called to load previously cached clice data from disk
- used before processing to check for stale cache

### `clore::extract::cache::load_extract_cache`

Declaration: `extract/cache.cppm:88`

Definition: `extract/cache.cppm:457`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::load_extract_cache` 的核心流程分为三个部分：路径解析与存在性检查、缓存文件的解析与版本校验、以及缓存记录的重构。首先通过 `cache_file_path` 将 `workspace_root` 转换为缓存文件路径 `path`，若文件不存在则直接返回空的 `std::unordered_map`；否则读取其 UTF-8 文本内容，并用 `json::from_json` 反序列化为 `SerializedCacheData`。若反序列化失败或 `data.format_version` 与常量 `kExtractCacheFormatVersion` 不匹配，函数记录警告并返回空映射以忽略失效缓存。

在通过校验后，函数遍历 `data.entries` 中的每个 `entry`：检查 `entry.source_file` 是否在 `data.paths` 的合法范围内，然后调用 `decode_dependency_snapshot` 将 `entry.ast_deps` 解码为 `DependencySnapshot`。接着使用 `normalize_path_string` 规范化源路径，并通过 `build_cache_key` 组合规范化路径和 `entry.compile_signature` 生成缓存键。每个有效条目被封装为 `CacheRecord` 并插入到结果映射中。该函数依赖 `cache_file_path`、`json::from_json`、`decode_dependency_snapshot` 及 `build_cache_key` 等内部函数，以及 `kExtractCacheFormatVersion` 常量和 `SerializedCacheData`、`CacheRecord` 等数据结构。

#### Side Effects

- reads file from filesystem via `clore::support::read_utf8_text_file`
- logs warnings via `logging::warn` when cache format is stale or invalid
- allocates memory for the returned map and its entries

#### Reads From

- parameter `workspace_root`
- filesystem cache file at path derived from `workspace_root`
- constant `kExtractCacheFormatVersion`
- serialised cache data fields `data.paths` and `data.entries`

#### Usage Patterns

- load cached extraction results before performing extraction
- check cache validity and optionally fall back to empty cache
- used in conjunction with `save_extract_cache` for cache round-trip

### `clore::extract::cache::save_clice_cache`

Declaration: `extract/cache.cppm:97`

Definition: `extract/cache.cppm:710`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数首先通过 `clice_cache_file_path` 获得目标缓存路径，并创建其父目录。随后调用 `json::to_json` 将 `CliceCacheData` 序列化为 JSON 字符串。为了安全地写入，它构造一个唯一的临时文件路径：在一个最多 32 次的重试循环中，利用当前时间戳、进程 ID、当前尝试序号和这些值的 `llvm::xxh3_64bits` 哈希生成候选文件名，并检查该文件是否已存在，若不存在则选定为临时路径。若循环结束后仍未找到不冲突的路径，则返回 `CacheError`。

选定临时路径后，使用 `clore::support::write_utf8_text_file` 将序列化内容写入该临时文件。接着尝试通过 `fs::rename` 将临时文件原子地替换到目标路径。若 `rename` 因 `permission_denied`、`file_exists` 或 `operation_not_permitted` 而失败，则先删除目标文件（若不存在则忽略错误），再重新尝试 `rename`。若最终重命名仍失败，则返回 `CacheError`。整个过程中所有的文件系统操作和序列化错误都会被捕获并包装为 `CacheError` 返回。

#### Side Effects

- Creates a clice cache directory if it does not exist.
- Writes a temporary file on disk containing serialized JSON data.
- Renames the temporary file to the final cache path, potentially overwriting an existing cache file.
- Removes the existing cache file if rename fails due to permission or file-exists issues.

#### Reads From

- `std::string_view workspace_root` parameter
- `const CliceCacheData& data` parameter
- Filesystem state (existence of temp candidate paths via `fs::exists`)
- System clock (for timestamp used in temp file name)
- Process ID through `llvm::sys::Process::getProcessId()`

#### Writes To

- Clice cache file at path derived from `workspace_root`
- Temporary file (deleted or moved after rename)
- Parent directory of clice cache (via `fs::create_directories`)

#### Usage Patterns

- Called to save/update clice cache data after extraction.
- Used similarly to `save_extract_cache` but for a different data type.
- Expected to be paired with `load_clice_cache` for persistence.

### `clore::extract::cache::save_extract_cache`

Declaration: `extract/cache.cppm:91`

Definition: `extract/cache.cppm:533`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数首先通过调用 `clore::extract::cache::cache_directory` 获取缓存根目录，并尝试创建该目录。接着构造一个 `SerializedCacheData` 对象，其内部维护路径索引表 `path_ids` 和序列化条目列表。对于每个输入的 `CacheRecord`，它使用 `clore::extract::cache::split_cache_key` 解析缓存键，验证记录中的 `compile_signature` 与键解析结果一致，然后通过 `clore::extract::cache::encode_dependency_snapshot` 将依赖快照编码为扁平化结构，同时借助内部 lambda `intern_path` 对路径字符串进行规范化并分配唯一索引，最终组装成 `SerializedCacheEntry` 并追加到 `data.entries` 中。整个数据结构最终由 `json::to_json` 序列化为 JSON 字符串。

写入阶段采用“先写临时文件，再原子重命名”的策略。它利用系统时间戳、进程 ID 和随机数生成唯一的临时文件路径，最多重试 32 次避免冲突。序列化内容通过 `clore::support::write_utf8_text_file` 写入临时文件，然后使用 `fs::rename` 将其移动到目标路径 `cache.json`。若重命名因权限或文件冲突失败，则先尝试删除目标文件再重试。任何步骤失败都会立即返回带有描述性消息的 `CacheError`。

#### Side Effects

- creates the extract cache directory via `fs::create_directories` if it does not exist
- writes a temporary JSON file under the cache directory
- renames the temporary file to `cache.json`, replacing any existing file
- may call `fs::remove` on the target path if the initial rename fails with certain errors

#### Reads From

- parameter `workspace_root` of type `std::string_view`
- parameter `records` of type `const std::unordered_map<std::string, CacheRecord>&`
- internal state of `split_cache_key` and `normalize_path_string`
- internal `json::to_json` reading the built `SerializedCacheData` object

#### Writes To

- filesystem: the cache directory (created if missing)
- filesystem: temporary file with name based on process id, timestamp, and nonce
- filesystem: the final `cache.json` file under the cache root directory
- filesystem: if rename conflict, the existing `cache.json` file is removed before replacement

#### Usage Patterns

- called during extract caching to persist processed `CacheRecord` data to disk
- used after building a collection of cache records for the workspace

### `clore::extract::cache::split_cache_key`

Declaration: `extract/cache.cppm:79`

Definition: `extract/cache.cppm:238`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

The function first locates the last occurrence of the delimiter constant `kCacheKeyDelimiter` within the input `cache_key` using `rfind`. If no delimiter is found, or if either the path portion or the signature portion is empty after splitting, an `CacheError` is returned with a descriptive message. Otherwise, the signature portion is parsed as a `std::uint64_t` via `std::from_chars`; if parsing fails or does not consume the entire substring, an error is again returned.

On success, a `CacheKeyParts` structure is returned, populating its `path` field with a `std::string` copy of the path substring and its `compile_signature` field with the parsed integer. The implementation relies on the character-level operations of `std::string_view` for substring extraction and on the standard library’s `std::from_chars` for error‑safe integer conversion. No external dependencies beyond the language runtime and the module’s own error and part types are introduced.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache_key` 参数
- `kCacheKeyDelimiter` 常量

#### Usage Patterns

- 解析由 `build_cache_key` 生成的缓存键
- 从组合缓存键中提取文件路径和签名
- 在使用缓存键组件前验证其格式

## Internal Structure

`extract:cache` 是代码提取系统的持久化与失效管理层，负责将提取结果（AST、扫描数据、依赖状态）按缓存键存储和恢复，并检测依赖文件是否变更以决定缓存是否有效。模块依赖 `extract:ast`、`extract:compiler` 和 `extract:scan` 提供的上层数据结构，同时使用 `support` 模块的基础工具；内部通过匿名命名空间隔离实现细节，包括序列化格式（`SerializedCacheData`、`SerializedCacheEntry`、`SerializedDependencySnapshot`）、缓存文件路径生成、路径规范化与哈希，以及基于多线程的依赖快照捕获与变更检测。公开接口（如 `build_cache_key`、`split_cache_key`、`capture_dependency_snapshot`、`dependencies_changed`、`save_extract_cache`/`load_extract_cache`、`save_clice_cache`/`load_clice_cache`）围绕编译签名和文件哈希构建稳定标识，实现可复用的缓存逻辑，与上层提取流程（编译、扫描、AST）形成清晰的调用边界。

## Related Pages

- [Module extract:ast](ast.md)
- [Module extract:compiler](compiler.md)
- [Module extract:scan](scan.md)
- [Module support](../support/index.md)

