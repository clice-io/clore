---
title: 'Module generate:common'
description: '模块 generate:common 属于文档生成渲染阶段的公共工具模块。它基于 generate:model 中的核心数据结构和 extract 模块提取的符号信息，提供了一系列辅助函数来构建 Markdown 文档内容、管理页面间链接以及生成符号分析章节。主要公开内容包括：结构化表示链接目标的 LinkTarget 类型和 SymbolDocView 枚举，以及用于修剪文本、获取符号名称、查询命名空间、构建提示段落、生成符号链接列表、解析声明页和实现页、收集命名空间与实现符号等通用操作。这些功能被上层页面生成模块所调用，从而实现统一的文档渲染流程。'
layout: doc
template: doc
---

# Module `generate:common`

## Summary

模块 `generate:common` 属于文档生成渲染阶段的公共工具模块。它基于 `generate:model` 中的核心数据结构和 `extract` 模块提取的符号信息，提供了一系列辅助函数来构建 Markdown 文档内容、管理页面间链接以及生成符号分析章节。主要公开内容包括：结构化表示链接目标的 `LinkTarget` 类型和 `SymbolDocView` 枚举，以及用于修剪文本、获取符号名称、查询命名空间、构建提示段落、生成符号链接列表、解析声明页和实现页、收集命名空间与实现符号等通用操作。这些功能被上层页面生成模块所调用，从而实现统一的文档渲染流程。

该模块亦暴露了若干可调用变量（如 `push_link_paragraph`、`add_symbol_analysis_sections`），用于在渲染流水线中的特定位置插入段落或章节。通过依赖 `config`、`extract`、`generate:markdown` 和 `generate:model` 等模块，`generate:common` 在生成上下文中封装了大量重复性逻辑，降低了文档生成代码的复杂度并提升了可维护性。

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

`clore::generate::LinkTarget` 是一个公开聚合类型，用于保存单一链接目标所需的全部数据。三个字段直接暴露：`label`（显示文本）、`target`（链接地址或标识符）以及 `code_style`（指示是否应以代码样式渲染该链接的布尔标志）。没有任何内部不变量或特殊成员函数，所有字段均可在构造后独立修改，默认构造时 `code_style` 为 `false`。该结构体的设计意图是作为轻量级的数据载体，在生成管线中传递链接信息。

#### Invariants

- `label` and `target` are expected to be valid strings
- `code_style` is a boolean flag, initialized to `false`

#### Key Members

- `label`
- `target`
- `code_style`

#### Usage Patterns

- Used to represent a link in code generation contexts
- Passed to functions that render hyperlinks with optional code styling

### `clore::generate::SymbolDocView`

Declaration: `generate/render/common.cppm:17`

Definition: `generate/render/common.cppm:17`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

枚举 `clore::generate::SymbolDocView` 定义于 `generate/render/common.cppm`，其底层类型显式指定为 `std::uint8_t`，确保枚举值以单字节存储，适用于频繁传递视图选项的代码路径。成员 `Declaration`、`Implementation` 和 `Details` 是三个互斥的枚举值，作为不透明标签在内部渲染流程中通过开关分支选择不同的输出路径；枚举值的数值与顺序本身不提供额外约束，仅承担类型安全标识符的角色，防止整型误用。

内部结构上，该枚举不附带任何数据成员或自定义运算符，其唯一的不变量是各成员保持唯一且底层值无重叠。`clore::generate::SymbolDocView` 自身不提供转换或取值方法，所有使用均依赖枚举值的模式匹配或开关判断，从而保持渲染逻辑的清晰与封闭。

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

No evidence available on its role, how it is read, or its participation in surrounding logic.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::add_symbol_analysis_detail_sections`

Declaration: `generate/render/common.cppm:170`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

No further usage or mutation information is provided in the evidence.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::add_symbol_analysis_sections`

Declaration: `generate/render/common.cppm:176`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

