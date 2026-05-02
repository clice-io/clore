---
title: 'Module generate:evidence'
description: '模块 generate:evidence 负责文档生成管线中的证据收集与组装环节。它根据不同的生成场景（索引概览、命名空间摘要、模块架构、函数声明/实现、类型分析/声明/实现等）构建结构化的 EvidencePack，每个证据包封装了目标符号的事实信息、上下文依赖、源码片段及相关页面摘要。公开的 build_evidence_for_* 系列函数是模块的入口，调用方提供适当的上下文标识符和分析数据后获得一个整数句柄，该句柄可传递给 format_evidence_text 或 format_evidence_text_bounded 以生成受长度约束的 Markdown 证据文本，或通过 build_prompt 组装完整的 LLM 提示。内部通过 collect_facts、collect_namespace_facts_cached 等辅助函数完成高效的事实收集与缓存，避免重复遍历同一命名空间的符号，保证了多符号批次处理时的性能。'
layout: doc
template: doc
---

# Module `generate:evidence`

## Summary

模块 `generate:evidence` 负责文档生成管线中的**证据收集与组装**环节。它根据不同的生成场景（索引概览、命名空间摘要、模块架构、函数声明/实现、类型分析/声明/实现等）构建结构化的 `EvidencePack`，每个证据包封装了目标符号的事实信息、上下文依赖、源码片段及相关页面摘要。公开的 `build_evidence_for_*` 系列函数是模块的入口，调用方提供适当的上下文标识符和分析数据后获得一个整数句柄，该句柄可传递给 `format_evidence_text` 或 `format_evidence_text_bounded` 以生成受长度约束的 Markdown 证据文本，或通过 `build_prompt` 组装完整的 LLM 提示。内部通过 `collect_facts`、`collect_namespace_facts_cached` 等辅助函数完成高效的事实收集与缓存，避免重复遍历同一命名空间的符号，保证了多符号批次处理时的性能。

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

`clore::generate::EvidencePack` 是一个只包含公共数据成员的结构体，用作生成证据的纯数据容器。其内部结构由三个字符串字段（`page_id`、`prompt_kind`、`subject_name`、`subject_kind`）和六个 `std::vector` 字段（`target_facts`、`local_context`、`dependency_context`、`reverse_usage_context`、`related_page_summaries`、`source_snippets`）组成，全部为 `std::string` 或 `std::vector<SymbolFact>` 类型。结构体未定义任何自定义构造函数或赋值运算符，依赖编译器生成的默认实现，因此所有成员通过默认初始化保持确定状态（例如空字符串或空向量）。关键的实现不变式是：一旦填充，`subject_name` 与 `subject_kind` 必须一致，且 `source_snippets` 不应在未设置相关上下文的情况下独立包含代码段，尽管这些约束仅在构造逻辑中强制要求（而非结构体自身内部检查）。

#### Invariants

- 所有字段在默认构造后均为空
- 字段填充由外部证据收集过程完成
- 不保证特定字段非空

#### Key Members

- `page_id`
- `prompt_kind`
- `subject_name`
- `subject_kind`
- `target_facts`
- `local_context`
- `dependency_context`
- `reverse_usage_context`
- `source_snippets`
- `related_page_summaries`

#### Usage Patterns

- 作为 `clore::generate` 命名空间中生成函数的输入
- 由证据收集步骤填充并传递给生成流水线
- 为提示构造提供完整的上下文事实

### `clore::generate::PromptError`

Declaration: `generate/evidence.cppm:90`

