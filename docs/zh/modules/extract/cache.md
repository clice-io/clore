---
title: 'Module extract:cache'
description: 'extract:cache 模块是 clore::extract 中负责提取结果缓存的持久化与失效管理子系统。它提供了一组正交的公开操作：通过 build_cache_key 和 build_compile_signature 生成标准化的缓存键与编译签名；利用 capture_dependency_snapshot 和 dependencies_changed 捕获当前依赖状态并判断是否过期；以 load_extract_cache / save_extract_cache 和 load_clice_cache / save_clice_cache 分别支持简单的整数值缓存和完整的 Clice 工作区缓存数据的存储与加载。此外，hash_file 用于计算文件内容的哈希值以支撑依赖追踪，split_cache_key 可将组合键解析为组成部分供外部检查。这些函数共同构成了一个既可独立使用又可组合的缓存工具集，调用者需自行处理返回的 CacheError 以保证缓存一致性。'
layout: doc
template: doc
---

# Module `extract:cache`

## Summary

`extract:cache` 模块是 `clore::extract` 中负责提取结果缓存的持久化与失效管理子系统。它提供了一组正交的公开操作：通过 `build_cache_key` 和 `build_compile_signature` 生成标准化的缓存键与编译签名；利用 `capture_dependency_snapshot` 和 `dependencies_changed` 捕获当前依赖状态并判断是否过期；以 `load_extract_cache` / `save_extract_cache` 和 `load_clice_cache` / `save_clice_cache` 分别支持简单的整数值缓存和完整的 Clice 工作区缓存数据的存储与加载。此外，`hash_file` 用于计算文件内容的哈希值以支撑依赖追踪，`split_cache_key` 可将组合键解析为组成部分供外部检查。这些函数共同构成了一个既可独立使用又可组合的缓存工具集，调用者需自行处理返回的 `CacheError` 以保证缓存一致性。

在内部实现上，模块定义了多个用于序列化的数据结构（如 `SerializedCacheData`、`SerializedDependencySnapshot`），并采用多线程并行方式（通过 `DependencyHashTaskResult` 和并行阈值）加速依赖快照的捕获。缓存路径的确定、临时文件写入与原子重命名（如 `make_unique_tmp_path`）等细节被封装在匿名命名空间函数中，确保公开接口的简洁性与正确性。无论是用于提取流水线的增量更新，还是用于 Clice 工作区缓存的一致性校验，该模块都承担了缓存生命周期管理的核心逻辑。

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

`CacheError` 是一个简单的值类型，其唯一的数据成员 `message` 类型为 `std::string`，用于存储错误描述。该结构依赖编译器隐式生成的特殊成员函数（默认构造函数、拷贝构造函数、移动构造函数、拷贝赋值运算符、移动赋值运算符与析构函数），这些函数对 `message` 执行对应的浅层或移动语义操作。由于未定义额外的不变量，`CacheError` 的构造与赋值行为完全由 `std::string` 的接口决定，内部状态仅受 `message` 内容的合法性约束。

#### Invariants

- The `message` member must be set to a descriptive error string when an error occurs.

#### Key Members

- `message`: a `std::string` that stores the error description.

#### Usage Patterns

- Can be thrown as an exception or returned as an error result in cache-related functions.

### `clore::extract::cache::CacheKeyParts`

Declaration: `extract/cache.cppm:24`

Definition: `extract/cache.cppm:24`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

实现中，`clore::extract::cache::CacheKeyParts` 是一个平凡的聚合结构体，其内部仅由两个公开数据成员组成：`std::string path` 用于存放资源路径，以及 `std::uint64_t compile_signature` 用于存放编译签名。该结构体没有定义任何构造函数、析构函数或运算符重载，完全依赖默认的复制、移动及比较语义。因此，它的生命周期管理和值传递行为完全由编译器生成的成员函数处理，不隐含任何额外的不变式约束；所有对等判别或哈希计算均需要由外部逻辑实现。这种设计使得结构体在作为缓存键的组成部分时保持轻量，并允许调用方按成员顺序直接进行初始化或构造。

#### Invariants

- No explicit invariants documented; likely expects `path` to be a valid file path and `compile_signature` to be non-zero? Not confirmed.

#### Key Members

- `std::string path`
- `std::uint64_t compile_signature`

#### Usage Patterns

- Used to uniquely identify cache entries based on a file and its compile-time signature.