The variable participates in rendering symbol documentation pages by adding analysis sections to a symbol's output. It is not mutated in the provided evidence. Its role appears to involve consuming analysis data from `analyses`, `plan`, and other rendering context to generate appropriate sections.

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::push_link_paragraph`

Declaration: `generate/render/common.cppm:92`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该变量可能用于生成链接段落，但证据中未描述其定义或使用方式，因此无法确定其在代码中的确切角色。

#### Mutation

No mutation is evident from the extracted code.

### `clore::generate::push_location_paragraph`

Declaration: `generate/render/common.cppm:399`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

It is used in `clore::generate::build_symbol_source_locations` to generate location information, formatting and pushing a paragraph describing the symbol's source location.

#### Mutation

No mutation is evident from the extracted code.

#### Usage Patterns

- called from `clore::generate::build_symbol_source_locations`

### `clore::generate::push_optional_link_paragraph`

Declaration: `generate/render/common.cppm:111`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该变量在生成过程中可能作为标志或函数对象，但证据未提供具体逻辑。

#### Mutation

No mutation is evident from the extracted code.

## Functions

### `clore::generate::build_list_section`

Declaration: `generate/render/common.cppm:133`

Definition: `generate/render/common.cppm:133`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_list_section` 接受一个标题字符串 `heading`、一个层级值 `level` 和一个 `BulletList` 对象 `list`，并返回一个 `SemanticSectionPtr`。其实现首先调用 `make_section` 来创建一个 `SemanticKind::Section` 类型的节，并将 `heading` 和 `level` 分别作为标题和层级参数传递。随后，它检查 `list` 中的 `items` 容器是否为空；若不为空，则将整个 `list` 对象包装在 `MarkdownNode` 中，并通过 `push_back` 添加到节对象的 `children` 向量末尾。整个流程不涉及额外的分支或循环，也未依赖其他生成函数的复杂交互，仅利用 `make_section` 和 `BulletList` 的数据成员完成节结构的构建。依赖的符号包括 `SemanticSectionPtr`、`SemanticKind` 枚举、`MarkdownNode` 以及 `BulletList` 类型。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- heading parameter
- level parameter
- list parameter
- list`.items``.empty()`

#### Usage Patterns

- Used within page generation pipelines to wrap a bullet list under a section heading

### `clore::generate::build_prompt_section`

Declaration: `generate/render/common.cppm:124`

Definition: `generate/render/common.cppm:124`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`build_prompt_section` 首先调用 `make_section`，以 `SemanticKind::Section`、空的初始内容列表、传入的 `heading`（通过 `std::move` 转移）和 `level` 创建一个 `SemanticSectionPtr`。随后，检查可选的 `output` 指针：如果指针非空且经 `trim_ascii` 修剪后的内容非空，则构造一个 `make_raw_markdown(*output)` 原始 Markdown 节点，并将其追加到 `section->children` 中。该函数仅依赖 `make_section`、`trim_ascii` 和 `make_raw_markdown` 这三个辅助函数，不涉及循环或递归，控制流为简单的顺序加条件分支。

#### Side Effects

- Heap allocation for a `SemanticSection` and potentially a `MarkdownNode`

#### Reads From

- parameter `heading`
- parameter `level`
- pointer `output` (dereferenced if non-null, content trimmed by `trim_ascii`)

#### Usage Patterns

- Building prompt sections with or without additional markdown content

### `clore::generate::build_related_page_targets`

Declaration: `generate/render/common.cppm:504`

Definition: `generate/render/common.cppm:504`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数遍历 `plan.linked_pages` 中的每个条目，将其解析为可选的模块或命名空间前缀与实体名称。根据前缀分别调用 `links.resolve_module`、`links.resolve_namespace` 或 `links.resolve` 来获得目标路径。对于每个新发现的路径，使用 `links.resolve_page_title` 获取标题，并经由 `strip_inline_markdown` 清洗后，与路径一同传入 `make_link_target` 构建 `LinkTarget` 对象。结果通过一个 `std::unordered_set<std::string>` 去重后追加到输出向量中。主要依赖 `LinkResolver` 的路径解析接口和 `strip_inline_markdown`、`make_link_target` 两个辅助函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `plan.linked_pages`
- `links.resolve_module`
- `links.resolve_namespace`
- `links.resolve`
- `links.resolve_page_title`
- `current_page_path`

