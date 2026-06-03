---
title: 'Module generate:common'
description: 'generate:common 模块位于渲染子系统的公用层，负责提供一组面向文档生成管线的通用渲染工具。它封装了构建 Markdown 结构所需的公共函数与数据类型，包括符号分析摘要的生成、链接目标的构建与解析、文档章节的排版（如列表、提示、符号位置）以及与符号文档视图相关的枚举与转换。该模块并不直接生成完整页面，而是作为“工具箱”，被其他渲染模块（如 generate:markdown）或更高层生成器所调用，从而保持渲染逻辑的可复用性和一致性。'
layout: doc
template: doc
---

# Module `generate:common`

## Summary

`generate:common` 模块位于渲染子系统的公用层，负责提供一组面向文档生成管线的通用渲染工具。它封装了构建 Markdown 结构所需的公共函数与数据类型，包括符号分析摘要的生成、链接目标的构建与解析、文档章节的排版（如列表、提示、符号位置）以及与符号文档视图相关的枚举与转换。该模块并不直接生成完整页面，而是作为“工具箱”，被其他渲染模块（如 `generate:markdown`）或更高层生成器所调用，从而保持渲染逻辑的可复用性和一致性。

公开接口覆盖了从源代码位置到最终 Markdown 段落之间的常见转换路径：例如从符号分析数据提取摘要（`symbol_analysis_markdown_for`）、计算文件间相对链接（`make_relative_link_target`）、插入可选的链接段落（`push_optional_link_paragraph`）等。模块依赖 `config`、`extract`、`generate:markdown` 与 `generate:model`，说明它基于项目配置、代码提取结果和文档模型来产出标准化的 Markdown 片段，是生成管线中连接分析数据与最终文本表示的关键中间层。

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:markdown`](markdown.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:page`](page.md)
- [`generate:symbol`](symbol.md)

## Types

### `clore::generate::LinkTarget`

Declaration: `src/generate/render/common.cppm:22`

Definition: `src/generate/render/common.cppm:22`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

结构体 `clore::generate::LinkTarget` 是一个聚合类型，包含三个公开成员：两个 `std::string` 字段 `label` 与 `target`，以及一个默认值为 `false` 的 `bool` 字段 `code_style`。除各成员自身的类型约束外，不强制维护任何不变量；该结构体仅作为纯数据容器使用。通过 `code_style` 的默认初始化，确保了在未显式赋值时，代码样式渲染处于禁用状态。

#### Invariants

- `label` and `target` are valid `std::string` objects
- `code_style` is a boolean, defaulting to `false`

#### Key Members

- `clore::generate::LinkTarget::label`
- `clore::generate::LinkTarget::target`
- `clore::generate::LinkTarget::code_style`

#### Usage Patterns

- Used as a data container for link information in code generation
- Likely constructed and passed during rendering of links

### `clore::generate::SymbolDocView`

Declaration: `src/generate/render/common.cppm:28`

Definition: `src/generate/render/common.cppm:28`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::SymbolDocView` 是一个以 `std::uint8_t` 为底层类型的枚举，其三个成员 `Declaration`、`Implementation` 和 `Details` 按声明顺序隐式赋值为 0、1、2。该枚举作为简单选择器，控制文档生成过程中渲染的符号视图部分，每个值唯一对应一个视图子部分，无重叠或组合语义。枚举成员数量固定，无需动态扩展，在 `render` 模块内部作为轻量级标签使用。

#### Invariants

- 每个枚举成员代表一个互斥的视图选项
- 枚举值范围仅限于声明的三个成员

#### Key Members

- `clore::generate::SymbolDocView::Declaration`
- `clore::generate::SymbolDocView::Implementation`
- `clore::generate::SymbolDocView::Details`

#### Member Variables

##### `clore::generate::SymbolDocView::Declaration`

Declaration: `src/generate/render/common.cppm:29`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Declaration
```

##### `clore::generate::SymbolDocView::Details`

Declaration: `src/generate/render/common.cppm:31`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Details
```

##### `clore::generate::SymbolDocView::Implementation`

Declaration: `src/generate/render/common.cppm:30`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Implementation
```

## Functions

### `clore::generate::add_prompt_output`

Declaration: `src/generate/render/common.cppm:153`

