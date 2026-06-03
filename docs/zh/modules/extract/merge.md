---
title: 'Module extract:merge'
description: 'extract:merge 模块负责将多线程或分阶段提取的符号信息合并到统一的模型表示中，保证数据的一致性与唯一性。它提供的公开接口包括 merge_symbol_info（接受左值或右值源）、append_unique 与 append_unique_range（条件追加）、deduplicate（容器去重），以及 rebuild_lookup_maps 和 rebuild_model_indexes（在合并后重建索引与查找结构）。该模块还管理合并过程中的并行化参数（如硬件线程数）和局部索引数据结构（LocalIndexData），结合 ensure_namespace_hierarchy 等内部工具来维护命名空间层级。'
layout: doc
template: doc
---

# Module `extract:merge`

## Summary

`extract:merge` 模块负责将多线程或分阶段提取的符号信息合并到统一的模型表示中，保证数据的一致性与唯一性。它提供的公开接口包括 `merge_symbol_info`（接受左值或右值源）、`append_unique` 与 `append_unique_range`（条件追加）、`deduplicate`（容器去重），以及 `rebuild_lookup_maps` 和 `rebuild_model_indexes`（在合并后重建索引与查找结构）。该模块还管理合并过程中的并行化参数（如硬件线程数）和局部索引数据结构（`LocalIndexData`），结合 `ensure_namespace_hierarchy` 等内部工具来维护命名空间层级。

## Imports

- [`config`](../config/index.md)
- [`extract:filter`](filter.md)
- [`extract:model`](model.md)
- [`support`](../support/index.md)

## Functions

### `clore::extract::append_unique`

Declaration: `src/extract/merge.cppm:26`

Definition: `src/extract/merge.cppm:26`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数的实现采用线性搜索策略：通过 `std::find` 在目标容器 `values` 中查找传入的 `value`，若未找到（即返回值为 `values.end()`），则调用 `values.push_back(value)` 将其追加到容器末尾。此算法依赖标准库的线性查找算法以及 `std::vector` 的动态增补操作，时间复杂度为 O(n)，其中 n 为容器当前元素数量。内部控制流仅包含一次条件分支，无额外状态或并发逻辑，保证了语义的简洁性与可预测性。

#### Side Effects

- Modifies the `values` vector by potentially appending the `value`.

#### Reads From

- `values` vector content (via `std::find`)
- `value` parameter

#### Writes To

- `values` vector (via `push_back`)

#### Usage Patterns

- Used to maintain a unique set of elements in a vector
- Called in contexts where duplicate entries should be avoided when collecting items sequentially

### `clore::extract::append_unique_range`

Declaration: `src/extract/merge.cppm:33`

