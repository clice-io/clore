---
title: 'Module generate:evidence'
description: 'generate:evidence 模块负责为文档生成管线构建和组织结构化的证据数据，这些数据随后用于格式化文本或直接构造面向语言模型的提示。它公开了一系列 build_evidence_for_* 函数，针对命名空间摘要、模块架构、索引概览以及各类符号（函数、类型、变量）的分析与实现摘要等不同页面类型生成证据包。此外，模块还提供了 format_evidence_text 和 format_evidence_text_bounded 用于将证据包渲染为文本，以及 build_prompt 用于将证据组装为完整的提示模板。核心数据结构 SymbolFact 和 EvidencePack 也是该模块公共实现的一部分，为下游消费提供统一的证据表示。'
layout: doc
template: doc
---

# Module `generate:evidence`

## Summary

`generate:evidence` 模块负责为文档生成管线构建和组织结构化的证据数据，这些数据随后用于格式化文本或直接构造面向语言模型的提示。它公开了一系列 `build_evidence_for_*` 函数，针对命名空间摘要、模块架构、索引概览以及各类符号（函数、类型、变量）的分析与实现摘要等不同页面类型生成证据包。此外，模块还提供了 `format_evidence_text` 和 `format_evidence_text_bounded` 用于将证据包渲染为文本，以及 `build_prompt` 用于将证据组装为完整的提示模板。核心数据结构 `SymbolFact` 和 `EvidencePack` 也是该模块公共实现的一部分，为下游消费提供统一的证据表示。

## Imports

- [`extract`](../extract/index.md)
- [`generate:model`](model.md)
- `std`

## Imported By

- [`generate:analysis`](analysis.md)
- [`generate:evidence_builder`](index.md)
- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::EvidencePack`

Declaration: `generate/evidence.cppm:22`

Definition: `generate/evidence.cppm:22`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::EvidencePack` 将生成提示所需的全部证据汇总为单一结构，通过划分不同的事实向量（`local_context`、`dependency_context`、`reverse_usage_context` 和 `target_facts`）分别承载主体自身的上下文、其依赖项的上下文、被使用方的反向上下文以及目标事实。`subject_name` 与 `subject_kind` 标识证据所描述的对象，而 `source_snippets` 保留原始代码片段，`related_page_summaries` 收集关联文档页的摘要。`page_id` 和 `prompt_kind` 记录来源页面与提示种类，用于后续编排。这些字段均以标准容器存储，空向量表示对应证据缺失，但结构本身不强制非空约束——所有字段在构造后可被自由填充，调用方需保证在构建提示前已按需组装完整。

#### Invariants

- All fields are intended to be fully populated before use
- `page_id` and `prompt_kind` uniquely identify the generation task
- `subject_name` and `subject_kind` describe the target symbol
- Vectors may be empty if no relevant evidence exists

#### Key Members

- `subject_name` and `subject_kind`
- `target_facts`
- `local_context`, `dependency_context`, `reverse_usage_context`
- `page_id` and `prompt_kind`

#### Usage Patterns

- Constructed by evidence gathering subsystems
- Passed to prompt builders or directly to LLM-based documenters
- Consumed by documentation generation pipelines for a single symbol

### `clore::generate::PromptError`

Declaration: `generate/evidence.cppm:90`

Definition: `generate/evidence.cppm:90`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该结构体仅包含一个类型为 `std::string` 的数据成员 `message`，用于携带错误描述文本。其默认构造函数、拷贝构造函数、移动构造函数、拷贝赋值运算符、移动赋值运算符以及析构函数均由编译器隐式生成，这些操作均会按成员对 `std::string` 执行相应的浅层或深层复制/移动及析构。由于不涉及自定义资源管理或其他非平凡成员，该类型在对象生命周期内的行为完全由 `std::string` 的语义保证，无额外不变式需要维持。所有隐式特殊成员函数均为 inline 并在定义处由编译器合成，因此该结构体可以高效地用于值传递或异常抛出等场景。

#### Invariants