Definition: `src/generate/render/common.cppm:153`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::add_prompt_output` 的核心流程是一个简单的预检‑追加操作。它首先检查传入的 `output` 指针是否为 `nullptr`，若不是，则调用 `clore::generate::trim_ascii` 去除字符串的前后空白，并判定结果是否为空。仅当字符串在修剪后非空时，才通过 `make_raw_markdown` 创建一个原始 Markdown 节点并追加到 `nodes` 容器末尾。整个过程没有任何迭代或条件分支之外的逻辑，唯一的外部依赖是 `clore::generate::trim_ascii`，用于确保输出内容不包含仅由空白构成的无效文本。

#### Side Effects

- Modifies the `nodes` parameter by appending a new `MarkdownNode` via `push_back`

#### Reads From

- The `output` pointer parameter
- The string pointed to by `output` (if non‑null)
- Result of `clore::generate::trim_ascii(*output)`

#### Writes To

- The `nodes` vector (passed by reference)

#### Usage Patterns

- Called during construction of markdown content for prompt sections
- Used to conditionally include raw markdown output in a list of nodes

### `clore::generate::add_symbol_analysis_detail_sections`

Declaration: `src/generate/render/common.cppm:181`

Definition: `src/generate/render/common.cppm:196`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::add_symbol_analysis_detail_sections` 根据符号类型在解析后的分析数据上插入细节部分到 `nodes` 容器。算法首先通过 `make_symbol_target_key` 从 `sym` 生成目标键，然后依次尝试查找函数分析、类型分析、变量分析。对于每个成功的查找，它使用本地 lambda `make_list_node`（调用 `build_string_list` 和 `build_list_section`）将列表部分（如副作用、读写、使用模式、不变量、关键成员、变异源）添加到 `nodes`。控制流额外处理 `plan.page_type` 不是 `PageType::Namespace` 的情况：对函数分析添加显式的“无副作用”段落（当 `analysis->has_side_effects` 为假时）；对变量分析添加“无突变”段落（当 `analysis->is_mutated` 为假时）。依赖 `find_function_analysis`、`find_type_analysis`、`find_variable_analysis` 以及 `make_section`、`make_paragraph` 等工具。

#### Side Effects

- Mutates the `nodes` vector by appending new `MarkdownNode` objects.

#### Reads From

- `analyses` (a `const SymbolAnalysisStore&`)
- `plan` (a `const PagePlan&`, specifically `plan.page_type`)
- `sym` (a `const extract::SymbolInfo&`, used to compute `target_key`)
- Fields of the found analysis objects: `has_side_effects`, `side_effects`, `reads_from`, `writes_to`, `usage_patterns`, `invariants`, `key_members`, `is_mutated`, `mutation_sources`

#### Writes To

- `nodes` (a `std::vector<MarkdownNode>&`) — appends `MarkdownNode` objects representing sections and paragraphs.

#### Usage Patterns

- Called during page rendering to populate analysis detail sections for a symbol.
- Used in conjunction with `add_symbol_analysis_sections` or similar page builders.
- Expected to be invoked after overall section structure is established.

### `clore::generate::add_symbol_analysis_sections`

Declaration: `src/generate/render/common.cppm:187`

Definition: `src/generate/render/common.cppm:187`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::add_symbol_analysis_sections` 将分析数据组织成两个主要阶段。首先，它通过调用 `add_prompt_output` 将 `symbol_analysis_markdown_for` 返回的字符串添加到 `nodes` 容器中，从而注入由大模型生成的初步分析文本。随后，它委托给 `add_symbol_analysis_detail_sections`，由该函数根据 `analyses`、`plan` 和 `sym` 生成更精细的符号分析子节（如声明、实现、细节等），并递归地调用自身以构建完整的章节层次。两个步骤均依赖 `level` 参数控制标题层级，并直接操作传入的 `nodes` 列表以实现按序输出。

#### Side Effects

- Appends markdown nodes to the provided `std::vector<MarkdownNode>` via `add_prompt_output` and `add_symbol_analysis_detail_sections`.

#### Reads From

- `analyses` (`const SymbolAnalysisStore&`)
- `plan` (`const PagePlan&`)
- `sym` (`const extract::SymbolInfo&`)
- `level` (`std::uint8_t`)

#### Writes To

- `nodes` (`std::vector<MarkdownNode>&`) - appended with new `MarkdownNode` elements

#### Usage Patterns

- Called during page generation to insert symbol analysis sections into the document tree.

### `clore::generate::build_list_section`

Declaration: `src/generate/render/common.cppm:144`

Definition: `src/generate/render/common.cppm:144`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数创建一个 `SemanticSectionPtr` 对象，通过调用 `make_section` 并传入 `SemanticKind::Section`、一个空选项、从参数移入的 `heading` 以及 `level` 来初始化。随后检查传入的 `BulletList list` 的 `items` 成员是否非空；若非空，则将包含该列表的 `MarkdownNode` 追加到 `section` 的子节点容器中。函数最终返回构造好的 `section`。整个流程没有其他条件分支或循环，依赖于 `make_section` 创建节实例以及 `MarkdownNode` 对 `BulletList` 的封装能力。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `heading` parameter
- `level` parameter
- `list` parameter

#### Usage Patterns

- Creates a documentation section containing a bullet list
- Used by page generation functions to add list-based content

### `clore::generate::build_prompt_section`

Declaration: `src/generate/render/common.cppm:135`

Definition: `src/generate/render/common.cppm:135`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_prompt_section` 首先调用内部函数 `make_section`，传入参数 `heading` 和 `level` 以创建一个 `SemanticSectionPtr` 对象 `section`。随后检查参数 `output` 是否非空，且通过 `clore::generate::trim_ascii` 修剪空白后其内容不为空。若条件满足，则创建一个原始 Markdown 节点（通过 `make_raw_markdown`）并压入 `section` 的子节点列表 `children`。最终返回构造完成的 `section` 对象。该函数的控制流极为简洁，仅包含一个可选的分支，无循环或嵌套逻辑；其核心依赖为 `clore::generate::trim_ascii` 用于判定输出内容是否有效，以及未在摘要中显式列出的 `make_section` 与 `make_raw_markdown` 两个内部构造函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `heading` (by move)
- parameter `level`
- parameter `output` (dereferenced if non-null)
- content of string pointed to by `output`
- return value of `clore::generate::trim_ascii`

