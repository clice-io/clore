---
title: 'Module extract'
description: '模块 extract 负责对项目进行代码提取并管理相关的 AST 缓存，以支持后续的符号查询、引用分析等操作。它提供了一个异步入口 extract_project_async，调用方传入项目上下文句柄和事件循环后，该函数调度整个提取流程，包括加载扫描缓存、评估缓存有效性、批量解析 AST、构建模块信息，并将结果持久化到本地缓存中。提取完成后，可通过该命名空间下的同步查询函数（如 find_symbol、lookup_symbol）访问提取到的项目数据。'
layout: doc
template: doc
---

# Module `extract`

## Summary

模块 `extract` 负责对项目进行代码提取并管理相关的 AST 缓存，以支持后续的符号查询、引用分析等操作。它提供了一个异步入口 `extract_project_async`，调用方传入项目上下文句柄和事件循环后，该函数调度整个提取流程，包括加载扫描缓存、评估缓存有效性、批量解析 AST、构建模块信息，并将结果持久化到本地缓存中。提取完成后，可通过该命名空间下的同步查询函数（如 `find_symbol`、`lookup_symbol`）访问提取到的项目数据。

在公开实现层面上，模块仅暴露 `extract_project_async` 作为启动提取的唯一接口，以及 `ExtractError` 作为错误类型。内部复杂的缓存加载、AST 解析、依赖图构建等逻辑均封装在匿名命名空间和私有函数中，不对模块外部可见，从而保持了接口的简洁性和稳定性。模块依赖于 `config` 和 `support` 模块提供的配置和通用工具支持。

## Imports

- [`config`](../config/index.md)
- [`support`](../support/index.md)

## Imported By

- [`agent`](../agent/index.md)
- [`agent:tools`](../agent/tools.md)
- [`generate`](../generate/index.md)
- [`generate:analysis`](../generate/analysis.md)
- [`generate:common`](../generate/common.md)
- [`generate:diagram`](../generate/diagram.md)
- [`generate:dryrun`](../generate/dryrun.md)
- [`generate:evidence`](../generate/evidence.md)
- [`generate:evidence_builder`](../generate/index.md)
- [`generate:model`](../generate/model.md)
- [`generate:page`](../generate/page.md)
- [`generate:planner`](../generate/planner.md)
- [`generate:scheduler`](../generate/scheduler.md)
- [`generate:symbol`](../generate/symbol.md)

## Types

### `clore::extract::ExtractError`

Declaration: `src/extract/extract.cppm:43`

Definition: `src/extract/extract.cppm:43`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

struct `clore::extract::ExtractError` 的实现仅由一个 `std::string message` 成员构成，该成员负责存储错误描述信息。该类没有自定义的构造、析构或赋值操作，因此依赖编译器生成的默认函数，其内部状态完全由 `message` 的字符串内容所定义。任何对 `ExtractError` 实例的拷贝或移动都遵循 `std::string` 的对应语义。

#### Invariants

- The `message` string should contain a meaningful error description.

#### Key Members

- `message` (public data member of type `std::string`)

#### Usage Patterns

- Likely used as the exception type or error result in extraction functions where a descriptive string is sufficient.
- Other code can construct an `ExtractError` by providing a `std::string` argument to `message`.

## Functions

### `clore::extract::extract_project_async`

Declaration: `src/extract/extract.cppm:47`

Definition: `src/extract/extract.cppm:561`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数实现了一个异步协程工作流，用于从编译数据库提取完整的项目模型。它首先调用 `load_compdb` 加载编译数据库条目，然后根据 `config.filter` 和 `filter_root` 过滤条目，并标准化文件路径与缓存键。接着通过 `load_caches_async` 异步加载已有的 extract 缓存和 clice 缓存，并对每个条目进行 `CacheEvaluation`，利用 `compile_signature` 和 `source_hash` 判断 `scan_valid` 和 `ast_valid` 状态，从而决定后续是否可以跳过扫描或 AST 解析。之后构建 `DependencyGraph`（依赖 `build_dependency_graph_async`），并按拓扑顺序确定文件处理顺序。随后启动 `extract_ast_batch_async` 并行处理所有条目的 AST 解析。