- `message` 包含可读的错误描述

#### Key Members

- `message`

#### Usage Patterns

- 作为错误结果返回
- 用于传递生成阶段的错误信息

### `clore::generate::SymbolFact`

Declaration: `generate/evidence.cppm:9`

Definition: `generate/evidence.cppm:9`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`SymbolFact` 是一个纯数据聚合类型，用作从符号提取到代码生成管线的中间传输载体。其所有成员均为公有且无自定义构造或复制控制，保持平凡聚合性质。核心字段包括唯一标识符 `extract::SymbolID id`、`qualified_name`、`signature`、`kind_label`、`access`、模板标志 `is_template`（默认为 `false`）与 `template_params`、声明位置 `declaration_file` 和 `declaration_line`（默认 `0`），以及可选文档注释 `doc_comment`。结构内部无额外不变式：任何字段均可独立设置，且默认值仅用于未显式初始化时的安全状态。整个结构体的设计在于简单、零开销地传递符号事实，不承担任何业务逻辑。

#### Invariants

- `id` is globally unique
- `qualified_name` is the fully qualified name
- `is_template` is `true` if the symbol is a template specialization

#### Key Members

- `id`
- `qualified_name`
- `kind_label`
- `doc_comment`
- `declaration_file`
- `declaration_line`
- `signature`
- `access`
- `is_template`
- `template_params`

#### Usage Patterns

- Populated by extraction phase
- Consumed by generation phase to produce documentation output

## Functions

### `clore::generate::__detail::collect_analysis_summaries`

Declaration: `generate/evidence.cppm:121`

Definition: `generate/evidence.cppm:241`

函数 `clore::generate::__detail::collect_analysis_summaries` 作为对外的转发入口，其实现完全委托给 `clore::generate::__detail::collect_analysis_summaries_impl`。传入的 `analyses`（`SymbolAnalysisStore` 类型）、`model`（`extract::ProjectModel` 类型）和 `ids`（`std::vector<extract::SymbolID>` 类型）原样传递，同时提供一个恒等映射 lambda 作为 `SymbolIDOf` 参数，使得每个符号 ID 直接映射到自身。内部控制流由此进入 `collect_analysis_summaries_impl`，该函数会遍历 `ids`，为每个符号 ID 调用 `collect_facts` 收集事实，并依据符号种类（如函数、类型、变量、命名空间等）调用对应的 `build_evidence_*` 系列函数（例如 `build_evidence_for_function_analysis`、`build_evidence_for_type_declaration_summary` 等）构建 `EvidencePack`，再通过 `format_evidence_text_bounded` 或 `format_evidence_text` 格式化证据为文本摘要。此外，该实现还依赖 `collect_namespace_facts_cached` 进行跨符号的缓存查询，并利用 `render_summary_item`、`render_detailed_fact` 等辅助函数控制输出格式和长度约束，最终生成 `std::vector<std::string>` 返回。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `analyses`
- 参数 `model`
- 参数 `ids`

#### Usage Patterns

- 作为收集分析摘要的顶层入口

### `clore::generate::__detail::collect_analysis_summaries_impl`

Declaration: `generate/evidence.cppm:116`

Definition: `generate/evidence.cppm:213`

The function iterates over each element in the `items` range, applying the callable `symbol_id_of` to extract an `extract::SymbolID`. A `std::unordered_set` named `seen` deduplicates these `IDs`; if an ID is already present, the element is skipped. For a new ID, the function performs a `extract::lookup_symbol` on the `model` to obtain a pointer to the corresponding symbol. If the pointer is null, the iteration continues. Otherwise, it retrieves an analysis overview string by calling `analysis_overview_markdown` on the `analyses` and the symbol; if that result is null or empty, the element is also skipped.

