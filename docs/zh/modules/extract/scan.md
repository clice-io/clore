---
title: 'Module extract:scan'
description: 'extract:scan 模块是 clore::extract 中负责 C++ 源文件依赖关系扫描与模块信息提取的核心模块。它通过 Clang 的预处理指令扫描器（scan_module_decl）快速获取模块声明、接口单元标识和导入列表，并可基于完整编译数据库对单个文件进行深层扫描（scan_file），返回包含模块名称、导入、包含指令及错误的 ScanResult。同时，该模块支持异步构建模块间的 DependencyGraph（通过 build_dependency_graph_async），并提供了 topological_order 函数对图进行拓扑排序，为后续编译或分析提供有序的依赖列表。'
layout: doc
template: doc
---

# Module `extract:scan`

## Summary

`extract:scan` 模块是 `clore::extract` 中负责 C++ 源文件依赖关系扫描与模块信息提取的核心模块。它通过 Clang 的预处理指令扫描器（`scan_module_decl`）快速获取模块声明、接口单元标识和导入列表，并可基于完整编译数据库对单个文件进行深层扫描（`scan_file`），返回包含模块名称、导入、包含指令及错误的 `ScanResult`。同时，该模块支持异步构建模块间的 `DependencyGraph`（通过 `build_dependency_graph_async`），并提供了 `topological_order` 函数对图进行拓扑排序，为后续编译或分析提供有序的依赖列表。

在公开的实现范围上，该模块定义了 `ScanResult`、`ScanError`、`ScanCache`、`DependencyGraph`、`DependencyEdge` 以及 `IncludeInfo` 等核心数据结构。`ScanCache` 作为跨次扫描的持久缓存，可避免重复解析相同文件；而依赖图构建过程利用 `MissingScanTask`、`PreparedScanEntry` 等内部机制高效管理未扫描任务。调用者应通过上述公开接口触发扫描流程，并根据返回的结果或错误信息进行后续处理，同时注意在文件系统或编译数据库变更时清除缓存以保证数据一致性。

## Imports

- [`extract:compiler`](compiler.md)
- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:cache`](cache.md)

## Types

### `clore::extract::DependencyEdge`

Declaration: `extract/scan.cppm:51`

Definition: `extract/scan.cppm:51`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The struct `clore::extract::DependencyEdge` stores a directed dependency edge using two `std::string` members: `clore::extract::DependencyEdge::from` and `clore::extract::DependencyEdge::to`. No invariants are enforced beyond the usual validity of the string values; the edge is represented purely by these two named fields, and the struct relies on implicitly defined special member functions for construction, copy, and move operations. The absence of user‑defined constructors or member functions means that the two strings can be set directly, and the struct is trivially aggregate‑initializable.

#### Invariants

- The struct has exactly two `std::string` members: `from` and `to`.
- Both members are publicly accessible.

#### Key Members

- `from`: the source node identifier.
- `to`: the target node identifier.

#### Usage Patterns

- Used in the `clore::extract` module to represent dependencies between extracted symbols.
- Edges are combined into lists or sets to form dependency graphs.

### `clore::extract::DependencyGraph`

Declaration: `extract/scan.cppm:56`

Definition: `extract/scan.cppm:56`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::DependencyGraph` 使用简单的边列表表示来存储依赖关系图。其核心数据是两个平行的 `std::vector`：`files` 存放所有被扫描的源文件路径（作为 `std::string`），而 `edges` 存放由 `DependencyEdge` 实例描述的依赖边。每条边隐含地关联 `files` 中的两个元素（通常通过索引或字符串引用），形成有向依赖关系。这种结构避免了复杂的邻接表，便于序列化并对所有依赖进行线性遍历，但查找特定节点或逆依赖时需要扫描整个 `edges` 向量。

#### Invariants

- Each element in `edges` references indices or paths in `files` (implied by typical dependency graph usage, but not explicitly guaranteed by the evidence).

#### Key Members

- `files`: a `std::vector<std::string>` of file paths.
- `edges`: a `std::vector<DependencyEdge>` of dependency relationships.

#### Usage Patterns

- Used to represent the complete dependency information extracted from source files.
- Consumed by downstream analysis or transformation passes.

### `clore::extract::IncludeInfo`

Declaration: `extract/scan.cppm:24`

