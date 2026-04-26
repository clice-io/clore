---
title: 'Module extract:merge'
description: '该模块负责将多个来源的符号提取结果合并到统一的模型表示中。它公开发布了 merge_symbol_info、rebuild_model_indexes 和 rebuild_lookup_maps 等核心函数，用于将传入的符号信息整合到目标模型内，并重建内部索引与查找映射以保持查询逻辑的一致性。此外，模块还提供了 append_unique、append_unique_range 和 deduplicate 等辅助模板变量，用于处理容器中元素的去重追加，以及 LocalIndexData 结构体以支持并行环境下的局部索引构建。整体上，该模块是提取流水线的关键组成部分，在 config、extract:filter 与 extract:model 等模块的基础上，实现了从原始提取数据到可查询模型状态的转换。'
layout: doc
template: doc
---

# Module `extract:merge`

## Summary

该模块负责将多个来源的符号提取结果合并到统一的模型表示中。它公开发布了 `merge_symbol_info`、`rebuild_model_indexes` 和 `rebuild_lookup_maps` 等核心函数，用于将传入的符号信息整合到目标模型内，并重建内部索引与查找映射以保持查询逻辑的一致性。此外，模块还提供了 `append_unique`、`append_unique_range` 和 `deduplicate` 等辅助模板变量，用于处理容器中元素的去重追加，以及 `LocalIndexData` 结构体以支持并行环境下的局部索引构建。整体上，该模块是提取流水线的关键组成部分，在 `config`、`extract:filter` 与 `extract:model` 等模块的基础上，实现了从原始提取数据到可查询模型状态的转换。

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

The variable `clore::extract::append_unique` appears in the context of symbol merging and deduplication within the `clore::extract` module. It is referenced as a standalone entity; however, the provided evidence does not describe its initialization, mutation, or specific usage patterns. It likely serves as a helper or policy object for appending unique elements, but no direct consumption is observed in the snippets.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::append_unique_range`

Declaration: `extract/merge.cppm:19`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

The variable's role cannot be determined from the provided evidence. It may be related to appending unique elements to a range during merge operations, but no initialization, mutation, or usage is documented.

#### Mutation

No mutation is evident from the extracted code.

### `clore::extract::deduplicate`

Declaration: `extract/merge.cppm:49`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

{
  "`overview_markdown`": "A template function `deduplicate` is declared as `void deduplicate`.",
  "`details_markdown`": "No additional details are available from the evidence.",
  "`is_mutated`": false,
  "`mutation_sources`": "",
  "`usage_patterns`": ""
}

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:55`

Definition: `extract/merge.cppm:215`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::merge_symbol_info` 的实现主要委托给 `merge_symbol_info_impl`，后者是实际合并逻辑的所在。该函数将传入的符号信息 `incoming` 合并到当前模型 `model` 的符号 `current` 中，处理符号属性（如名称、定义、代码片段）的并集或覆盖，并依据 `prefer_incoming_snippet` 和 `prefer_incoming_definition` 等配置决定优先级。合并过程中会调用 `ensure_namespace_hierarchy` 和 `find_enclosing_namespace` 来维护命名空间层级结构，利用 `append_unique`、`append_unique_range` 和 `deduplicate` 去重和追加数据。在多线程场景下，`merge_symbol_info_impl` 通过 `run_parallel_chunks` 和 `per_thread` 结构并行处理，并在合并完成后触发 `rebuild_model_indexes` 和 `rebuild_lookup_maps` 重建模型的索引和查找表，确保一致性。

#### Side Effects

- Modifies `current` by merging data from `incoming`.

#### Reads From

- `current` (also written)
- `incoming` (read only)

#### Writes To

- `current` (mutated)

#### Usage Patterns

- Updating an existing symbol info with data from another symbol info.
- Consolidating symbol information from multiple sources.

### `clore::extract::merge_symbol_info`

Declaration: `extract/merge.cppm:54`

Definition: `extract/merge.cppm:211`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数是 `clore::extract` 命名空间中符号信息合并的核心入口。它接收一个当前符号信息引用 `current` 和一个临时符号信息对象 `incoming`，通过 `std::move` 将 `incoming` 的所有权转移后，立即委托给内部实现函数 `clore::extract::(anonymous namespace)::merge_symbol_info_impl`。后者是一个模板函数，依据配置选项（如 `prefer_incoming_snippet` 和 `prefer_incoming_definition`）决定冲突解决策略，并通过 `append_unique` 和 `append_unique_range` 维护符号项的唯一性。

在实现过程中，`merge_symbol_info_impl` 会调用 `ensure_namespace_hierarchy` 和 `find_enclosing_namespace` 以确保命名空间层级结构的一致性，同时利用 `run_parallel_chunks` 将工作分派至多个线程（基于 `hardware_threads` 和 `num_threads`）。合并完成后，会触发 `rebuild_model_indexes` 或 `rebuild_lookup_maps` 以更新模型索引，保证后续查询的正确性。整个流程依赖于 `LocalIndexData` 结构内的 `explicit_namespaces` 和 `known_namespace_names` 等中间数据。

#### Side Effects

- 修改了 `current` 参数所引用的 `SymbolInfo` 对象，其内容被来自 `incoming` 的数据合并更新。

#### Reads From

- `current` 参数（作为合并的目标，其原有数据被读取以参与合并）
- `incoming` 参数（其资源通过移动操作被窃取）

#### Writes To

- `current` 参数所引用的 `SymbolInfo` 对象（被修改以包含合并后的数据）

#### Usage Patterns

- 在符号提取过程中，用于将新解析的符号信息合并到已有的符号信息结构中
- 作为高效更新接口，利用右值引用避免拷贝开销

### `clore::extract::rebuild_lookup_maps`

Declaration: `extract/merge.cppm:59`

Definition: `extract/merge.cppm:428`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::rebuild_lookup_maps` 首先清空 `model` 中的 `symbol_ids_by_qualified_name` 与 `module_name_to_sources` 两个查找映射。随后遍历 `model.symbols`，对每个拥有非空 `qualified_name` 的符号，将 `symbol_id` 追加到对应限定名的列表中。接着对每个限定名下的 `symbol_ids` 执行稳定排序，比较依据依次为 `signature`、`declaration_location.file`、`declaration_location.line` 以及 `SymbolID` 自身，再调用 `std::unique` 移除连续重复，若列表长度大于 1 则通过 `logging::info` 报告重载候选数。  

