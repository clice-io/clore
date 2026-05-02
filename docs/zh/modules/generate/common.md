---
title: 'Module generate:common'
description: '模块 generate:common 为文档生成管线提供共享的基础设施与辅助函数，涵盖字符串处理（trim_ascii, strip_inline_markdown, short_name_of, namespace_of）、链接目标构造（LinkTarget 及配套工厂函数）、符号列表收集与筛选（collect_namespace_symbols, collect_implementation_symbols）、以及 Markdown 片段构建（build_list_section, build_prompt_section, build_symbol_link_list, push_link_paragraph, push_optional_link_paragraph, add_symbol_analysis_sections 等）。该模块位于 generate/render/common.cppm，是 config、extract、generate:model 与 generate:markdown 之间的粘合层，负责将提取的语义数据转换为可供页面渲染直接调用的结构化输出。'
layout: doc
template: doc
---

# Module `generate:common`

## Summary

模块 `generate:common` 为文档生成管线提供共享的基础设施与辅助函数，涵盖字符串处理（`trim_ascii`, `strip_inline_markdown`, `short_name_of`, `namespace_of`）、链接目标构造（`LinkTarget` 及配套工厂函数）、符号列表收集与筛选（`collect_namespace_symbols`, `collect_implementation_symbols`）、以及 Markdown 片段构建（`build_list_section`, `build_prompt_section`, `build_symbol_link_list`, `push_link_paragraph`, `push_optional_link_paragraph`, `add_symbol_analysis_sections` 等）。该模块位于 `generate/render/common.cppm`，是 `config`、`extract`、`generate:model` 与 `generate:markdown` 之间的粘合层，负责将提取的语义数据转换为可供页面渲染直接调用的结构化输出。

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:markdown`](markdown.md)
- [`generate:model`](model.md)
- `std`

## Imported By

- [`generate:page`](page.md)
- [`generate:symbol`](symbol.md)

## Types

### `clore::generate::LinkTarget`

Declaration: `generate/render/common.cppm:11`

Definition: `generate/render/common.cppm:11`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

结构体 `clore::generate::LinkTarget` 是一个轻量级的数据聚合，用于存储渲染链接所需的三个核心信息：可见文本存储在 `label` 中，目标地址位于 `target`，布尔标志 `code_style` 控制是否以代码风格呈现该链接（默认为 `false`）。所有成员均为公共字段，不存在显式的不变量约束；其设计意图是作为简单的值对象，在各个生成阶段之间传递链接参数。

#### Invariants

- `code_style` is initialized to `false`
- `label` and `target` are mutable `std::string` values with no constraints on content

#### Key Members

- `clore::generate::LinkTarget::label`
- `clore::generate::LinkTarget::target`
- `clore::generate::LinkTarget::code_style`

#### Usage Patterns

- Used as a plain data holder for constructing or rendering link elements
- Likely populated before being passed to a rendering function or stored in a container

### `clore::generate::SymbolDocView`

Declaration: `generate/render/common.cppm:17`

Definition: `generate/render/common.cppm:17`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

枚举 `clore::generate::SymbolDocView` 是一个以 `std::uint8_t` 为底层类型的作用域枚举，其三个枚举值 `Declaration`、`Implementation` 和 `Details` 分别对应文档的不同视图模式。这些枚举值按顺序递增，但在实现中未附加额外数据或不变量；它们仅作为轻量级标签，用于在代码生成过程中区分文档内容的渲染阶段。

#### Invariants

- The enum has exactly three distinct enumerator values
- The underlying type is `std::uint8_t`
- Enumerators are fixed at compile time

#### Key Members

- `Declaration`
- `Implementation`
- `Details`

#### Usage Patterns

- The enumerator names suggest it controls which section of a symbol's documentation is rendered
- No explicit usage or dependency evidence is provided in the source

#### Member Variables

##### `clore::generate::SymbolDocView::Declaration`

Declaration: `generate/render/common.cppm:18`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Declaration
```

##### `clore::generate::SymbolDocView::Details`