Definition: `generate/evidence.cppm:90`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::PromptError` 是一个简单的聚合体，其唯一的数据成员 `message` 类型为 `std::string`。该结构体未声明任何用户提供的特殊成员函数，因此其二进制布局完全由 `message` 成员决定，而 `message` 的内部状态（如分配的内存和长度）由 `std::string` 自身管理。没有额外的不变量强加于 `message` 的值，除了它应当是一个合法的 `std::string` 实例。由于是聚合，实例可以直接通过花括号初始化，例如 `PromptError{"..."}`，此时 `message` 将被递归地构造。该类型不包含任何成员函数或重载操作符，因此所有行为均继承自 `std::string` 的接口。

#### Invariants

- `message` 存储错误描述的文本内容
- `message` 可以为空字符串

#### Key Members

- `message` 字段

#### Usage Patterns

- 作为函数或操作的错误返回类型
- 在异常不可用或无需堆栈展开时用于错误传递

### `clore::generate::SymbolFact`

Declaration: `generate/evidence.cppm:9`

Definition: `generate/evidence.cppm:9`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

结构体 `clore::generate::SymbolFact` 是符号事实的数据载体，其成员直接映射自提取阶段产生的符号元信息。核心成员包括由 `extract::SymbolID` 类型的 `id` 唯一标识的符号，以及 `qualified_name`、`signature`、`kind_label` 和 `access` 等描述性字符串。`is_template` 默认为 `false`，当其值为 `true` 时，`template_params` 应包含模板参数列表的完整文本表示。源代码位置通过 `declaration_file` 和 `declaration_line`（默认值为 0）记录，而 `doc_comment` 则存储关联的文档注释。该结构体仅进行成员的直接初始化，不维护额外不变量，但调用方应保证 `is_template` 与 `template_params` 的内容一致。

#### Invariants

- `id` is always present as an `extract::SymbolID`
- `is_template` defaults to `false`
- `declaration_line` defaults to `0`
- `qualified_name`, `signature`, `kind_label`, `access`, `template_params`, `declaration_file`, and `doc_comment` are default-constructed empty strings unless explicitly set
- No guarantees about the content or validity of string fields beyond what is provided by the extraction process

#### Key Members

- `id`
- `qualified_name`
- `signature`
- `kind_label`
- `access`
- `is_template`
- `doc_comment`

#### Usage Patterns

- Instantiated to store the result of symbol extraction
- Passed or stored as a complete record of a symbol's facts for further processing or generation
- Fields are accessed individually to retrieve specific attributes of the symbol

## Functions

### `clore::generate::__detail::collect_analysis_summaries`

Declaration: `generate/evidence.cppm:121`

Definition: `generate/evidence.cppm:241`

此函数是 `collect_analysis_summaries_impl` 的薄转发器。它接收 `analyses`、`model` 和 `ids` 三个参数，并将它们直接传递给实现函数，同时提供一个恒等 lambda（`[](extract::SymbolID id) { return id; }`）作为 `symbol_id_of` 参数，使得每个 `extract::SymbolID` 在内部被视作自身的标识符。这种设计避免了在调用处重复构造映射逻辑，并为将来需要不同标识映射的场景保留了相同的实现底层。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore&`)
- `model` (const `extract::ProjectModel&`)
- `ids` (const `std::vector<extract::SymbolID>&`)

#### Usage Patterns

- Called to obtain analysis summaries for a list of symbol `IDs`
- Used as an entry point for summary collection, hiding implementation details

### `clore::generate::__detail::collect_analysis_summaries_impl`

Declaration: `generate/evidence.cppm:116`

Definition: `generate/evidence.cppm:213`

函数 `clore::generate::__detail::collect_analysis_summaries_impl` 是模板函数，负责从给定的 `items` 范围中提取唯一的符号标识符，并在已有的分析数据集中查找对应的分析概要，最终返回一个概要字符串的向量。其核心流程为：首先通过 `std::unordered_set` 对已处理的符号 ID 进行去重，对每个 `item` 使用可调用对象 `symbol_id_of` 获取 `extract::SymbolID`，若该 ID 已处理则跳过；否则通过 `extract::lookup_symbol` 在 `model` 中查找对应的符号对象，若符号不存在也跳过；最后调用 `analysis_overview_markdown` 从 `analyses` 和符号中获取 Markdown 格式的概要文本，仅当非空时追加到结果中。

该函数不修改外部状态，依赖 `extract::lookup_symbol` 和 `analysis_overview_markdown` 两个外部函数完成符号查找和概要生成，同时利用 `items` 的迭代器与 `symbol_id_of` 的转发调用实现多态性。整体控制流为顺序过滤与收集，无递归或复杂分支。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `analyses`（`SymbolAnalysisStore` 类型）
- 参数 `model`（`extract::ProjectModel` 类型）
- 参数 `items`（`Range` 类型）
- 参数 `symbol_id_of`（可调用对象）
- 通过 `symbol_id_of` 获取的 `SymbolID`
- 通过 `extract::lookup_symbol` 从 `model` 中读取的符号数据
- 通过 `analysis_overview_markdown` 从 `analyses` 中读取的摘要

#### Usage Patterns

- 作为 `collect_analysis_summaries` 的实现细节
- 用于收集一组符号的分析摘要，通常用于生成文档概述