#### Writes To

- returned `std::vector<LinkTarget>`

#### Usage Patterns

- used in page rendering to generate navigation links to related pages

### `clore::generate::build_string_list`

Declaration: `generate/render/common.cppm:148`

Definition: `generate/render/common.cppm:148`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::build_string_list` 遍历输入的 `std::vector<std::string>` 容器 `items`，对每个元素调用 `trim_ascii` 并检查其是否为空字符串，跳过空白项。对于非空元素，使用 `code_spanned_fragments` 将字符串解析为格式化片段序列，并构造一个 `ListItem` 对象，其 `fragments` 成员接收该序列，然后将该列表项追加到返回值 `BulletList list` 的 `items` 向量中。该函数依赖于 `trim_ascii` 进行空白检测、`code_spanned_fragments` 实现内联代码高亮，以及 `BulletList` 和 `ListItem` 的结构定义。内部流程为纯线性循环，无分支或异常处理，保证返回一个包含所有非空项的格式化列表对象。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `items` parameter

#### Writes To

- local `BulletList` and its `ListItem` elements (returned as result)

#### Usage Patterns

- building a bullet list of code-spanned text items
- helper for rendering markdown lists

### `clore::generate::build_symbol_link_list`

Declaration: `generate/render/common.cppm:360`

Definition: `generate/render/common.cppm:360`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数构造一个 `BulletList`，遍历输入的 `symbols` 向量，为每个 `SymbolInfo` 指针对应生成一个列表条目。在控制流上，首先提取符号的种类名称（通过 `extract::symbol_kind_name`），然后根据 `use_full_name` 决定显示标签：或使用 `sym->qualified_name`，或通过 `short_name_of` 取短名。接着以 `links.resolve` 查询该符号的已解析目标路径；若存在，则调用 `make_link` 并传入经 `make_relative_link_target` 处理的相对路径以生成内部链接；否则仅以 `make_code` 渲染标签作为纯代码文本。每个条目的片段先后追加种类名称文本和链接/代码片段，最终聚合成完整的 `BulletList`。依赖方面，该函数借用了 `extract::symbol_kind_name`、`short_name_of`、`links` 的解析方法以及 `make_relative_link_target`、`make_text`、`make_link`、`make_code` 等底层工具函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `symbols` vector of `extract::SymbolInfo*` pointers
- `sym->kind`
- `sym->qualified_name`
- `current_page_path`
- `links` resolver (via `links.resolve`)
- `use_full_name` flag

#### Writes To

- returned `BulletList` object

#### Usage Patterns

- Called when generating page content that lists symbols with navigation links
- Used to produce symbol lists in documentation pages

### `clore::generate::build_symbol_source_locations`

Declaration: `generate/render/common.cppm:412`

Definition: `generate/render/common.cppm:412`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::build_symbol_source_locations` 负责为给定的符号构造一组描述其源代码位置的 Markdown 节点。其实现采用线性顺序检查：首先判断 `sym.declaration_location` 是否已知（通过 `is_known()`），若已知则调用 `push_location_paragraph` 并传入标签 `"Declaration: "` 以及由 `make_source_link_target` 基于该位置、`config`、`links` 和 `current_page_path` 生成的链接目标。随后检查 `sym.definition_location` 是否存在且其位置已知，条件满足时以同样方式追加定义位置的段落。最终返回累积的节点向量。

该函数依赖于三个外部组件：`make_source_link_target`（负责将原始位置解析为 `LinkTarget` 对象）、`push_location_paragraph`（向节点容器添加格式化段落）以及 `LinkTarget` 结构体本身。其控制流是纯粹的顺序分支，不涉及循环或递归，因此执行效率完全取决于位置信息的准确性和链接构造函数的开销。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `sym.declaration_location`
- `sym.definition_location`
- `config`
- `links`
- `current_page_path`