Declaration: `generate/render/common.cppm:20`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Details
```

##### `clore::generate::SymbolDocView::Implementation`

Declaration: `generate/render/common.cppm:19`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

###### Implementation

```cpp
Implementation
```

## Variables

### `clore::generate::add_prompt_output`

Declaration: `generate/render/common.cppm:142`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

No usage or mutation of this variable is shown in the evidence. It appears only in the list of local variables, indicating it is perhaps used elsewhere in the function or module, but those details are not included.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::add_symbol_analysis_detail_sections`

Declaration: `generate/render/common.cppm:170`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The variable `clore::generate::add_symbol_analysis_detail_sections` is defined without evidence of reassignment after initialization. Its role as a callable object is to add detail sections for symbol analysis. No direct mutation or usage is observed in the provided evidence.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::add_symbol_analysis_sections`

Declaration: `generate/render/common.cppm:176`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The variable is part of a larger documentation generation routine that processes symbols. No evidence describes its exact role, how it is read, or how it participates in surrounding logic beyond its declaration.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::push_link_paragraph`

Declaration: `generate/render/common.cppm:92`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The role of `clore::generate::push_link_paragraph` within the surrounding logic is unclear from the provided evidence; no explicit reads, calls, or assignments involving this variable are listed. It may be part of the rendering machinery that constructs link-rich Markdown content, but its exact participation cannot be confirmed.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::push_location_paragraph`

Declaration: `generate/render/common.cppm:399`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The variable appears to be a callable object, possibly a lambda or function pointer, that is used by `clore::generate::build_symbol_source_locations` to generate a paragraph containing a symbol's source location information. No evidence of mutation is present.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- Referenced in `clore::generate::build_symbol_source_locations`

### `clore::generate::push_optional_link_paragraph`

Declaration: `generate/render/common.cppm:111`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该变量可能作为可调用对象或配置值，参与链接段落的生成逻辑，但其具体类型和初始化方式在当前证据中未明确。

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::generate::build_list_section`

Declaration: `generate/render/common.cppm:133`