### `clore::generate::__detail::collect_facts`

Declaration: `generate/evidence.cppm:103`

Definition: `generate/evidence.cppm:163`

函数 `clore::generate::__detail::collect_facts` 接收一个 `extract::ProjectModel` 引用、一个 `std::vector<extract::SymbolID>` 容器以及一个 `std::string` 形式的项目根路径。它通过局部 `std::unordered_set<extract::SymbolID>` 对输入 ID 进行去重：对于每个 ID，若尚未被处理，则调用 `extract::lookup_symbol` 在 `model` 中查找对应的符号；若存在，则使用 `to_symbol_fact` 将该符号与 `project_root` 转换成 `SymbolFact` 并追加到结果向量中。整个流程不涉及外部缓存或递归，仅依赖 `extract` 模块的符号查找能力与 `to_symbol_fact` 的转换逻辑，最终返回收集到的 `std::vector<SymbolFact>`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model`: `const extract::ProjectModel&`
- `ids`: `const std::vector<extract::SymbolID>&`
- `project_root`: `const std::string&`
- `extract::lookup_symbol(model, id)` return value (if non-null)

#### Usage Patterns

- Called to deduplicate and convert symbol `IDs` into facts before further processing
- Used as part of a larger fact-collection pipeline

### `clore::generate::__detail::collect_merged_facts`

Declaration: `generate/evidence.cppm:108`

Definition: `generate/evidence.cppm:179`

该函数通过折叠表达式对每个传入的 `Groups...` 参数包应用内部 lambda `append_group`，将分散在各组中的符号 ID 聚合并去重。具体流程为：维护一个 `seen` 集合跟踪已处理过的 `extract::SymbolID`；对于每个 `id`，若首次出现则通过 `extract::lookup_symbol` 从 `model` 中解析出原生符号对象，再调用 `to_symbol_fact` 将其转换为 `SymbolFact` 并追加到结果容器 `facts` 中。最终返回包含所有独特事实的 `std::vector<SymbolFact>`。该实现依赖于 `extract::lookup_symbol` 进行符号解析，并依赖 `to_symbol_fact` 完成从底层模型到事实结构的转换，从而为后续的基于事实的证据生成奠定基础。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (const `extract::ProjectModel&`)
- `project_root` (const `std::string&`)
- The elements of each `Groups...` parameter (containers of `extract::SymbolID`)

#### Usage Patterns

- Called to collect facts from multiple symbol ID groups
- Used to merge facts from different input sources

### `clore::generate::__detail::collect_namespace_facts_cached`

Declaration: `generate/evidence.cppm:133`

Definition: `generate/evidence.cppm:275`

The function `clore::generate::__detail::collect_namespace_facts_cached` implements a two‑level caching strategy to avoid repeated traversals of the same namespace. On entry it checks whether `namespace_name` is empty and, if so, returns an empty vector immediately. Otherwise it constructs a `cache_key` from `namespace_name` and `project_root` and looks it up in the module‑local `namespace_facts_cache`. On a cache miss it creates a `NamespaceFactsCacheEntry`, looks up the namespace in `model.namespaces`, and iterates over its `symbols`. For each symbol it calls `to_symbol_fact` to produce a `SymbolFact` and then appends it to one of the three vectors inside the entry (`all_functions`, `all_types`, or `all_variables`) depending on the symbol kind. The entry is then emplaced into the cache.

After a cache hit (or insertion) the function selects the appropriate source vector based on `target_kind` (using the same kind predicates); if no source is selected it returns an empty vector. Finally it copies every `SymbolFact` from that source whose `id` differs from `exclude_id` into the result vector, preserving the original order, and returns the result. The function depends externally on `extract::ProjectModel`, `to_symbol_fact`, and the kind‑checking helpers `is_function_kind`, `is_type_kind`, and `is_variable_kind`.

#### Side Effects

- Stores computed namespace facts into a static cache for reuse

#### Reads From

- `model` parameter (`extract::ProjectModel`)
- `namespace_name` parameter (`std::string`)
- `project_root` parameter (`std::string`)
- `target_kind` parameter (`extract::SymbolKind`)
- `exclude_id` parameter (`extract::SymbolID`)
- static cache `namespace_facts_cache()`
- `model.namespaces` map
- symbol objects via `extract::lookup_symbol`
- symbol kind via `sym->kind`

#### Writes To

- internal static cache `namespace_facts_cache()` via `emplace`

#### Usage Patterns

- Called by other evidence collection functions to avoid redundant namespace traversals
- Used when building evidence for multiple symbols within the same namespace

### `clore::generate::__detail::collect_summaries`

Declaration: `generate/evidence.cppm:112`

Definition: `generate/evidence.cppm:200`

函数 `clore::generate::__detail::collect_summaries` 通过接收一个 `PageSummaryCache` 引用和一个 `std::vector<std::string>` 类型的键列表，遍历每个键并在缓存中进行查找。对于每个键，如果缓存中存在对应的条目且该条目的值不为空，则将该值追加到结果向量中。该函数不依赖其他外部状态，仅使用 `PageSummaryCache` 的查找操作和 `std::vector` 的成员函数。其内部控制流是简单的线性扫描，无分支或递归，目的是从预填充的缓存中筛选出非空摘要文本，供调用者进一步使用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache` parameter (`PageSummaryCache`)
- `keys` parameter (`std::vector<std::string>`)
- each key `std::string`

