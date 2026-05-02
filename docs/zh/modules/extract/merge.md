---
title: 'Module extract:merge'
description: 'extract:merge 模块负责整合来自多个来源的符号信息，确保数据的一致性与完整性。它公开了合并符号信息（merge_symbol_info）、重建查找映射（rebuild_lookup_maps）、重建模型索引（rebuild_model_indexes）等核心操作，并提供了去重（deduplicate）和唯一追加（append_unique、append_unique_range）的工具函数。在内部，该模块依赖于 extract:model、extract:filter 和 support 模块，利用并行分块处理（run_parallel_chunks）与线程局部数据结构（LocalIndexData）来高效完成符号合并与索引重建。'
layout: doc
template: doc
---

# Module `extract:merge`

## Summary

`extract:merge` 模块负责整合来自多个来源的符号信息，确保数据的一致性与完整性。它公开了合并符号信息（`merge_symbol_info`）、重建查找映射（`rebuild_lookup_maps`）、重建模型索引（`rebuild_model_indexes`）等核心操作，并提供了去重（`deduplicate`）和唯一追加（`append_unique`、`append_unique_range`）的工具函数。在内部，该模块依赖于 `extract:model`、`extract:filter` 和 `support` 模块，利用并行分块处理（`run_parallel_chunks`）与线程局部数据结构（`LocalIndexData`）来高效完成符号合并与索引重建。

该模块的公共实现范围涵盖了所有用于合并提取数据的对外接口，同时隐藏了命名空间层级构建、唯一性检查和内部索引维护等实现细节。调用者通过传入模型或索引的整数标识符即可触发合并与重建流程，而无需关心底层数据的组织方式。

## Imports

- [`config`](../config/index.md)
- [`extract:filter`](filter.md)
- [`extract:model`](model.md)
- `std`
- [`support`](../support/index.md)

## Variables

### `clore::extract::append_unique`

Declaration: `extract/merge.cppm:12`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

This function template participates in merge operations where duplicate entries must be avoided. It reads the input value and the target container, and modifies the container by adding the value if it does not already exist.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::append_unique_range`

Declaration: `extract/merge.cppm:19`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

No usage, mutation, or initialization details are available from the evidence. The variable does not appear in any surrounding logic in the provided context.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::deduplicate`

Declaration: `extract/merge.cppm:49`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

根据现有证据，该函数仅以声明形式出现，未提供其具体实现或调用上下文，因此无法确定其内部逻辑或对外部状态的影响。

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- 作为函数被调用

## Functions

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:55`

Definition: `extract/merge.cppm:215`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::merge_symbol_info` 的实现直接委托给匿名命名空间中的模板函数 `merge_symbol_info_impl`。后者首先通过 `ensure_namespace_hierarchy` 和 `find_enclosing_namespace` 确保目标命名空间层次结构存在，并将当前符号信息合并到模型中。合并过程中，会利用 `append_unique`、`append_unique_range` 和 `deduplicate` 处理重复项，同时根据 `prefer_incoming_snippet` 和 `prefer_incoming_definition` 等配置决定来自 `incoming` 符号的优先级。对于大规模合并，实现使用 `run_parallel_chunks` 配合每线程工作区（`per_thread`）进行并行处理，每个线程处理一个符号块，最后通过 `rebuild_model_indexes` 和 `rebuild_lookup_maps` 重建内部索引与查找映射。该函数依赖 `LocalIndexData` 结构管理线程局部命名空间信息，并通过 `known_namespace_names` 加速路径查找。

#### Side Effects

- mutates the `current` `SymbolInfo` object by merging data from `incoming`

#### Reads From

- `current` `SymbolInfo` parameter
- `incoming` `const SymbolInfo` parameter

#### Writes To

- `current` `SymbolInfo` parameter (the referenced object)

#### Usage Patterns

- updating symbol information during index merging
- combining symbol details from multiple compilation units
- callers that have a mutable symbol to augment with additional attributes

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:54`

Definition: `extract/merge.cppm:211`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数将 `incoming` 中的符号信息合并到 `current` 中，通过委托给内部实现 `clore::extract::(anonymous namespace)::merge_symbol_info_impl` 完成实际工作。后者使用 `run_parallel_chunks` 和 `hardware_threads` 将符号分区并行处理，每个工作线程维护一个局部索引结构 `LocalIndexData`。对于每个传入符号，它会调用 `find_enclosing_namespace` 确定所属命名空间，并通过 `ensure_namespace_hierarchy` 确保命名空间层级存在，最后借助 `deduplicate`、`append_unique` 以及 `append_unique_range` 来去重并合并各项属性（如定义、代码片段等）。

合并结束后，函数调用 `rebuild_model_indexes` 重建全文索引和跨文件引用，并调用 `rebuild_lookup_maps` 更新符号名称到 ID 的查找映射。整个流程依赖 `config` 中的 `prefer_incoming_definition` 和 `prefer_incoming_snippet` 等设置来控制冲突时的优先级，并通过 `num_threads` 控制并行度。

#### Side Effects

- Modifies the first argument `current` by merging data from `incoming`.
- Moves from `incoming`, which may leave it in a valid-but-unspecified state and potentially transfers ownership of resources.

#### Reads From

- `incoming` (the second parameter, via the move constructor or assignment)
- `current` (the first parameter, its current state is read for merging)

#### Writes To

- `current` (the first parameter, its contents are mutated)
- `incoming` (indirectly, by moving from it)

#### Usage Patterns

- Used to combine symbol information from two sources, such as when encountering duplicate symbols or applying incremental updates.
- Typically called when a new `SymbolInfo` instance (rvalue) is available and its data should be incorporated into an existing one.

### `clore::extract::rebuild_lookup_maps`

Declaration: `extract/merge.cppm:59`

Definition: `extract/merge.cppm:428`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::rebuild_lookup_maps` 以两个阶段重建 `ProjectModel` 中的查找表。第一阶段清空并重新填充 `model.symbol_ids_by_qualified_name`：遍历 `model.symbols`，将每个非空 `qualified_name` 对应的 `symbol_id` 追加到映射中；随后对每个名称下的 `symbol_ids` 列表依次按 `signature`、`declaration_location.file`、`declaration_location.line` 和 `symbol_id` 自身排序，再通过 `std::unique` 与 `erase` 去除相邻重复项，最后若列表长度大于 1 则通过 `logging::info` 记录重载候选数。第二阶段重建 `model.module_name_to_sources`：遍历 `model.modules`，将每个非空模块名称的源文件追加到映射中；随后对每个模块名称对应的源文件列表排序并去重，并统计其中 `is_interface` 为 `true` 的源文件数量，若超过一个则通过 `logging::warn` 报告重复的模块接口。整个函数不依赖外部库，仅操作 `model.symbols`、`model.modules` 以及 `logging` 基础设施，为后续基于限定名和模块名称的快速查询提供稳定的排序容器。