Only after passing all checks is the non‑empty summary string appended to the result vector. The result is pre‑reserved using `items.size()` to minimize reallocations. The function therefore implements a filtered, deduplicated collection of existing analysis summaries for the symbols identified by the given range and projection.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` parameter
- `model` parameter
- `items` parameter
- `symbol_id_of` callable
- `extract::lookup_symbol` return value
- `analysis_overview_markdown` return value

#### Usage Patterns

- Called to collect analysis summaries for a range of items
- Used by `collect_analysis_summaries`
- Filters duplicates and missing summaries

### `clore::generate::__detail::collect_facts`

Declaration: `generate/evidence.cppm:103`

Definition: `generate/evidence.cppm:163`

该函数遍历 `ids` 中的每个 symbol ID，对每个 ID 检查是否已在 `seen` 集合中出现；若重复则跳过，否则尝试通过 `extract::lookup_symbol` 在当前 `model` 中查找对应的符号对象。若找到，则调用 `to_symbol_fact` 构建 `SymbolFact` 并加入结果列表。最后返回去重后的 `facts` 向量。

内部控制流完全依赖于 `std::unordered_set` 实现的线性去重以及 `extract::lookup_symbol` 的查找结果。函数本身不执行任何分析或摘要生成，仅负责将原始的符号 ID 集合转换为对应的 `SymbolFact` 对象集合，并为后续的摘要收集阶段提供结构化的输入数据。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model (parameter)
- ids (parameter)
- `project_root` (parameter)
- `extract::lookup_symbol` (reads model's internal data)

#### Usage Patterns

- Called to collect facts for a set of symbol `IDs` from a project model
- Used by higher-level functions that need a deduplicated list of `SymbolFact` objects

### `clore::generate::__detail::collect_merged_facts`

Declaration: `generate/evidence.cppm:108`

Definition: `generate/evidence.cppm:179`

该函数将多个传入的符号 ID 集合（`groups`）合并为一个无重复的 `SymbolFact` 向量。它通过一个 `seen` 无序集合来跟踪已处理过的 `extract::SymbolID`，确保每个符号只被转换一次。内部使用折叠表达式 `(append_group(groups), ...)` 依次处理每组 ID，对每个组内的每个 ID 检查是否已在 `seen` 中；若未出现过则插入并调用 `extract::lookup_symbol` 在 `model` 中查找符号，再通过 `to_symbol_fact` 转换为 `SymbolFact` 追加到结果列表中。

控制流完全由 `append_group` lambda 驱动，依赖的外部函数包括 `extract::lookup_symbol`（从项目模型中按 ID 获取符号）和 `to_symbol_fact`（将符号表示转换为带项目根路径的事实对象）。整个过程是纯线性扫描，利用集合去重保证最终返回的 `facts` 中每个符号仅出现一次。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `extract::ProjectModel`& model
- const `std::string`& `project_root`
- const Groups&... groups
- elements of each group
- data accessed by `extract::lookup_symbol` and `to_symbol_fact`

#### Usage Patterns

- called with multiple group containers to aggregate facts
- used in generating documentation by merging facts from different sources
- employs fold expression to process variadic group arguments

### `clore::generate::__detail::collect_namespace_facts_cached`

Declaration: `generate/evidence.cppm:133`

Definition: `generate/evidence.cppm:275`

该函数从 `model` 中收集指定 `namespace_name` 下属于给定 `target_kind`（函数、类型或变量）的符号事实，并利用静态缓存 `namespace_facts_cache()` 避免重复遍历。缓存键由 `namespace_name` 和 `project_root` 拼接构成。若缓存未命中，则遍历命名空间内的所有符号，通过 `to_symbol_fact` 转换为 `SymbolFact`，并按符号种类分别存入 `NamespaceFactsCacheEntry` 的 `all_functions`、`all_types` 或 `all_variables` 向量中；若命名空间不存在，则对应向量保持为空。后续调用根据 `target_kind` 选择对应子向量，并排除 `exclude_id` 对应的符号后返回结果。若命名空间为空或 `target_kind` 不在指定范围内，则返回空向量。

#### Side Effects

- Mutates namespace facts cache (`namespace_facts_cache`)
- Allocates memory for cache entries and result vector

#### Reads From

- model`.namespaces`
- `namespace_name`
- `project_root`
- `target_kind`
- `exclude_id`
- global `namespace_facts_cache`

#### Writes To

- `namespace_facts_cache`

#### Usage Patterns

- Called when collecting facts for multiple symbols in the same namespace to avoid redundant traversals
- Used by higher-level fact collection functions

### `clore::generate::__detail::collect_summaries`

Declaration: `generate/evidence.cppm:112`

Definition: `generate/evidence.cppm:200`

函数 `clore::generate::__detail::collect_summaries` 遍历 `keys` 容器中的每一个键，对每个键在 `cache`（一个 `PageSummaryCache` 类型的映射）中执行查找。若查找成功且对应的摘要值非空，则将该摘要追加到结果向量中。最终返回包含所有非空摘要的 `std::vector<std::string>`。该函数的控制流是线性迭代与条件筛选的结合，其依赖仅涉及 `PageSummaryCache` 的 `find` 接口以及 `keys` 的迭代能力；没有嵌套循环或递归分支，逻辑简洁且专注于从缓存中按指定键提取有效数据。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache` (a `PageSummaryCache` object)
- `keys` (a `std::vector<std::string>`)