#### Usage Patterns

- Used to collect non-empty summary strings from a cache given a set of keys.

### `clore::generate::__detail::to_symbol_fact`

Declaration: `generate/evidence.cppm:101`

Definition: `generate/evidence.cppm:147`

`clore::generate::__detail::to_symbol_fact` 是一个工厂函数，负责将提取阶段生成的 `extract::SymbolInfo` 对象转换为生成器内部使用的 `clore::generate::SymbolFact` 结构体。该函数通过逐字段直接映射完成转换：`id`、`qualified_name`、`signature`、`kind_label`（由 `extract::symbol_kind_name` 转换而来）、`access`、`is_template`、`template_params`、`declaration_line` 和 `doc_comment` 均从 `sym` 对应字段原样复制；唯一需要加工的是 `declaration_file`——它调用 `clore::generate::make_source_relative` 将原始绝对路径转换为相对于 `project_root` 的路径，从而保证路径表示的便携性。整个转换过程无分支或循环，其控制流完全由输入参数决定，不涉及任何外部状态或缓存。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.id`
- `sym.qualified_name`
- `sym.signature`
- `sym.kind`
- `sym.access`
- `sym.is_template`
- `sym.template_params`
- `sym.declaration_location.file`
- `sym.declaration_location.line`
- `sym.doc_comment`
- `project_root`
- the result of `extract::symbol_kind_name(sym.kind)`
- the result of `clore::generate::make_source_relative` (which reads `sym.declaration_location.file` and `project_root`)

#### Usage Patterns

- Used to create a `SymbolFact` from a `SymbolInfo` and project root for further analysis or collection.

### `clore::generate::build_evidence_for_function_analysis`

Declaration: `generate/evidence.cppm:40`

Definition: `generate/evidence_builder.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_function_analysis` 的核心算法围绕遍历项目结构并收集与函数分析相关的证据展开。内部控制流首先通过 `project_root` 定位根目录，并初始化 `root` 节点，随后迭代包（`pack`）、模块（`mod`）和命名空间（`ns`）层级。在每一层级，它会解析符号（`sym`）并调用辅助函数 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 来处理未解析的片段，同时更新 `page_summaries` 和 `analyses` 容器。证据的构建依赖于将 `model` 中的抽象语法树信息与文件迭代器 `file_it` 驱动的文件系统映射相结合，最终形成 `imported_summaries` 供分析使用。

实现依赖 `project_root` 路径下的文件系统抽象和 `pack` 管理的代码元数据模型。函数通过 `namespace_fact` 和 `target` 变量追踪当前作用域，确保证据与特定函数分析目标对齐。整个流程利用 `maybe_resolve_snippet` 的动态路由来决定是否将当前节点包含在最终证据集中，从而在递归遍历中高效过滤无关内容。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the two `const int&` parameters
- the `int` parameter

#### Usage Patterns

- Local context includes other evidence-building functions such as `build_evidence_for_function_implementation_summary` and `build_evidence_for_function_declaration_summary`, suggesting it is part of a family of `build_evidence_for_*` functions used in documentation generation.

### `clore::generate::build_evidence_for_function_declaration_summary`

Declaration: `generate/evidence.cppm:67`

Definition: `generate/evidence_builder.cppm:238`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数遍历由 `analyses` 和 `model` 提供的分析数据，并结合 `project_root` 下的 `page_summaries`、`imported_summaries` 以及 `root` 节点，逐步构造证据。内部控制流从解析 `target` 相关的符号开始，通过 `maybe_resolve_snippet` 处理代码片段，随后围绕 `pack`、`mod`、`ns` 等模块结构展开，最终聚合 `sym`、`namespace_fact` 等事实，并将结果写入 `page_summaries`。主要依赖包括 `project_root` 定位源文件、`model` 提供语义信息，以及 `analyses` 中存储的预先计算结果。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the four parameters (`const int &` for declaration identifier, context, and another context; `int` for an extra index)
- potentially an underlying symbol analysis store or evidence cache

#### Usage Patterns

- Called during documentation generation to prepare evidence for function declaration summary pages
- Used alongside `build_evidence_for_function_implementation_summary` and other evidence builders
- Invoked within page construction functions such as `render_page_markdown` or `build_page_doc_layout`

### `clore::generate::build_evidence_for_function_implementation_summary`

Declaration: `generate/evidence.cppm:72`

Definition: `generate/evidence_builder.cppm:268`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数实现一个多阶段聚合流程，用于收集并整合与目标函数实现摘要相关的证据。首先从 `project_root` 开始遍历文件系统，通过 `file_it` 操作迭代项目源文件，并利用 `maybe_resolve_snippet` 判断是否将特定代码片段解析为可引用的摘要内容。随后在遍历过程中收集来自多个维度的分析结果：`analyses` 提供静态分析信息，`model` 承载语义模型，`pack` 管理包级别的上下文，而 `imported_summaries` 则记录跨模块引入的摘要。对于每个目标符号 `sym`，函数在 `page_summaries` 中查找已有的页面摘要，并通过 `namespace_fact` 和 `ns` 建立命名空间层级关系，最终将 `target`、`mod`（模块）、`root`（根节点）等关键对象组装到证据集合中。整个流程依赖 `maybe_resolve_snippet` 作为片段解析的决策点，并依赖 `analyses`、`model` 和 `pack` 等组件提供结构化的输入，最终生成一个可用于下游摘要生成的证据包。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- Constructs evidence for function implementation summary pages
- Called during documentation generation pipeline

### `clore::generate::build_evidence_for_index_overview`

Declaration: `generate/evidence.cppm:64`

Definition: `generate/evidence_builder.cppm:204`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数从 `root` 和 `project_root` 开始，通过 `file_it` 遍历文件结构，以构建 `page_summaries`。内部主要依赖 `analyses` 和 `model` 对象进行内容分析，并调用 `maybe_resolve_snippet` 来逐项处理可能的内嵌代码片段。控制流围绕 `pack` 汇总构建，同时引用 `imported_summaries` 和 `namespace_fact` 等表示已导入的摘要与命名空间事实。最终通过 `target` 整合这些信息并返回一个整数结果，代表构建过程的完成状态或计数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` first parameter
- `const int &` second parameter

