---
title: 'Module generate:symbol'
description: '该模块负责将代码分析结果中的符号（symbol）渲染为可直接发布的文档页面，是文档生成管线中符号层面的核心渲染层。它定义了符号文档的页面布局（PageDocLayout）和文档计划（SymbolDocPlan）两类关键数据结构，并通过一系列公开函数完成从符号模型到 Markdown 页面的全流程转化：包括根据符号构建页面计划与索引路径（build_page_doc_layout、find_doc_index_path），将符号内容追加为文档页面（append_symbol_doc_pages），为页面添加类型成员章节（append_type_member_sections）以及符号间导航链接（add_symbol_doc_links）。同时，它提供元数据规范化（normalize_frontmatter_title）和子页面支持检测（page_supports_symbol_subpages）等辅助接口，并通过 for_each_symbol_doc_group 支持外部对符号文档分组的高阶遍历。该模块依赖于 generate:model 提供的符号分析模型，并利用 generate:common、generate:markdown、generate:diagram 等子模块完成实际的内容编排与格式化输出。'
layout: doc
template: doc
---

# Module `generate:symbol`

## Summary

该模块负责将代码分析结果中的符号（symbol）渲染为可直接发布的文档页面，是文档生成管线中符号层面的核心渲染层。它定义了符号文档的页面布局（`PageDocLayout`）和文档计划（`SymbolDocPlan`）两类关键数据结构，并通过一系列公开函数完成从符号模型到 Markdown 页面的全流程转化：包括根据符号构建页面计划与索引路径（`build_page_doc_layout`、`find_doc_index_path`），将符号内容追加为文档页面（`append_symbol_doc_pages`），为页面添加类型成员章节（`append_type_member_sections`）以及符号间导航链接（`add_symbol_doc_links`）。同时，它提供元数据规范化（`normalize_frontmatter_title`）和子页面支持检测（`page_supports_symbol_subpages`）等辅助接口，并通过 `for_each_symbol_doc_group` 支持外部对符号文档分组的高阶遍历。该模块依赖于 `generate:model` 提供的符号分析模型，并利用 `generate:common`、`generate:markdown`、`generate:diagram` 等子模块完成实际的内容编排与格式化输出。

## Imports

- [`config`](../config/index.md)
- [`extract`](../extract/index.md)
- [`generate:common`](common.md)
- [`generate:diagram`](diagram.md)
- [`generate:markdown`](markdown.md)
- [`generate:model`](model.md)

## Imported By

- [`generate:page`](page.md)
- [`generate:scheduler`](scheduler.md)

## Types

### `clore::generate::PageDocLayout`

Declaration: `src/generate/render/symbol.cppm:37`

Definition: `src/generate/render/symbol.cppm:37`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该结构体将同一文档页面上所有符号文档计划按类别分组存储，是生成阶段与渲染阶段之间的数据传递单元。`type_docs`、`variable_docs` 和 `function_docs` 分别存放 `SymbolDocPlan` 向量，用于区分不同种类的符号；`index_paths` 是一个从索引键到目标文件路径的哈希映射，用于快速查找或去重建索引页条目。所有字段都通过默认初始化构造，没有引入额外的构造逻辑或自定义成员函数。内部不变量要求每个向量内的计划对象在语义上属于同一个文档页面，且 `index_paths` 中的键应是唯一的，以确保索引生成的确定性。

#### Invariants

- 每个向量可包含零个或多个 `SymbolDocPlan` 对象
- `index_paths` 的键和值均为字符串类型

#### Key Members

- `type_docs`
- `variable_docs`
- `function_docs`
- `index_paths`

#### Usage Patterns

- 在文档生成流水线中各模块填充这些字段
- 其他代码读取这些字段以渲染最终页面布局

### `clore::generate::SymbolDocPlan`

Declaration: `src/generate/render/symbol.cppm:31`

Definition: `src/generate/render/symbol.cppm:31`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