### `clore::extract::cache::CacheRecord`

Declaration: `extract/cache.cppm:36`

Definition: `extract/cache.cppm:36`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CacheRecord` 的核心职责是将一次完整提取操作的所有可缓存状态聚合为单一记录，以便序列化、反序列化以及后续的失效判断。其内部字段可划分为校验元数据与提取结果两部分：`compile_signature` 和 `source_hash` 分别记录编译环境签名和源文件的哈希值，用于快速判定缓存是否过期；`ast_deps`、`scan` 和 `ast` 则依次保存依赖快照、扫描结果以及最终的 AST 结果。关键的不变量要求：当 `source_hash` 或 `compile_signature` 在加载时与当前计算值不匹配时，整条记录必须视为无效，其余字段的内容不得用于后续编译流程。成员初始化顺序与声明顺序一致，所有可平凡复制的整数成员被显式零初始化，而复合类型成员则通过默认构造保持空状态，从而保证一个默认构造的 `CacheRecord` 始终处于“完全失效”的基态。

#### Invariants

- Fields are default-initialized to zero or default-constructed.
- `source_hash` and `compile_signature` are expected to be non-zero after a successful extraction.
- `ast_deps`, scan, and ast hold consistent results from the same extraction.

#### Key Members

- `compile_signature`
- `source_hash`
- `ast_deps`
- scan
- ast

#### Usage Patterns

- Populated by the extraction pipeline after a successful extraction.
- Retrieved by the cache module to return cached results.
- Stored in a cache container keyed by source hash and compile signature.

### `clore::extract::cache::CliceCacheData`

Declaration: `extract/cache.cppm:68`

Definition: `extract/cache.cppm:68`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

结构体 `clore::extract::cache::CliceCacheData` 是缓存系统的核心数据容器，内部由三个 `std::vector` 成员构成：`paths` 存储文件路径字符串，`pch` 与 `pcm` 分别持有与路径对应的预编译头缓存条目和预编译模块缓存条目。这三个向量通过隐式的索引关联保持同步：对于任意有效索引 `i`，`paths[i]`、`pch[i]` 与 `pcm[i]` 共同描述同一个编译单元的缓存状态。不变量要求所有向量长度始终相等，且 `pch` 与 `pcm` 中的条目必须与 `paths` 中的路径一一对应，读取或写入操作需确保此同步关系不被破坏。

#### Invariants

- `paths`、`pch`、`pcm` 中的各个字段是相互独立的向量
- 无显式的不变式约束

#### Key Members

- `paths`
- `pch`
- `pcm`

#### Usage Patterns

- 作为缓存系统的一部分，存储提取的文件路径和对应的编译产物数据
- 可能由缓存填充逻辑写入，由读取逻辑遍历并使用

### `clore::extract::cache::CliceCacheDepEntry`

Declaration: `extract/cache.cppm:46`

Definition: `extract/cache.cppm:46`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CliceCacheDepEntry` 的实现是一个极简的聚合体，仅包含两个整数成员：`path`（`std::uint32_t` 类型，默认值为 `0`）和 `hash`（`std::uint64_t` 类型，默认值为 `0`）。这种布局直接呼应其文档声明的目的——与 `clice/src/server/workspace.cpp` 中的 `CacheData` 保持模式兼容。因此，该结构体未定义任何自定义构造函数、析构函数或赋值运算符，完全依赖编译器生成的默认实现，以确保其对象表示（包括成员顺序、对齐和填充）与外部缓存格式精确匹配。两个成员均初始化为 `0`，这在内部被用作“空”或“未使用”依存条目的哨兵值，任何有效条目必须至少设置 `path` 或 `hash` 中的一个非零值。

#### Invariants

- Fields are default-initialized to zero.
- The struct layout is compatible with an external `CacheData` schema.

#### Key Members

- `path`
- `hash`

#### Usage Patterns

- Used in clice workspace cache structures for dependency tracking.
- Serialized or interpreted in a schema-compatible manner with `CacheData`.

### `clore::extract::cache::CliceCachePCHEntry`

Declaration: `extract/cache.cppm:51`