#### Usage Patterns

- Called during page generation for index overview evidence construction

### `clore::generate::build_evidence_for_module_architecture`

Declaration: `generate/evidence.cppm:58`

Definition: `generate/evidence_builder.cppm:173`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过遍历模块架构的拓扑结构来构建证据集合。它从传入的 `root` 节点出发，结合 `project_root` 和 `target` 参数，递归地处理所有子节点。对于每个节点，首先调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 判断是否需要解析关联的代码片段；若需要，则从 `pack` 中提取对应的 `sym` 和 `model`，并将解析结果写入 `page_summaries`。随后，函数会收集 `analyses` 中的分析数据，以及来自 `imported_summaries` 的跨模块依赖摘要，最终将这些信息整合进一个 `pack` 结构中，并返回一个整数状态码。

内部控制流主要依赖 `root` 及其子节点的递归迭代，并通过 `file_it` 和 `namespace_fact` 等变量跟踪路径与作用域。关键依赖包括 `clore::generate` 命名空间内的 `maybe_resolve_snippet` 辅助函数，以及用于表示模块、分析和摘要的 `model`、`analyses` 和 `page_summaries` 等类型。函数最终将 `pack` 中的证据与 `ns`、`mod` 等上下文约束关联，并返回表示成功或失败的结果标识。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the four `const int &` parameters representing module architecture identifiers
- the single `int` parameter likely indicating a limit or flag

#### Usage Patterns

- called when constructing evidence for module architecture pages
- used in the evidence generation pipeline for module documentation

### `clore::generate::build_evidence_for_module_summary`