Definition: `extract/scan.cppm:24`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::IncludeInfo` 是一个简单的值类型，用于封装一个 include 指令的解析结果。其内部由两个公有成员组成：`path` 是一个 `std::string`，默认初始化为空字符串，存储被包含文件的路径；`is_angled` 是一个 `bool`，默认初始化为 `false`，指示该 include 使用的是尖括号形式（`<...>`）还是引号形式（`"..."`）。该结构体没有自定义构造函数或成员函数，所有操作完全依赖默认成员初始化。由于其成员均为公有且直接可写，外部代码可在解析过程中独立设置每个字段，无需保持任何内部一致性的额外步骤。唯一的隐含不变量是 `path` 的内容应为一个合法的文件路径（或为空），但该条件并非由类型本身强制。

#### Invariants

- The `path` member holds the include path as specified in the source
- The `is_angled` member distinguishes between `#include <...>` (true) and `#include "..."` (false)

#### Key Members

- `path`
- `is_angled`

#### Usage Patterns

- Created when parsing include directives from source code
- Used to reconstruct or analyze include statements

### `clore::extract::ScanCache`

Declaration: `extract/scan.cppm:40`

Definition: `extract/scan.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ScanCache` 的内部实现是一个简单的包装，其唯一的数据成员是 `std::unordered_map<std::string, ScanResult> scan_results`。该映射将扫描对象的标识符（通常为文件路径）与被缓存的分析结果相关联，从而避免在连续依赖扫描中重复执行代价高昂的文件解析。实现层面没有任何额外的同步或生命周期管理；调用者必须遵从约定的不变性：当编译数据库或文件系统状态发生变化时，必须通过丢弃或清空整个 `ScanCache` 实例来强制失效，否则映射中陈旧的 `ScanResult` 条目可能导致错误的扫描结果。`scan_results` 默认初始化为空映射，插入和查找完全依赖标准库的无序映射行为。

#### Invariants

- Cache entries are valid only as long as the compilation database and file system state remain unchanged.
- The map key uniquely identifies a scan target (e.g., source file or header).
- The cache should be cleared or replaced when external state changes.

#### Key Members

- `scan_results`: the underlying hash map storing cached scan results.

#### Usage Patterns

- Used by scan functions to avoid redundant re‑scanning of unchanged input files.
- Callers are responsible for clearing or discarding the cache when the compilation DB or file system is modified.
- Typically passed by reference to scan utilities so that results accumulate across multiple calls.

### `clore::extract::ScanError`

Declaration: `extract/scan.cppm:20`

Definition: `extract/scan.cppm:20`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该结构体被实现为一个简单的聚合类型，仅包含一个 `std::string` 类型的 `message` 成员，用于保存扫描过程中产生的错误描述。不要求任何特殊的构造、赋值或析构逻辑，因为 `std::string` 本身负责内存管理并满足值语义。不变量方面，`message` 可能为空字符串以表示无具体错误，但该行为取决于构造和赋值处；没有额外的约束由结构体自身强制。实现上不涉及虚函数、继承或自定义内存分配器，因此实例的生存期和复制完全由标准字符串的规则决定。

#### Key Members

- `message` field (type `std::string`)

### `clore::extract::ScanResult`

Declaration: `extract/scan.cppm:29`

