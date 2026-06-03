---
title: 'Module generate:evidence'
description: '该模块负责从代码提取与分析结果中收集、缓存并格式化结构化的“证据”数据，为后续的提示构建与文档生成提供输入。它公开了一系列 build_evidence_for_* 函数（如针对命名空间摘要、模块摘要、函数分析、类型声明等），这些函数接受内部句柄与标识符，返回代表证据包的整数引用或直接生成格式化文本。同时提供 format_evidence_text 及其有界版本 format_evidence_text_bounded，将证据包转换为适合填入提示模板的字符串，以及 build_prompt 用于组合提示模板与证据输出最终提示。模块内部维护命名空间级别的事实缓存，避免重复遍历，并通过 SymbolFact、EvidencePack 等数据结构封装符号属性与上下文信息，形成证据生成管线的核心中转层。'
layout: doc
template: doc
---

# Module `generate:evidence`

## Summary

该模块负责从代码提取与分析结果中收集、缓存并格式化结构化的“证据”数据，为后续的提示构建与文档生成提供输入。它公开了一系列 `build_evidence_for_*` 函数（如针对命名空间摘要、模块摘要、函数分析、类型声明等），这些函数接受内部句柄与标识符，返回代表证据包的整数引用或直接生成格式化文本。同时提供 `format_evidence_text` 及其有界版本 `format_evidence_text_bounded`，将证据包转换为适合填入提示模板的字符串，以及 `build_prompt` 用于组合提示模板与证据输出最终提示。模块内部维护命名空间级别的事实缓存，避免重复遍历，并通过 `SymbolFact`、`EvidencePack` 等数据结构封装符号属性与上下文信息，形成证据生成管线的核心中转层。

## Imports