Declaration: `generate/evidence.cppm:52`

Definition: `generate/evidence_builder.cppm:142`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先通过 `project_root` 和 `root` 变量建立目录或节点层次结构，然后迭代 `file_it` 遍历文件或模块。对于每个目标（由 `target` 表示），它调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 检查是否解析成功；若成功，则利用 `analyses` 和 `model` 收集分析结果，填充 `page_summaries`。同时，`imported_summaries` 和 `namespace_fact` 涉及跨模块的依赖关系，可能通过 `pack` 和 `mod` 进行打包与模块化处理。最终汇总所有证据形成模块摘要，依赖内部 `maybe_resolve_snippet` 解析片段，以及 `analyses` 和 `model` 提供的语义分析数据。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameters: `const int&` (four references) and `int`

#### Usage Patterns

- Called when generating module summary pages
- Part of the evidence building pipeline for module-level documentation

### `clore::generate::build_evidence_for_namespace_summary`

Declaration: `generate/evidence.cppm:35`

Definition: `generate/evidence_builder.cppm:21`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数为命名空间摘要构建证据，其实现围绕遍历项目中的文件和分析结果展开。首先，从 `project_root` 出发，通过 `file_it` 迭代文件，结合 `analyses` 和 `model` 中存储的语义信息，逐步填充 `page_summaries`。对于每个需要处理的代码实体（由 `sym`、`target` 等标识），函数调用辅助函数 `maybe_resolve_snippet` 来解析内联代码片段，并将解析结果与 `imported_summaries` 中的已有摘要进行合并，形成统一的摘要结构。