Definition: `generate/render/common.cppm:133`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::build_list_section` 构造一个语义章节，其内容为可选的带项目符号列表。它首先调用 `make_section` 并传入 `SemanticKind::Section`、空的参数、标题和级别以创建章节对象。接着检查输入的 `BulletList`：若其 `items` 数组非空，则将整个列表包装为 `MarkdownNode` 并追加到章节的 `children` 向量中。最终返回该章节的智能指针。该函数直接依赖 `make_section` 和 `MarkdownNode` 的构造，并使用 `BulletList` 的 `items` 成员来判断是否需要添加列表内容。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `heading` parameter
- `level` parameter
- `list` parameter
- `list.items.empty()`

#### Writes To

- local `section` object's `children` vector
- returned `SemanticSectionPtr`

#### Usage Patterns

- building a section with a bullet list
- used to wrap a `BulletList` into a `SemanticSection`

### `clore::generate::build_prompt_section`

Declaration: `generate/render/common.cppm:124`

Definition: `generate/render/common.cppm:124`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先通过 `make_section` 构造一个 `SemanticSection` 对象，指定类型为 `SemanticKind::Section`、空元数据、移动后的标题文本和层级。随后检查 `output` 指针是否非空，以及经 `clore::generate::trim_ascii` 处理后是否不为空，若满足条件则将 `make_raw_markdown` 生成的原始 Markdown 节点追加到该 section 的子节点列表中。步骤仅由一次可选添加构成，控制流为单分支判断，核心依赖包括 `make_section`、`make_raw_markdown` 和 `trim_ascii`。

#### Side Effects

- Allocates a new `SemanticSection` object
- Potentially allocates a new `MarkdownNode` object and inserts it into the section's child vector

#### Reads From

- parameter `heading`
- parameter `level`
- parameter `output` (the pointer and the pointed string if not null)
- call to `clore::generate::trim_ascii` on `*output`

#### Writes To

- The returned `SemanticSectionPtr`'s underlying object
- The `children` vector of the created section
- The created `MarkdownNode` if output is provided

#### Usage Patterns

- Used to generate a section of a prompt or document with an optional block of raw markdown content
- Likely invoked within prompt building pipelines such as `clore::generate::build_prompt`

### `clore::generate::build_related_page_targets`

Declaration: `generate/render/common.cppm:504`

Definition: `generate/render/common.cppm:504`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过遍历`plan.linked_pages`中的链接标识，为每个关联页面生成一个`LinkTarget`对象。对每个标识，它首先查找冒号分隔符：如果存在，则提取前缀（如"module"或"namespace"）并调用`links.resolve_module`或`links.resolve_namespace`；否则降级为通用`links.resolve`。若解析结果为空或已存在于`seen`集合中，则跳过重复项。随后通过`links.resolve_page_title`获取标题，经`strip_inline_markdown`清理，若结果为空则回退到原始实体名。最终调用`make_link_target`创建包含当前页面路径、标签和解析路径的`LinkTarget`。算法依赖`LinkResolver`的多个解析方法和`strip_inline_markdown`、`make_link_target`辅助函数，并通过`seen`集合保证输出无重复。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- plan
- links
- `current_page_path`
- plan`.linked_pages`
- `LinkResolver` methods (resolve, `resolve_module`, `resolve_namespace`, `resolve_page_title`)

#### Usage Patterns

- building navigation or related-page link lists
- generating page context for markdown rendering

### `clore::generate::build_string_list`

Declaration: `generate/render/common.cppm:148`

Definition: `generate/render/common.cppm:148`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该实现遍历输入 `items`，对每个字符串调用 `trim_ascii` 进行修剪，跳过结果为空的条目。对于非空项，创建一个 `ListItem`，将其 `fragments` 设为 `code_spanned_fragments(item_text)` 的返回值，并将该项追加到 `BulletList` 中。控制流为简单的循环与条件过滤，其行为依赖于 `trim_ascii` 和 `code_spanned_fragments` 两个辅助函数。

#### Side Effects

- allocates and populates a `BulletList`
- calls `trim_ascii` on each string element
- calls `code_spanned_fragments` on each non-empty trimmed string
- returns by value (ownership transfer)

#### Reads From

- parameter `items` (const reference to vector of strings)
- function `trim_ascii`
- function `code_spanned_fragments`

#### Writes To

- local variable `list` (constructs and modifies via `push_back`)
- return value `BulletList` (constructed and returned)

#### Usage Patterns

- converts user-facing text into a formatted bullet list for markdown rendering
- used by page-building functions in the `clore::generate` namespace

### `clore::generate::build_symbol_link_list`

Declaration: `generate/render/common.cppm:360`

Definition: `generate/render/common.cppm:360`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数遍历传入的符号列表，为每个符号生成一个列表项。对于每个符号，它首先取得种类名称（通过 `extract::symbol_kind_name`）并作为文本前缀。然后根据参数 `use_full_name` 决定标签为符号的完整限定名或缩短后的短名（调用 `short_name_of`）。接着，通过 `links.resolve` 在链接解析器中查找该限定名对应的目标路径：若存在则用 `make_link` 构造可跳转的链接，目标路径利用 `make_relative_link_target` 转换为相对于当前页面的路径；否则直接以代码样式呈现标签（若标签为空则回退到完整限定名）。最终所有项目被依次压入 `BulletList` 并返回。

该函数依赖于符号提取层的种类表示与链接解析服务，并借助 `short_name_of`、`make_link` 等内部工具完成格式化输出。其设计意图是将抽象符号引用转换为可供文档页面直接渲染的链接列表，按种类前缀分组并适应全称/短称的灵活显示需求。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `symbols` parameter (`const std::vector<const extract::SymbolInfo*>&`)
- `current_page_path` parameter (`std::string_view`)
- `links` parameter (`const LinkResolver&`)
- `use_full_name` parameter (`bool`)
- `sym->kind` and `sym->qualified_name` members of each `SymbolInfo`

#### Writes To

- constructs and returns a `BulletList` object

#### Usage Patterns

- called during page rendering to produce a bullet list of symbol links
- used in documentation generation to display related symbols with navigation

### `clore::generate::build_symbol_source_locations`

Declaration: `generate/render/common.cppm:412`

Definition: `generate/render/common.cppm:412`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_symbol_source_locations` 接收一个符号信息、任务配置、链接解析器和当前页面路径，生成一个 `std::vector<MarkdownNode>` 表示符号的源位置段落。内部控制流为两个独立的条件分支：首先检查 `sym.declaration_location.is_known()`，若已知则调用 `push_location_paragraph` 并传入标签 "Declaration: " 以及通过 `make_source_link_target` 从声明位置生成的链接目标；随后检查 `sym.definition_location` 是否有值且已知，同样通过 `push_location_paragraph` 添加定义位置的段落。该函数不执行循环或递归，仅依赖 `make_source_link_target` 解析链接目标，以及 `push_location_paragraph` 构建输出节点结构；其行为完全由 `extract::SymbolInfo` 中位置字段的已知性驱动。