Definition: `extract/cache.cppm:51`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::CliceCachePCHEntry` 是一个仅包含数据的聚合体，用于持久化单个预编译头缓存条目。它的成员直接映射到磁盘上缓存的字段：`filename` 是 PCH 文件路径；`source_file` 是源文件索引；`hash` 是内容的校验和；`bound` 记录该 PCH 可覆盖的依赖边界计数器；`build_at` 是构建时间戳；`deps` 是一个 `std::vector<CliceCacheDepEntry>` 列表，记录该 PCH 所依赖的其它条目。该结构体没有用户定义的构造函数或特殊成员函数，依靠聚合初始化来设置所有字段，并且所有成员在初始化后均保持平凡的可复制语义。为保持缓存一致性，关键不变量包括：`hash` 必须与 `deps` 所对应的依赖图状态相匹配，且 `bound` 必须在同一缓存文件内的所有条目中单调递增。

### `clore::extract::cache::CliceCachePCMEntry`

Declaration: `extract/cache.cppm:60`

Definition: `extract/cache.cppm:60`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

这是一个普通的聚合体，其所有数据成员均为公有。字段 `filename` 和 `module_name` 为 `std::string` 类型，分别存储 PCM 文件的路径和模块名称；`source_file` 和 `build_at` 被默认初始化为 `0`，分别用于引用源文件索引和记录构建时间戳；`deps` 是 `std::vector<CliceCacheDepEntry>` 类型的空向量，负责容纳该 PCM 条目的依赖项列表。该结构体没有自定义构造函数、析构函数或赋值运算符，完全依赖编译器生成的默认实现，因此不存在额外的运行时校验或资源管理逻辑。

### `clore::extract::cache::DependencySnapshot`

Declaration: `extract/cache.cppm:29`

Definition: `extract/cache.cppm:29`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该结构体封装了构建时依赖项的快照，内部使用三个平行向量 `files`、`hashes` 和 `mtimes` 分别存储依赖项的文件路径、内容哈希值和最后修改时间。这三个向量的长度始终一致，构成隐式的不变量，任何修改操作都必须维护这一关系。`build_at` 记录快照生成时的单调时间戳（默认初始化为 `0`），用于后续判断快照是否已过时。所有成员均通过公共访问，但外部代码不应直接篡改向量长度的匹配性。

## Functions

### `clore::extract::cache::build_cache_key`

Declaration: `extract/cache.cppm:76`

Definition: `extract/cache.cppm:228`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::build_cache_key` 通过字符串拼接构造缓存键。它首先为目标字符串预留空间，大小为 `normalized_path` 的长度加 1（分隔符）再加 20（编译签数字符串的典型长度）。随后依次追加 `normalized_path`、常量 `kCacheKeyDelimiter` 以及通过 `std::to_string` 将 `compile_signature` 转换成的字符串。整个操作不涉及复杂控制流或其他外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `normalized_path`
- `compile_signature`

#### Writes To

- returned `std::string`

#### Usage Patterns

- Used to build a key for caching extraction results based on a file path and a compile signature.

### `clore::extract::cache::build_compile_signature`

Declaration: `extract/cache.cppm:74`

Definition: `extract/cache.cppm:224`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::build_compile_signature` 作为缓存层的入口，负责为传入的 `CompileEntry` 生成编译签名。其实现极为简洁：完全委托给非缓存计算函数 `clore::extract::build_compile_signature`，将 `entry` 原样转发。因此，该函数不包含自身算法或状态修改，仅作为适配器；实际签名计算、依赖关系和内部控制流全部交由下游函数处理，缓存层仅利用其返回值作为后续缓存键的组成部分。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `entry` parameter (type `const CompileEntry&`)

#### Usage Patterns

- Used to generate a unique compile signature for cache key computation.
- Called in caching logic to identify compile configurations.

### `clore::extract::cache::capture_dependency_snapshot`

Declaration: `extract/cache.cppm:83`

Definition: `extract/cache.cppm:282`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数生成一个排序去重后的依赖文件快照，记录每个文件的路径、内容哈希和修改时间，并附上快照创建时间戳。首先对输入的文件路径列表进行规范化、排序与去重，然后依据硬件并发数分配并行任务。在每个工作线程中，对分配的文件依次获取文件状态（记录修改时间）并调用 `hash_file` 计算哈希值；若文件无法访问（如已被删除），则将哈希值置为 `0` 以标记变更，而非直接报错。所有线程完成后，若未出现错误，则将各线程的结果合并到 `DependencySnapshot` 的 `files`、`hashes` 和 `mtimes` 字段中，最终返回该快照。如果任一线程在早期检测到错误（由互斥锁保护的 `first_error` 指示），则提前返回该错误。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- input parameter `files: const std::vector<std::string>&`
- file content and metadata (via `hash_file` and `llvm::sys::fs::status`)

#### Writes To

- returned `DependencySnapshot` object (local allocation and population)

#### Usage Patterns

- called to capture dependency information for cache invalidation checks
- used before comparing with a previous snapshot via `dependencies_changed`

### `clore::extract::cache::dependencies_changed`

Declaration: `extract/cache.cppm:86`

Definition: `extract/cache.cppm:401`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::dependencies_changed` 首先执行快速边界检查：当 `snapshot.build_at` 非正或 `snapshot.files` 为空，或 `files`、`hashes`、`mtimes` 三个容器大小不一致时，立即返回 `true` 表示依赖已变更。之后根据文件数量选择执行路径：若数量不超过 `kParallelThreshold`（16），则顺序遍历每个索引并调用 `check_single_dependency`；一旦发现变更即提前停止并返回 `true`，否则返回 `false`。