#### Usage Patterns

- Constructing a prompt section with a heading and optional output content for documentation generation.
- Used in building structured sections within prompt assembly for analysis or summary generation.

### `clore::generate::build_related_page_targets`

Declaration: `src/generate/render/common.cppm:515`

Definition: `src/generate/render/common.cppm:515`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_related_page_targets` 从 `plan.linked_pages` 中逐个解析条目，生成对应的 `LinkTarget` 集合。对于每个链接条目，先检查是否有形如 `"module:"` 或 `"namespace:"` 的前缀：若存在，则分别调用 `links.resolve_module` 或 `links.resolve_namespace` 获取目标路径；否则或当前缀解析失败时，回退到 `links.resolve`。若得到的路径非空且未被 `seen` 集合记录，则通过 `links.resolve_page_title` 获取页面标题，经过 `strip_inline_markdown` 清洗后作为标签，否则直接使用实体名称。最终调用 `make_link_target` 构造 `LinkTarget` 并加入结果向量。

该函数内部维护 `std::unordered_set<std::string> seen` 以保证同一路径只输出一次；控制流完全围绕对 `linked_pages` 的线性遍历、前缀判断与多重解析降级。依赖 `PagePlan` 的 `linked_pages` 字段、`LinkResolver` 的多个解析方法与 `resolve_page_title`，以及辅助函数 `strip_inline_markdown` 和 `make_link_target`。返回的 `LinkTarget` 向量被上层用于生成页面间的关联导航。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.linked_pages`
- `links` of type `LinkResolver`
- `current_page_path` parameter

#### Usage Patterns

- Called during page generation to gather cross-reference links
- Used to build a list of related pages for navigation or inclusion in rendered output

### `clore::generate::build_string_list`

Declaration: `src/generate/render/common.cppm:159`

Definition: `src/generate/render/common.cppm:159`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数遍历输入的 `std::vector<std::string>`，对每个元素首先调用 `trim_ascii` 去除首尾空白。若结果为空，则跳过该元素；否则，调用 `code_spanned_fragments` 将文本解析为样式化片段，并填充到一个 `ListItem` 结构的 `fragments` 字段中。最终将所有非空 `ListItem` 按顺序存入返回的 `BulletList` 容器。

控制流仅包含一次循环与空白过滤判断，无递归或分支。其依赖包括 `trim_ascii`（字符串清理）、`code_spanned_fragments`（文本到片段转换），以及 `BulletList` / `ListItem`（输出模型类型）。未涉及外部状态或文件系统操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `std::vector<std::string>`& items parameter
- calls `trim_ascii` on each `item_text`
- calls `code_spanned_fragments` on each `item_text`

#### Usage Patterns

- Used within the generation pipeline to construct bullet lists from collections of strings for documentation output

### `clore::generate::build_symbol_link_list`

Declaration: `src/generate/render/common.cppm:371`

Definition: `src/generate/render/common.cppm:371`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The function iterates over each symbol in the supplied `symbols` vector. For each entry, it determines the kind name via `extract::symbol_kind_name`, then builds a label using either the full `qualified_name` or only the short form (when `use_full_name` is `false`). It queries the `links` resolver for a matching target path; if one exists, it produces a clickable hyperlink with a relative path computed from `make_relative_link_target`; otherwise, it falls back to plain code-format text. Each resulting fragment is wrapped in a `ListItem` and appended to the output `BulletList`.