#### Side Effects

- Clears and repopulates `model.symbol_ids_by_qualified_name`
- Clears and repopulates `model.module_name_to_sources`
- Logs an info message for each qualified name with multiple overload candidates
- Logs a warning for each module name that has more than one interface source

#### Reads From

- `model.symbols`
- `model.modules`
- Fields `qualified_name`, `signature`, `declaration_location` of symbol entries
- Fields `name`, `is_interface` of module unit entries

#### Writes To

- `model.symbol_ids_by_qualified_name`
- `model.module_name_to_sources`
- Logging subsystem (info and warn)

#### Usage Patterns

- Called after symbols or modules are added, removed, or modified to keep lookup maps consistent
- Typically invoked during project model finalization or after merging extraction results

### `clore::extract::rebuild_model_indexes`

Declaration: `extract/merge.cppm:57`

Definition: `extract/merge.cppm:219`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::rebuild_model_indexes` 的实现首先重置 `model` 中所有文件的符号列表、命名空间集合以及每个符号的 `calls`、`references`、`children` 和派生相关集合。接着，它收集所有已知的命名空间限定名用于严格验证。核心流程是并行遍历符号集合：通过 `run_parallel_chunks` 将符号分块，每个线程局部构建文件‑符号映射、命名空间‑符号映射和父子关系对，并记录显式声明的命名空间。然后合并各线程的局部结果到全局映射中。基于合并后的映射，它更新 `model.files`、`model.namespaces` 和 `model.symbols` 的父子关系，并通过 `ensure_namespace_hierarchy` 确保命名空间层次存在。最后对文件、命名空间和符号的多组集合再次并行执行去重操作。整个过程中依赖 `deduplicate`、`find_enclosing_namespace` 和 `ensure_namespace_hierarchy` 等辅助函数。

#### Side Effects

- Modifies `model.files`, `model.symbols`, and `model.namespaces` in place
- Clears and repopulates index structures within the model
- Launches and joins multiple threads using `std::thread`
- Calls `std::thread::hardware_concurrency` to determine thread count
- Modifies symbol `children`, `derived`, `called_by`, and `referenced_by` lists (deduplication)

#### Reads From

- config`.filter`
- config (via `filter_root_path`)
- model`.symbols` (including `kind`, `qualified_name`, `declaration_location`, `parent`, existence)
- model`.files` (to clear)
- model`.namespaces` (to clear)
- `known_namespace_names` (derived from symbols with `SymbolKind::Namespace`)
- `filter_root` (computed from config)

#### Writes To

- model`.files` (per-file symbol list)
- model`.namespaces` (namespace info and symbol lists)
- model`.symbols` (children, derived, `called_by`, `referenced_by` lists)
- Local maps `file_symbols`, `namespace_symbols`, `parent_children`, `explicit_namespaces` (merged into model)

#### Usage Patterns

- Called after extraction to rebuild indexes for efficient querying
- Used to reinitialize index structures when model has been modified externally
- Invoked after changes to `model.files`, `model.symbols`, or `model.namespaces`

## Internal Structure

模块 `extract:merge` 负责将新提取的符号信息合并到已有的 `ProjectModel` 中，核心关注点在于增量整合与去重。其公共接口包括 `merge_symbol_info`、`rebuild_lookup_maps` 和 `rebuild_model_indexes`，分别处理单符号合并、查找索引重建以及完整模型索引的重新生成。内部则通过匿名命名空间组织细化逻辑：`merge_symbol_info_impl` 执行实际属性合并（如定义、片段优先规则），`ensure_namespace_hierarchy` 和 `find_enclosing_namespace` 维护命名空间的层次结构，而 `append_unique`、`append_unique_range` 与 `deduplicate` 作为通用模板工具确保集合的元素唯一性。

在实现结构上，模块依赖 `extract:model` 获取数据定义，依赖 `extract:filter` 进行路径过滤，同时利用 `support` 模块提供的文本与并发工具。合并操作基于并行分块设计：通过 `hardware_threads` 和 `num_threads` 确定线程数，将符号范围划分为 `per_thread` 块，在循环中使用局部索引 (`LocalIndexData`) 暂存中间状态，最后汇总到全局模型。这种内部分层将数据访问、索引重建与并行调度解耦，使核心合并逻辑不直接耦合于并发细节。

## Related Pages

- [Module config](../config/index.md)
- [Module extract:filter](filter.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

