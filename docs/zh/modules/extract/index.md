---
title: 'Module extract'
description: 'extract 模块负责符号提取的核心流程，主要职责包括加载与评估缓存状态、并行解析源文件 AST、构建模块依赖关系，以及持久化提取结果。模块内部通过匿名命名空间封装了多个辅助结构与函数（如 CacheEvaluation、ParallelASTResult、LoadedCaches 以及各类 load_*_async / save_*_async 函数），实现异步 I/O 与并发任务调度。模块向外界暴露的唯一公有接口是 extract_project_async，调用者传入项目标识符和事件循环即可启动异步提取操作，操作结果以整型错误码返回。整个实现依赖于 config 模块的配置与 support 模块的基础工具（如路径规范化、缓存键构建及文件读写）。'
layout: doc
template: doc
---

# Module `extract`

## Summary

`extract` 模块负责符号提取的核心流程，主要职责包括加载与评估缓存状态、并行解析源文件 AST、构建模块依赖关系，以及持久化提取结果。模块内部通过匿名命名空间封装了多个辅助结构与函数（如 `CacheEvaluation`、`ParallelASTResult`、`LoadedCaches` 以及各类 `load_*_async` / `save_*_async` 函数），实现异步 I/O 与并发任务调度。模块向外界暴露的唯一公有接口是 `extract_project_async`，调用者传入项目标识符和事件循环即可启动异步提取操作，操作结果以整型错误码返回。整个实现依赖于 `config` 模块的配置与 `support` 模块的基础工具（如路径规范化、缓存键构建及文件读写）。

## Imports

- [`config`](../config/index.md)
- `std`
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

Declaration: `extract/extract.cppm:21`

Definition: `extract/extract.cppm:21`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该结构体仅包含一个 `std::string` 成员 `message`，用于存储错误描述信息。由于未定义任何用户提供的构造函数、赋值操作符或析构函数，`clore::extract::ExtractError` 是一个聚合类型，其构造、复制和销毁完全依赖 `std::string` 的相应操作。该设计使得错误消息的存储和传递具有自然的值语义，且不引入额外的资源管理复杂性。任何对 `message` 的修改都会直接改变错误对象的有效状态，因此保持 `message` 内容的完整性是该结构体的主要不变性。

## Functions

### `clore::extract::extract_project_async`

Declaration: `extract/extract.cppm:25`

Definition: `extract/extract.cppm:539`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::extract_project_async` 的整体算法按阶段组织，异步控制流通过 `kota::event_loop` 驱动。首先加载编译数据库并过滤条目，标准化文件路径；接着从 `workspace_root` 启动 `load_caches_async` 获得 `extract_cache_records` 和 `clice_cache`。对于每个条目，利用 `compile_signature`、`source_hash` 及依赖变化检测（`cache::dependencies_changed`）计算出 `CacheEvaluation`，分别记录 `scan_valid` 和 `ast_valid` 状态，并填充 `seeded_scan_cache`。然后通过 `build_dependency_graph_async` 构建 `DependencyGraph`，随后执行拓扑排序 `topological_order` 得到文件处理顺序。并行 AST 提取由 `extract_ast_batch_async` 完成，它接收 `filtered_db.entries`、`prepared_entries`、`cache_evaluations` 和 `dep_graph`，返回包含每个条目 `ParallelASTResult` 或错误的结果向量。

在按序处理每个条目时，若 `cache_state_it->second.ast_valid` 为真且缓存记录存在，则直接从 `cache_record_it->second.ast` 获取 AST 视图，否则从并行结果中取 `ast_data` 并获取对应的 `ast_deps_snapshot`。对于每个符号，通过 `resolve_symbol_location_path` 解析声明和定义的文件路径（使用 `resolved_path_cache` 对同一路径的解析结果去重），应用 `filter_root` 过滤，再在 `model.symbols` 中插入或合并符号。关系处理包括继承、调用和引用，分别推入派生、调用和被调用列表。根据 `source_hash` 的有无决定是更新/插入 `cache_records` 还是移除该记录。所有条目处理完毕后，依次调用 `rebuild_model_indexes`、`build_module_info`、`rebuild_lookup_maps`，并通过 `kota::queue` 在后台线程异步解析源代码片段。最后通过 `save_caches_async` 将缓存写回磁盘。控制流中任何阶段遇到错误都会先尝试 `fail_after_persist` 以尽最大努力持久化缓存，再传播 `ExtractError`。

#### Side Effects

- Loads compilation database from disk
- Loads caches from disk (cache records and clice cache)
- Persists caches to disk after extraction or on failure
- Logs progress and statistics via `logging::info` and `logging::cache_hit_rate`
- Mutates `ProjectModel` object by adding symbols, files, namespaces, and relations
- Rebuilds model indexes and lookup maps
- Resolves source snippets for symbols
- Updates in-memory cache records map and clice cache

#### Reads From

- `config` parameter: `compile_commands_path`, `filter`, `workspace_root`
- Compilation database entries (file paths, directories, compile commands)
- Cache records map (previously persisted AST and scan results)
- Clice cache (PCH and PCM entries)
- Event loop `loop` for async operations
- Scan cache (seeded from disk)
- Filter root from `config.filter`

#### Writes To

- Cache records map (updated or inserted AST and scan data)
- Clice cache (possibly unchanged, but passed to persistence)
- `ProjectModel` object: `symbols`, `files`, `namespaces`, `file_order`, `modules`
- Resolved path cache (internal map for repeated path resolutions)
- Log output via `logging` functions
- Disk via `save_caches_async` (persisting caches)

#### Usage Patterns

- Called as the main entry point for extracting a project's symbols asynchronously
- Uses coroutine pattern with `kota::task` and `co_await`
- Cooperates with caching subsystem to avoid redundant AST extraction
- Relies on parallel AST extraction batch task for performance
- Handles errors via `kota::fail` and custom error type `ExtractError`

## Internal Structure

extract 模块是符号提取的核心实现，对外仅暴露 `extract_project_async` 这一异步入口点。它向上承接调用者传入的项目标识符和事件循环，向下依赖 `config`（应用配置）、`std`（标准库）以及 `support`（基础工具与日志）三个模块。模块内部完全使用匿名命名空间封装细节，所有辅助函数、状态结构体和类型别名均不对外可见，实现了严格的接口隔离。

在内部，extract 被组织为三层异步流水线：底层是缓存 I/O 层，以模板函数 `run_cache_io_async` 和 `run_worker_task_async` 为基础，统一封装了对 `clice_cache` 和 `extract_cache_records` 的加载与持久化（体现在 `load_caches_async`、`save_caches_async` 等函数中）。中间层是 AST 提取层，由 `extract_ast_batch_async` 协调批量任务，每个条目通过 `extract_ast_entry` 生成 `ParallelASTResult`（包含 AST 数据、依赖关系及符号信息）。上层为模块构建层，`build_module_info` 利用扫描缓存内的 `GroupedModuleInfo`（含接口标志、模块名、导入列表）填充项目模型中的模块信息。整个流程以 `CacheEvaluation`（编译签名、源哈希、AST/扫描有效性）和 `PreparedEntryState`（归一化文件路径、缓存键）作为状态跟踪单元，结合 `LoadedCaches` 与 `cache_records` 实现增量复用，最终通过 `extract_project_async` 返回操作结果。

## Related Pages

- [Module config](../config/index.md)
- [Module support](../support/index.md)