#### Writes To

- the returned `std::vector<std::string>`

#### Usage Patterns

- Used to collect summary strings from a cache by a list of keys
- Called to filter out empty summary entries

### `clore::generate::__detail::to_symbol_fact`

Declaration: `generate/evidence.cppm:101`

Definition: `generate/evidence.cppm:147`

函数 `clore::generate::__detail::to_symbol_fact` 通过聚合初始化直接构造并返回一个 `clore::generate::SymbolFact` 对象。它将输入的 `extract::SymbolInfo` 的 `id`、`qualified_name`、`signature`、`access`、`is_template`、`template_params` 和 `doc_comment` 字段原样复制到 `SymbolFact` 的对应成员；通过调用 `extract::symbol_kind_name` 将 `kind` 枚举转换为字符串标签填入 `kind_label`；并将 `declaration_location.file` 经 `clore::generate::make_source_relative` 处理为相对项目根的路径后赋给 `declaration_file`，同时复制 `declaration_location.line` 至 `declaration_line`。

整个函数不包含分支或循环，完全依赖外部函数 `extract::symbol_kind_name` 进行类型名称转换以及 `clore::generate::make_source_relative` 进行路径规范化，其依赖的输入结构体 `extract::SymbolInfo` 的字段布局决定了输出的映射关系。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym` (all fields)
- `project_root`

#### Writes To

- returned `SymbolFact` object (local, not externally observable)

#### Usage Patterns

- Called during symbol fact collection pipelines
- Converts extraction results to internal fact representation

### `clore::generate::build_evidence_for_function_analysis`

Declaration: `generate/evidence.cppm:40`

Definition: `generate/evidence_builder.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数的核心逻辑围绕遍历 `project_root` 下的索引树展开，通过 `root` 和 `file_it` 等迭代器定位目标模块与命名空间。内部控制流首先利用 `maybe_resolve_snippet` 检查并解析可能存在的代码片段，随后收集来自 `page_summaries`、`imported_summaries` 以及直接分析的 `analyses` 信息，这些信息通过 `pack` 逐步聚合。

在数据组装阶段，`sym`、`target` 以及 `namespace_fact` 等变量用于标识待分析函数的目标上下文，而 `model` 则作为最终证据结构被填充。函数依赖 `mod` 和 `ns` 进行模块与命名空间的解析，并通过 `root` 和 `project_root` 维持路径关系，最终返回一个 `int` 类型的摘要标识（如成功码或证据索引），完成从原始索引到结构化分析证据的构建。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- reads the first `const int &` parameter (likely a function analysis context)
- reads the second `const int &` parameter (likely an evidence pack or related data)
- reads the `int` parameter (likely a limit or index)

#### Usage Patterns

- Called during evidence construction for function analysis
- Possibly used in a builder pattern for generating evidence packs