#### Writes To

- returned vector of `MarkdownNode`

#### Usage Patterns

- Used to generate source-location markdown for symbol documentation pages

### `clore::generate::collect_implementation_symbols`

Declaration: `generate/render/common.cppm:314`

Definition: `generate/render/common.cppm:314`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::collect_implementation_symbols` 通过遍历 `plan.owner_keys` 并依据 `plan.page_type` 分派收集逻辑，从 `model` 中提取符合要求的符号。当页面类型为 `PageType::Module` 时，它调用 `extract::find_modules_by_name` 取得模块对象，遍历其 `symbols` 并利用 `seen` 集合去重，再通过 `extract::lookup_symbol` 获取符号指针，经 `is_page_level_symbol` 和 `predicate` 过滤后加入结果。对于其他页面类型（即文件类型），它直接在 `model.files` 中查找键对应的文件对象，遍历其 `symbols` 并执行相同的去重、查找与过滤逻辑。最后收集到的符号按 `qualified_name` 进行升序排序后返回。

该函数核心依赖 `extract::find_modules_by_name`、`extract::lookup_symbol` 以及内部辅助 `is_page_level_symbol` 完成符号检索与验证，并借助 `std::unordered_set` 实现 O(1) 去重，最终通过 `std::sort` 保证输出顺序一致。控制流程中，`PageType` 的分支避免了不必要的文件查找，而 `seen` 集合防止同一符号因多个 `owner_key` 重复加入。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- plan`.owner_keys`
- plan`.page_type`
- model`.files`
- model (via `extract::find_modules_by_name` and `extract::lookup_symbol`)
- predicate argument

#### Usage Patterns

- Called during page generation to gather symbols for documentation output
- Used by page-building functions like `build_page_root` and `collect_namespace_symbols`

### `clore::generate::collect_namespace_symbols`

Declaration: `generate/render/common.cppm:289`

Definition: `generate/render/common.cppm:289`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先创建一个空的存储向量 `symbols`，然后通过 `model.namespaces` 映射查找给定的 `namespace_name`。若未找到该命名空间，直接返回空结果。否则遍历该命名空间下所有符号 ID，对每个 ID 调用 `extract::lookup_symbol` 获取 `extract::SymbolInfo` 指针，并在指针有效且通过 `is_page_level_symbol` 以及传入的 `predicate` 检查后将其追加至 `symbols`。完成收集后，按 `qualified_name` 的字典序对 `symbols` 进行排序并返回。该函数依赖 `model.namespaces` 的存储结构、`extract::lookup_symbol` 查询接口以及 `extract::SymbolInfo` 的 `qualified_name` 成员，内部仅通过迭代和简单过滤器实现筛选。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (const `extract::ProjectModel`&)
- `namespace_name` (`std::string_view`)
- `predicate` (Predicate&)

#### Usage Patterns

- Used to gather symbols for namespace page generation
- Called with `is_page_level_symbol` and custom predicates

### `clore::generate::doc_label`

Declaration: `generate/render/common.cppm:279`

Definition: `generate/render/common.cppm:279`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::doc_label` 将一个 `SymbolDocView` 枚举值映射到对应的文档标签字符串，其实现是一个直接的 switch 语句。根据传入的 `view` 参数，它分别返回 `"Declaration"`、`"Implementation"` 或 `"Details"`，分别对应 `SymbolDocView::Declaration`、`SymbolDocView::Implementation` 和 `SymbolDocView::Details` 三个枚举成员。

该函数没有外部依赖，所有字符串字面量均为硬编码。对于未匹配的枚举值，它通过一个 fallback 路径返回 `"Details"` 作为默认值。这一简单映射在其他需要生成章节标题或标签的上下文中被引用（如变量 `section`、`heading` 等），但函数自身不涉及复杂的算法或控制流。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- view parameter of type `SymbolDocView`

#### Usage Patterns