#### Side Effects

- allocates and returns a new vector of `MarkdownNode` objects

#### Reads From

- `sym.declaration_location.is_known()`
- `sym.definition_location.has_value()`
- `sym.definition_location->is_known()`
- `config`, `links`, `current_page_path` (passed to `make_source_link_target`)

#### Usage Patterns

- used in page-building functions to embed source location links for symbols

### `clore::generate::collect_implementation_symbols`

Declaration: `generate/render/common.cppm:314`

Definition: `generate/render/common.cppm:314`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数的核心算法按页面类型分流搜索：对于模块类型的计划（`plan.page_type == PageType::Module`），它遍历 `plan.owner_keys` 中的模块名，通过 `extract::find_modules_by_name` 定位模块，再收集所有模块内符号；否则，它假定 owner keys 是文件路径，直接在 `model.files` 中查找文件并遍历其符号。所有符号的去重由 `std::unordered_set<extract::SymbolID> seen` 通过 `insert` 返回值保证，仅当符号尚未被记录且同时满足 `is_page_level_symbol` 检查与 `predicate` 条件时，才将其指针压入结果。最终，`symbols` 按 `extract::SymbolInfo::qualified_name` 升序排序后返回。

内部流程依赖 `extract` 命名空间的多个查询函数（`find_modules_by_name`、`lookup_symbol`、`is_page_level_symbol`）以及 `PagePlan` 与 `ProjectModel` 的数据结构。排序步骤依赖 `std::sort` 并借助字符串比较完成稳定顺序。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.owner_keys`
- `plan.page_type`
- `model` (via `model.files`, `extract::find_modules_by_name`, `extract::lookup_symbol`)
- `predicate` (called on each qualifying symbol)
- `lhs->qualified_name` and `rhs->qualified_name` during sort

#### Writes To

- locally allocated `symbols` and `seen` containers
- returned vector of symbol pointers

#### Usage Patterns

- Called to gather implementation symbols when building evidence for module, file, function, or type summaries
- Used with a predicate that filters by symbol kind or other criteria to produce documentation content

### `clore::generate::collect_namespace_symbols`

Declaration: `generate/render/common.cppm:289`

Definition: `generate/render/common.cppm:289`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数首先在传入的 `model.namespaces` 字典中查找指定的 `namespace_name`；若未找到则立即返回空向量。随后遍历该命名空间条目所持有的符号标识符集合，通过 `extract::lookup_symbol` 解析每个标识符得到符号指针，并依次检查该符号是否为页面级符号（`is_page_level_symbol`）以及是否满足外部传入的 `predicate` 谓词，符合条件的符号被收集到结果向量中。最后对所有收集到的符号按 `qualified_name` 字段进行字典序排序并返回。此过程依赖于 `extract` 命名空间中的符号查找与模型结构，并借助 `std::sort` 完成排序。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` parameter of type `const extract::ProjectModel&`
- `model.namespaces` map
- `ns_it->second.symbols` container of symbol `IDs`
- `extract::lookup_symbol` function
- `is_page_level_symbol` function
- `predicate` callable argument
- `qualified_name` of each `extract::SymbolInfo`