### `clore::generate::build_evidence_for_function_declaration_summary`

Declaration: `generate/evidence.cppm:67`

Definition: `generate/evidence_builder.cppm:238`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数从 `project_root` 出发遍历 `pack` 中的每个模块，对每个模块使用 `model` 获取命名空间事实与函数声明。核心循环对每个 `target` 声明调用匿名辅助函数 `maybe_resolve_snippet` 来判断相关的代码片段能否内联解决，然后将结果收集到 `page_summaries` 中。同时，函数利用 `analyses` 中的分析对象驱动证据生成，并通过 `imported_summaries` 处理跨模块的摘要依赖，最终将各模块的摘要归并到 `root` 结构中。

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::build_evidence_for_function_implementation_summary`

Declaration: `generate/evidence.cppm:72`

Definition: `generate/evidence_builder.cppm:268`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过整合多个内部数据源来构建证据包。首先，它从 `project_root` 和 `root` 初始化基础路径，随后遍历 `page_summaries` 并依赖 `maybe_resolve_snippet` 解析其中嵌入的代码片段引用，确保所有动态内容均被展开为具体文本。之后，函数组合 `model`、`analyses`、`imported_summaries` 以及命名空间事实 `namespace_fact`，并借助 `sym`、`target` 等变量建立符号映射与目标关联，最终将所有元素打包为 `pack` 结构返回。`file_it` 和 `mod` 用于遍历文件与模块层次结构，控制整个循环的进程。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters `const int &`, `const int &`, `int`
- associated analysis data from `SymbolAnalysisStore` or `EvidencePack`

#### Usage Patterns

- called when generating page markdown for function implementation summaries
- used in conjunction with `build_evidence_for_function_analysis` and `render_page_markdown`

### `clore::generate::build_evidence_for_index_overview`

Declaration: `generate/evidence.cppm:64`

Definition: `generate/evidence_builder.cppm:204`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该实现首先在 `project_root` 下定位 `root`，并通过 `file_it` 遍历文件系统结构以收集 `page_summaries`。对于每个页面摘要，算法调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 来判断是否需要解析内联片段，若返回真则触发片段解析过程。在此过程中，函数维护一个 `pack` 容器，用于累积模块（`mod`）、命名空间（`ns`、`namespace_fact`）及符号（`sym`）等分析单元，并借助 `analyses` 和 `model` 对象来存储推理结果与索引模型。最终，`imported_summaries` 从外部模型合并，所有信息被整合为 `target` 的证据结构，以支持索引概览的生成。控制流主要围绕对 `page_summaries` 的循环、对 `maybe_resolve_snippet` 的条件分支，以及通过 `pack` 对多层级命名空间和模块的递归处理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The two `const int &` parameters, likely representing symbol or page identifiers for the index overview.

#### Usage Patterns

- Called during page generation to produce evidence for the index overview.

### `clore::generate::build_evidence_for_module_architecture`

Declaration: `generate/evidence.cppm:58`

Definition: `generate/evidence_builder.cppm:173`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数首先获取 `project_root` 和 `root` 节点，然后遍历模块层次结构，为每个 `pack`、`mod` 和 `namespace_fact` 调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 来解析代码片段，并将结果累积到 `analyses`、`page_summaries` 和 `imported_summaries` 中。内部控制流通过嵌套循环处理 `target` 下的子模块，并使用 `model` 对象来暂存分析数据，同时依赖 `project_root` 和 `root` 来定位资源并避免重复导入。最终，函数将这些收集到的证据整合为模块架构的完整描述。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- All integer parameters (module identifier, file identifiers, analysis context)

#### Writes To

- Return value (int) indicating success or failure

#### Usage Patterns

- Called during page generation for module-level documentation
- Used to populate evidence for architecture summary sections
- Invoked by `clore::generate::build_page_root` or similar page builders

### `clore::generate::build_evidence_for_module_summary`

Declaration: `generate/evidence.cppm:52`

Definition: `generate/evidence_builder.cppm:142`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过迭代根结点 `root` 下的文件结构来构建证据。它使用 `file_it` 遍历每个文件，结合 `project_root` 确定文件路径，并对每个文件调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 判断是否需要解析其中出现的代码片段。解析后收集的摘要信息被存入 `page_summaries`，并进一步汇入 `analyses` 和 `model` 对象中。

后续处理涉及 `imported_summaries` 与 `namespace_fact`，函数将这些数据整合到 `pack`、`mod`、`ns` 等更粗粒度的表示中，同时更新 `target` 和 `sym` 等符号信息。最终 `model` 结合 `analyses` 与 `project_root` 生成证据，返回一个表示构建结果的值。整个控制流以内联函数 `maybe_resolve_snippet` 和各类集合操作为依赖，未依赖外部公开接口。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the four `const int&` parameters
- the `int` parameter

#### Usage Patterns

- called when generating module documentation pages

### `clore::generate::build_evidence_for_namespace_summary`

Declaration: `generate/evidence.cppm:35`

Definition: `generate/evidence_builder.cppm:21`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过遍历 `page_summaries` 和 `pack` 来构建证据。首先，它从 `root` 和 `project_root` 出发，对每个 `file_it` 迭代，调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 解析 `pack` 中的片段，并将结果存入 `imported_summaries`。随后，函数为每个 `target` 收集 `analyses`，并与 `model` 中的 `namespace_fact` 和 `ns` 信息结合，形成 `page_summaries`。

内部控制流依赖 `analyses` 和 `model` 之间的交互：它从 `pack` 中提取 `mod` 和 `sym`，利用 `model` 查询相关摘要，最后汇总到 `page_summaries` 中。函数返回一个 `int` 值，该值可能代表成功构建的摘要数量或处理状态。最终结果依赖于 `project_root`、`imported_summaries` 和 `pack` 的一致性。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the three `const int &` parameters (namespace, module, file identifiers)
- the `int` parameter (depth)

#### Usage Patterns

- called from page building functions for namespace summaries
- used in `clore::generate::build_namespace_page_root` or similar

### `clore::generate::build_evidence_for_type_analysis`

Declaration: `generate/evidence.cppm:44`

Definition: `generate/evidence_builder.cppm:82`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_type_analysis` 通过遍历 `project_root` 下的文件（由 `file_it` 驱动），收集 `page_summaries`，并调用 `maybe_resolve_snippet` 对代码片段进行解析，构建用于类型分析的证据。内部流程围绕 `pack` 对象展开，将 `root`、`sym`、`target`、`ns` 与 `mod` 等符号实体关联到 `model` 和 `analyses` 结构上，并处理 `imported_summaries` 和 `namespace_fact`。该函数依赖 `clore::generate` 命名空间内的模型抽象和文件系统迭代，最终生成一个表示证据数量的整型返回值。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters `const int &` (likely analysis store or symbol `IDs`)
- parameters `const int &` (likely analysis store or symbol `IDs`)
- parameter `int` (likely depth or index)