- Used to obtain display labels for `SymbolDocView` values in documentation generation
- Likely called when rendering section headings or labels for symbol documentation views

### `clore::generate::find_declaration_page`

Declaration: `generate/render/common.cppm:473`

Definition: `generate/render/common.cppm:473`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::find_declaration_page` 尝试为给定符号定位一个可链接的“声明页面”。它首先通过调用 `links.resolve` 在 `LinkResolver` 中查找该符号的完全限定名，如果找到的目标路径与 `current_page_path` 不同，则立即构造一个 `LinkTarget`——标签取自 `sym.enclosing_namespace`（若存在）或通过 `namespace_of` 计算出的命名空间名称，格式为 `"Namespace " + ns_name`，若命名空间为空则直接使用 `"Declaration"`。若第一次查找失败或指向当前页面，则回退到检查符号的命名空间：仅当 `ns_name` 非空时，调用 `links.resolve_namespace` 查找该命名空间的页面，若找到则构造对应的 `LinkTarget`。若两次尝试均未找到合适的页面，函数返回 `std::nullopt`。该实现依赖于 `LinkResolver` 的两种解析方法以及 `namespace_of` 和 `make_link_target` 辅助函数。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- sym`.qualified_name`
- sym`.enclosing_namespace`
- links (via resolve and `resolve_namespace` methods)
- `current_page_path`

#### Usage Patterns

- Used internally during page generation to locate a link to a symbol's declaration page.
- Often invoked when rendering pages that need to provide a cross-reference to the declaration of a symbol.

### `clore::generate::find_implementation_pages`

Declaration: `generate/render/common.cppm:433`

Definition: `generate/render/common.cppm:433`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先将结果存入一个 `std::vector<LinkTarget>`，并使用一个 `std::unordered_set<std::string>` 记录已添加的链接路径以避免重复。核心逻辑由一个 lambda `try_add` 驱动，它接受一个 `file_path`：如果文件路径非空，则尝试通过 `extract::find_module_by_source` 查找文件所属的模块；若找到模块，则通过 `links.resolve_module` 获取模块目标路径，并通过 `make_link_target` 构造带模块名称的链接；否则直接通过 `links.resolve` 解析原始文件路径，并使用 `make_source_relative` 生成相对路径作为标签。随后按顺序处理 `sym.definition_location` 的可选文件以及 `sym.declaration_location` 的必然文件，依次调用 `try_add`。最终返回 `results`。

控制流简洁：先尝试添加定义文件（若存在），再尝试添加声明文件。依赖的外部函数包括 `extract::find_module_by_source`、`links.resolve_module` 与 `links.resolve`，以及构建 `LinkTarget` 的 `make_link_target` 和路径格式化的 `make_source_relative`。整个过程中 `seen` 集合确保同一目标路径不会被重复插入。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- sym`.definition_location`
- sym`.declaration_location``.file`
- model (via `extract::find_module_by_source`)
- links (resolve and `resolve_module` methods)
- `current_page_path`
- `project_root`

#### Usage Patterns

- Generates implementation page links for symbol documentation
- Used in page rendering to provide navigation to implementation details

### `clore::generate::find_module_for_file`

Declaration: `generate/render/common.cppm:496`

Definition: `generate/render/common.cppm:496`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::find_module_for_file` 的实现仅做单一委托：它调用 `extract::find_module_by_source`，传入 `model` 与 `file_path`。若该调用返回指向模块对象的非空指针，则函数提取其 `name` 字段并包装成 `std::optional<std::string>` 返回；否则直接返回 `std::nullopt`。

内部控制流极简，无循环或分支嵌套。依赖关系集中于 `extract` 命名空间下的 `ProjectModel` 类型与 `find_module_by_source` 查询函数，后者负责执行实际的模块查找逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `extract::ProjectModel` object passed as `model`
- `std::string_view` parameter `file_path`

#### Usage Patterns

- Determining module membership for a source file
- Mapping file paths to module names for documentation generation