#### Writes To

- local `std::vector<const extract::SymbolInfo*>` variable `symbols`
- the returned vector (ownership transferred to caller)

#### Usage Patterns

- Used to gather all page-level symbols for a namespace during documentation generation.
- Likely called by functions that build namespace pages or analyze symbol lists.

### `clore::generate::doc_label`

Declaration: `generate/render/common.cppm:279`

Definition: `generate/render/common.cppm:279`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::doc_label` 的核心逻辑是一个基于 `SymbolDocView` 枚举值的直接映射。它通过 `switch` 语句将每个枚举成员（`SymbolDocView::Declaration`、`SymbolDocView::Implementation`、`SymbolDocView::Details`）分别对应到固定的字符串 `"Declaration"`、`"Implementation"`、`"Details"`。`switch` 的 `default` 分支同样返回 `"Details"`，保证所有输入都能产生合法的输出字符串。

该函数不依赖任何外部模块或复杂数据结构；它的运行完全由编译时已知的枚举值驱动，控制流为单一入口的简单跳转。由于返回值类型为 `std::string_view`，实际字符串字面量由编译器嵌入到目标文件中。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `view` parameter of type `SymbolDocView`

#### Usage Patterns

- Obtain a display label for a documentation section based on the symbol view kind

### `clore::generate::find_declaration_page`

Declaration: `generate/render/common.cppm:473`

Definition: `generate/render/common.cppm:473`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::find_declaration_page` 首先尝试通过调用 `links.resolve(sym.qualified_name)` 直接定位符号的声明页面。如果解析结果非空且目标路径不等于当前页面路径 `current_page_path`，则基于 `sym.enclosing_namespace` 或 `namespace_of(sym.qualified_name)` 构造标签（格式为 `"Declaration"` 或 `"Namespace "` 加上命名空间名），并通过 `make_link_target` 生成并返回一个链接目标。若直解析失败，函数转而提取符号的命名空间名：优先使用 `sym.enclosing_namespace`，否则使用 `namespace_of(sym.qualified_name)`。若命名空间非空，则通过 `links.resolve_namespace(ns_name)` 尝试查找该命名空间的声明页面；若找到则返回以 `"Namespace " + ns_name` 为标签的链接目标。上述两种情况均未产生结果时，函数返回 `std::nullopt`。该实现依赖于 `LinkResolver` 的两个解析方法、`namespace_of` 辅助函数以及 `make_link_target` 构造器，内部逻辑仅涉及两次条件分支和标签格式的简单字符串拼接。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.qualified_name`
- `sym.enclosing_namespace`
- `current_page_path`
- `links` (`LinkResolver`)

#### Usage Patterns

- Used to generate a navigation link to a symbol's declaration page
- Invoked during page rendering to provide a "Declaration" or namespace page link
- Callers pass the current page path to avoid self-referencing links

### `clore::generate::find_implementation_pages`

Declaration: `generate/render/common.cppm:433`

Definition: `generate/render/common.cppm:433`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数通过收集与符号相关的源文件路径并解析为目标页面的超链接，构造一个去重的 `LinkTarget` 列表。算法核心是一个局部 lambda `try_add`，它对每个非空文件路径依次尝试两种解析策略：首先调用 `extract::find_module_by_source` 检查文件是否属于某个模块，如果找到则使用 `links.resolve_module` 将模块名映射为页面路径，并调用 `make_link_target` 生成 `code_style` 为 `true` 的链接；否则回退到 `links.resolve` 直接通过文件路径获取页面链接。两种情形都使用 `seen` 集合（`std::unordered_set<std::string>`）保证同一页面不会被重复添加。之后，函数先尝试处理 `sym.definition_location` 中的文件路径（若存在），再处理 `sym.declaration_location` 中的文件路径，最终返回收集到的 `results` 向量。

内部依赖主要包括符号信息类型 `extract::SymbolInfo` 的字段 `definition_location` 与 `declaration_location`、模型类型 `extract::ProjectModel`、链接解析器 `LinkResolver` 的 `resolve_module` 和 `resolve` 方法、辅助函数 `extract::find_module_by_source`、`make_link_target` 以及 `make_source_relative`。该函数不处理符号分析或文档生成的其他阶段，仅专注于从文件路径到页面链接的翻译与去重。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym` (symbol info, specifically `sym.definition_location` and `sym.declaration_location.file`)
- `model` (project model, used in `extract::find_module_by_source`)
- `links` (link resolver, used for `resolve_module` and `resolve`)
- `current_page_path` (used in `make_link_target`)
- `project_root` (used in `make_source_relative`)