#### Usage Patterns

- called to build evidence for type analysis pages
- used in documentation generation pipeline

### `clore::generate::build_evidence_for_type_declaration_summary`

Declaration: `generate/evidence.cppm:77`

Definition: `generate/evidence_builder.cppm:302`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数遍历 `root` 所表示的语法树节点，聚焦于类型声明节点。对于每个由 `target` 标识的类型声明，它从 `project_root` 定位源文件，并使用 `file_it` 迭代文件内容。内部控制流首先检查并解析代码片段，调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 以确定是否可以直接引用嵌入的上下文。随后，函数从 `analyses` 和 `model` 中提取相关分析结果，包括模块 `mod`、命名空间事实 `namespace_fact` 以及符号信息 `sym`。它会合并 `imported_summaries` 中来自其他翻译单元的摘要数据，并将结果整理为 `page_summaries` 条目。依赖关系主要围绕 `model` 提供的类型层次结构和 `pack` 中封装的声明集合，最终将证据汇总到 `root` 对应的页面摘要结构中。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `const int &` parameters representing the type declaration identifier, analysis store, and page context

#### Writes To

- the returned integer value representing an evidence pack or summary identifier

#### Usage Patterns

- called during `build_page_root` or `render_page_markdown` to provide evidence for type declaration pages
- used in conjunction with `build_evidence_for_type_analysis` and `build_evidence_for_type_implementation_summary`