Internally, the function relies on three primary external utilities: symbol-name helpers (`short_name_of` from the same generation module), link resolution (`LinkResolver::resolve`), and markup construction (`make_text`, `make_link`, `make_code`). The control flow is a straightforward linear loop with a conditional branch per symbol — no nested iteration or recursion. The only dependency beyond the standard library is on the `extract` and `clore::generate` namespaces for symbol metadata and presentation building.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `symbols` vector of `extract::SymbolInfo*`
- `current_page_path` `string_view`
- `links` resolver
- `use_full_name` flag
- `extract::symbol_kind_name` function
- `short_name_of` function

#### Writes To

- local `BulletList list` and its `items`

#### Usage Patterns

- building navigation links for symbol lists in documentation pages
- creating link lists for symbol overview sections
- generating page indexes with links to symbols

### `clore::generate::build_symbol_source_locations`

Declaration: `src/generate/render/common.cppm:423`

Definition: `src/generate/render/common.cppm:423`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_symbol_source_locations` 的实现围绕两个顺序的条件检查构建。它首先创建一个空的 `nodes` 向量。如果 `sym.declaration_location.is_known()` 返回 `true`，则调用 `push_location_paragraph` 向 `nodes` 追加一个标记为 `"Declaration: "` 的段落节点，该节点的链接通过 `make_source_link_target` 从 `sym.declaration_location`、`config`、`links` 和 `current_page_path` 构造。随后，如果 `sym.definition_location` 持有值且该值的 `is_known()` 方法返回 `true`，则向 `nodes` 追加第二个段落节点，标记为 `"Definition: "`，链接同样通过 `make_source_link_target` 构造。最后返回填充完毕的 `nodes`。

该函数依赖 `push_location_paragraph` 和 `make_source_link_target` 两个辅助函数（均声明于同一模块），以及 `extract::SymbolInfo` 的 `declaration_location` 与 `definition_location` 字段。其控制流为最简单的顺序分支结构，不含循环或递归调用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.declaration_location`
- `sym.definition_location`
- `config`
- `links`
- `current_page_path`

#### Usage Patterns

- Building source location sections in symbol documentation pages
- Called during page generation to add declaration and definition links

### `clore::generate::collect_implementation_symbols`

Declaration: `src/generate/render/common.cppm:325`

Definition: `src/generate/render/common.cppm:325`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过遍历 `plan.owner_keys` 收集符合条件的符号。对于每个键，若页面类型为 `PageType::Module`，则通过 `extract::find_modules_by_name` 获取模块并迭代其符号；否则从 `model.files` 中查找文件并获取其符号。使用 `std::unordered_set<extract::SymbolID>` 实现去重，并通过 `extract::lookup_symbol` 解析符号信息，然后由 `is_page_level_symbol` 和用户提供的 `predicate` 联合过滤。最终按符号的 `qualified_name` 排序后返回结果向量。该算法依赖于 `extract` 命名空间下的模型查询函数以及页面类型枚举 `PageType`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `plan.page_type`
- `model.files`
- `model` 中的模块（通过 `extract::find_modules_by_name`）
- `model` 中的符号（通过 `extract::lookup_symbol`）
- 谓词 `predicate`

#### Usage Patterns

- 用于收集实现页面所需的符号，可能被 `clore::generate::build_evidence_for_function_implementation_summary` 等函数调用
- 与 `clore::generate::collect_namespace_symbols` 类似，但专门针对实现符号

### `clore::generate::collect_namespace_symbols`

Declaration: `src/generate/render/common.cppm:300`

Definition: `src/generate/render/common.cppm:300`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该实现首先在 `model.namespaces` 容器中通过构造的 `std::string` 查找 `namespace_name` 对应的条目。若未找到，立即返回空向量。随后遍历该条目中的 `symbols` 集合，对每个 `sym_id` 调用 `extract::lookup_symbol` 获取符号指针，并依次检查指针非空、满足 `is_page_level_symbol` 条件以及给定的 `predicate` 谓词；通过全部检查的符号指针被追加到局部向量 `symbols` 中。

收集完成后，使用 `std::sort` 配合基于 `qualified_name` 的字典序比较对 `symbols` 进行稳定排序，最终返回排序后的结果。整个过程依赖于 `extract::ProjectModel` 的命名空间索引结构、`extract::lookup_symbol` 的符号解析、`is_page_level_symbol` 的页面级别判定，以及外部的模板化谓词 `Predicate`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- 参数 `model` 的 `model.namespaces` 映射
- 参数 `namespace_name`
- 通过 `extract::lookup_symbol` 读取的 `model` 全局数据
- 每个符号的 `qualified_name` 字段（用于排序）
- 通过 `predicate` 谓词读取符号属性

#### Usage Patterns

- 用于构建命名空间页面的符号列表
- 被 `build_namespace_page_root` 等页面构建函数调用
- 典型地配合 `is_page_level_symbol` 和自定义谓词过滤符号