`SymbolDocPlan` 是一个递归数据结构，通过其 `children` 成员形成树形层次。每个节点存储一个指向 `extract::SymbolInfo` 的指针 `symbol`，用于携带提取阶段获得的符号元数据，以及一个 `index_path` 字符串，该字符串标识该符号在文档输出中的索引路径。`children` 向量允许嵌套子计划，从而对应命名空间、类或函数内的嵌套符号结构。在实现中，`symbol` 指针由外部对象管理生命周期，且通常保证非空（除非节点作为纯容器占位存在）；`index_path` 在遍历树时累积构造，确保每个节点具有唯一的文档内定位标识。

#### Invariants

- `symbol` 可能为 `nullptr`
- `children` 可能为空向量
- 每个 `children` 元素自身是一个有效的 `SymbolDocPlan`

#### Key Members

- `symbol`
- `index_path`
- `children`

#### Usage Patterns

- 用于文档生成流水线中表示符号及其子符号的计划
- 通过递归遍历 `children` 构建嵌套的文档结构
- 由上层模块填充 `symbol` 和 `index_path` 后传递给渲染阶段

## Functions

### `clore::generate::add_symbol_doc_links`

Declaration: `src/generate/render/symbol.cppm:61`

Definition: `src/generate/render/symbol.cppm:828`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数首先通过 `find_doc_index_path` 在给定的 `layout` 中查找与符号 `sym` 的 `qualified_name` 对应的索引路径。如果找到且该路径不等于 `current_page_path`，则调用 `make_link_target` 创建一个指向该索引的链接目标，其链接文本由 `doc_label(view)` 生成。最后，调用 `push_link_paragraph` 将前缀文本（`doc_label(view)` 后加冒号和空格）以及包含的链接目标追加到 `nodes` 向量中。内部控制流仅包含一个条件分支，依赖关系包括 `find_doc_index_path`、`make_link_target`、`doc_label` 和 `push_link_paragraph`。

#### Side Effects

- appends a paragraph to the `nodes` vector containing a link to the documentation index page for the given symbol, if such a page exists and is different from the current page

#### Reads From

- reads `current_page_path` parameter
- reads `layout` parameter via `find_doc_index_path`
- reads `sym.qualified_name` parameter
- reads `view` parameter via `doc_label`

#### Writes To

- appends a paragraph to the `nodes` vector reference

#### Usage Patterns

- called during documentation generation to add cross-reference links for symbols
- inserts a link to the symbol's documentation index page from other pages

### `clore::generate::append_symbol_doc_pages`

Declaration: `src/generate/render/symbol.cppm:78`

Definition: `src/generate/render/symbol.cppm:975`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数递归地为给定的符号文档计划集合生成分页文档。它遍历输入参数 `doc_plans` 中的每一个 `SymbolDocPlan` 对象：调用 `render_symbol_page` 渲染当前计划对应的页面，并将结果追加到输出容器 `pages` 中；随后对当前计划的子计划集合 `doc_plan.children` 进行递归调用 `append_symbol_doc_pages`，以生成嵌套的子页面。若任一渲染或递归步骤返回错误，函数立即提前返回对应的 `std::expected<void, RenderError>` 错误值；全部成功则返回空值。控制流表现为典型的前序深度优先遍历，依赖 `render_symbol_page` 完成单页生成，并依赖递归自身处理文档计划树中的嵌套结构。

#### Side Effects

- Appends `GeneratedPage` objects to the `pages` vector
- Returns `std::unexpected<RenderError>` on failure

#### Reads From

- `doc_plans` (vector of `SymbolDocPlan`)
- `owner_plan` (`PagePlan`)
- `config` (`config::TaskConfig`)
- `model` (`extract::ProjectModel`)
- `outputs` (map of strings)
- `analyses` (`SymbolAnalysisStore`)
- `links` (`LinkResolver`)
- `layout` (`PageDocLayout`)

#### Writes To

- `pages` vector (via `push_back`)
- Error state via `std::unexpected`

#### Usage Patterns

- Recursive traversal of symbol document plans
- Called during page generation to build symbol documentation pages
- Used in `generate_pages` or similar page-building functions

### `clore::generate::append_type_member_sections`

Declaration: `src/generate/render/symbol.cppm:67`