### `clore::generate::make_link_target`

Declaration: `generate/render/common.cppm:81`

Definition: `generate/render/common.cppm:81`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::make_link_target` 的实现在构造一个 `LinkTarget` 实例时依次填充其三个字段：`label` 通过 `std::move` 直接接管传入的 `std::string` 参数，避免拷贝；`target` 通过调用 `make_relative_link_target` 计算并生成从 `current_page_path` 到 `target_page_path` 的相对路径；`code_style` 则直接复制 `bool` 参数。整个函数无分支或循环，唯一的依赖是对 `make_relative_link_target` 的调用，该调用假定已正确链接并返回可用于 `LinkTarget::target` 的类型，以及 `LinkTarget` 结构体本身的定义。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `current_page_path`
- `label`
- `target_page_path`
- `code_style`

#### Writes To

- 返回的 `LinkTarget` 对象的 `.label`、`.target` 和 `.code_style` 字段

#### Usage Patterns

- 在生成文档页面时创建链接目标
- 配合 `make_relative_link_target` 使用以生成相对路径

### `clore::generate::make_relative_link_target`

Declaration: `generate/render/common.cppm:57`

Definition: `generate/render/common.cppm:57`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数根据两个文件系统路径计算相对链接目标。它首先将 `current_page_path` 和 `target_page_path` 分别通过 `std::filesystem::path::lexically_normal` 标准化，然后以 `current` 的父目录（若无父目录则用 `"."`）作为 `base`，调用 `target.lexically_relative(base)` 生成相对路径。若 `rel` 为空（即 `base` 与 `target` 相同或无法表达为相对关系），则直接返回 `target.generic_string()`；否则返回 `rel.generic_string()`。整个实现依赖 `std::filesystem` 提供的路径操纵原语，无其他外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `current_page_path`
- parameter `target_page_path`

#### Usage Patterns

- generates relative href attributes in page links
- used when constructing cross-references between documentation pages

### `clore::generate::make_source_link_target`

Declaration: `generate/render/common.cppm:383`

Definition: `generate/render/common.cppm:383`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先通过 `make_source_relative` 将 `location.file` 转换为相对于 `config.project_root` 的路径，并与行号拼接构造出 `label` 字符串。随后尝试调用 `links.resolve(location.file)` 查找该源文件是否在链接解析器中有对应的目标页面；若找到，则委托 `make_link_target` 构造一个包含目标路径的完整 `LinkTarget`（并强制设置 `code_style` 为 `true`）。若解析失败，则直接返回一个 `LinkTarget`，其 `label` 为上述拼接结果，`target` 为空，`code_style` 同样置为 `true`，表示该链接仅用于代码风格展示而不生成超链接。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `location.file`
- `location.line`
- `config.project_root`
- `links`
- `current_page_path`

#### Usage Patterns

- Used in page rendering to create source code links

### `clore::generate::namespace_of`

Declaration: `generate/render/common.cppm:53`

Definition: `generate/render/common.cppm:53`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::namespace_of` 的实现直接委托给 `extract::namespace_prefix_from_qualified_name`，二者具有相同的签名并共享相同的语义。内部无分支或循环，仅通过参数 `qualified_name` 进行一次转发调用。依赖仅限于 `extract` 命名空间下的该工具函数，用于从完全限定名称中提取命名空间前缀。该函数在生成过程中被其他组件（如 `symbol_analysis_markdown_for`）间接使用，用于构造页面路径或标签，但其自身不承载任何额外的算法逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter

#### Usage Patterns

- Used to compute the namespace portion of a qualified symbol name for page generation or analysis.

### `clore::generate::prompt_output_of`

Declaration: `generate/render/common.cppm:71`