### `clore::generate::doc_label`

Declaration: `src/generate/render/common.cppm:290`

Definition: `src/generate/render/common.cppm:290`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::doc_label` 实现从 `SymbolDocView` 枚举到字符串字面量的直接映射。函数体为一个 switch 语句，根据枚举值 `SymbolDocView::Declaration`、`SymbolDocView::Implementation` 和 `SymbolDocView::Details` 分别返回 `"Declaration"`、`"Implementation"` 和 `"Details"`；对于未覆盖的枚举值，默认返回 `"Details"`。该实现无其他外部依赖，仅依赖其参数类型 `SymbolDocView` 的枚举成员定义，属于纯常量转换，无分支之外的副作用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `SymbolDocView view` parameter

#### Usage Patterns

- used to produce section labels for symbol documentation views

### `clore::generate::find_declaration_page`

Declaration: `src/generate/render/common.cppm:484`

Definition: `src/generate/render/common.cppm:484`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::find_declaration_page` 实现了一个两阶段回退算法，用于解析给定符号的声明页面 `LinkTarget`。它首先通过 `LinkResolver::resolve` 尝试直接查找符号的完全限定名（`sym.qualified_name`）对应的目标页面路径。如果返回的非空路径与当前页面路径（`current_page_path`）不同，则利用 `namespace_of` 或符号的 `enclosing_namespace` 提取命名空间名称，构造形如 `"Namespace <ns_name>"` 或 `"Declaration"` 的标签，并调用 `make_link_target` 创建一个指向该路径的链接目标。若直接解析失败或返回当前页面，则进入第二阶段：重新获取命名空间名称（若为空则返回 `std::nullopt`），并使用 `LinkResolver::resolve_namespace` 查找命名空间页面。若存在，同样构造链接目标返回，否则返回 `std::nullopt`。该函数依赖 `extract::SymbolInfo`、`LinkResolver`、`make_link_target` 以及 `namespace_of`，其内部逻辑确保了优先链接到具体声明页面，并在不可行时优雅回退到命名空间页面。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- sym`.qualified_name`
- sym`.enclosing_namespace`
- `current_page_path`
- `LinkResolver::resolve`
- `LinkResolver::resolve_namespace`

#### Usage Patterns

- Used to generate a link to a declaration page or namespace page for a symbol during page rendering.
- Called by page-building functions such as `render_page_markdown` when constructing symbol references.

### `clore::generate::find_implementation_pages`

Declaration: `src/generate/render/common.cppm:444`

Definition: `src/generate/render/common.cppm:444`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数收集给定符号的实现页面链接，避免重复。算法首先检查符号的 `definition_location`（若存在），然后总是尝试添加 `declaration_location` 对应的文件路径。对于每个文件路径，先通过 `extract::find_module_by_source` 在项目模型 `model` 中查找所属模块；若找到则使用 `links.resolve_module` 解析模块页面路径，否则使用 `links.resolve` 直接解析文件路径。每个有效目标路径通过 `make_link_target` 构造为 `LinkTarget` 对象，并用 `seen` 集合去重后添加到结果向量中。依赖包括 `extract::find_module_by_source`、 `LinkResolver` 的解析方法、 `make_link_target` 以及路径相对化辅助函数 `make_source_relative`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.definition_location.file`
- `sym.declaration_location.file`
- `model` (via `extract::find_module_by_source`)
- `links` (via `links.resolve_module` and `links.resolve`)
- `current_page_path`
- `project_root`
- `mod->name` (field of the found module)

#### Usage Patterns

- called during documentation generation to collect implementation page links for a symbol
- used to populate links to module or source file pages in symbol documentation

### `clore::generate::find_module_for_file`

Declaration: `src/generate/render/common.cppm:507`

Definition: `src/generate/render/common.cppm:507`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过委托给 `extract::find_module_by_source` 来定位与给定 `file_path` 关联的模块。它首先将 `model` 和 `file_path` 转发给 `extract::find_module_by_source`，如果返回非空指针，则提取其 `name` 字段并返回包装在 `std::optional` 中的结果；否则返回 `std::nullopt`。

内部控制流是线性的：一次函数调用加一个条件分支。关键依赖是 `extract::find_module_by_source`，它负责根据源文件路径在项目模型中查找对应的模块定义。该函数本身不执行任何解析或搜索逻辑，完全依赖该外部查询操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `const extract::ProjectModel&` parameter `model`
- `std::string_view` parameter `file_path`
- `mod->name` if a module is found

#### Usage Patterns

- Look up the module name for a source file path

### `clore::generate::make_link_target`

Declaration: `src/generate/render/common.cppm:92`

Definition: `src/generate/render/common.cppm:92`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数`clore::generate::make_link_target`通过直接赋值字段构造一个`LinkTarget`对象。它将`label`参数移入`LinkTarget::label`，调用`clore::generate::make_relative_link_target`从`current_page_path`和`target_page_path`计算出相对路径并存入`LinkTarget::target`，最后将`code_style`参数赋值给`LinkTarget::code_style`。整个过程不涉及分支或循环，依赖于`make_relative_link_target`完成路径转换。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- Parameter `current_page_path`
- Parameter `label`
- Parameter `target_page_path`
- Parameter `code_style`

#### Writes To

- Returned `LinkTarget` object's `label`, `target`, and `code_style` fields

#### Usage Patterns

- Constructs link targets for cross‑references during documentation page generation
- Called by higher‑level rendering functions to prepare relative links between pages

### `clore::generate::make_relative_link_target`

Declaration: `src/generate/render/common.cppm:68`

Definition: `src/generate/render/common.cppm:68`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/make-relative-link-target.md)

该函数使用 `std::filesystem` 库计算两个规范化路径之间的相对链接目标。首先将 `current_page_path` 和 `target_page_path` 通过 `lexically_normal` 标准化，消除冗余的 `.` 和 `..` 成分。以当前路径的父目录（若无父目录则使用 `"."`）作为基础，调用 `target.lexically_relative(base)` 生成相对路径。若结果为空（表明目标路径与基础相同或无法相对表达），则直接返回目标路径的通用字符串形式；否则返回该相对路径的通用字符串形式（均使用正斜杠分隔），确保结果符合 URI 或文件系统链接规范。整个逻辑仅依赖标准文件系统设施，无外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path`
- `target_page_path`