### `clore::generate::build_evidence_for_type_implementation_summary`

Declaration: `generate/evidence.cppm:82`

Definition: `generate/evidence_builder.cppm:334`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_type_implementation_summary` 的执行始于初始化 `root` 和 `project_root`，然后构造空的 `page_summaries` 容器。内部主循环遍历 `pack` 及其包含的 `mod` 与 `sym`，对每个符号位点调用 `maybe_resolve_snippet` 以尝试解析代码片段，并将成功解析的结果与当前 `target`、`analyses` 以及 `model` 上下文关联，逐步填充 `imported_summaries`。算法通过 `namespace_fact` 收集命名空间层面的证据，最终将每个符号的汇总信息写入 `page_summaries`。

核心控制流依赖 `project_root` 定位项目文件，依赖 `analyses` 容器提供静态分析数据，并通过 `model` 维护类型层次与导入关系。内部调用了匿名命名空间中的 `maybe_resolve_snippet` 来按需解析代码片段，其返回值直接影响证据是否被采纳。整个过程紧密整合了 `pack` 的模块结构、`root` 的目录布局以及 `ns` 的命名空间信息，最终将多源证据合并到 `page_summaries` 中。

#### Side Effects

- Allocates and stores evidence data in an internal registry or cache

#### Reads From

- `SymbolAnalysisStore` global instance
- type analysis data
- implementation page plan
- function parameters `a`, `b`, `c`

#### Writes To

- Internal evidence cache or registry
- return value representing evidence identifier

#### Usage Patterns

- Called during page generation for type implementation summary pages
- Used by page planning functions like `build_page_plan_set`

### `clore::generate::build_evidence_for_variable_analysis`

Declaration: `generate/evidence.cppm:48`

Definition: `generate/evidence_builder.cppm:113`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过遍历当前项目中的源文件来构建变量分析的证据。首先，它获取 `project_root` 并初始化 `page_summaries` 和 `analyses` 等容器，然后循环处理每个文件，调用 `maybe_resolve_snippet` 决定是否解析代码片段，并从 `pack` 中提取 `sym` 和 `target` 等符号信息。对于每个分析目标，它使用 `model` 进行上下文建模，并收集 `imported_summaries` 和 `namespace_fact` 等事实。最后，它将生成的证据与 `root` 关联，并可能对 `mod` 和 `ns` 进行递归处理，所有结果均写入 `page_summaries` 和 `analyses` 结构中。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const int & (analysis store)
- const int & (variable identifier)
- int (additional parameter)

#### Usage Patterns

- called to build evidence for variable analysis prompts
- used in symbol analysis pipeline for variables

### `clore::generate::build_prompt`

Declaration: `generate/evidence.cppm:94`

Definition: `generate/evidence.cppm:651`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_prompt` 依据给定的 `PromptKind` 选择对应的提示词模板，然后结合证据包中的结构化数据生成最终的提示文本。内部流程首先调用 `prompt_template_of(kind)` 获取模板字符串；若返回空，则构造并返回一个包含错误消息的 `PromptError`。否则，将模板、`evidence` 以及通过 `format_evidence_text(evidence)` 生成的证据文本一同传递给 `instantiate_prompt_with_evidence`，由后者完成模板填充并返回结果字符串。核心依赖包括 `prompt_template_of` 的模板选择逻辑、`instantiate_prompt_with_evidence` 的模板实例化机制，以及 `format_evidence_text` 对 `EvidencePack` 的文本格式化能力。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `PromptKind kind`
- `const EvidencePack& evidence`
- `prompt_template_of(kind)`
- `format_evidence_text(evidence)`