Definition: `src/extract/merge.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数使用两路分支来控制在追加操作期间需要去重的空间开销。当输入范围 `incoming` 的大小不大于当前集合 `values` 的大小，`append_unique_range` 会从 `incoming` 构建一个临时 `std::unordered_set` 作为 `remaining`，然后扫描 `values` 并将其中已存在的元素从集合中移除；随后再遍历 `incoming`，仅当元素仍存在于 `remaining` 中（即未被 `values` 包含）时才将其追加至 `values`，并立即从集合中删除以避免重复处理。反之，当 `incoming` 更大时，函数从 `values` 构建一个 `std::unordered_set` 作为 `seen`，预留足够容量后遍历 `incoming`，通过 `seen.insert` 的返回值确定元素是否是新出现，并仅对新元素调用 `push_back`。这种基于输入大小差异的策略避免了在 `values` 反复增长时频繁构造大容量的哈希集合，从而维持线性的平均时间复杂度。控制流完全由 `if` 分支驱动，两个分支均依赖 `std::unordered_set` 的擦除与插入操作，未使用其他容器或第三方依赖。

#### Side Effects

- Modifies the `values` vector by appending unique elements from `incoming`
- Allocates a temporary `std::unordered_set` for deduplication

#### Reads From

- `values` vector (existing elements)
- `incoming` vector (candidate elements to append)
- Temporary `std::unordered_set` of type `T` (insertions, erasures, and lookups)

#### Writes To

- `values` vector (appended new elements)
- Temporary `std::unordered_set` (insertions and erasures)

#### Usage Patterns

- Deduplicated merge of one collection into another
- Building a unique set of elements from multiple ranges
- Used in merge or deduplicate operations within the `clore::extract` namespace

### `clore::extract::deduplicate`

Declaration: `src/extract/merge.cppm:63`

Definition: `src/extract/merge.cppm:63`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数实现了一个经典的排序后去重算法。首先，它对输入的 `std::vector<T>` 类型的 `values` 调用 `std::sort`，使所有重复元素相邻排列。然后使用 `std::unique` 将不重复的元素移至容器前端，并返回指向新逻辑末尾的迭代器。最后，`values.erase` 删除从该迭代器到 `end()` 之间的元素，完成原地去重。整个流程依赖标准库的排序和算法设施，其性能由排序决定，适用于任何可比较且可赋值的 `T` 类型。

#### Side Effects

- Modifies the contents of the input vector by removing duplicate elements

#### Reads From

- The elements of the input vector `values` (to compare for sorting and uniqueness)

#### Writes To

- The input vector `values` (its element order and size are changed)

#### Usage Patterns

- Used to deduplicate a vector before further processing that requires unique elements
- Commonly called with a vector of symbols, `IDs`, or other comparable types

### `clore::extract::merge_symbol_info`

Declaration: `src/extract/merge.cppm:69`

Definition: `src/extract/merge.cppm:229`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数实现了将新索引中的符号信息合并到当前模型中的核心逻辑。它直接委托给 `merge_symbol_info_impl`，后者根据 `prefer_incoming_definition` 和 `prefer_incoming_snippet` 配置决定是否优先采纳传入符号的定义和代码片段。合并过程会利用 `append_unique` 和 `append_unique_range` 对符号列表进行去重，并通过 `ensure_namespace_hierarchy` 确保命名空间层次结构的正确性。`merge_symbol_info` 通常由 `rebuild_model_indexes` 调用，后者在并行阶段使用 `run_parallel_chunks` 划分任务，每个线程处理各自分区的符号，并将结果分片后再合并。最终通过 `rebuild_lookup_maps` 重建查找映射。依赖方面，该函数间接依赖于 `clore::extract` 命名空间下的 `deduplicate`、`append_unique`、`append_unique_range`、`rebuild_model_indexes`、`rebuild_lookup_maps` 等函数，以及 `LocalIndexData` 结构中存储的 `file_symbols`、`namespace_symbols`、`parent_children`、`explicit_namespaces` 等字段。

#### Side Effects

- modifies `current` parameter

#### Reads From

- `incoming` parameter

#### Writes To

- `current` parameter

#### Usage Patterns

- merging symbol information from multiple sources
- updating existing symbol with new data
- deduplication

### `clore::extract::merge_symbol_info`

Declaration: `src/extract/merge.cppm:68`

Definition: `src/extract/merge.cppm:225`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数`clore::extract::merge_symbol_info`通过委托给匿名命名空间中的`clore::extract::(anonymous namespace)::merge_symbol_info_impl`实现其核心逻辑。该实现首先处理传入的`incoming`符号信息，通过`clore::extract::(anonymous namespace)::ensure_namespace_hierarchy`为所有涉及的命名空间名称（如`ns_name`、`namespace_name`、`parent_name`）建立层级关系，确保`known_namespace_names`集合和`explicit_namespaces`列表正确。接着，针对`incoming`中的每个符号，使用`clore::extract::deduplicate`消除与`model`中已有符号的重复项，并借助`clore::extract::append_unique`和`clore::extract::append_unique_range`将唯一符号附加到对应的容器（如`file_symbols`、`namespace_symbols`、`parent_children`）。整个过程利用`hardware_threads`和`num_threads`控制并行度，通过`run_parallel_chunks`将符号拆分为多个`per_thread`工作块，每个线程独立处理`begin`到`end`范围内的符号，并使用`seen`机制避免跨线程冲突。最后，调用`clore::extract::rebuild_model_indexes`和`clore::extract::rebuild_lookup_maps`重建模型的内部索引与查找映射，完成合并。

#### Side Effects

- modifies the state of the `current` `SymbolInfo` object by merging data from `incoming`
- moves resources from `incoming` into `current`, potentially invalidating `incoming`

#### Reads From

- the `incoming` `SymbolInfo` rvalue reference (data accessed during merge)
- possibly the `current` `SymbolInfo` (data read to decide merge strategy)

#### Writes To

- the `current` `SymbolInfo` object (non-const reference passed as first argument)

#### Usage Patterns

- called to merge symbol info during extraction or deduplication
- used when consolidating symbol information from multiple sources into a canonical representation

### `clore::extract::rebuild_lookup_maps`

Declaration: `src/extract/merge.cppm:73`

Definition: `src/extract/merge.cppm:442`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::rebuild_lookup_maps` 接收一个 `ProjectModel` 引用，用于重建模型中的两个查找映射表：`symbol_ids_by_qualified_name` 和 `module_name_to_sources`。首先，该函数清空这两个映射，然后遍历 `model.symbols` 中的每个符号，对于具有非空 `qualified_name` 的符号，将其 `SymbolID` 追加到对应名称的列表中。每个名称对应的 ID 列表随后按签名、声明位置等字段进行排序并去重，若存在多个候选则通过 `logging::info` 记录。接着，函数遍历 `model.modules`，对于每个有名称的模块单元，将源文件路径加入 `module_name_to_sources`，并对每个模块名称下的源文件列表排序去重；若检测到同一个模块存在多个接口单元，则通过 `logging::warn` 报告重复。该函数不依赖外部库，仅操作 `ProjectModel` 内部数据结构并使用日志工具。