Definition: `generate/render/common.cppm:71`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::prompt_output_of` 的实现核心是一个键值查找操作。它接受一个输出映射 `outputs`（类型为 `std::unordered_map<std::string, std::string>`）、一个 `PromptKind` 枚举值 `kind` 以及一个可选的 `target_key`（默认空字符串）。内部首先构造一个 `PromptRequest` 对象，将 `kind` 和 `target_key` 填入其对应的字段，然后调用 `prompt_request_key` 将此请求序列化为一个字符串键。随后，使用 `std::unordered_map::find` 在该键上查找 `outputs` 映射：若找到，则返回指向对应字符串值的常量指针；否则返回 `nullptr`。该函数依赖于 `prompt_request_key` 函数的键生成逻辑以及 `PromptRequest` 结构体的定义，而不涉及其他复杂的控制流或外部依赖。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `outputs` parameter of type `const std::unordered_map<std::string, std::string>&`
- `kind` parameter of type `PromptKind`
- `target_key` parameter of type `std::string_view`
- result of `prompt_request_key(PromptRequest{...})`

#### Usage Patterns

- Used to retrieve prompt output for a specific kind and target key
- Called by `build_evidence_for_*` functions to access cached analysis results
- Checked for `nullptr` to handle missing outputs

### `clore::generate::short_name_of`

Declaration: `generate/render/common.cppm:45`

Definition: `generate/render/common.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::short_name_of` 从给定的限定名中提取最短的名称部分。它首先调用 `extract::split_top_level_qualified_name` 将 `qualified_name` 分割成作用域组成部分，存储在 `parts` 中。若 `parts` 为空，函数立即返回一个空字符串。否则，返回 `parts` 的最后一个元素，即限定名中末级标识符。这一实现完全依赖 `extract::split_top_level_qualified_name` 的语义来完成分割任务，其控制流简洁。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter

#### Usage Patterns

- Used to obtain a symbol's simple name for display purposes
- Can be used in formatting documentation output
- Likely called by rendering functions needing short names

### `clore::generate::strip_inline_markdown`

Declaration: `generate/render/common.cppm:33`

Definition: `generate/render/common.cppm:33`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数的实现通过一个简单的单遍扫描算法，在输出字符串中剔除常见的内联 Markdown 格式字符。它依次检查输入中的每个字符，如果当前字符是 `` ` ``、`*`、`_`、`[`、`]` 或 `#` 之一，则直接跳过；否则就将该字符追加到结果中。这种过滤方式保留了所有其他字符，包括普通文本和特殊对 Markdown 无影响的符号。在循环结束后，结果字符串会通过 `clore::generate::trim_ascii` 去除首尾的空白字符，最终返回处理后的字符串。

内部控制流完全依赖于基于范围的 `for` 循环和条件跳转（`continue`），没有任何嵌套分支或复杂的数据结构。依赖方面，除了标准库类型 `std::string_view` 和 `std::string`，本函数只调用了同一模块的辅助函数 `clore::generate::trim_ascii`，该函数负责最终的空白修剪。整个实现体现了极简的过滤逻辑，并未涉及其他生成基础设施中的成员或类型。

#### Side Effects

- allocates a new string via the standard library
- calls `clore::generate::trim_ascii`

#### Reads From

- `text` parameter

#### Writes To

- returned `std::string`

#### Usage Patterns

- cleaning markdown for plain text output
- preprocessing text for display in contexts where markdown is not allowed

### `clore::generate::symbol_analysis_markdown_for`

Declaration: `generate/render/common.cppm:161`

Definition: `generate/render/common.cppm:161`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`clore::generate::symbol_analysis_markdown_for` 根据页面计划的类型将符号分析结果分派到不同的 Markdown 生成路径。它首先检查 `plan.page_type` 是否为 `PageType::Namespace`，若是则调用 `analysis_overview_markdown` 生成命名空间级别的总览；否则调用 `analysis_details_markdown` 生成单个符号的详细分析。该函数依赖 `PagePlan`、`SymbolAnalysisStore` 和 `extract::SymbolInfo` 这三个核心输入类型，并通过内部分支函数 `analysis_overview_markdown` 与 `analysis_details_markdown` 完成实际的 Markdown 构建，后者自身又依赖 `SymbolDocView` 枚举和多个辅助工具（如 `build_list_section`、`push_link_paragraph` 等）来组合不同视图下的输出段落。控制流简单直接，但通过 `page_type` 的判定实现了两种截然不同的内容组织逻辑，体现了对模块化与关注点分离的设计意图。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `analyses` (const `SymbolAnalysisStore`&)
- `plan` (const `PagePlan`&)
- `sym` (const `extract::SymbolInfo`&)