#### Usage Patterns

- Called during page generation to find implementation pages for a symbol
- Used to gather links to a symbol's definition or declaration in generated documentation

### `clore::generate::find_module_for_file`

Declaration: `generate/render/common.cppm:496`

Definition: `generate/render/common.cppm:496`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数将给定的 `file_path` 委托给 `extract::find_module_by_source`，以在 `model` 中查找所属模块。如果找到匹配的模块（非空指针），则返回其 `name`；否则返回 `std::nullopt`。整个实现仅依赖于 `extract::find_module_by_source` 的查找结果，没有额外的控制流分支或处理逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model`
- `file_path`

#### Usage Patterns

- Used to map source files to their containing module for documentation generation.

### `clore::generate::make_link_target`

Declaration: `generate/render/common.cppm:81`

Definition: `generate/render/common.cppm:81`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::make_link_target` 接受当前页面路径 `current_page_path`、标签 `label`、目标页面路径 `target_page_path` 以及可选的代码样式标记 `code_style`，并返回一个 `LinkTarget` 对象。该函数使用 `make_relative_link_target` 将 `current_page_path` 与 `target_page_path` 规约化为相对目标路径，再以聚合初始化方式构造返回的 `LinkTarget`，直接将其 `label`、`target` 和 `code_style` 字段赋值。整个实现不包含任何分支或循环，完全依赖于 `LinkTarget` 的指定初始化器语法以及 `make_relative_link_target` 的路径规约逻辑。

#### Side Effects

- moves the label string argument, leaving the original empty or unspecified

#### Reads From

- `current_page_path` parameter
- label parameter (before move)
- `target_page_path` parameter
- `code_style` parameter
- `make_relative_link_target` (internal, unspecified)

#### Writes To

- returned `LinkTarget` object's `.label` field (set via move)
- returned `LinkTarget` object's `.target` field
- returned `LinkTarget` object's `.code_style` field

#### Usage Patterns

- called to create link targets for page navigation or cross-references in documentation generation

### `clore::generate::make_relative_link_target`

Declaration: `generate/render/common.cppm:57`

Definition: `generate/render/common.cppm:57`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::make_relative_link_target` 通过 `std::filesystem` 计算从 `current_page_path` 到 `target_page_path` 的相对路径。首先使用 `lexically_normal()` 对两个路径进行规范化，然后以 `current` 的父路径（若无父路径则使用当前目录的 `"."`）作为基路径，调用 `lexically_relative(base)` 获取相对关系。若结果为空（即目标不在基路径的子树上），则直接返回 `target.generic_string()`；否则返回相对路径的通用字符串形式。该函数完全不依赖其他 `clore::generate` 内部函数，仅借助标准库文件系统接口。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path` parameter
- `target_page_path` parameter

#### Usage Patterns

- Used by page generation logic to create relative links between markdown pages
- Called during link resolution for documentation cross-references

### `clore::generate::make_source_link_target`

Declaration: `generate/render/common.cppm:383`

Definition: `generate/render/common.cppm:383`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先构造一个人类可读的标签，方法是将 `location.file` 通过 `make_source_relative` 转换为相对路径，再拼接行号。然后调用 `links.resolve` 尝试将源文件路径映射到文档目标路径；若解析成功，则通过 `make_link_target` 生成一个包含完整链接目标的 `LinkTarget`，并设置代码样式标记为 `true`。若解析失败，则返回一个仅包含标签、空目标路径且同样标记代码样式的 `LinkTarget`。整个过程依赖 `LinkResolver` 的路径解析能力以及 `LinkTarget` 的三字段结构，没有分支以外的复杂控制流。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `location.file`
- `location.line`
- `config.project_root`
- `current_page_path`
- `links` (via `LinkResolver::resolve`)