Definition: `extract/scan.cppm:29`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ScanResult` 是扫描器输出的聚合结构，内部通过四个字段组装模块元数据。`module_name` 记录当前扫描单元的名称，`is_interface_unit` 标记是否为模块接口单元，两者配合标识模块身份。`includes` 存储扫描过程中收集的所有 `IncludeInfo` 条目，`module_imports` 记录该单元导入的其他模块名列表；两个容器在扫描完成后应保持完整且无重复项。结构体直接暴露所有字段，未定义构造函数或成员函数，因此字段初始值（空字符串、`false`、空向量）构成了正确的默认不变量——未扫描或扫描失败时返回的 `ScanResult` 应处于该干净状态。

#### Invariants

- All vectors are properly initialized (default-constructed empty).
- `module_name` is a valid empty or non-empty string.
- `is_interface_unit` is either `true` or `false`.

#### Key Members

- `module_name`
- `is_interface_unit`
- `includes`
- `module_imports`

#### Usage Patterns

- Returned from scanning functions to represent the parsed module data.
- Inspected by callers to access the module name, imports, includes, and interface status.

## Functions

### `clore::extract::build_dependency_graph_async`

Declaration: `extract/scan.cppm:61`

Definition: `extract/scan.cppm:370`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数首先清空 `graph.files` 和 `graph.edges`，然后遍历 `db.entries`，对每个条目调用 `prepare_scan_entry` 得到 `PreparedScanEntry`，将其 `normalized_file` 加入 `graph.files` 并存入 `prepared_entries`。接着利用 `cache` 中的 `scan_results` 查找每个条目的 `cache_key`：若命中则记录在 `cached_results` 中；否则将未命中的条目按 `cache_key` 去重后构造 `MissingScanTask` 列表，并通过 `missing_task_indices` 记录每个条目在缺失任务中的索引。随后并发运行所有缺失任务的异步扫描（`kota::when_all`），等待结果并处理可能的 `ScanError`。

处理完所有扫描结果后，函数再次遍历每个数据库条目，根据 `cached_results` 或 `scanned_results` 获取对应的 `ScanResult`。对每个文件的 `result.includes`，将包含路径规范化后，若该路径存在于 `entry_files` 集合中，则构造一条从当前文件 `normalized` 到包含文件 `inc_normalized` 的 `DependencyEdge`，并通过 `emitted_edges` 集合去重。最后，若 `cache` 非空，则将本次获得的 `ScanResult` 写回 `cache->scan_results` 以供后续复用。

#### Side Effects

- clears `graph.files` and `graph.edges`
- pushes file paths into `graph.files`
- pushes `DependencyEdge` objects into `graph.edges`
- inserts or assigns scan results into `scan_results` map when cache is non-null

#### Reads From

- `db.entries`
- `prepared_entries[idx]` fields including `normalized_file`, `cache_key`
- `entry.directory`
- `scan_result->includes`
- `scan_results->find(cache_key)`

#### Writes To

- `graph.files`
- `graph.edges`
- `scan_results` map (via `insert_or_assign`)

#### Usage Patterns

- called to asynchronously construct a dependency graph before topological ordering
- used in build or analysis pipelines where concurrency is needed
- invoked with a `ScanCache*` to accelerate repeated scans

### `clore::extract::scan_file`

Declaration: `extract/scan.cppm:44`

Definition: `extract/scan.cppm:238`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::scan_file` 首先检查 `entry.arguments` 是否为空，若为空则立即返回包含错误信息的 `std::unexpected<ScanError>`。随后，它通过标准化路径读取源文件内容，并调用 `scan_module_decl` 快速扫描模块声明，填充 `ScanResult` 中的模块名称和接口单元标识。接着，函数创建一个编译器实例，并将预处理动作设为仅运行预处理器，然后构造 `ScanAction` 对象（关联同一个 `ScanResult`），依次调用 `BeginSourceFile` 和 `Execute` 执行实际的预处理扫描，借助 `ScanPPCallbacks` 收集包含指令、导入声明等依赖信息。若任何步骤失败（如文件读取错误、编译器实例创建失败、预处理执行错误），函数都会以对应的 `ScanError` 提前返回；成功时则返回填充完整的 `ScanResult`。整个流程依赖 `scan_module_decl` 进行快速模块声明扫描，并依赖编译器基础设施（`clang::CompilerInstance`、`ScanAction`）完成预处理阶段的依赖解析。

#### Side Effects

- Reads source file from disk
- Creates a Clang compiler instance (may query toolchain cache)
- Mutates the `ScanResult` object passed by reference inside `ScanAction`
- Calls `scan_module_decl` which modifies `ScanResult`
- Executes Clang preprocessor, which may produce diagnostic output
- Consumes LLVM errors via `llvm::consumeError`

#### Reads From

- `entry` compilation entry parameters
- Source file on disk (via `std::ifstream`)
- Toolchain cache (via `create_compiler_instance`)

#### Writes To

- `result` local variable of type `ScanResult` (populated with symbol information)
- Compiler instance frontend options (sets `ProgramAction`, clears output paths)
- LLVM error handling (consumes error)

#### Usage Patterns

- Called for each file in a compilation database during project scanning
- Used as the primary entry point for extracting symbols from a single translation unit

### `clore::extract::scan_module_decl`

Declaration: `extract/scan.cppm:49`

Definition: `extract/scan.cppm:141`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/scan-module-decl.md)