Definition: `src/generate/render/symbol.cppm:842`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数首先检查传入的符号 `sym` 是否为类型；若 `is_type_kind(sym.kind)` 为 false 则立即返回。否则，它会连续三次调用 `append_member_section`，每次传入不同的标题字符串和通过 `collect_member_symbols` 收集到的子符号集合。每次收集时使用一个谓词 lambda 来筛选成员：分别筛选出 `is_type_kind`（成员类型）、`is_variable_kind`（成员变量）和 `is_function_kind`（成员函数）的子符号。这三次调用共享相同的 `nodes`、`config`、`model`、`analyses`、`plan`、`links`、`layout`、`current_page_path` 和 `level` 参数，从而在文档节点树中依次追加对应的类型成员、变量成员和函数成员段落。

#### Side Effects

- Appends `MarkdownNode` entries to the `nodes` vector

#### Reads From

- `nodes` (`std::vector<MarkdownNode>`&)
- `sym` (const `extract::SymbolInfo`&)
- `config` (const `config::TaskConfig`&)
- `model` (const `extract::ProjectModel`&)
- `analyses` (const `SymbolAnalysisStore`&)
- `plan` (const `PagePlan`&)
- `links` (const `LinkResolver`&)
- `layout` (const `PageDocLayout`&)
- `current_page_path` (`std::string_view`)
- `level` (`std::uint8_t`)
- `collect_member_symbols` (reads model and sym)
- `is_type_kind`, `is_variable_kind`, `is_function_kind` (reads sym`.kind`)

#### Writes To

- `nodes` vector (via `append_member_section`)

#### Usage Patterns

- Called during page generation for a type symbol to add subsections listing its members
- Used by higher‑level generation functions such as `append_symbol_doc_pages`
- Invoked once per type symbol in documentation rendering

### `clore::generate::build_page_doc_layout`

Declaration: `src/generate/render/symbol.cppm:55`

Definition: `src/generate/render/symbol.cppm:915`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数按以下步骤构建 `PageDocLayout`：首先通过 `page_supports_symbol_subpages` 检查页面是否支持符号子页面，若不支持则返回空布局。接下来调用 `page_directory_of` 获取基础目录，若目录为空仍返回空布局。然后根据 `plan.page_type` 分别调用 `collect_namespace_symbols` 或 `collect_implementation_symbols` 收集该页面所有符号。接着遍历这些符号，依据 `is_type_kind`、`is_variable_kind`、`is_function_kind` 将其分类到 `type_symbols`、`variable_symbols`、`function_symbols` 三个向量中。最后为每类符号调用 `build_symbol_doc_plans` 生成对应的文档计划，并填充 `layout.type_docs`、`layout.variable_docs`、`layout.function_docs`；再通过 `for_each_symbol_doc_group` 迭代所有文档组，对每个计划调用 `register_symbol_doc_plan` 进行注册，最终返回完整布局。该函数依赖大量辅助函数和数据结构，如 `PagePlan`、`SymbolInfo`、`SymbolDocPlan` 以及收集、分类和注册操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- const `PagePlan` & plan (含 `relative_path`, `page_type`, `owner_keys`)
- const `extract::ProjectModel` & model

#### Usage Patterns

- 在页面生成流程中用于构建符号文档布局
- 根据页面类型和模型收集子符号并生成文档方案

### `clore::generate::find_doc_index_path`

Declaration: `src/generate/render/symbol.cppm:58`