#### Usage Patterns

- Creating source link targets for documentation page rendering
- Generating clickable links from code locations to rendered output

### `clore::generate::namespace_of`

Declaration: `generate/render/common.cppm:53`

Definition: `generate/render/common.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::namespace_of` 是一个轻量包装函数，它将限定名称解析委托给 `extract::namespace_prefix_from_qualified_name`。实现直接返回该底层函数的结果，自身不包含任何算法或控制流逻辑。这一设计将命名空间前缀提取的细节隔离在 `extract` 命名空间中的独立工具函数内，而此函数则为文档生成流程的其他部分提供了统一的调用接口。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter of type `std::string_view`

#### Writes To

- returned `std::string` value containing the namespace prefix

#### Usage Patterns

- extracts namespace prefix from qualified names in the documentation generation pipeline

### `clore::generate::prompt_output_of`

Declaration: `generate/render/common.cppm:71`

Definition: `generate/render/common.cppm:71`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::prompt_output_of` 封装了从预生成的 prompt 输出集合中按类型和可选目标键查找对应文本的逻辑。它接受一个 `const std::unordered_map<std::string, std::string>&` 类型的 `outputs` 映射、一个 `PromptKind` 枚举值 `kind`，以及一个 `std::string_view` 类型的默认空 `target_key`。内部首先使用 `prompt_request_key` 构造一个 `PromptRequest` 对象（其中 `target_key` 被转换为 `std::string`），生成唯一的搜索键，然后在 `outputs` 中执行 `find` 操作。若找到对应条目，则返回指向该值的指针；否则返回 `nullptr`。该函数依赖于 `PromptKind`、`PromptRequest` 和 `prompt_request_key` 的定义，不涉及额外控制流或副作用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `outputs`
- `kind`
- `target_key`
- result of `prompt_request_key`

#### Usage Patterns

- retrieving cached prompt output for a specific prompt kind
- checking if a prompt output already exists in a generation pipeline

### `clore::generate::short_name_of`

Declaration: `generate/render/common.cppm:45`

Definition: `generate/render/common.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::short_name_of` 的实现完全依赖于 `extract::split_top_level_qualified_name` 对输入的 `qualified_name` 进行分割。它首先检查分割后得到的字符串列表是否为空，若为空则直接返回一个空字符串；否则返回列表的最后一个元素。整个控制流仅包含一个简单的条件分支，无其他循环或递归。该函数不涉及任何外部依赖或模块级状态，所有逻辑均围绕标准库的字符串视图和向量操作展开。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `qualified_name`

#### Usage Patterns

- Extract the short (unqualified) name from a fully qualified name
- Used wherever only the local name of a symbol is needed

### `clore::generate::strip_inline_markdown`

Declaration: `generate/render/common.cppm:33`

Definition: `generate/render/common.cppm:33`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::strip_inline_markdown` 实现了一个简洁的字符级过滤算法。它遍历输入字符串 `text` 中的每个字符，跳过所有属于 Markdown 内联语法的字符：`` ` ``、`*`、`_`、`[`、`]` 和 `#`；其余字符则追加到输出字符串中。过滤完成后，调用 `clore::generate::trim_ascii` 对结果进行首尾空白修剪并返回。该算法不涉及 Markdown 的解析或上下文判断，仅依赖标准库字符串操作和 `trim_ascii` 函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `text`

#### Usage Patterns

- producing plain text from Markdown fragments for contexts that disallow inline formatting

### `clore::generate::symbol_analysis_markdown_for`

Declaration: `generate/render/common.cppm:161`