对于超过阈值的较大依赖集合，函数改用并行策略。它通过 `std::thread::hardware_concurrency()` 获取硬件线程数，构造至少一个线程。每个工作线程处理连续的索引子区间，并在循环中先检查 `changed` 原子标志（使用 `memory_order_relaxed`），若已检测到变更则立即返回；否则调用 `check_single_dependency` 检查当前依赖，一旦发现变更则置位 `changed` 并退出。主线程等待所有线程结束后返回 `changed` 的最终值。该实现通过细粒度的原子检查避免了不必要的计算，同时保持了对大集合的并行加速。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `snapshot.build_at`
- `snapshot.files`
- `snapshot.hashes`
- `snapshot.mtimes`
- `std::thread::hardware_concurrency()`

#### Usage Patterns

- called before loading extract cache to determine freshness
- used to decide whether to re-extract dependencies

### `clore::extract::cache::hash_file`

Declaration: `extract/cache.cppm:81`

Definition: `extract/cache.cppm:270`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先调用 `llvm::MemoryBuffer::getFile`，尝试以只读方式打开由 `path` 指定的文件。如果文件无法读取（例如不存在或无权限），函数立即返回 `std::unexpected<CacheError>`，其中错误信息包含原始系统错误的描述。若读取成功，则对得到的 `llvm::MemoryBuffer` 内容调用 `llvm::xxh3_64bits`，利用 XXH3 算法快速计算整个文件的 64 位哈希值，并将其作为成功结果返回。整个实现仅依赖 LLVM 的文件读取与哈希工具，不含任何外部状态或缓存逻辑。

#### Side Effects

- reads from the file system via `llvm::MemoryBuffer::getFile`
- observable I/O: file read operation with potential latency and system calls

#### Reads From

- parameter `path`: the file path to read
- file content at `path`

#### Usage Patterns

- used by cache-key building functions to hash source files
- called by `build_compile_signature` or `capture_dependency_snapshot` to incorporate file content into cache keys
- part of the cache layer's file integrity verification

### `clore::extract::cache::load_clice_cache`

Declaration: `extract/cache.cppm:95`

Definition: `extract/cache.cppm:670`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

`clore::extract::cache::load_clice_cache` 首先通过 `clice_cache_file_path` 将 `workspace_root` 转换为缓存的磁盘路径。若该路径对应的文件不存在（且无系统错误），立即返回一个空的 `CliceCacheData`；若存在系统错误则返回 `CacheError`。文件存在时，使用 `clore::support::read_utf8_text_file` 读取其全部文本内容，若读取失败同样返回错误。成功读取后，调用 `json::from_json` 将 JSON 内容反序列化为 `CliceCacheData`；若反序列化失败，函数记录一条警告并返回一个空的 `CliceCacheData`（视为缓存已过期），否则返回解析得到的 `data`。整个过程不修改外部状态，完全依赖于文件系统和 JSON 解析库。

#### Side Effects

- performs filesystem existence check via `fs::exists`
- reads file content via `clore::support::read_utf8_text_file`
- logs a warning via `logging::warn` on JSON deserialization failure

#### Reads From

- the clice cache file path derived from `workspace_root`
- the file system at that path
- the content of the cache file

#### Writes To