内部控制流主要依赖对 `pack`、`mod` 和 `ns` 等集合的循环，并基于 `namespace_fact` 记录的事实做条件分支。函数在每次迭代中更新 `root` 和 `target` 状态，最终通过聚合所有 `page_summaries` 中的证据条目，返回一个整数结果。该过程紧密依赖 `model` 和 `analyses` 提供的数据模型，以及 `maybe_resolve_snippet` 完成的片段解析逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` parameters (namespace ID, analysis store reference, etc.)
- `int` parameter (possibly a page kind or flags)

#### Usage Patterns

- Called during namespace summary page generation
- Part of the evidence-building pipeline for summary pages

### `clore::generate::build_evidence_for_type_analysis`

Declaration: `generate/evidence.cppm:44`

Definition: `generate/evidence_builder.cppm:82`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先通过给定的入口参数确定 `project_root` 和 `root` 的初始值，然后利用 `root` 遍历 `pack` 容器中的各个模块或命名空间实体。内部控制流依赖 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 对代码片段进行选择性解析：只有当该函数返回 `true` 时，对应的片段才会被展开为 `page_summaries` 中的条目。随后，函数在 `model` 上迭代，收集 `imported_summaries` 以聚合跨模块的类型信息，同时根据 `namespace_fact` 和 `ns` 建立命名空间层级关系。所有收集到的数据被合并到 `analyses` 中，作为最终输出的证据包。

实现的核心依赖包括 `model` 提供的类型层次结构、`pack` 中的模块描述符以及 `maybe_resolve_snippet` 的解析能力。函数通过控制 `root` 和 `project_root` 的多次赋值来切换不同的遍历上下文（例如在 `mod` 和 `target` 之间跳转），从而在单个调用中完成从全局作用域到特定分析目标的证据构建。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Parameter `const int &` (first)
- Parameter `const int &` (second)
- Parameter `int` (third)

#### Writes To

- Return value `int`

#### Usage Patterns

- Called to build evidence for type analysis pages
- Used in conjunction with `find_type_analysis` and `analysis_markdown` functions

### `clore::generate::build_evidence_for_type_declaration_summary`

Declaration: `generate/evidence.cppm:77`

Definition: `generate/evidence_builder.cppm:302`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数首先初始化多个本地状态变量，包括 `root`、`project_root`、`pack`、`model` 和 `analyses`，并利用这些变量遍历传入的 `page_summaries` 及相关结构。内部控制流围绕解析和组装类型声明摘要所需的证据展开：它调用匿名命名空间中的 `maybe_resolve_snippet` 对潜在的代码片段进行解析，同时收集 `imported_summaries`、`namespace_fact`、`ns` 和 `mod` 中的信息，最终将分析结果整合到 `target` 所指向的模型对象中。整个过程依赖外部分析结果和项目根目录 `project_root` 来定位上下文，并通过多次处理 `pack` 和 `model` 来逐步构建完整的证据链。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the three `const int &` parameters (presumably representing context or symbol identifiers)
- the `int` parameter (possibly a flag or count)

#### Writes To

- the return `int` value (may represent a size, status, or index)

#### Usage Patterns

- called when generating evidence summaries for type declarations
- likely invoked within the documentation generation pipeline for type symbols

### `clore::generate::build_evidence_for_type_implementation_summary`

Declaration: `generate/evidence.cppm:82`

Definition: `generate/evidence_builder.cppm:334`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数的核心任务是为给定的类型实现摘要构建证据。它在内部初始化一组局部变量（如 `root`、`project_root`、`pack`、`model`、`analyses`、`page_summaries` 和 `imported_summaries`），并依赖辅助函数 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 来完成对代码片段的解析。算法通过遍历 `pack` 中的元素，结合 `model` 和 `analyses` 执行类型分析，同时收集 `page_summaries` 中的摘要信息。它还利用 `namespace_fact` 和 `ns` 等变量处理命名空间相关的证据。

控制流主要围绕 `project_root` 和 `root` 构建目录结构，然后循环处理模块（`mod`）和包中的符号（`sym`），并调用 `maybe_resolve_snippet` 来确认片段是否可解析。最终，函数将多个分析结果（包括 `model` 中的数据和 `analyses` 中的统计）聚合到 `page_summaries` 中，并返回一个 `int` 值（可能表示处理状态或计数）。整个过程依赖于对 `pack`、`target` 和 `root` 的多次交叉引用，确保每个证据项与正确的模块和项目根路径关联。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- first `const int &` parameter (likely a type analysis store or identifier)
- second `const int &` parameter (possibly a symbol store or context index)
- third `int` parameter (likely an additional index or flag)

#### Usage Patterns

- Called during documentation generation to build evidence for type implementation summaries

### `clore::generate::build_evidence_for_variable_analysis`

Declaration: `generate/evidence.cppm:48`

Definition: `generate/evidence_builder.cppm:113`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_variable_analysis` 从多个整数参数（可能代表根标识符、项目根目录和目标值）出发，通过局部变量 `root`、`project_root`、`target`、`pack` 和 `model` 管理生成过程中的上下文。内部首先调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 来解析代码片段，并以此初始化 `page_summaries` 与 `imported_summaries` 两个集合。随后，函数在 `analyses` 集合上执行核心循环：对每个分析条目，依次提取或构建对应的 `namespace_fact`、`sym`、`ns`、`mod` 等字段，并更新 `model` 与 `pack` 的状态；同时利用 `file_it` 和 `project_root` 定位文件系统路径，将 `page_summaries` 中的摘要证据合并到 `model` 中。整个流程通过嵌套变量赋值和条件分支完成证据的累积，最终返回一个整数结果（可能表示证据数量或处理状态）。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` 参数（符号分析存储的句柄）
- `const int &` 参数（变量标识符）
- `int` 参数（用于上下文或限制）

#### Usage Patterns

- 在生成变量分析文档页面时调用
- 配合 `clore::generate::build_evidence_for_function_analysis` 等相似函数使用

### `clore::generate::build_prompt`

Declaration: `generate/evidence.cppm:94`

Definition: `generate/evidence.cppm:651`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_prompt` 是 prompt 生成流程的入口。它首先调用 `prompt_template_of(kind)` 获取与传入的 `PromptKind` 对应的原始模板字符串；若返回空字符串，则说明该 kind 不受支持，立即返回一个包含错误描述的 `PromptError`。否则，它将模板字符串、`EvidencePack` 引用以及由 `format_evidence_text(evidence)` 生成的格式化证据文本一并传递给 `instantiate_prompt_with_evidence`，由后者完成模板占位符的替换并返回最终的 prompt 字符串。该函数的内部控制流简洁，仅包含一次空模板检查与一次委派调用，其核心依赖集中于两个匿名命名空间辅助函数（`prompt_template_of` 和 `instantiate_prompt_with_evidence`）以及公有函数 `format_evidence_text`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `kind` parameter
- `evidence` parameter
- template returned by `prompt_template_of(kind)`
- result of `format_evidence_text(evidence)`

#### Usage Patterns

- generating an LLM prompt from a kind and collected evidence
- handling unsupported prompt kinds by returning an error