#### Usage Patterns

- Called during prompt construction for different prompt kinds
- Used by higher-level prompt builders

### `clore::generate::format_evidence_text`

Declaration: `generate/evidence.cppm:86`

Definition: `generate/evidence.cppm:580`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::format_evidence_text` 的实现直接委托给 `clore::generate::format_evidence_text_bounded`，并将 `std::numeric_limits<std::size_t>::max()` 作为最大长度参数传入，从而生成完整的、未截断的证据文本。`format_evidence_text_bounded` 内部将 `EvidencePack` 分解为符号节和文本节，依次使用 `append_section_bounded` 填充内容，并依赖一系列具体构建函数（如 `build_evidence_for_function_analysis`、`build_evidence_for_type_analysis`）和事实收集辅助函数（如 `collect_facts`、`collect_namespace_facts_cached`）来组装各节。最终通过 `instantiate_prompt_with_evidence` 将组合后的文本与对应提示模板合并，得到最终的证据文本字符串。这种设计使 `format_evidence_text` 保持简洁的入口，将长度约束和复杂的布局逻辑完全交由有界版本处理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `pack` parameter of type `EvidencePack`

#### Usage Patterns

- called when the full evidence text is needed without truncation

### `clore::generate::format_evidence_text_bounded`

Declaration: `generate/evidence.cppm:88`

Definition: `generate/evidence.cppm:584`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先预留 `4096` 字节的字符串容量，然后尝试追加标题 `## EVIDENCE\n\n`；若空间不足（通过 `append_if_fits` 判断）则返回空字符串。随后，它依次处理四个 `SymbolSection`（`Target`、`Local Context`、`Dependencies`、`Used By`）和两个 `TextSection`（`Source Snippets`、`Related Page Summaries`）。每个章节的结构包含标题、指向对应 `EvidencePack` 字段的指针（如 `pack.target_facts`、`pack.local_context` 等）以及一个渲染回调函数（例如 `render_detailed_fact`、`render_context_fact`、`render_source_snippet`、`render_summary_item`）。遍历章节时调用辅助函数 `append_section_bounded`，该函数负责在不超过 `max_length` 的前提下逐个追加项目标题和渲染后的内容；若某个项目超出剩余容量则跳过。最终返回拼接好的字符串。

依赖项包括匿名命名空间中的 `append_if_fits`、`append_section_bounded`，以及多个渲染函数；数据结构依赖 `EvidencePack` 的各字段和内部结构体 `SymbolSection`、`TextSection`。控制流核心是多次调用边界检查的追加操作，无复杂分支或递归。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- pack`.target_facts`
- pack`.local_context`
- pack`.dependency_context`
- pack`.reverse_usage_context`
- pack`.source_snippets`
- pack`.related_page_summaries`

#### Usage Patterns

- Called by `clore::generate::build_prompt` to format evidence with a size limit.

## Internal Structure

该模块将符号提取和模型定义的结果组装为供下游提示生成使用的证据包。公开接口位于 `clore::generate` 命名空间，包含一系列 `build_evidence_for_*` 函数，每种函数针对一种分析类型（函数、类型、变量等）生成对应的 `EvidencePack`，后者内部持有目标事实、上下文标记和相关页面摘要。对证据文本的格式化与边界约束由 `format_evidence_text_bounded` 和 `append_section_bounded` 等辅助函数处理。

内部实现了分层的缓存和收集机制：`__detail` 子命名空间通过 `collect_facts`、`collect_summaries` 和带缓存的 `collect_namespace_facts_cached` 避免在同一命名空间内重复遍历符号，匿名命名空间则封装了具体的提示模板常量、渲染逻辑和键值构造。模块依赖 `generate:model` 定义的核心数据类型（如 `SymbolFact`、`EvidencePack`），并导入 `extract` 模块以获取原始符号和关系数据。

## Related Pages

- [Module extract](../extract/index.md)
- [Module generate:model](model.md)