#### Usage Patterns

- Called by `clore::generate::make_link_target` to compute a relative path for a link target.

### `clore::generate::make_source_link_target`

Declaration: `src/generate/render/common.cppm:394`

Definition: `src/generate/render/common.cppm:394`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该实现首先通过`make_source_relative`将`location.file`转换为相对于`config.project_root`的路径，并与行号拼接形成一个可读的标签。随后它查询`links.resolve(location.file)`试图从链接解析器`LinkResolver`中获取对应的文档目标路径；如果找到，就委托给`make_link_target`生成一个完整的`LinkTarget`，其中包含标签、目标路径以及硬编码的代码样式标记。若解析器未返回有效目标，则直接构造一个`LinkTarget`，其`target`为空、`code_style`为`true`，仅保留纯文本标签以表示无法链接的源位置。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `location.file`
- `location.line`
- `config.project_root`
- `links`
- `current_page_path`

#### Usage Patterns

- Creating link targets for source locations during page generation
- Building inline code references to source files with optional page links
- Used in `push_location_paragraph` to add source location links

### `clore::generate::namespace_of`

Declaration: `src/generate/render/common.cppm:64`

Definition: `src/generate/render/common.cppm:64`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::namespace_of` 是 `extract::namespace_prefix_from_qualified_name` 的一个简单转发函数。它接受一个合格名称（`std::string_view`），直接委托给底层抽取例程，并返回结果字符串。该函数本身不包含任何控制流、分支或错误处理；其全部逻辑来自 `extract::namespace_prefix_from_qualified_name`，后者在上下文中是一个既定的工具函数，负责从合格名称中提取命名空间前缀部分。因此，该函数仅作为将命名空间抽取操作暴露给生成层的一个轻量适配器。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter of type `std::string_view`

#### Usage Patterns

- used to obtain the namespace prefix for code generation or symbol analysis

### `clore::generate::prompt_output_of`

Declaration: `src/generate/render/common.cppm:82`

Definition: `src/generate/render/common.cppm:82`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::prompt_output_of` 利用由 `prompt_request_key` 返回的键从传入的 `outputs` 无序映射中检索之前存储的生成输出。它基于给定的 `PromptKind` 和可选的 `target_key` 构造一个 `PromptRequest` 对象，将其转换为字符串键，然后在映射中执行一次 `find` 操作。如果找到，函数返回指向对应值的常量指针；否则返回 `nullptr`。该实现不进行任何额外的错误处理或解析，完全依赖哈希映射的查找语义，因此其效率取决于 `std::unordered_map` 的均摊常数时间查找。唯一的外部依赖是通过 `prompt_request_key` 建立的键生成逻辑，该逻辑定义了如何将组合的请求参数序列化为映射键。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `outputs` parameter
- `kind` parameter
- `target_key` parameter

#### Usage Patterns

- Retrieve a specific prompt output by kind and target key
- Access cached prompt results in a map

### `clore::generate::push_link_paragraph`

Declaration: `src/generate/render/common.cppm:103`