在遍历每个条目时，该函数根据缓存命中情况从缓存记录或并行 AST 结果中获取 AST 数据，并解析每个符号的声明位置路径（通过 `resolve_symbol_location_path` 进行路径规范化与缓存）。它按过滤器筛选保留的符号和包含项，维护 `ProjectModel` 中的 symbol 映射与关系（继承、调用、引用），同时根据源哈希更新或删除 `cache_records`。全部条目处理完毕后，调用 `rebuild_model_indexes`、`build_module_info`（解析模块单元）和 `rebuild_lookup_maps` 完善模型结构。最后异步执行 `resolve_source_snippet` 为符号填充源代码片段，并调用 `save_caches_async` 持久化更新后的缓存，再统计并记录耗时信息后返回 `ProjectModel`。

#### Side Effects

- Writes log messages via `logging::info` and `logging::warn`
- Performs file I/O: reads compilation database file and cache files, writes cache files asynchronously
- Modifies global or passed-in state: fills `cache_evaluations`, `prepared_entries`, `seeded_scan_cache`, `dep_graph`, `model`, `cache_records`, `clice_cache`, `resolved_path_cache`
- Spawns asynchronous tasks on the event loop (`kota::task`, `kota::queue`, `co_await`)
- Potentially modifies `cache_records` by inserting, erasing, or updating entries
- Allocates memory for `ProjectModel`, `ScanCache`, `CacheEvaluation`, `PreparedEntryState`, `DependencyGraph`, and other containers

#### Reads From

- `config::TaskConfig &config` (fields: `compile_commands_path`, `workspace_root`, `filter`)
- `kota::event_loop &loop`
- Compilation database file on disk (via `load_compdb`)
- Cache files on disk (via `load_caches_async`)
- Source files (via `resolve_symbol_location_path`, `resolve_source_snippet`)
- `scan_cache.scan_results` for each processed entry
- `cache_records` (cache records map)

#### Writes To

- `ProjectModel &model` (symbols, files, namespaces, modules, etc.)
- `cache_records` (inserts, updates, or erases entries)
- Cache files on disk (via `save_caches_async`)
- Log output (via `logging::info`, `logging::warn`)

#### Usage Patterns

- Called as the main entry point for project extraction from a compilation database
- Invoked with application configuration and event loop for asynchronous execution
- Used in conjunction with cache management to optimize incremental extractions
- Part of the `clore::extract` module for building project models in a code analysis tool

## Internal Structure

模块 `extract` 在整体上承担从项目源码中异步提取结构化信息的职责，其设计围绕缓存驱动的流水线展开。内部依赖 `config` 和 `support`：前者提供全局配置（如根目录、过滤规则），后者封装文件路径规范化、缓存键构造等通用工具。模块入口是 `extract_project_async`，它接收项目句柄和事件循环，通过一系列在匿名命名空间中定义的异步函数协调工作。

分解上，模块按阶段组织：缓存加载（`load_caches_async` 加载 `clice` 与 `extract` 两类缓存）、条目准备与依赖图构建、并行 AST 提取（`extract_ast_batch_async` 按批次调度 `extract_ast_entry`）、模块信息构建（`build_module_info` 利用扫描缓存填充模型），最后持久化新缓存（`save_caches_async`）。内部数据结构如 `CacheEvaluation` 记录每个文件的编译签名、哈希与 AST/扫描有效性，`PreparedEntryState` 保存规范化路径与缓存键，`ParallelASTResult` 整合 AST 与依赖结果，而 `ExtractError` 统一错误传递。这种分层使得缓存命中检测、任务并行与结果聚合清晰分离，整体实现是一个面向事件循环的异步有限状态机。

## Related Pages

- [Module config](../config/index.md)
- [Module support](../support/index.md)