- [`extract`](../extract/index.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:analysis`](analysis.md)
- [`generate:evidence_builder`](index.md)
- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::EvidencePack`

Declaration: `src/generate/evidence.cppm:34`

Definition: `src/generate/evidence.cppm:34`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::EvidencePack` 是一个纯聚合结构体，用于打包生成提示所需的所有证据数据。其内部由三类成员组成：标识字段 (`page_id`、`prompt_kind`、`subject_name`、`subject_kind`) 明确目标代码实体的身份和当前生成场景；四个事实向量 (`target_facts`、`local_context`、`dependency_context`、`reverse_usage_context`) 分别存储来自不同语义来源的符号事实，彼此之间在逻辑上互不重叠；文本材料字段 (`source_snippets`、`related_page_summaries`) 提供原始代码片段和相关页面的摘要文本。各向量之间的内容没有强制的不变性约束，但设计上要求 `subject_name` 和 `subject_kind` 与 `target_facts` 中包含的事实一致，且 `page_id` 与 `prompt_kind` 的组合在整个生成流程中保持唯一。所有字段均为简单值类型，无需额外的初始化或清理逻辑，填充顺序不影响结构的正确性。

#### Invariants

- All vector fields are default-constructible and may be empty.
- `page_id` and `prompt_kind` are expected to be non-empty when used for generation.
- `subject_name` and `subject_kind` identify the symbol under analysis.

#### Key Members

- `clore::generate::EvidencePack::page_id`
- `clore::generate::EvidencePack::prompt_kind`
- `clore::generate::EvidencePack::subject_name`
- `clore::generate::EvidencePack::subject_kind`
- `clore::generate::EvidencePack::target_facts`
- `clore::generate::EvidencePack::local_context`
- `clore::generate::EvidencePack::dependency_context`
- `clore::generate::EvidencePack::reverse_usage_context`
- `clore::generate::EvidencePack::related_page_summaries`
- `clore::generate::EvidencePack::source_snippets`

#### Usage Patterns

- Filled by evidence collection code before invoking a generation prompt.
- Passed as a single argument to generation functions to provide all necessary symbol context.
- Vector fields are iterated over to format prompt entries.

### `clore::generate::PromptError`

Declaration: `src/generate/evidence.cppm:102`

Definition: `src/generate/evidence.cppm:102`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::PromptError` 的实现是一个简单的错误信息持有者，其内部仅包含一个 `std::string message` 成员。该结构体不定义任何额外的不变量或行为；消息内容由构造或赋值时直接设定，其有效性完全由调用方保证。`message` 成员直接存储错误描述的文本，没有进行格式化、编码或截断处理，整个结构体生命周期内不涉及资源管理以外的任何逻辑。

#### Invariants

- `message` 成员存储任意字符串，无格式或长度约束
- 结构体本身不提供任何错误分类或代码机制

#### Key Members

- `std::string message`：存储错误详细描述的字符串成员

#### Usage Patterns

- 其他代码可能返回或抛出 `PromptError` 对象以传递生成错误
- 使用方通过访问 `.message` 获取具体错误文本

### `clore::generate::SymbolFact`

Declaration: `src/generate/evidence.cppm:21`

Definition: `src/generate/evidence.cppm:21`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::SymbolFact` 是一个纯粹的聚合结构，用于封装从代码基中提取的单个符号的全部元数据。其成员 `id` 类型为 `extract::SymbolID` 作为唯一标识符；五个 `std::string` 字段 `qualified_name`、`signature`、`kind_label`、`access` 和 `doc_comment` 分别存储符号的限定名称、调用签名、种类标签（例如“函数”或“类”）、访问级别和关联文档注释。模板特性由布尔 `is_template`（默认 `false`）和 `template_params` 字符串表示，而声明位置通过 `declaration_file` 和 `declaration_line`（默认 `0`）记录。

重要的不变性包括：`is_template` 和 `declaration_line` 具有稳定的默认值，从而在聚合初始化时提供合理的默认状态；所有字符串成员预期包含有效的 UTF-8 文本，尽管这一假设并未通过运行时代码强制执行。结构体的所有字段按成员声明顺序可直接用于聚合初始化，且未定义任何用户提供的构造函数、析构函数或赋值运算符，因此该类型是平凡可复制的，并保持标准布局。

#### Invariants

- `is_template` 的默认值为 `false`
- `declaration_line` 的默认值为 `0`

#### Key Members

- `id`
- `qualified_name`
- `signature`
- `kind_label`
- `access`
- `is_template`
- `template_params`
- `declaration_file`
- `declaration_line`
- `doc_comment`

#### Usage Patterns

- 在代码生成管线中作为数据传输对象，将符号信息从提取阶段传递到生成阶段

## Functions

### `clore::generate::__detail::collect_analysis_summaries`

Declaration: `src/generate/evidence.cppm:133`

Definition: `src/generate/evidence.cppm:253`

`clore::generate::__detail::collect_analysis_summaries` 的实现将核心工作委托给 `collect_analysis_summaries_impl`，并传递一个恒等投影 lambda：`[](extract::SymbolID id) { return id; }`。该函数本身不包含循环或分支，仅作为转发层，将外部传入的 `analyses`、`model` 和 `ids` 原样传递给下层泛型实现。控制流由此完全取决于 `collect_analysis_summaries_impl`：它会遍历 `ids` 中的每个符号标识符，通过投影函数获取实际 ID，然后从 `analyses` 中检索对应的分析摘要，并利用 `model` 提供的项目模型信息进行聚合，最终返回一个字符串向量。依赖方面，它间接依赖于 `collect_analysis_summaries_impl` 的内部逻辑以及 `SymbolAnalysisStore`、`extract::ProjectModel` 和 `extract::SymbolID` 的接口定义。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses`
- `model`
- `ids`

#### Usage Patterns

- used to retrieve summaries for specified symbol `IDs` from the analysis store and project model

### `clore::generate::__detail::collect_analysis_summaries_impl`

Declaration: `src/generate/evidence.cppm:128`

Definition: `src/generate/evidence.cppm:225`

函数 `clore::generate::__detail::collect_analysis_summaries_impl` 实现了对一组符号的分析摘要的批量收集与去重。它接受一个 `SymbolAnalysisStore` 对象 `analyses`、一个 `extract::ProjectModel` 对象 `model`、一个表示待处理条目范围的 `Range items` 以及一个可调用对象 `symbol_id_of`，该可调用对象从每个条目中提取出 `extract::SymbolID`。函数内部维护一个 `std::unordered_set<extract::SymbolID>` 类型变量 `seen` 用于记录已处理的符号 ID，从而确保每个摘要只被收集一次。对于 `items` 中的每个条目，先通过 `std::forward<SymbolIDOf>(symbol_id_of)(item)` 获得符号 ID，若 `seen` 中已存在则直接跳过；否则插入 `seen` 并尝试通过 `extract::lookup_symbol(model, symbol_id)` 获得对应的符号对象。若符号为 `nullptr`（即模型中没有该符号的信息），则也跳过。随后调用 `analysis_overview_markdown(analyses, *symbol)` 获取该符号的分析概要以 Markdown 字符串形式返回，若返回的指针非空且字符串非空，则将该字符串追加到结果 `std::vector<std::string> result` 中。最终返回所有收集到的非空摘要字符串。

该函数的控制流核心是“遍历 – 去重 – 查找 – 收集”四步流水线，通过 `seen` 集合和 `nullptr` 检查高效过滤无效和重复条目。其依赖集中在 `extract::lookup_symbol`（根据 ID 在模型中查找符号）与 `analysis_overview_markdown`（从分析存储中生成概要文本）这两个外部函数上，且要求 `symbol_id_of` 可调用对象能为每个条目生成对应的符号 ID。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- The `analyses` parameter (const reference)
- The `model` parameter (const reference)
- The `items` range
- The `symbol_id_of` callable
- The `extract::lookup_symbol` function reading the model
- The `analysis_overview_markdown` function reading the analyses store

#### Usage Patterns

- Used internally to deduplicate and collect analysis summaries for a set of items
- Typically called from `clore::generate::__detail::collect_analysis_summaries` or similar
- Callers provide a range and a callable to extract symbol `IDs`

### `clore::generate::__detail::collect_facts`

Declaration: `src/generate/evidence.cppm:115`

Definition: `src/generate/evidence.cppm:175`

该函数遍历传入的符号标识符列表，并在每个标识符上执行查重与转换操作。它维护一个 `std::unordered_set<extract::SymbolID>` 类型的局部变量 `seen`，用于记录已经处理过的标识符。对于每个 `id`，如果其已存在于 `seen` 中则跳过；否则将其插入 `seen` 并尝试通过 `extract::lookup_symbol` 在 `model` 中查找对应的符号对象。若查找成功，则调用 `to_symbol_fact` 将该符号对象与 `project_root` 转换为 `SymbolFact` 实例，并将该实例追加到结果 `facts` 向量中。最终返回包含所有已去重符号事实的向量。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `extract::ProjectModel` passed as `model`
- `std::vector<extract::SymbolID>` passed as `ids`
- `std::string` passed as `project_root`
- internal state of `extract::ProjectModel` (via `extract::lookup_symbol`)
- elements of `ids` (the `extract::SymbolID` values)

#### Usage Patterns

- core collector in fact generation pipeline
- called after obtaining a list of symbol `IDs` from a project model
- used to produce a deduplicated vector of `SymbolFact` objects for downstream processing

### `clore::generate::__detail::collect_merged_facts`

Declaration: `src/generate/evidence.cppm:120`

Definition: `src/generate/evidence.cppm:191`

函数 `clore::generate::__detail::collect_merged_facts` 是一个变参模板函数，其核心任务是从多个来源收集并合并 `SymbolFact` 对象，同时消除重复。它接受 `extract::ProjectModel`、项目根路径以及多个 `Groups` 形参包，每个 `Groups` 应为一个可遍历的 `extract::SymbolID` 容器。内部使用一个 `std::unordered_set<extract::SymbolID>` 类型的 `seen` 集合记录已处理的 ID，并通过一个名为 `append_group` 的 lambda 表达式处理每个组。该 lambda 遍历组内所有 ID，若 `seen.insert(id).second` 为 true（即该 ID 此前未被加入），则调用 `extract::lookup_symbol` 从模型中查找对应的符号；若符号指针非空，则通过 `to_symbol_fact` 将其转换为 `SymbolFact` 并添加到结果向量 `facts` 中。所有组通过折叠表达式 `(append_group(groups), ...)` 依次处理。最终返回这个已合并去重的事实列表。

该函数不依赖任何外部缓存或全局状态，仅依赖于 `extract` 命名空间中的符号查找与事实构造函数。其控制流完全由折叠表达式和 `seen` 集合驱动，保证了每个符号在整个调用中只被处理一次，从而避免下游重复生成证据。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `model`（用于查找符号）
- 参数 `project_root`
- 参数 `groups...` 中每个 group 包含的符号 ID 集合

#### Usage Patterns

- 合并多个 group 的符号事实并去重
- 将符号 ID 转换为 `SymbolFact` 并收集
- 作为事实收集流水线中的聚合步骤

### `clore::generate::__detail::collect_namespace_facts_cached`

Declaration: `src/generate/evidence.cppm:145`

Definition: `src/generate/evidence.cppm:287`

该函数实现了命名空间内符号事实的缓存收集。首先检查 `namespace_name` 是否为空，若是则直接返回空向量。随后构造 `cache_key`（由 `namespace_name` 与 `project_root` 拼接而成），并从 `namespace_facts_cache()` 返回的全局缓存中查找。若未命中，则遍历 `model.namespaces` 中对应命名空间的所有符号 ID，依次调用 `extract::lookup_symbol` 和 `to_symbol_fact` 构造 `SymbolFact`，再根据符号的种类（函数、类型或变量）分别存入 `NamespaceFactsCacheEntry` 的 `all_functions`、`all_types` 或 `all_variables`，最后将条目插入缓存。之后根据 `target_kind` 选取对应的向量作为数据源，并剔除与 `exclude_id` 匹配的条目，收集剩余事实并返回。

整个过程通过 `namespace_facts_cache()` 返回的 static 局部缓存避免对同一命名空间反复遍历。数据分类依赖于 `is_function_kind`、`is_type_kind` 和 `is_variable_kind` 等种类判定函数，并最终依赖 `extract::ProjectModel` 的命名空间符号索引与符号解析。

#### Side Effects

- Modifies the static cache via `namespace_facts_cache()` by inserting or accessing entries
- Allocates heap memory for cached `SymbolFact` vectors and the returned result vector

#### Reads From

- Parameter `model` (const reference to `extract::ProjectModel`) — specifically `model.namespaces` and `extract::lookup_symbol`
- Parameter `namespace_name` (string)
- Parameter `project_root` (string)
- Parameter `target_kind` (`extract::SymbolKind`)
- Parameter `exclude_id` (`extract::SymbolID`)
- Static cache returned by `namespace_facts_cache()`
- `to_symbol_fact` function (reads symbol and `project_root`)
- Predicates `is_function_kind`, `is_type_kind`, `is_variable_kind` (read symbol kind)

#### Writes To

- Static cache (inserts `NamespaceFactsCacheEntry`)
- Local variables `entry`, `result`, `cache`
- Vector elements in the cache entry (`all_functions`, `all_types`, `all_variables`)
- Returned `std::vector<SymbolFact>`

#### Usage Patterns

- Called by other evidence-collection functions (e.g., `collect_facts`) to avoid redundant namespace traversal
- Used when building evidence for multiple symbols within the same namespace

### `clore::generate::__detail::collect_summaries`

Declaration: `src/generate/evidence.cppm:124`

Definition: `src/generate/evidence.cppm:212`

函数 `clore::generate::__detail::collect_summaries` 遍历给定的 `keys` 向量，对每个键在 `cache`（一个 `PageSummaryCache` 类型的关联容器）中执行查找。若找到且对应的值非空，则将该摘要文本追加到结果向量中并返回。该函数用于从缓存中批量提取已存在的摘要，以支持后续的摘要合并或分包逻辑，仅保留实际可用的缓存条目。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `cache` (a `PageSummaryCache` map)
- `keys` (a `std::vector<std::string>`)

#### Usage Patterns

- Collecting summary strings from a cache for a set of keys
- Filtering out missing or empty summaries

### `clore::generate::__detail::to_symbol_fact`

Declaration: `src/generate/evidence.cppm:113`

Definition: `src/generate/evidence.cppm:159`

该函数将 `extract::SymbolInfo` 转换为 `clore::generate::SymbolFact`。其控制流本质上是字段到字段的映射：从 `sym.id`、`sym.qualified_name`、`sym.signature` 等源字段直接初始化目标结构体成员。`kind_label` 通过调用 `extract::symbol_kind_name(sym.kind)` 获得，`declaration_file` 则通过 `clore::generate::make_source_relative` 将原始路径转为相对于 `project_root` 的表示。其他字段如 `access`、`is_template`、`template_params`、`declaration_line` 和 `doc_comment` 均直接复制。

此函数作为生成证据包（如 `EvidencePack` 的 `target_facts`）的底层转换单元，被 `collect_facts`、`collect_namespace_facts_cached` 等汇总函数间接使用。它不涉及条件分支或循环，是整个事实收集管道中的纯数据转换环节。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const extract::SymbolInfo &sym` 的成员：`id`、`qualified_name`、`signature`、`kind`、`access`、`is_template`、`template_params`、`declaration_location.file`、`declaration_location.line`、`doc_comment`
- `const std::string &project_root`

#### Writes To

- 返回的 `SymbolFact` 对象（通过值传递），包括其所有字段

#### Usage Patterns

- 在事实收集过程中将 `extract::SymbolInfo` 转换为 `SymbolFact`
- 作为 `collect_facts` 或类似函数的辅助函数，处理单个符号信息

### `clore::generate::build_evidence_for_function_analysis`

Declaration: `src/generate/evidence.cppm:52`

Definition: `src/generate/evidence_builder.cppm:61`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_function_analysis` 首先根据 `project_root` 初始化局部变量，包括 `model`、`pack`、`analyses` 和 `summary_keys`。它遍历 `page_summaries` 和 `imported_summaries`，对每个条目调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 尝试解析摘要片段，并收集 `dependency_ids`。随后，基于 `target` 和 `sym` 从 `model` 中提取符号信息，结合 `ns` 和 `namespace_fact` 处理命名空间上下文，并通过 `root`、`mod` 和 `file_it` 等变量遍历包与模型结构，填充 `member_ids` 与 `symbol_ids`。最终，函数将累积的 `analyses` 与关联ID整合，返回一个表示构造结果或状态码的整数值。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` parameter (first)
- `const int &` parameter (second)
- `std::string_view` parameter

#### Usage Patterns

- Called during documentation generation to build evidence for a function analysis
- Used in combination with other evidence-building functions to assemble a complete `EvidencePack`

### `clore::generate::build_evidence_for_function_declaration_summary`

Declaration: `src/generate/evidence.cppm:79`

Definition: `src/generate/evidence_builder.cppm:246`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_function_declaration_summary` 实现一个构建过程：根据传入的四个整数引用和一个 `std::string_view` 参数，从当前项目上下文中收集并关联函数声明摘要的证据。内部首先初始化 `project_root`、`page_summaries`、`imported_summaries`、`target` 等局部变量，随后通过 `model` 和 `analyses` 遍历 `dependency_ids`、`symbol_ids`、`member_ids` 等集合，生成 `summary_keys` 并填充 `root` 节点。控制流中包含对 `maybe_resolve_snippet` 的调用，用于解析摘要中的代码片段；同时利用 `namespace_fact` 和 `local_ids` 整合命名空间与局部符号信息。该函数的依赖集中于 `project_root`、`root`、`pack` 以及 `mod` 等上下文变量，最终返回一个整数，表征证据构建状态。

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::build_evidence_for_function_implementation_summary`

Declaration: `src/generate/evidence.cppm:84`

Definition: `src/generate/evidence_builder.cppm:276`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数首先通过 `root` 和 `project_root` 定位目标符号的上下文，然后遍历 `pack` 和 `model` 获取与目标相关的符号信息。它使用 `analyses` 集合收集 `symbol_ids` 和 `member_ids`，并结合 `dependency_ids` 确定依赖关系。通过 `page_summaries` 和 `imported_summaries` 聚合已有的摘要信息，调用 `maybe_resolve_snippet` 解决潜在的片段引用。内部的控制流围绕多个范围迭代器（如 `file_it`、`mod`）展开，并借助 `namespace_fact` 和 `local_ids` 进行作用域判断。整个过程依赖于对 `project_root` 下的文件系统扫描以及预先构建的模型和分析数据。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- first `const int &` parameter (likely a symbol or entity identifier)
- second `const int &` parameter (likely a context or store identifier)
- `std::string_view` parameter (likely a name or key)
- global or function-level analysis stores (implied by related functions like `find_function_analysis`)

#### Writes To

- return value of type `int` (success/failure indicator)

#### Usage Patterns

- called when generating implementation-specific evidence for functions
- used within the documentation generation pipeline after analysis is complete

### `clore::generate::build_evidence_for_index_overview`

Declaration: `src/generate/evidence.cppm:76`

Definition: `src/generate/evidence_builder.cppm:212`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_index_overview` 以 `project_root` 为起点，依次获取 `page_summaries` 与 `imported_summaries`，并通过匿名空间内的辅助函数 `maybe_resolve_snippet` 尝试解析代码片段。随后，它围绕 `target` 及其关联的 `sym`、`pack`、`root`、`mod` 等实体展开遍历，逐步填充 `analyses`、`summary_keys`、`dependency_ids`、`member_ids`、`symbol_ids` 等存储结构，同时引入 `model`、`ns`、`file_it`、`namespace_fact`、`local_ids` 等局部变量以跟踪索引概览所需的上下文信息。整个流程依赖 `project_root` 确定的文件系统根目录，并利用 `page_summaries` 与 `imported_summaries` 所提供的摘要数据完成证据的拼装，最终返回一个 `int` 标识处理结果。

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::generate::build_evidence_for_module_architecture`

Declaration: `src/generate/evidence.cppm:70`

Definition: `src/generate/evidence_builder.cppm:181`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数遍历 `page_summaries` 和 `imported_summaries`，以 `project_root` 为根目录，通过 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 判断哪些片段需要解析。内部控制流从 `pack` 和 `analyses` 构建层次化的 `model`，并填充 `symbol_ids`、`member_ids`、`dependency_ids`、`local_ids` 等集合，同时依据 `target` 和 `ns` 进行过滤。`mod` 和 `root` 分别表示模块节点与根节点，`file_it` 和 `namespace_fact` 用于进一步的细化筛选，最终返回一个 `int` 结果，通常表示证据项数量或状态码。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const int &` parameters (likely identifiers for module, namespace, file, and symbol)
- `std::string_view` source text

#### Usage Patterns

- Called during the generation of documentation evidence for module architecture pages

### `clore::generate::build_evidence_for_module_summary`

Declaration: `src/generate/evidence.cppm:64`

Definition: `src/generate/evidence_builder.cppm:150`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数从输入参数确定 `project_root` 和 `target` 后，首先遍历 `model` 中的所有模块（`mod`），通过 `pack` 和 `analyses` 收集每个模块的 `summary_keys`、`dependency_ids` 以及 `member_ids`。对于每个模块，它调用 `clore::generate::(anonymous namespace)::maybe_resolve_snippet` 检查是否需要解析代码片段，并将解析结果记录到 `namespace_fact` 中。同时，它利用 `page_summaries` 和 `imported_summaries` 分别聚合本地摘要和跨模块导入摘要，并通过 `local_ids` 和 `symbol_ids` 跟踪实体引用关系。

在遍历过程中，函数将 `root` 节点与 `model` 中的各层结构（如 `ns`、`file_it`）进行关联，最终将收集到的所有摘要键、依赖 ID 和成员 ID 合并到 `pack` 中，返回一个整数代表成功处理的实体数量或状态码。整个流程高度依赖输入参数中的 `project_root` 和 `target` 来定位项目路径和上下文，并通过 `model`、`analyses` 等数据结构驱动控制流。

#### Side Effects

- allocates memory for evidence data
- populates internal evidence structures
- may modify shared analysis or page state

#### Reads From

- the four `const int &` parameters (likely module and page identifiers)
- the `std::string_view` parameter (module name or key)
- global or input-dependent analysis stores (e.g., `SymbolAnalysisStore`)
- link resolver state

#### Writes To

- the returned `int` (success/failure or handle)
- internal evidence pack buffers or caches

#### Usage Patterns

- called during module summary page generation
- used as part of evidence building for prompts
- likely invoked from `clore::generate::build_page_plan_set` or similar higher-level generators

### `clore::generate::build_evidence_for_namespace_summary`

Declaration: `src/generate/evidence.cppm:47`

Definition: `src/generate/evidence_builder.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先初始化若干局部结构，包括用于存储 `page_summaries` 和 `imported_summaries` 的容器，并从 `pack` 中获取目标符号 `sym` 的引用。接着，它遍历 `analyses` 集合，为每个分析构建 `summary_keys` 和 `dependency_ids`，以将当前命名空间与文档的其他部分关联起来。在每次迭代中，函数调用 `maybe_resolve_snippet` 处理内联代码引用，并将解析结果填入 `model`。所有分析处理完毕后，函数构造一个 `namespace_fact`，并将其与文档树的 `root` 相关联。最后一步是用新生成的摘要证据更新 `project_root` 的状态。整个过程依赖 `pack` 提供符号数据、`analyses` 驱动证据构建，以及 `project_root` 执行文件系统操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 三个 `const int &` 参数
- 一个 `std::string_view` 参数

#### Usage Patterns

- 在命名空间摘要页面生成时被调用
- 作为证据构建管线的一部分使用

### `clore::generate::build_evidence_for_type_analysis`

Declaration: `src/generate/evidence.cppm:56`

Definition: `src/generate/evidence_builder.cppm:90`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数首先接收三个参数——分别表示根标识符、目标标识符和一个命名空间字符串——随后通过一组局部变量构建证据集。核心算法围绕 `model` 对象展开，该对象聚合了 `page_summaries`、`imported_summaries` 和 `analyses` 等数据，并利用 `maybe_resolve_snippet` 来判断每个片段是否应当在当前上下文中被解析。内部控制流首先确定 `project_root` 和 `root` 节点，然后遍历 `pack`、`sym` 和 `target` 引用的符号与包结构，同时收集 `member_ids`、`local_ids` 和 `symbol_ids` 等标识符集合。此外，`namespace_fact` 用于记录命名空间事实，而 `dependency_ids` 和 `summary_keys` 则用于关联分析和摘要键值。最终函数通过 `file_it` 迭代文件，并结合 `mod` 和 `root` 完成证据的归并与返回。依赖方面，它使用匿名命名空间中的辅助函数 `maybe_resolve_snippet` 以及外部的 `model` 和 `project_root` 定义。

#### Side Effects

- Modifies or populates an evidence cache or output structure related to type analysis

#### Reads From

- Parameters `const int &` (analysis store handles or indices)
- `std::string_view` (type name or identifier)
- Internal analysis store (e.g., `SymbolAnalysisStore`)

#### Writes To

- Evidence structure (likely referenced via a handle or stored in a caller-provided context)
- Cached analysis results for the type

#### Usage Patterns

- Invoked during page generation for type documentation
- Part of the `build_evidence_for_*` family for symbols
- Used to supply evidence data to higher-level rendering functions

### `clore::generate::build_evidence_for_type_declaration_summary`

Declaration: `src/generate/evidence.cppm:89`

Definition: `src/generate/evidence_builder.cppm:310`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先解析传入的 `target` 与 `project_root`，并初始化内部的 `model`、`pack`、`root` 等中间状态。随后，它遍历 `pack` 中的 `analyses` 集合，对每个分析项通过 `maybe_resolve_snippet` 决定是否展开片段；同时收集 `summary_keys`、`dependency_ids`、`member_ids`、`symbol_ids` 等关键标识符，并利用 `namespace_fact` 和 `local_ids` 构建命名空间与局部依赖关系。控制流在 `file_it` 与 `mod` 之间循环，从 `analyses` 中提取信息并填充 `page_summaries` 与 `imported_summaries`，最终将结果组装成一个表征类型声明摘要的证据对象返回。

整个算法依赖 `project_root` 定位文件系统、`pack` 与 `model` 的结构层次，以及 `maybe_resolve_snippet` 的解析逻辑，并通过多次遍历 `analyses`、`member_ids` 等集合来聚合依赖与摘要数据，从而完成对类型声明证据的构建。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- its parameters: three unnamed `const int &` and one `std::string_view`

#### Usage Patterns

- likely invoked during evidence generation for type declaration summaries

### `clore::generate::build_evidence_for_type_implementation_summary`

Declaration: `src/generate/evidence.cppm:94`

Definition: `src/generate/evidence_builder.cppm:342`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先基于指定的 `project_root` 和 `model` 中的 `root` 节点，遍历 `pack` 中的模块与符号。通过收集 `page_summaries` 与 `imported_summaries`，结合 `dependency_ids` 和 `summary_keys`，确定当前 `target` 需要引用的外部摘要。内部使用 `maybe_resolve_snippet` 来按需解析代码片段，并根据 `namespace_fact`、`member_ids`、`local_ids` 和 `symbol_ids` 等标识符集合，在 `analyses` 上下文中汇总结构信息。最终函数将所有这些信息聚合并返回一个整数（可能代表状态或计数）。其依赖主要集中于代码模型解析基础设施（如 `model`、`pack`、`root`）、文件迭代器 `file_it` 以及 `mod` 模块对象，并通过 `ns` 控制命名空间作用域。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Usage Patterns

- Likely called during documentation generation for type implementation summaries
- Could be invoked by functions like `build_evidence_for_type_analysis` or similar evidence builders

### `clore::generate::build_evidence_for_variable_analysis`

Declaration: `src/generate/evidence.cppm:60`

Definition: `src/generate/evidence_builder.cppm:121`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_evidence_for_variable_analysis` 首先根据 `target` 和 `project_root` 初始化一个本地 `model`，并从 `page_summaries` 和 `imported_summaries` 中提取相关数据。内部通过遍历 `pack`（可能是符号包或分析包）中的元素，依次处理 `analyses`、`member_ids` 和 `symbol_ids` 等集合。对于每个分析条目，会调用匿名命名空间中的 `maybe_resolve_snippet` 来解析片段依赖，并根据 `namespace_fact` 和本地 ID 映射关系填充 `model` 的关键成员。

控制流主要围绕 `project_root` 和 `root` 展开，将 `target` 与 `dependency_ids`、`summary_keys` 等元数据关联起来，最终通过写入 `model` 中的 `analyses`、`local_ids` 和 `member_ids` 等容器，构造出变量分析所需的证据结构。该过程依赖 `file_it` 遍历文件级别的分析结果，并借助 `mod` 和 `ns` 调整命名空间上下文，确保生成的数据与 `page_summaries` 中的页面摘要一致。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 两个 `const int &` 参数（可能分别代表符号分析存储引用和符号标识符）
- 一个 `std::string_view` 参数（可能代表变量名称或模块名称）

#### Usage Patterns

- 用于构建变量分析的证据，作为文档生成流水线的一部分
- 通常由更高层的生成函数调用，例如 `build_page_plan_set` 或 `generate_pages`

### `clore::generate::build_prompt`

Declaration: `src/generate/evidence.cppm:106`

Definition: `src/generate/evidence.cppm:663`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_prompt` 首先通过 `prompt_template_of` 根据传入的 `PromptKind` 获取对应的模板字符串。若模板为空，则构造一个带有错误信息的 `PromptError` 并返回 `std::unexpected`；否则，调用 `format_evidence_text` 将 `EvidencePack` 格式化为证据文本，然后委托给 `instantiate_prompt_with_evidence`，将模板字符串、证据包和证据文本三者结合，得到最终的提示字符串并返回。

控制流简单直接：唯一的分支是模板是否存在。整个函数的正确性依赖于 `prompt_template_of` 返回有效的模板（当 `PromptKind` 受支持时）、`format_evidence_text` 生成结构化的证据文本，以及 `instantiate_prompt_with_evidence` 正确执行模板插值。这些依赖全部位于 `clore::generate` 的匿名命名空间或公开 API 中。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `kind`
- parameter `evidence`
- internal prompt templates accessed via `prompt_template_of`
- result of `format_evidence_text(evidence)`

#### Writes To

- returned `std::expected<std::string, PromptError>` value

#### Usage Patterns

- Called by higher-level prompt builders such as `build_symbol_analysis_prompt` and `build_page_summary_prompt` to generate LLM input strings
- Used in the prompt caching and generation pipeline within `clore::generate`

### `clore::generate::format_evidence_text`

Declaration: `src/generate/evidence.cppm:98`

Definition: `src/generate/evidence.cppm:592`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/format-evidence-text.md)

函数 `clore::generate::format_evidence_text` 的实现是一个轻量级转发器。它直接调用 `clore::generate::format_evidence_text_bounded`，并将参数 `pack` 与 `std::numeric_limits<std::size_t>::max()` 一同传递。该有界函数（实际承担所有工作）随后会使用最大可能的长度限制构建证据文本，从而在效果上不施加任何长度截断。这种设计将格式化逻辑集中到有界变体，同时通过无界包装器提供便利入口，避免了重复实现。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `pack` parameter of type `const EvidencePack&`

#### Writes To

- returned `std::string`

#### Usage Patterns

- Used by `build_prompt` to format evidence for prompts

### `clore::generate::format_evidence_text_bounded`

Declaration: `src/generate/evidence.cppm:100`

Definition: `src/generate/evidence.cppm:596`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/format-evidence-text-bounded.md)

函数 `clore::generate::format_evidence_text_bounded` 的核心算法是将证据数据按语义段组织并逐段累积文本，同时严格遵循长度限制。它首先声明两个内部结构 `SymbolSection` 和 `TextSection` 用于描述每个段的结构（标题、项目容器指针和渲染函数指针）。函数初始化一个预留 4096 字节的 `std::string text`，然后通过 `append_if_fits` 尝试追加 `## EVIDENCE\n\n` 标题；若失败（标题本身已超出 `max_length`）则直接返回空字符串。成功后再依次处理四个符号段（`"### Target\n"`、`"### Local Context\n"`、`"### Dependencies\n"`、`"### Used By\n"`）和两个文本段（`"### Source Snippets\n"`、`"### Related Page Summaries\n"`）。每个段通过 `append_section_bounded` 进行追加，该函数会在内部迭代每个项目，使用对应的渲染函数（如 `render_detailed_fact`、`render_context_fact`、`render_source_snippet`、`render_summary_item`）将 `SymbolFact` 或 `std::string` 转换为文本块，并在每次追加前检查剩余可用空间，确保总长度不超过 `max_length`。

整个控制流是线性的顺序追加，没有任何分支判断段的存在性——所有段都无条件尝试追加，但 `append_section_bounded` 可能会因为长度限制而跳过部分或全部项目。关键依赖是 `append_if_fits` 和 `append_section_bounded`（两者均在匿名命名空间中），它们共同实现了长度约束下的安全拼接；渲染函数则作为回调由调用方提供（已在匿名命名空间中定义）。内部结构 `SymbolSection` 和 `TextSection` 仅仅是简单的数据聚合器，没有额外逻辑。该函数本身不涉及任何外部输入的状态突变，所有数据均来自传入的 `EvidencePack` 常量引用，长度边界由 `max_length` 参数硬性限定。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `pack.target_facts`
- `pack.local_context`
- `pack.dependency_context`
- `pack.reverse_usage_context`
- `pack.source_snippets`
- `pack.related_page_summaries`
- `max_length`
- `pack` (the `EvidencePack` parameter)

#### Usage Patterns

- Called by `clore::generate::format_evidence_text`

## Internal Structure

模块 `generate:evidence` 是整个文档生成管线中负责为特定代码实体（符号、命名空间、模块等）采集并格式化上下文证据的核心组件。它导入 `extract` 模块的符号提取与缓存能力，并依赖 `generate:model` 提供的页面计划与分析表示来驱动证据构造流程。

内部结构分为三层：最外层是公开的 `build_evidence_for_*` 系列函数，它们接收来自 `generate:model` 的上下文句柄，协调事实收集与格式化，并返回结构化 `EvidencePack` 或字符串；中间层是 `__detail` 命名空间，包含可复用的事实收集（`collect_facts`、`collect_merged_facts`、`collect_analysis_summaries` 等）和命名空间级缓存机制（`collect_namespace_facts_cached`），以避免重复遍历同一命名空间下的多个符号；最内层是匿名命名空间，封装了提示模板字面量、证据渲染函数（`render_context_fact`、`render_detailed_fact`）、边界感知的截断追加（`append_if_fits`、`append_section_bounded`）以及提示实例化逻辑（`instantiate_prompt_with_evidence`）。这种分层确保了高内聚低耦合：公开接口专注于编排，下层函数只负责各自具体的数据变换与文本格式化。

## Related Pages

- [Module extract](../extract/index.md)
- [Module generate:model](model.md)