- logging subsystem (warning message)

#### Usage Patterns

- load cached clice data before extraction
- check if a valid clice cache exists
- used in pair with `save_clice_cache`

### `clore::extract::cache::load_extract_cache`

Declaration: `extract/cache.cppm:88`

Definition: `extract/cache.cppm:457`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::load_extract_cache` 通过以下流程从磁盘加载提取缓存：首先调用 `cache_file_path` 计算缓存文件路径，若文件不存在则直接返回空映射；若文件存在但读取失败或 JSON 反序列化失败，则记录警告并同样返回空映射。若 `SerializedCacheData` 中的 `format_version` 与 `kExtractCacheFormatVersion` 不匹配，也视为过期缓存而返回空映射。随后遍历 `data.entries`，对每个条目验证 `source_file` 索引是否越界，调用 `decode_dependency_snapshot` 将 `ast_deps` 解码为 `DependencySnapshot`，并使用 `normalize_path_string` 和 `build_cache_key` 构建缓存键，最终填充返回的 `std::unordered_map<std::string, CacheRecord>`。任何解码或索引错误都会导致返回 `std::unexpected<CacheError>`。

#### Side Effects

- reads the extract cache file from disk
- logs warnings via `logging::warn` when cache is stale or corrupted

#### Reads From

- filesystem: cache file path derived from `workspace_root`
- parameter `workspace_root`

#### Writes To

- logging output via `logging::warn`

#### Usage Patterns

- called before extraction to check if a cached result exists
- used in conjunction with `save_extract_cache` for read-write caching

### `clore::extract::cache::save_clice_cache`

Declaration: `extract/cache.cppm:97`

Definition: `extract/cache.cppm:710`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先通过 `clice_cache_file_path` 解析出目标缓存文件路径 `cache_path`，并确保其父目录存在。随后调用 `json::to_json` 将 `CliceCacheData` 序列化为 JSON 字符串；若序列化失败则立即返回 `CacheError`。为了原子地写入缓存文件，函数采用“先写临时文件再重命名”的策略：利用 `llvm::xxh3_64bits` 对 `cache_path`、进程 ID `pid`、当前时间戳 `timestamp` 和重试序号 `attempt` 进行散列生成随机后缀，构造一个唯一的临时路径 `tmp_path`（最多尝试 32 次以避开已存在的文件）。通过 `clore::support::write_utf8_text_file` 将 JSON 内容写入 `tmp_path`。之后尝试将 `tmp_path` 重命名为 `cache_path`；若因权限不足、文件已存在或不允许操作等原因失败，则先尝试删除 `cache_path` 再重试一次重命名。所有文件系统操作均检查错误码并构造详细的 `CacheError` 返回。整个流程依赖 `clore::extract::cache` 内部的路径生成函数、JSON 序列化工具以及 filesystem 操作，并利用进程 ID 和单调时间戳确保临时文件唯一性，最终实现崩溃安全的缓存写入。

#### Side Effects

- Creates cache directories
- Writes temporary file with unique name
- Renames temporary file to final cache path
- Removes existing cache file if rename fails due to permission or file exists

#### Reads From

- `workspace_root` parameter
- `data` parameter
- system clock for timestamp
- current process ID
- filesystem state for existence checks

#### Writes To

- cache directory
- temporary cache file
- final cache file

#### Usage Patterns

- Persist clice extract cache
- Called after extraction to update cache data

### `clore::extract::cache::save_extract_cache`

Declaration: `extract/cache.cppm:91`

Definition: `extract/cache.cppm:533`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

该函数首先通过 `cache_directory(workspace_root)` 获取缓存根目录，并创建该目录。随后构造 `SerializedCacheData`：遍历每条 `CacheRecord`，调用 `split_cache_key(file)` 拆解键并验证 `compile_signature` 是否匹配，然后使用 `intern_path` 闭包对路径字符串进行标准化并分配 ID，将记录字段转换为 `SerializedCacheEntry`，其中依赖快照通过 `encode_dependency_snapshot` 序列化。全部记录填充至 `data` 后，调用 `json::to_json` 序列化为 JSON 字符串。

为确保写入原子性，函数在目标 `cache_path` 所在目录下生成唯一临时文件路径：循环最多 32 次，每次基于当前时间戳、进程 PID 和递增尝试号，通过 `llvm::xxh3_64bits` 计算随机数构造候选路径，直到 `fs::exists` 检测该路径不存在。将序列化内容通过 `clore::support::write_utf8_text_file` 写入临时文件。最后使用 `fs::rename` 将临时文件原子地移动到 `cache_path`；若因权限或文件存在错误导致重命名失败，则先删除目标文件再重试。整个过程中任何文件系统错误都会返回带有描述信息的 `CacheError`。

#### Side Effects

- Creates cache directory if it does not exist
- Writes a temporary JSON file to the filesystem
- Atomically renames the temporary file to the final cache path
- May remove an existing cache file before rename on permission or file-exists errors

#### Reads From

- `workspace_root` parameter
- `records` parameter (a map of file paths to `CacheRecord`)
- Filesystem: checks existence of temporary file candidates
- System clock for timestamp
- Process ID via `llvm::sys::Process::getProcessId`

#### Writes To

- Filesystem: cache directory under `workspace_root`
- Filesystem: `cache.json` file inside cache directory
- Filesystem: temporary files with `.tmp` extension

#### Usage Patterns

- Called to persist extract cache after a successful extraction pass
- Used to store incremental build cache for later reuse

### `clore::extract::cache::split_cache_key`

Declaration: `extract/cache.cppm:79`

Definition: `extract/cache.cppm:238`

Declaration: [`Namespace clore::extract::cache`](../../namespaces/clore/extract/cache/index.md)

函数 `clore::extract::cache::split_cache_key` 的核心算法是解析缓存键字符串。它首先使用 `std::string_view::rfind` 查找 `kCacheKeyDelimiter` 在输入 `cache_key` 中的最后一次出现位置；若未找到，则立即返回一个带有描述性消息的 `CacheError`。找到分隔符后，函数将字符串分割为 `path_part` 和 `signature_part` 两个子串，并对两者进行非空校验。随后，利用 `std::from_chars` 将 `signature_part` 解析为 `std::uint64_t` 类型的 `signature`，并检查解析是否完全消耗了整个子串。任一验证失败都会导致返回 `CacheError`。解析成功后，构造并返回 `CacheKeyParts` 对象，其 `path` 字段取自 `path_part` 的拷贝，`compile_signature` 字段为解析得到的 `signature`。该函数的实现完全依赖于 `kCacheKeyDelimiter` 常量的定义、`CacheKeyParts` 和 `CacheError` 的数据结构，以及 `std::from_chars` 的标准库功能。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The input parameter `cache_key` of type `std::string_view`
- The constant `kCacheKeyDelimiter`

#### Usage Patterns

- Parsing cache keys previously built by `build_cache_key`
- Extracting path and signature before cache lookup operations

## Internal Structure

模块 `extract:cache` 是提取流水线中专司缓存持久化与失效检测的内层组件，其外部接口与内部实现严格分离。公共 API 层提供键值构造（`build_cache_key`、`build_compile_signature`、`split_cache_key`）、文件哈希（`hash_file`）、依赖快照捕获与比较（`capture_dependency_snapshot`、`dependencies_changed`）以及针对提取结果与 Clice 缓存结构的存取操作（`load_extract_cache`/`save_extract_cache`、`load_clice_cache`/`save_clice_cache`）。这些函数依赖 `support` 模块的路径归一化、签名计算等基础工具，并通过模块导入（`extract:ast`、`extract:compiler`、`extract:scan`）获得上游提取与扫描数据结构，从而将编译产物与中间结果关联到唯一的缓存条目。

在内部实现上，模块通过匿名命名空间封装了序列化结构体（如 `SerializedCacheData`、`SerializedCacheEntry`、`SerializedDependencySnapshot`）、缓存路径解析以及依赖快照的编码/解码逻辑。并发哈希捕获采用线程池（`per_thread` 变量）与阈值（`kParallelThreshold`）控制，通过 `DependencyHashTaskResult` 收集并行任务结果；所有 I/O 操作（文件创建、重命名、删除）均经由原子写入（临时文件→重命名）策略保证缓存的文件级一致性。这种分层结构使得缓存管理逻辑独立于上层提取算法，仅通过清晰的公共接口和内部辅助函数完成数据的序列化、校验与失效判定。

## Related Pages

- [Module extract:ast](ast.md)
- [Module extract:compiler](compiler.md)
- [Module extract:scan](scan.md)
- [Module support](../support/index.md)