之后遍历 `model.modules`，将每个非空 `name` 对应的源文件追加至 `module_name_to_sources` 列表，再对每个模块名称下的 `sources` 排序并去重。若检测到同一模块名称存在超过一个接口源文件（`is_interface` 为真），则调用 `logging::warn` 输出重复接口警告，包含前两个接口文件的路径。该函数不依赖外部并行工具或复杂数据结构，完全基于 `ProjectModel` 内部的容器（`std::unordered_map` 和 `std::vector`）与标准库算法实现。

#### Side Effects

- modifies `model.symbol_ids_by_qualified_name`
- modifies `model.module_name_to_sources`
- allocates memory for container elements
- logs info messages about overload candidates
- logs warning messages about duplicate module interfaces

#### Reads From

- `model.symbols`
- `model.modules`

#### Writes To

- `model.symbol_ids_by_qualified_name`
- `model.module_name_to_sources`

#### Usage Patterns

- called after symbol extraction to update lookup structures
- called to prepare the model for subsequent lookups by qualified name or module name

### `clore::extract::rebuild_model_indexes`

Declaration: `extract/merge.cppm:57`

Definition: `extract/merge.cppm:219`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先重置 `model` 中的索引数据：清空所有文件的 `symbols` 与 `includes`，清空 `model.namespaces`，并对每个符号的 `calls`、`references`、`derived`、`called_by`、`referenced_by` 执行 `deduplicate` 操作，同时清空 `children` 字段。随后，通过并行分块处理所有符号，为每个符号计算其在文件系统上的归属路径（结合 `config.filter` 和 `filter_root_path` 过滤），若匹配则将其 `SymbolID` 加入 `file_symbols` 映射；若非命名空间符号，则利用 `find_enclosing_namespace` 推断其所在的命名空间名并存入 `namespace_symbols`；若符号有父符号，则记录 `parent_children` 对。同时，显式声明的命名空间名被收集到 `explicit_namespaces` 中。

各线程的局部结果通过 `LocalIndexData` 结构聚合后，依次填充 `model.files` 的文件符号列表、通过 `ensure_namespace_hierarchy` 建立命名空间层级并将符号加入 `model.namespaces`，最后利用 `parent_children` 重建符号间的父子关系。整个过程中，多次调用 `run_parallel_chunks` 进行并发去重（对文件符号、命名空间符号及子项、符号的 `children` 和 `derived`），依赖 `deduplicate` 和基于 `std::thread` 的并行调度机制。

#### Side Effects

- Modifies the input `ProjectModel` by clearing and repopulating all index fields (file symbols, namespace symbols, symbol children, derived, etc.)
- Creates threads for parallel execution via `std::thread`
- Calls `std::thread::hardware_concurrency` to determine number of threads

#### Reads From

- `config` parameter: `config.filter` and result of `filter_root_path(config)`
- `model` parameter: `model.files`, `model.symbols`, `model.namespaces`
- Symbol fields: `kind`, `qualified_name`, `declaration_location.file`, `parent`, `calls`, `references`, `derived`, `called_by`, `referenced_by`

#### Writes To

- `model.files`: clears and repopulates `symbols` and sets `path` for each file entry
- `model.namespaces`: cleared then repopulated with namespace info
- `model.symbols`: clears and repopulates `children`, `derived`, `calls`, `references`, `called_by`, `referenced_by` (deduplicates)
- Local data structures merged into global maps

#### Usage Patterns

- Called after merging symbol data to rebuild all derived indexes
- Part of the extraction pipeline to ensure model consistency

## Internal Structure

extract:merge 模块实现了将提取的符号信息合并到统一模型的核心逻辑，分解为公共接口与内部辅助函数两层。公共接口包括 `merge_symbol_info`、`rebuild_model_indexes` 和 `rebuild_lookup_maps`，分别负责符号数据的融合、索引重建及查找映射刷新。内部实现借助匿名命名空间中的 `merge_symbol_info_impl`、`ensure_namespace_hierarchy` 和 `find_enclosing_namespace` 完成细节处理，并引入 `LocalIndexData` 结构管理每线程局部索引状态，支持基于硬件线程数的并行分块处理。模块直接导入 `extract:model`、`extract:filter`、`config` 和 `support` 等依赖，形成自底向上的数据流：原始符号经过过滤后，由合并逻辑整合到模型中，再重建索引以供后续查询。

## Related Pages

- [Module config](../config/index.md)
- [Module extract:filter](filter.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