函数 `clore::extract::scan_module_decl` 利用 Clang 的依赖指令扫描器（`clang::scanSourceForDependencyDirectives`）对给定的文件内容进行快速模块声明分析，避免运行完整预处理器。内部首先将源文本解析为 `tokens` 和 `directives` 向量；若扫描失败则直接返回，不修改 `ScanResult`。随后遍历每个指令：对于 `cxx_export_module_decl` 或 `cxx_module_decl` 类型的指令，跳过 `export` 和 `module` 关键字，通过收集后续标识符（排除空白和纯标点）构建模块名称，并据此设置 `result.module_name` 和 `result.is_interface_unit`（仅 `export module` 表示接口单元）；对于 `cxx_import_decl` 类型的指令，跳过 `import` 关键字后收集导入名称，调用 `normalize_partition_import` 进行规范化，并在去重后加入 `result.module_imports`。该函数依赖 `ScanResult` 结构体字段和 `normalize_partition_import` 辅助函数，同时借助两个内联 lambda（`is_whitespace_only` 和 `is_punctuation_only`）辅助令牌分类。

#### Side Effects

- Modifies the `ScanResult` parameter by setting `module_name`, `is_interface_unit`, and `module_imports`.
- Allocates memory for `std::string` and `std::vector` members of `ScanResult`.

#### Reads From

- `file_content` (`string_view`) - source text of a translation unit.
- `result` (`ScanResult`&) - reads `module_name` and `module_imports` for duplicate checking.
- Directives and tokens produced by `clang::scanSourceForDependencyDirectives`.

#### Writes To

- `result.module_name` - set to the extracted module name.
- `result.is_interface_unit` - set to `true` if `export module` declaration, else `false`.
- `result.module_imports` - appended with normalized import names.

#### Usage Patterns

- Called by `clore::extract::scan_file` to perform fast module scanning on source files.
- Used as a lightweight alternative to full preprocessing for module dependency discovery.

### `clore::extract::topological_order`

Declaration: `extract/scan.cppm:66`

Definition: `extract/scan.cppm:495`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数从给定的 `DependencyGraph` 中提取 `graph.files` 的拓扑顺序。首先，它为每个文件名初始化一个零入度的映射 `in_degree`。然后，遍历 `graph.edges`，对于每条边，将 `edge.to` 作为邻接键，将 `edge.from` 追加到 `adj[edge.to]` 中，并递增 `in_degree[edge.from]`，从而构建一个有向图：入边从依赖者指向被依赖者，即 `edge.from` 依赖于 `edge.to`。之后，将 `graph.files`、`adj` 和 `in_degree` 转发给 `clore::support::topological_order` 执行标准的 Kahn 算法。若该调用返回无值，则函数返回一个包含描述性消息的 `ScanError`，表明检测到循环依赖；否则返回计算得到的顺序向量。整个过程完全依赖于 `DependencyGraph` 的数据结构以及通用排序工具，不涉及任何扫描或预处理逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `DependencyGraph`& graph (graph`.files`, graph`.edges`)

#### Usage Patterns

- Called after building a dependency graph to determine a valid compilation order
- Used to detect cycles in the include graph

## Internal Structure

`extract:scan` 模块负责 C++ 模块文件的依赖扫描和依赖图构建。其核心分解为：公开的扫描入口 `scan_file` 和 `scan_module_decl` 分别执行完整扫描和快速模块声明扫描；异步构建依赖图的 `build_dependency_graph_async` 以及拓扑排序 `topological_order` 构成高层工作流。内部实现利用匿名命名空间隔离具体细节，包括 `ScanAction`（管理一次扫描的预处理器回调）、`ScanPPCallbacks`（捕获 `#include` 和模块导入指令）、`PreparedScanEntry`（缓存键和归一化路径准备）以及 `MissingScanTask`（表示需扫描的缺失文件）。依赖方面，模块导入 `extract:compiler` 获取编译数据库信息，并依赖 `support` 模块提供的文件读写、路径归一化和缓存键生成等基础工具。

模块内部按职责分为三层：底层为 `scan_module_decl` 和 `ScanPPCallbacks` 实现的词法和预处理回调逻辑；中层由 `scan_file` 和 `run_scan_task` 协调条目扫描，并利用 `ScanCache` 持久化重复扫描结果；顶层是依赖图构建——`build_dependency_graph_async` 通过事件循环 `kota::event_loop` 调度 `MissingScanTask`，将扫描结果填充至 `DependencyGraph`（包含文件列表和依赖边），最后 `topological_order` 对图进行拓扑排序。缓存机制贯穿各层：`ScanCache` 缓存 `ScanResult`，通过 `cache_key` 和 `compile_signature` 实现快速命中，避免重复解析。实现结构清晰分离了同步扫描、异步调度和图构建的关注点。

## Related Pages

- [Module extract:compiler](compiler.md)
- [Module support](../support/index.md)