#### Usage Patterns

- Called to produce symbol analysis markdown for a page
- Uses page type to choose between overview and detailed analysis

### `clore::generate::symbol_doc_view_for`

Declaration: `generate/render/common.cppm:269`

Definition: `generate/render/common.cppm:269`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数根据页面类型 `plan.page_type` 和符号类别 `sym.kind` 决定返回哪种 `SymbolDocView` 枚举值。核心逻辑是一个 `switch` 分支：若页面类型为 `PageType::Namespace`，则直接返回 `SymbolDocView::Declaration`；若为 `PageType::Module`，则调用 `is_variable_kind` 检查符号类型，变量返回 `SymbolDocView::Details`，否则返回 `SymbolDocView::Implementation`；其他所有情况均默认返回 `SymbolDocView::Details`。

实现仅依赖 `PageType` 枚举、`is_variable_kind` 辅助谓词以及 `SymbolDocView` 枚举定义。整个函数没有额外的循环或条件嵌套，控制流清晰直接，完全由计划页面类型主导，符号类型仅在模块页面中起次要筛选作用。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- plan`.page_type`
- sym`.kind`

#### Usage Patterns

- Determining symbol documentation view during page generation

### `clore::generate::trim_ascii`

Declaration: `generate/render/common.cppm:23`

Definition: `generate/render/common.cppm:23`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该实现通过两个独立的 `while` 循环移除 `text` 的前导和尾随空白字符。每个循环在每次迭代中调用 `std::isspace`（参数经 `static_cast<unsigned char>` 转换以避免未定义行为）检查首或尾字符，并分别通过 `remove_prefix(1)` 或 `remove_suffix(1)` 缩小 `std::string_view` 的范围。函数最终返回一个引用原字符串中非空白子序列的新视图，所有操作均不涉及内存分配或复制。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `text` parameter (a `std::string_view`)

#### Usage Patterns

- Used as a utility to sanitize or normalize text input before further processing
- Likely called by other generate functions to trim whitespace from strings

## Internal Structure

模块 `generate:common` 是文档生成层中的共享基础设施模块，专注于提供符号渲染所需的通用工具函数与类型。该模块公开了文本清洗（`trim_ascii`、`strip_inline_markdown`）、名称解析（`short_name_of`、`namespace_of`）、链接目标构造（`make_link_target`、`make_relative_link_target`、`make_source_link_target`、`LinkTarget` 结构体）、符号收集与过滤（`collect_namespace_symbols`、`collect_implementation_symbols`）、页面定位（`find_declaration_page`、`find_implementation_pages`、`find_module_for_file`、`build_related_page_targets`）以及文档片段生成（`build_symbol_link_list`、`build_symbol_source_locations`、`symbol_analysis_markdown_for`、`build_prompt_section` 等）等一系列功能。其内部通过无状态函数和少量值类型（如 `SymbolDocView` 枚举）实现，依赖 `extract`、`generate:model` 和 `generate:markdown` 模块提供的数据结构与渲染能力，并通过 `config` 模块获取配置参数。

在实现结构上，`generate:common` 遵循平面函数群组的设计：所有公共接口均声明于同一模块单元 `generate/render/common.cppm` 中，并通过显式导入外部模块实现解耦。模块内部没有复杂的类层次，而是以面向过程的风格将不同职责（如链接计算、符号收集、文档片段组装）拆分为独立的函数，每个函数处理具体的一小步变换。这种分层使得上层页面生成器可以按需组合这些工具，无需关心内部状态管理或全局数据流，从而保持整体架构的清晰与可测试性。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