Definition: `generate/render/common.cppm:161`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::symbol_analysis_markdown_for` 根据 `PagePlan` 的 `page_type` 字段决定输出格式。当页面类型为 `PageType::Namespace` 时，将当前符号的摘要信息委托给 `analysis_overview_markdown` 处理；否则调用 `analysis_details_markdown` 生成更详细的视图。这两个辅助函数均接收 `SymbolAnalysisStore` 和 `SymbolInfo` 作为参数，分别产出面向命名空间总览或单符号详情的内容。该函数本身不涉及循环或深层嵌套，仅通过一次 `if` 分支路由到正确的实现路径，并返回生成的 Markdown 字符串指针。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore&`)
- `plan` (const `PagePlan&`, specifically `plan.page_type`)
- `sym` (const `extract::SymbolInfo&`)
- `analysis_overview_markdown(analyses, sym)` and `analysis_details_markdown(analyses, sym)`

#### Usage Patterns

- Called during page generation to retrieve precomputed analysis markdown for a symbol.
- Dispatches to overview or detailed analysis based on the page type.

### `clore::generate::symbol_doc_view_for`

Declaration: `generate/render/common.cppm:269`

Definition: `generate/render/common.cppm:269`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::symbol_doc_view_for` 根据给定的 `plan.page_type` 决定符号文档应采用的视图模式。其逻辑是一个基于 `PagePlan` 枚举的 `switch` 语句：若页面类型为 `PageType::Namespace`，立即返回 `SymbolDocView::Declaration`；若为 `PageType::Module`，则进一步检查符号的 `kind`——通过调用 `is_variable_kind(sym.kind)` 判断是否为变量类型，是则返回 `SymbolDocView::Details`，否则返回 `SymbolDocView::Implementation`；对于所有其他页面类型，默认返回 `SymbolDocView::Details`。该函数仅依赖 `PagePlan`、`extract::SymbolInfo`、`PageType` 枚举、`SymbolDocView` 枚举以及 `is_variable_kind` 工具函数，无额外分支或循环。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.page_type`
- `sym.kind`

#### Usage Patterns

- Used in page rendering to choose documentation mode for symbols
- Called in contexts where page type and symbol kind influence presentation

### `clore::generate::trim_ascii`

Declaration: `generate/render/common.cppm:23`

Definition: `generate/render/common.cppm:23`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::trim_ascii` 的实现采用两步扫描策略：首先通过 `while` 循环与 `std::isspace` 检查并移除字符串视图 `text` 的前导空白（将字符强制转换为 `unsigned char` 以避免未定义行为），每次调用 `text.remove_prefix(1)`；随后以相同方式检测尾部空白，每次调用 `text.remove_suffix(1)`。该算法仅依赖 `std::string_view` 的 `remove_prefix` 与 `remove_suffix` 成员函数，以及标准库的 `std::isspace` 函数，未使用任何项目内部依赖。由于空白判定直接针对 ASCII 子集（由 `std::isspace` 在默认 C 语言环境下保证），函数无需处理多字节字符或 Unicode 空白，整体开销仅与空白字符数量线性相关。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `text` parameter

#### Usage Patterns

- used to clean up whitespace from string views before further processing
- called to normalize text for markdown rendering or comparison

## Internal Structure

模块 `generate:common` 是文档生成管线中的共享工具层，专注于提供跨渲染步骤的可复用函数与数据结构。它通过导入 `config`、`extract`、`generate:model` 和 `generate:markdown` 模块，将项目配置、符号模型及 Markdown 生成原语组装成更高级的构建块——例如链接目标构造、符号文档视图判定、命名空间符号收集、分析详情节生成、字符串列表构建、内联 Markdown 清理以及相对页面路径解析。这些函数不直接处理具体页面布局，而是作为底层能力被 `generate:render` 下的各个页面生成模块调用，从而实现分解清晰、职责单一的内部实现结构。该模块本身不引入复杂的内部层次，所有工具函数均置于单一推送单元（`common.cppm`）中，通过全局变量或公共函数对外暴露，模块的导入边界已明确区分配置、提取、模型与渲染输出，确保了上下游依赖的正向流动。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