Definition: `src/generate/render/common.cppm:103`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先检查 `targets` 是否为空；若为空则直接返回，避免添加空段落。随后构造一个 `Paragraph` 对象，将 `label` 作为纯文本片段加入。接着遍历 `targets` 中的每个 `LinkTarget`，在非首项之前插入一个分隔字符串 `" | "`，然后调用 `make_link` 为当前目标生成带有其 `label`、`target` 和 `code_style` 的链接片段。最终将填充完毕的 `Paragraph` 包装为 `MarkdownNode` 并追加到 `nodes` 向量末尾。整个过程依赖 `make_text` 和 `make_link` 两个辅助函数完成文本与链接的构造。

#### Side Effects

- modifies the `nodes` vector by appending a new `MarkdownNode`

#### Reads From

- the `label` string parameter
- the `targets` vector parameter and its elements' `label`, `target`, and `code_style` fields

#### Writes To

- the `nodes` vector parameter (appending a `MarkdownNode`)

#### Usage Patterns

- building link paragraphs for navigation or cross-references
- adding a 'See also' section to a Markdown document

### `clore::generate::push_location_paragraph`

Declaration: `src/generate/render/common.cppm:410`

Definition: `src/generate/render/common.cppm:410`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数构造一个段落，结构为：先插入由 `label` 参数生成的文本片段，然后根据 `target.target` 是否为空决定后续内容。若 `target.target` 为空，则通过 `make_code` 添加一个内联代码片段（使用 `target.label` 作为显示文本）；否则通过 `make_link` 添加一个超链接，其显示文本为 `target.label`，目标地址为 `target.target`，并应用 `target.code_style` 样式。最终将整个段落追加到 `nodes` 容器中。此过程依赖 `make_text`、`make_code`、`make_link` 等自由函数以及 `LinkTarget` 结构体的设计。

#### Side Effects

- Modifies the `nodes` vector by appending a new `MarkdownNode`.
- Allocates memory for a `Paragraph` and inline fragments (text, code, or link).

#### Reads From

- `label` parameter
- `target` parameter (members `target.target`, `target.label`, `target.code_style`)

#### Writes To

- `nodes` vector (appends a `MarkdownNode`)
- Allocated `Paragraph` and inline fragments stored in `nodes`

#### Usage Patterns

- Used to render location references for symbols, files, or modules.
- Called during page building to insert a location paragraph into a list of markdown nodes.
- Companion to `push_link_paragraph` and `push_optional_link_paragraph`.

### `clore::generate::push_optional_link_paragraph`

Declaration: `src/generate/render/common.cppm:122`

Definition: `src/generate/render/common.cppm:122`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先检查 `target` 是否包含有效值；若为空则直接返回，不产生任何输出。当 `target` 存在时，它构造一个 `Paragraph` 对象，依次向其中添加两个片段：一个由 `label` 生成的纯文本片段，以及一个由 `target` 的 `label`、`target` 和 `code_style` 字段构造的链接片段。最后将完整的段落追加到 `nodes` 容器中。

实现依赖 `make_text` 和 `make_link` 两个辅助函数来创建片段，并依赖 `MarkdownNode` 和 `Paragraph` 的数据结构。整个流程无循环或复杂分支，仅通过一个 `if` 守卫决定是否生成内容，保证了在可选链接缺失时的静默跳过。

#### Side Effects

- Modifies the `nodes` vector by appending a new `MarkdownNode` representing the paragraph.

#### Reads From

- `label` parameter
- `target` optional parameter (fields `label`, `target`, `code_style`)

#### Writes To

- `nodes` output parameter (appends a `MarkdownNode`)

#### Usage Patterns

- Used to conditionally add a link paragraph to a markdown node list.
- Similar to `push_location_paragraph` but for optional links.

### `clore::generate::short_name_of`

Declaration: `src/generate/render/common.cppm:56`

Definition: `src/generate/render/common.cppm:56`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过调用 `extract::split_top_level_qualified_name` 将输入的 `qualified_name` 分割成顶层限定名列表。如果结果列表为空，则函数返回一个空字符串；否则返回列表中的最后一个元素。此实现直接提取出完全限定名中最短的尾段名称，不涉及进一步的分析或格式化。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter

#### Usage Patterns

- Obtaining the unqualified name from a fully qualified name
- Used to extract the short name for display or linking purposes

### `clore::generate::strip_inline_markdown`

Declaration: `src/generate/render/common.cppm:44`