#### Side Effects

- clears and repopulates `model.symbol_ids_by_qualified_name`
- clears and repopulates `model.module_name_to_sources`
- sorts and deduplicates vectors within those maps
- logs informational messages via `logging::info`
- logs warnings via `logging::warn`

#### Reads From

- `model.symbols`
- `model.modules`
- `SymbolInfo::qualified_name`
- `SymbolInfo::signature`
- `SymbolInfo::declaration_location`
- `ModuleUnit::name`
- `ModuleUnit::is_interface`

#### Writes To

- `model.symbol_ids_by_qualified_name`
- `model.module_name_to_sources`

#### Usage Patterns

- called after the model is fully populated to ensure lookup maps are consistent
- prepares the model for efficient lookups by qualified name or module name
- typically invoked once before performing multiple queries on the model

### `clore::extract::rebuild_model_indexes`

Declaration: `src/extract/merge.cppm:71`

Definition: `src/extract/merge.cppm:233`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::rebuild_model_indexes` 以 `const config::TaskConfig &` 和 `ProjectModel &` 为参数，分阶段重构模型的全部索引。首先清空 `model` 中所有文件与符号的临时容器，并对 `includes`、`calls`、`references`、`derived`、`called_by`、`referenced_by` 做去重。接着收集全部 `SymbolID` 和已知命名空间限定名，然后利用 `run_parallel_chunks`（基于 `std::thread::hardware_concurrency` 计算线程数，按固定块大小并行）将符号分配到多个线程，每个线程在 `LocalIndexData` 中积累四类数据：匹配过滤器的符号按文件路径分组、非命名空间符号计算 `find_enclosing_namespace` 后按命名空间分组、从 `sym.parent` 产生的父子关系对、以及显式命名空间名。所有线程的数据通过 `emplace_back` 合并到全局 `file_symbols`、`namespace_symbols`、`parent_children` 和 `explicit_namespaces` 映射中。

合并后，用 `file_symbols` 填充 `model.files` 并设置 `owner_file_info.symbols`；通过 `ensure_namespace_hierarchy` 在 `model.namespaces` 中建立并规范化命名空间层次，再将 `namespace_symbols` 中的符号插入对应 `NamespaceInfo::symbols`；把 `parent_children` 中每对父-子 ID 追加到父 `SymbolInfo::children` 列表末尾。最后，对 `model.files` 中每个文件的 `symbols`、`model.namespaces` 中每个命名空间的 `symbols` 和 `children`、以及 `model.symbols` 中每个符号的 `children` 和 `derived` 分别并行调用 `deduplicate` 以确保唯一性。整个流程依赖 `clore::extract::deduplicate`、`find_enclosing_namespace`、`ensure_namespace_hierarchy`、`matches_filter` 以及 `std::filesystem` 路径操作来协调数据重置、并行收集与索引重建。

#### Side Effects

- 修改了 `ProjectModel` 中 `model.files` 的内容，包括路径和符号列表
- 修改了 `model.namespaces`，插入或更新命名空间信息
- 修改了 `model.symbols` 中符号的 `children` 和 `derived` 成员
- 对 `model.symbols` 中多个成员（`calls`、`references`、`called_by`、`referenced_by`）进行了去重
- 创建并启动了多个 `std::thread` 并行执行任务

#### Reads From

- `config` 参数，用于获取过滤路径和过滤规则
- `model.files`，读取现有文件信息
- `model.symbols`，读取所有符号的 `kind`、`qualified_name`、`declaration_location`、`parent` 等属性
- `model.namespaces`，用于构建已知命名空间集合 `known_namespace_names`
- `matches_filter` 函数，用于判断符号所属文件是否匹配过滤条件
- `filter_root_path` 函数，用于计算根路径

#### Writes To

- `model.files` 中的每个 `FileInfo` 对象的 `symbols` 和 `path`
- `model.namespaces` 中的 `NamespaceInfo` 的 `name`、`symbols`、`children`
- `model.symbols` 中符号的 `children` 和 `derived` 列表
- 临时局部变量（如 `local_data`、`file_symbols`、`namespace_symbols`、`parent_children`、`explicit_namespaces`）

#### Usage Patterns

- 在符号提取或合并流程之后调用，以重建索引
- 作为 `clore::extract` 模块中核心模型维护操作的一部分
- 通常每个项目模型只调用一次，用于确保后续查询的准确性

## Internal Structure

模块 `extract:merge` 负责整合来自多个源（如并行提取的结果）的符号信息，执行去重、索引重建和查找映射更新，是提取流水线的最终组装阶段。它依赖 `extract:model` 提供核心数据结构，依赖 `config` 获取运行时参数（如线程数），依赖 `extract:filter` 进行路径管理，并依赖 `support` 获取日志与实用工具。模块内部划分为三个层次：公开的顶层函数（如 `merge_symbol_info`、`rebuild_model_indexes`）、位于匿名命名空间中的实现辅助函数（如 `merge_symbol_info_impl`、`ensure_namespace_hierarchy`、`find_enclosing_namespace`），以及一个与外界隔离的内部结构 `LocalIndexData`，用于在并行重建索引时暂存线程局部数据。

实现结构围绕将操作分解为可并行化的块进行设计：`rebuild_model_indexes` 为每个线程预先计算工作范围，每个线程在 `LocalIndexData` 中构造本地索引（包括 `file_symbols`、`parent_children`、`namespace_symbols` 和 `explicit_namespaces`），最后再合并回全局索引。符号合并操作（`merge_symbol_info` 及其右值引用重载）利用 `append_unique`、`append_unique_range`、`deduplicate` 等模板函数在基于整数句柄的容器上执行增量式更新，确保元素唯一性。`rebuild_lookup_maps` 在合并完成后统一重建查找结构，保证后续查询的一致性。这种分解使得模块能在多线程环境下高效处理大量符号，同时借助匿名命名空间隔离内部细节，只暴露少量必需的接口。

## Related Pages

- [Module config](../config/index.md)
- [Module extract:filter](filter.md)
- [Module extract:model](model.md)
- [Module support](../support/index.md)