Definition: `src/generate/render/symbol.cppm:822`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::find_doc_index_path` 实现了在给定的 `PageDocLayout` 对象的 `index_paths` 映射中查找符号文档索引路径的逻辑。它接受一个 `std::string_view` 表示的完全限定名称，构造一个临时的 `std::string` 键，并在 `layout.index_paths`（通常是一个 `std::unordered_map`）中执行哈希查找。若找到匹配项，则返回指向对应值（`std::string`）的指针；否则返回 `nullptr`。

此函数是整个文档生成流程中符号到索引路径解析的关键环节，底层依赖 `PageDocLayout` 结构及其 `index_paths` 字段。算法复杂度为期望常数时间，主要由哈希映射的查找操作决定。函数不修改 `layout` 状态，仅执行只读查询。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- layout`.index_paths`
- `qualified_name`

#### Usage Patterns

- lookup doc index path for a qualified name
- used during page rendering to map symbols to their index paths

### `clore::generate::for_each_symbol_doc_group`

Declaration: `src/generate/render/symbol.cppm:45`

Definition: `src/generate/render/symbol.cppm:45`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

函数 `clore::generate::for_each_symbol_doc_group` 实现了一个简单的遍历机制，用于将传入的 `Visitor` 对象依次应用于 `PageDocLayout` 中按分类存储的符号文档组。其内部控制流由三个连续的 `visitor` 调用组成：首先处理 `layout.type_docs`，然后处理 `layout.variable_docs`，最后处理 `layout.function_docs`。该实现不包含分支或循环，完全依赖调用方提供的 `visitor` 来执行实际的文档生成或收集操作。依赖方面，函数仅依赖于 `PageDocLayout` 结构体的这些字段的存储布局，无其他外部函数或状态。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- layout`.type_docs`
- layout`.variable_docs`
- layout`.function_docs`
- visitor callable

#### Usage Patterns

- Iterate over symbol doc groups in a page layout
- Apply a visitor to each doc group
- Process type, variable, and function documentation groups

### `clore::generate::normalize_frontmatter_title`

Declaration: `src/generate/render/symbol.cppm:51`

Definition: `src/generate/render/symbol.cppm:903`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

Implementation: [Implementation](functions/normalize-frontmatter-title.md)

该函数先调用 `strip_inline_markdown` 从 `page_title` 中去除内联 Markdown 标记（例如 `**bold**`、`` `code` ``），得到纯文本结果 `plain`。若 `plain` 非空，则直接将其返回，这是为了优先使用去除了格式后仍有实际内容的标题。若 `plain` 为空（即原字符串完全由 Markdown 标记构成或者本身就是空白），则回退到 `trim_ascii` 处理原始 `page_title`，去除首尾 ASCII 空白后返回。整个控制流以一次条件判断和两种清理策略为基础，依赖的两个辅助函数 `strip_inline_markdown` 和 `trim_ascii` 均在相同匿名命名空间内定义，前者负责 Markdown 语法解析，后者仅处理空白修剪。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `page_title`

#### Writes To

- returned `std::string` value

#### Usage Patterns

- used by `build_symbol_frontmatter` to produce a plain title for frontmatter

### `clore::generate::page_supports_symbol_subpages`

Declaration: `src/generate/render/symbol.cppm:53`

Definition: `src/generate/render/symbol.cppm:911`

Declaration: [`Namespace clore::generate`](../../namespaces/clore/generate/index.md)

该函数根据传入的 `PagePlan` 对象中的 `page_type` 字段判断页面是否支持子符号页面：如果 `page_type` 等于 `PageType::Namespace` 或 `PageType::Module`，则返回 `true`。它没有复杂的控制流或外部依赖，仅执行一次常量级比较，用于在调用方（如 `build_symbol_doc_plans` 或 `render_symbol_page` 等函数）中决定是否递归生成子符号文档页面。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- plan`.page_type`

#### Usage Patterns

- Check if a page plan is for a namespace or module to enable symbol subpages

## Internal Structure

该模块是文档生成管线的核心环节，专门负责为单个符号（类型、函数、变量等）渲染完整的文档页面。它作为 `generate:model` 与 `generate:markdown` 之间的桥梁，将符号分析结果与页面布局结构转换为实际的 Markdown 输出。通过导入 `config`、`extract` 及 `generate:common` 等模块，它获取配置、符号元数据以及通用渲染工具，并利用 `generate:diagram` 生成结构图。内部实现了多层次的分解：使用 `SymbolDocPlan` 和 `PageDocLayout` 数据结构组织符号的索引路径、子页面计划与文档组；通过匿名命名空间中的辅助函数（如文档计划构建、子页面渲染、成员章节追加、关系链接生成等）实现渲染流水线。调用方通过公共接口（如 `append_symbol_doc_pages`、`append_type_member_sections`）触发渲染，同时支持符号子页面与前端元数据规范化。整个实现遵循“计划‑构建‑渲染”的分阶段模式，保持内部层次清晰且各步骤可独立测试。

## Related Pages

- [Module config](../config/index.md)
- [Module extract](../extract/index.md)
- [Module generate:common](common.md)
- [Module generate:diagram](diagram.md)
- [Module generate:markdown](markdown.md)
- [Module generate:model](model.md)