Definition: `src/generate/render/common.cppm:44`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::strip_inline_markdown` 通过对输入 `text` 中的每个字符 `ch` 进行线性扫描来实现。若 `ch` 为反引号、星号、下划线、左方括号、右方括号或井号，则直接跳过；否则将字符追加到输出字符串 `out` 中。当所有字符处理完毕后，对构建的 `out` 调用依赖函数 `clore::generate::trim_ascii` 以去除前导和尾随空白，并返回结果字符串。该实现仅依赖一个外部函数，不涉及其他控制流分支或复杂状态管理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter

#### Writes To

- local `out` string
- the returned `std::string`

#### Usage Patterns

- removing inline markdown formatting from text
- extracting plain text from markdown fragments

### `clore::generate::symbol_analysis_markdown_for`

Declaration: `src/generate/render/common.cppm:172`

Definition: `src/generate/render/common.cppm:172`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先检查 `plan` 的 `page_type` 成员：若为 `PageType::Namespace`，则派发至 `analysis_overview_markdown`；否则调用 `analysis_details_markdown`。两个辅助函数均接收相同的 `analyses` 和 `sym` 参数，返回指向字符串的指针。此实现将不同页面类型的符号分析文档生成策略解耦到单独的辅助函数中，从而避免了主函数中分支逻辑的膨胀。依赖项包括 `PagePlan` 枚举、`SymbolAnalysisStore` 和 `extract::SymbolInfo` 类型，以及两个未在片段中展示的内部辅助函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore&`)
- `plan` (const `PagePlan&`, specifically `plan.page_type`)
- `sym` (const `extract::SymbolInfo&`)

#### Usage Patterns

- Invoked by page rendering logic to fetch precomputed analysis markdown for a symbol
- Used in namespace, module, file, and symbol documentation pages
- Delegates to either `analysis_overview_markdown` or `analysis_details_markdown` based on page type

### `clore::generate::symbol_doc_view_for`

Declaration: `src/generate/render/common.cppm:280`

Definition: `src/generate/render/common.cppm:280`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::symbol_doc_view_for` 通过一个基于 `plan.page_type` 的 `switch` 语句决定返回的 `SymbolDocView` 值。当 `page_type` 为 `PageType::Namespace` 时，直接返回 `SymbolDocView::Declaration`；当为 `PageType::Module` 时，依据 `sym.kind` 是否属于变量种类（借助 `is_variable_kind` 判断）分别返回 `SymbolDocView::Details` 或 `SymbolDocView::Implementation`；其余所有情况均默认返回 `SymbolDocView::Details`。

该函数的控制流完全由枚举分支驱动，不涉及循环或递归，外部依赖集中于 `PageType` 枚举、`SymbolDocView` 枚举以及 `is_variable_kind` 谓词函数。其内部逻辑简洁，仅根据两个输入参数直接映射到视图类型。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.page_type`
- `sym.kind`

#### Usage Patterns

- Called during page rendering to select the appropriate symbol documentation view

### `clore::generate::trim_ascii`

Declaration: `src/generate/render/common.cppm:34`

Definition: `src/generate/render/common.cppm:34`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/trim-ascii.md)

函数通过两个 `while` 循环实现修剪操作。第一个循环从 `text` 的开头检查每个字符，若 `std::isspace` 对该字符（经 `static_cast<unsigned char>` 转换）返回非零值，则调用 `text.remove_prefix(1)` 移除该字符，直至遇到非空白字符或 `text` 为空。第二个循环类似地从末尾检查，利用 `text.back()` 和 `text.remove_suffix(1)` 移除尾部空白。两个循环均依赖 `std::isspace` 的 ASCII 行为，并通过强制类型转换避免负值传递给 `std::isspace` 所导致的未定义行为。最终返回修剪后的 `std::string_view`。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text` of type `std::string_view`

#### Usage Patterns

- Used by `build_prompt_section`, `strip_inline_markdown`, `add_prompt_output` to trim whitespace from string views

## Internal Structure

模块 `generate:common` 是文档生成管线中的通用渲染基础设施层，位于 `generate:render` 内部。它声明为 `common.cppm`，并导入 `config`、`extract`、`generate:markdown` 和 `generate:model` 四个模块，以此获取配置、提取的符号数据、Markdown 文档树构建能力以及页面计划等核心抽象。内部按职责分层：底层提供字符串工具（`trim_ascii`、`strip_inline_markdown`、`namespace_of`、`short_name_of`），中层负责链接目标与段落构建（`make_link_target`、`make_relative_link_target`、`push_location_paragraph`、`push_link_paragraph`、`push_optional_link_paragraph`），上层则封装更复杂的节构建器（`build_list_section`、`build_prompt_section`、`add_symbol_analysis_sections`、`build_symbol_source_locations` 等）。通过这种分解，该模块将页面渲染中常见的 Markdown 原语与符号解析逻辑集中化，使得更高层的渲染模块（如声明、实现、命名空间页面）可以专注于页面结构编排，而不必重复实现底层文本处理与链接生成。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