### `clore::generate::format_evidence_text`

Declaration: `generate/evidence.cppm:86`

Definition: `generate/evidence.cppm:580`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::format_evidence_text` 是一个便捷包装器，其核心实现直接委托给有界版本 `clore::generate::format_evidence_text_bounded`。内部控制流极其简单：它将传入的 `EvidencePack` 引用连同 `std::numeric_limits<std::size_t>::max()` 作为 `max_length` 参数传递给后者，从而允许生成任意长度的证据文本而不受截断限制。该函数本身不包含任何额外的校验或格式化逻辑，完全依赖 `clore::generate::format_evidence_text_bounded` 的内部细化控制流（包括按 `SymbolSection` 和 `TextSection` 分组、调用 `append_section_bounded` 等辅助函数，以及基于 `prompt_kind` 和 `subject_kind` 选择合适的提示模板与渲染器）来组装最终输出。主要依赖项包括 `EvidencePack` 类型及其字段（如 `target_facts`、`source_snippets`、`local_context` 等），以及匿名命名空间中的 `target_name_of`、`target_kind_of`、`prompt_template_of` 等工具函数和 `build_evidence_for_*` 系列证据构造函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `pack` parameter
- `std::numeric_limits<std::size_t>::max()` constant

#### Writes To

- returned `std::string` value

#### Usage Patterns

- called when unbounded evidence text formatting is needed
- used as a thin wrapper around `format_evidence_text_bounded`

### `clore::generate::format_evidence_text_bounded`

Declaration: `generate/evidence.cppm:88`

Definition: `generate/evidence.cppm:584`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先预留容量并尝试写入标题头 `## EVIDENCE\n\n`，若超过 `max_length` 则立即返回空字符串。随后通过两个局部结构体 `SymbolSection` 和 `TextSection` 组织四个符号事实节和两个文本项节，分别对应 `pack.target_facts`、`pack.local_context`、`pack.dependency_context`、`pack.reverse_usage_context`、`pack.source_snippets` 和 `pack.related_page_summaries`。对每个节，调用 `append_section_bounded` 依次追加节标题、遍历 `items` 并调用对应的 `render` 函数（`render_detailed_fact`、`render_context_fact`、`render_source_snippet` 或 `render_summary_item`），该函数内部使用 `append_if_fits` 逐项检查长度约束，放弃超出部分的剩余项。

内部控制流完全基于顺序追加与边界检查，无分支或循环外的退出条件（除开头的容量检查）。依赖集中在 `append_if_fits`、`append_section_bounded` 以及四个渲染回调，这些函数均定义在匿名命名空间中，并间接依赖 `EvidencePack` 各字段和 `SymbolFact` 的成员（如 `kind_label`、`qualified_name` 等）。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `pack` parameter of type `const EvidencePack &`
- `pack.target_facts`
- `pack.local_context`
- `pack.dependency_context`
- `pack.reverse_usage_context`
- `pack.source_snippets`
- `pack.related_page_summaries`
- `max_length` parameter

#### Writes To

- local `text` variable of type `std::string` (returned)

#### Usage Patterns

- Formatting evidence text for documentation pages with a size limit
- Called during evidence generation to produce bounded strings

## Internal Structure

模块 `generate:evidence` 位于 `clore::generate` 命名空间，负责从提取和模型分析数据中组装结构化的证据包（`EvidencePack`），供下游提示生成使用。其公开接口由一组针对不同页面类型（命名空间摘要、函数分析、类型声明等）的 `build_evidence_for_*` 函数组成，每个函数接收相关的上下文标识（符号ID、分析存储、项目根目录等）并返回一个整数句柄或封装的证据包。内部实现通过 `__detail` 命名空间隔离了事实收集与缓存逻辑，例如 `collect_namespace_facts_cached` 利用 `NamespaceFactsCacheEntry` 缓存避免重复遍历同一命名空间的符号，而 `collect_facts`、`collect_summaries` 等模板函数则处理通用的事实合并与摘要收集。匿名命名空间则封装了渲染辅助函数（如 `render_source_snippet`、`render_context_fact`）以及根据证据类型选择提示模板的逻辑。模块从 `extract` 和 `generate:model` 导入必要的数据模型与提取结果，并依赖标准库容器和算法，整体采用分层结构以分离关注点、提升缓存效率并保持接口简洁。

## Related Pages

- [Module extract](../extract/index.md)
- [Module generate:model](model.md)

