---
title: 'Module extract:model'
description: '模块 extract:model 定义了代码提取过程产生的结构化数据模型，是整个提取系统的核心数据层。它公开了 ProjectModel、SymbolInfo、FileInfo、NamespaceInfo、ModuleUnit、SymbolID、SourceLocation、SourceRange 以及 SymbolKind 枚举等类型，用于表示项目、符号、文件、模块单元及其关系。这些类型以只读方式提供对提取结果的访问，调用者可通过 find_symbol、lookup_symbol、find_module_by_name、find_module_by_source 等公开函数按标识符或源文件索引进行查找，以及通过 resolve_source_snippet 延迟加载源码片段。此外，模块还提供了 split_top_level_qualified_name、join_qualified_name_parts、namespace_prefix_from_qualified_name 等名称处理工具，以支持符号树的遍历和分析。'
layout: doc
template: doc
---

# Module `extract:model`

## Summary

模块 `extract:model` 定义了代码提取过程产生的结构化数据模型，是整个提取系统的核心数据层。它公开了 `ProjectModel`、`SymbolInfo`、`FileInfo`、`NamespaceInfo`、`ModuleUnit`、`SymbolID`、`SourceLocation`、`SourceRange` 以及 `SymbolKind` 枚举等类型，用于表示项目、符号、文件、模块单元及其关系。这些类型以只读方式提供对提取结果的访问，调用者可通过 `find_symbol`、`lookup_symbol`、`find_module_by_name`、`find_module_by_source` 等公开函数按标识符或源文件索引进行查找，以及通过 `resolve_source_snippet` 延迟加载源码片段。此外，模块还提供了 `split_top_level_qualified_name`、`join_qualified_name_parts`、`namespace_prefix_from_qualified_name` 等名称处理工具，以支持符号树的遍历和分析。

## Imports

- `std`
- [`support`](../support/index.md)

## Imported By

- [`extract:ast`](ast.md)
- [`extract:merge`](merge.md)

## Types

### `clore::extract::FileInfo`

Declaration: `extract/model.cppm:122`

Definition: `extract/model.cppm:122`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::FileInfo` 是一个纯数据聚合体，其内部直接暴露三个公开字段以保存单个源文件的提取结果。`path` 存储文件的磁盘路径；`symbols` 是一个 `std::vector<SymbolID>`，记录了在该文件中定义的符号标识符；`includes` 是一个 `std::vector<std::string>`，列出该文件直接包含的头文件路径。整个结构体没有封装或继承，所有成员均为公开且可直接赋值，因此不变量仅限于标准库容器自身的合法状态（如字符串非空、向量元素有效）。由于不存在私有数据或访问控制，该类仅作为携带相关信息的简单记录，外部代码可直接修改其字段以适应不同的构造场景。

#### Invariants

- 成员 `path` 为 `std::string` 类型，无隐含约束
- 成员 `symbols` 为 `std::vector<SymbolID>` 类型，表示可能为空的符号列表
- 成员 `includes` 为 `std::vector<std::string>` 类型，表示可能为空的包含路径列表

#### Key Members

- `path`
- `symbols`
- `includes`

#### Usage Patterns

- 用于表示提取操作的结果，将文件路径与其符号和包含项关联起来
- 可能被传递给其他处理函数或序列化

### `clore::extract::ModuleUnit`

Declaration: `extract/model.cppm:135`

Definition: `extract/model.cppm:135`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ModuleUnit` 的内部表示围绕五个核心字段展开，用于描述一个 C++20 模块单元（接口或分区）。`name` 存储完整的模块名（如 `"foo"` 或 `"foo:bar"`），而 `is_interface` 是一个布尔标志：当其值为 `true` 时表示该单元是 `export module` 导出的接口单元，为 `false` 时则为普通实现单元（`module` 声明）。`source_file` 记录归一化的源文件路径，`imports` 是一个字符串向量，列出该单元导入的所有外部模块名。`symbols` 则持有当前单元中声明的所有符号标识符（`SymbolID` 向量），是实现提取模块符号表的核心枢纽。这四个数据成员共同构成一个完整的模块单元抽象，其中 `name` 与 `source_file` 通常应保持非空，`is_interface` 的正确性依赖于解析阶段的判定，而 `imports` 和 `symbols` 的内容则分别由模块导入收集和符号提取过程填充，保证了模块间依赖与符号表的可追踪性。

### `clore::extract::NamespaceInfo`

Declaration: `extract/model.cppm:128`

Definition: `extract/model.cppm:128`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::NamespaceInfo` 是一个普通聚合，用于存储提取过程中单个命名空间的元数据。它的三个公有字段分别承担不同角色：`name` 持有命名空间的完整限定名（例如 `std::chrono`），`symbols` 记录该命名空间内直接声明的符号标识符（类型为 `SymbolID`），`children` 则按顺序列出其直接子命名空间的名称字符串。该结构体不维护额外的不变量——所有字段均可自由读写，且无特殊构造函数或成员函数，其完整性完全由调用者保证。重要的是，`children` 中的字符串与 `name` 共同构成子命名空间的全名，但此处仅存储相对名称。

#### Invariants

- `name` 是命名空间的标识字符串
- `symbols` 和 `children` 可能为空向量
- `symbols` 中的每个元素是有效的 `SymbolID` 值

#### Key Members

- `name`
- `symbols`
- `children`

#### Usage Patterns

- 在提取过程中由相关逻辑填充
- 在文档生成或其他下游处理中遍历 `symbols` 和 `children` 以构建命名空间树

### `clore::extract::ProjectModel`

Declaration: `extract/model.cppm:143`

Definition: `extract/model.cppm:143`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ProjectModel` 是提取过程的核心数据聚合体，将解构后的源码信息组织为相互关联的索引结构。内部由多个无序映射构成：`symbols` 按 `SymbolID` 存储所有声明信息，`namespaces` 和 `files` 分别按名称和路径索引命名空间与文件条目；`modules` 以规范化源文件路径为键存储模块单元信息，并利用透明字符串散列实现高效的路径查询。为支持生成与证据构建，`symbol_ids_by_qualified_name` 维护完全限定名到符号 ID 集合的映射（以容纳重载），而 `module_name_to_sources` 则记录模块名对应的源文件列表。`file_order` 向量忠实记录文件处理顺序，与 `files` 映射的键集合一一对应，确保迭代稳定性。`uses_modules` 布尔标志作为整体不变量，当检测到至少一个模块声明时置为 true，从而允许后续代码根据不同编译模型选择处理路径。这些字段共同维护一组隐含一致性约束：所有引用到的路径和名称必须在各自映射中有对应条目，且 `file_order` 不会包含 `files` 中不存在的键。

#### Invariants

- `symbols` 中的每个 `SymbolID` 映射到唯一 `SymbolInfo`。
- `files` 和 `modules` 的键是标准化源文件路径。
- `symbol_ids_by_qualified_name` 中一个限定名可对应多个 `SymbolID`（存在重载时）。
- `module_name_to_sources` 中一个模块名可对应多个源文件路径。
- `uses_modules` 为 `true` 当且仅当项目中发现至少一个模块声明。

#### Key Members

- `symbols`
- `files`
- `namespaces`
- `modules`
- `symbol_ids_by_qualified_name`
- `module_name_to_sources`
- `file_order`
- `uses_modules`

#### Usage Patterns

- 作为 `clore::extract::ProjectExtractor` 的输出结果，填充后传递给代码生成阶段。
- 其他组件通过 `symbol_ids_by_qualified_name` 进行符号的精确限定名查找。
- 模块依赖分析和交叉链接依赖 `modules` 和 `module_name_to_sources`。
- 文档生成、代码补全等功能读取 `symbols`、`namespaces` 等字段获取项目信息。

### `clore::extract::SourceLocation`

Declaration: `extract/model.cppm:64`

Definition: `extract/model.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该结构体通过 `line` 字段的零值约定来表示未知或无效源代码位置；当 `line` 为 `0` 时，`column` 和 `file` 的内容不具有语义意义。内置的 `is_known()` 成员函数直接检查此不变量，返回 `line != 0`，它被标记为 `noexcept` 且 `[[nodiscard]]`，以确保调用方不会忽略位置的有效性。

#### Member Functions

##### `clore::extract::SourceLocation::is_known`

Declaration: `extract/model.cppm:70`

Definition: `extract/model.cppm:70`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_known() const noexcept {
        return line != 0;
    }
```

### `clore::extract::SourceRange`

Declaration: `extract/model.cppm:75`

Definition: `extract/model.cppm:75`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::SourceRange` 是一个仅包含两个数据成员的结构体：`begin` 和 `end`，类型均为 `SourceLocation`。它将一个连续的源码位置区间表示为半开区间 `[begin, end)`，其中 `begin` 指向第一个字符，`end` 指向最后一个字符之后的位置。该结构体本身不提供构造或验证逻辑，因此调用者负责保证 `begin` 在 `end` 之前（或相等），否则范围语义将无效。

作为简单的聚合体，`SourceRange` 的两个成员直接暴露，没有额外的封装或成员函数。不变量完全由创建它的代码维护，例如在解析或词法分析过程中，通过逐字符推进位置来构造有效的区间。

#### Invariants

- `begin` 和 `end` 分别是范围的起始和结束位置
- 范围通常是左闭右开或左右均包含，但具体语义由使用上下文决定

#### Key Members

- `begin`：起始位置的 `SourceLocation`
- `end`：结束位置的 `SourceLocation`

#### Usage Patterns

- 用于表示解析器、词法分析器或错误报告中的源代码区间
- 作为其他结构体或函数的成员，传递代码片段范围

### `clore::extract::SymbolID`

Declaration: `extract/model.cppm:28`

Definition: `extract/model.cppm:28`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该结构体直接编码了一对整数作为标识符：一个64位 `hash` 和一个32位 `signature`，其中 `hash` 为0被保留为无效哨兵。`is_valid()` 通过检查 `hash` 是否非零来实现这一不变量；一旦构造完成，有效ID的 `hash` 和 `signature` 均不应被单独改变。默认的 `operator==` 和 `operator<=>` 对这两个字段进行逐成员比较，因此比较顺序是先 `hash` 后 `signature`。这种设计使极罕见的64位哈希碰撞可以通过 `signature` 进一步区分，而无需依赖外部映射或更宽的哈希值。

#### Invariants

- 有效 `SymbolID` 的 `hash` 必须非零
- `hash` 为 0 时 `signature` 应被忽略，是无效哨兵

#### Key Members

- `hash`: 主要哈希值，非零表示有效
- `signature`: 附加签名，用于消除哈希碰撞
- `is_valid()`: 通过检查 `hash != 0` 判断是否有效
- `operator==` 和 `operator<=>`: 默认实现的比较操作

#### Usage Patterns

- 作为符号的唯一键在集合或映射中使用
- 通过 `is_valid()` 快速判断标识是否初始化
- 依赖默认比较进行排序和去重

#### Member Functions

##### `clore::extract::SymbolID::is_valid`

Declaration: `extract/model.cppm:35`

Definition: `extract/model.cppm:35`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_valid() const noexcept {
        return hash != 0;
    }
```

##### `clore::extract::SymbolID::operator<=>`

Declaration: `extract/model.cppm:40`

Definition: `extract/model.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
auto operator<=>(const SymbolID&) const = default
```

##### `clore::extract::SymbolID::operator==`

Declaration: `extract/model.cppm:39`

Definition: `extract/model.cppm:39`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
bool operator==(const SymbolID&) const = default
```

### `clore::extract::SymbolInfo`

Declaration: `extract/model.cppm:80`

Definition: `extract/model.cppm:80`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::SymbolInfo` 是一个复合数据结构，用于存储单个符号的提取元数据。其内部包含符号标识符 `id`、类型 `kind`、多个名称字段（如 `name`、`qualified_name`、`enclosing_namespace`、`signature`），以及源代码位置信息 `declaration_location` 和可选的定义位置 `definition_location`。文档注释 `doc_comment` 和源代码片段 `source_snippet` 直接存储文本内容；当 `source_snippet` 为空时，实际源码文本按需从 `declaration_location` 指向的文件中获取，位置与完整性由 `source_snippet_offset`、`source_snippet_length`、`source_snippet_file_size` 和 `source_snippet_hash` 共同维护。关系字段（`parent`、`children`、`bases`、`derived`、`calls`、`called_by`、`references`、`referenced_by`）通过 `SymbolID` 列表构建符号图的邻接结构，其中 `parent` 为可选值，`lexical_parent_name` 与 `lexical_parent_kind` 记录了父对象的字符串标识和类型以备快速查询。访问控制 `access`、模板状态 `is_template` 及参数 `template_params` 补充了符号的上下文属性。

该结构的不变性主要体现在：当 `source_snippet` 为空时，所有四个 `source_snippet_*` 字段必须同时有效且对应 `declaration_location.file` 中的原始文本区间；关系向量中引用的 `SymbolID` 应存在于同一提取集合中（外部保证），`parent` 与 `children` 之间双向一致由调用方维护。`is_template` 为 `true` 时，`template_params` 包含模板参数列表的字符串表示；`kind` 决定了其他字段（如 `bases`、`access`）的实际含义，但类型安全由外部逻辑保证。

#### Invariants

- `id` is a unique identifier for the symbol
- `kind` defaults to `SymbolKind::Unknown`
- `declaration_location` always stores a valid `SourceLocation`
- If `source_snippet` is empty, the snippet text can be retrieved from the file indicated by `declaration_location.file` using the offset/length/`file_size`/hash fields
- `parent` is empty for top-level symbols; `children` lists immediate child symbols
- `bases` and `derived` are only populated for class types
- `calls` and `called_by` are used for functions and callable objects
- `references` and `referenced_by` indicate cross-references between symbols

#### Key Members

- `id`: unique identifier for the symbol
- `kind`: enum indicating the symbol type
- `name`: the simple name of the symbol
- `qualified_name`: fully qualified name
- `declaration_location`: `SourceLocation` of the declaration
- `parent`: optional `SymbolID` of the enclosing symbol
- `children`: list of direct child `SymbolID`s
- `bases` and `derived`: inheritance relationships
- `calls` and `called_by`: call graph edges
- `references` and `referenced_by`: reference graph edges
- `source_snippet_offset`, `source_snippet_length`, `source_snippet_file_size`, `source_snippet_hash`: lazy snippet resolution metadata

#### Usage Patterns

- Filled by the extraction engine when traversing translation units
- Used by documentation generators to render symbol pages
- Consumed by graph analysis tools to build dependency or call trees
- Referenced by other structures via `SymbolID` lists (e.g. `children`, `bases`, `calls`)
- The optional lazy-snippet fields allow on-demand reading of source text to reduce memory footprint

### `clore::extract::SymbolKind`

Declaration: `extract/model.cppm:8`

Definition: `extract/model.cppm:8`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

枚举 `clore::extract::SymbolKind` 是一个底层类型为 `std::uint8_t` 的枚举类，其成员按编译单元中符号种类的自然分类顺序排列：从 `Namespace` 开始，经 `Class`、`Struct`、`Union`、`Enum` 等类型实体，再到 `Function`、`Method`、`Variable`、`Field` 等值实体，最后是 `TypeAlias`、`Macro`、`Template`、`Concept` 以及作为后备值的 `Unknown`。各成员均未显式指定数值，其隐式赋值从 0 递增，因此枚举值的具体数值与其声明顺序严格对应。内部实现中，`SymbolKind` 通过窄底层类型实现紧凑存储，适合作为分析管线中频繁传递的轻量级标签。`Unknown` 成员作为哨兵值处理未识别或超出预定义类别的符号，维护了枚举的完整性。

#### Member Variables

##### `clore::extract::SymbolKind::Class`

Declaration: `extract/model.cppm:10`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Class
```

##### `clore::extract::SymbolKind::Concept`

Declaration: `extract/model.cppm:22`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Concept
```

##### `clore::extract::SymbolKind::Enum`

Declaration: `extract/model.cppm:13`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Enum
```

##### `clore::extract::SymbolKind::EnumMember`

Declaration: `extract/model.cppm:14`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
EnumMember
```

##### `clore::extract::SymbolKind::Field`

Declaration: `extract/model.cppm:18`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Field
```

##### `clore::extract::SymbolKind::Function`

Declaration: `extract/model.cppm:15`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Function
```

##### `clore::extract::SymbolKind::Macro`

Declaration: `extract/model.cppm:20`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Macro
```

##### `clore::extract::SymbolKind::Method`

Declaration: `extract/model.cppm:16`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Method
```

##### `clore::extract::SymbolKind::Namespace`

Declaration: `extract/model.cppm:9`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Namespace
```

##### `clore::extract::SymbolKind::Struct`

Declaration: `extract/model.cppm:11`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Struct
```

##### `clore::extract::SymbolKind::Template`

Declaration: `extract/model.cppm:21`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Template
```

##### `clore::extract::SymbolKind::TypeAlias`

Declaration: `extract/model.cppm:19`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
TypeAlias
```

##### `clore::extract::SymbolKind::Union`

Declaration: `extract/model.cppm:12`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Union
```

##### `clore::extract::SymbolKind::Unknown`

Declaration: `extract/model.cppm:23`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Unknown
```

##### `clore::extract::SymbolKind::Variable`

Declaration: `extract/model.cppm:17`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Variable
```

## Functions

### `clore::extract::find_module_by_name`

Declaration: `extract/model.cppm:188`

Definition: `extract/model.cppm:416`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先委托给 `clore::extract::find_modules_by_name`，获取所有名称匹配 `module_name` 的 `ModuleUnit` 指针列表。若列表为空，直接返回 `nullptr`；若仅有一个候选，则直接返回该指针。当存在多个同名模块时，算法遍历所有候选项，统计其 `is_interface` 标志为 `true` 的接口单元数量。若恰好有一个接口单元，则返回该单元；若没有任何接口单元，记录一条警告后回退到列表中的第一个模块作为结果；若多于一个接口单元，则认为模块名称存在歧义，记录另一条警告并返回 `nullptr`。该函数依赖 `clore::extract::find_modules_by_name` 完成初步检索，并利用 `ModuleUnit` 结构体的 `is_interface` 字段进行接口单元识别，从而在有多份同名模块定义时优先返回明确的接口单元。

#### Side Effects

- Logs warnings via `logging::warn` when multiple modules are found with the same name and no interface or multiple interfaces.

#### Reads From

- `model` parameter
- `module_name` parameter
- `is_interface` member of `ModuleUnit` objects returned by `find_modules_by_name`

#### Usage Patterns

- Resolve module ambiguity by preferring interface units
- Lookup module by name in project model
- Fallback to first unit when no interface

### `clore::extract::find_module_by_source`

Declaration: `extract/model.cppm:194`

Definition: `extract/model.cppm:449`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过给定源文件路径在 `ProjectModel::modules` 映射中查找对应的 `ModuleUnit` 实例。其内部流程非常简单：使用 `std::map::find` 在 `model.modules` 中搜索键为 `source_file` 的条目，若找到则返回关联值的指针，否则返回 `nullptr`。实现依赖 `ProjectModel` 类型中定义的 `modules` 成员（一个以源文件路径为键、`ModuleUnit` 为值的映射）以及 `ModuleUnit` 的完整定义。整个查找过程的时间复杂度为对数级别（取决于映射的底层实现）。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.modules` (the map inside `ProjectModel`)
- `source_file` parameter

#### Usage Patterns

- Used to retrieve a module unit by its source file path during extraction or analysis.

### `clore::extract::find_modules_by_name`

Declaration: `extract/model.cppm:191`

Definition: `extract/model.cppm:395`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数根据传入的模块名 `module_name` 在 `model.module_name_to_sources` 映射中执行查找操作。若未命中或关联的源文件列表为空，则立即返回空向量。否则，遍历源文件列表并对每个元素调用 `find_module_by_source` 获取对应的 `ModuleUnit` 指针，跳过返回空指针的情形。结果容器按每个模块的 `source_file` 字段升序排序后返回。实现仅依赖 `ProjectModel` 的内部映射和辅助函数 `find_module_by_source`，无额外全局状态。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.module_name_to_sources` map
- `find_module_by_source` function (reads the model and source ID)
- `lhs->source_file` and `rhs->source_file` for comparison during sorting

#### Usage Patterns

- Used to find all modules with a specific name in the project model
- Called during extraction to collect module units for processing

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:179`

Definition: `extract/model.cppm:371`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::find_symbol` 是 `clore::extract::find_symbols` 的简化封装。它接收一个 `qualified_name` 并调用 `find_symbols` 来获取所有匹配的符号集合；若集合大小恰好为 1，则返回该符号的指针，否则返回空指针。该函数不涉及额外的算法或状态维护，完全依赖 `find_symbols` 的查找逻辑和结果过滤。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model: `const ProjectModel &`
- `qualified_name`: `std::string_view`

#### Usage Patterns

- single symbol lookup by qualified name
- convenience wrapper around `clore::extract::find_symbols`
- returns `nullptr` when no unique symbol is found

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:181`

Definition: `extract/model.cppm:379`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

当 `signature` 非空时，该重载首先通过 `find_symbols` 获得与 `qualified_name` 匹配的所有 `SymbolInfo*` 的列表，然后遍历该列表，将每个符号的 `signature` 字段与传入的 `signature` 进行精确比较，一旦找到相同者即立即返回该指针；若遍历结束仍未匹配，则返回 `nullptr`。如果 `signature` 为空字符串，则直接委托给单参数的 `find_symbol` 重载完成查找。该函数依赖内部辅助函数 `find_symbols` 完成初步的名称‑符号映射，并依赖 `SymbolInfo` 的 `signature` 成员进行签名比对。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (parameter of type `const ProjectModel&`)
- `qualified_name` (parameter of type `std::string_view`)
- `signature` (parameter of type `std::string_view`)
- data accessed via `find_symbols` and `symbol->signature`

#### Usage Patterns

- Used to look up a symbol by both its qualified name and exact signature, typically to disambiguate overloaded names.
- Called internally by other extraction logic that needs to identify a specific symbol instance.

### `clore::extract::find_symbols`

Declaration: `extract/model.cppm:185`

Definition: `extract/model.cppm:354`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数`clore::extract::find_symbols`以有序查找的方式实现：首先在`model.symbol_ids_by_qualified_name`中搜索给定的`qualified_name`，若未命中则立即返回空向量；命中时预分配容量，然后遍历该键对应的每个`symbol_id`，通过`lookup_symbol(model, symbol_id)`获取指向`SymbolInfo`的指针（可能为空），有效的指针被收集到`matches`中。整个过程依赖`ProjectModel`内部的名称到ID映射结构以及`lookup_symbol`提供的符号解析服务，没有额外的排序或去重逻辑，输出顺序由映射中ID的存储顺序决定。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.symbol_ids_by_qualified_name`
- `lookup_symbol` 通过 `model` 和 `symbol_id` 读取符号信息

#### Usage Patterns

- 按限定名称查找所有符号实例
- 供符号搜索或重命名等工具使用

### `clore::extract::join_qualified_name_parts`

Declaration: `extract/model.cppm:59`

Definition: `extract/model.cppm:328`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::join_qualified_name_parts` 负责将限定名称的各部分重新拼接为完整的限定名称字符串。它接受一个 `parts` 向量（包含名称的各个片段）和一个 `count` 参数，仅拼接前 `count` 个片段，但通过 `safe_count` 保证不超过 `parts` 的实际大小，从而避免越界。循环从索引 0 开始，对非首元素在拼接前插入 `"::"` 分隔符，最终将结果累积到 `joined` 字符串中并返回。整个实现仅依赖标准库的 `std::vector` 和 `std::string`，没有外部依赖，算法简单直接，专注于安全地截取并连接路径片段。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `parts` 参数
- `count` 参数

#### Usage Patterns

- 构建命名空间前缀或完全限定类型名称

### `clore::extract::lookup_symbol`

Declaration: `extract/model.cppm:177`

Definition: `extract/model.cppm:349`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::lookup_symbol` 的实现基于直接查找 `ProjectModel` 中的 `symbols` 容器。它接受一个 `SymbolID` 参数，在 `model.symbols` 上调用 `find` 方法，若找到则返回指向对应 `SymbolInfo` 的指针，否则返回 `nullptr`。底层容器的查找性能取决于其类型（通常为平衡树或哈希表），因此该函数提供对数或期望常数时间的符号解析能力。

该函数是整个提取结果中按标识符快速定位符号的核心入口，其依赖仅包括 `ProjectModel` 的 `symbols` 成员和 `SymbolID` 的相等比较操作。由于实现极为精简，它不涉及额外的状态检查或复合逻辑，仅执行一次容器查找并返回结果。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model`.symbols`
- id

#### Usage Patterns

- check if a symbol exists in the model
- retrieve symbol info for a given ID
- used in symbol resolution and caching logic

### `clore::extract::namespace_prefix_from_qualified_name`

Declaration: `extract/model.cppm:62`

Definition: `extract/model.cppm:341`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过委托给 `clore::extract::split_top_level_qualified_name` 将输入的 `qualified_name` 拆分为段。若结果段数不超过 1，则认为无前缀，直接返回空字符串。否则，调用 `clore::extract::join_qualified_name_parts` 将所有除最后一个段之外的段重新连接，从而提取出命名空间前缀部分。内部逻辑仅依赖这两个辅助函数，无直接复杂循环或容器操作，整体控制流清晰且线性。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- parameter `qualified_name`

#### Usage Patterns

- Extract namespace prefix before a symbol name
- Check if a qualified name has a namespace prefix
- Prepare namespace scope for name resolution

### `clore::extract::resolve_source_snippet`

Declaration: `extract/model.cppm:200`

Definition: `extract/model.cppm:455`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先通过 `source_snippet_mutex_for` 获取与符号关联的互斥锁，确保线程安全。然后执行一系列快速短路检查：若 `sym.source_snippet` 已非空则直接返回 `true`；若 `sym.source_snippet_length` 为零或 `sym.declaration_location.file` 为空则返回 `false`。接着利用 `std::filesystem::file_size` 获取文件大小，若 `sym.source_snippet_file_size` 非零且与当前文件大小不一致、或读取偏移量与长度超出文件范围，均返回失败。成功通过边界验证后，以二进制模式打开文件，使用 `seekg` 定位到偏移量处并读取指定长度的字节。若读取失败或计算出的字节哈希与 `sym.source_snippet_hash` 不匹配（当该哈希非零时），仍返回 `false`。最后，将读出的原始字节中的 `\r\n` 序列标准化为 `\n`，并将结果存入 `sym.source_snippet` 并返回 `true`。该过程依赖 `std::filesystem`、`std::ifstream` 以及匿名命名空间中的辅助函数 `hash_source_snippet_bytes` 和 `source_snippet_mutex_for`。

#### Side Effects

- Modifies the `source_snippet` member of the passed `SymbolInfo`
- Performs file I/O to read the source file
- Allocates and manages memory for the snippet string

#### Reads From

- `sym.source_snippet`
- `sym.source_snippet_length`
- `sym.declaration_location.file`
- `sym.source_snippet_offset`
- `sym.source_snippet_file_size`
- `sym.source_snippet_hash`
- file system via `fs::file_size` and `ifstream` read

#### Writes To

- `sym.source_snippet`

#### Usage Patterns

- Populating source snippet for `SymbolInfo`
- Used before accessing `sym.source_snippet`
- Conditional resolution of snippet from disk

### `clore::extract::split_top_level_qualified_name`

Declaration: `extract/model.cppm:57`

Definition: `extract/model.cppm:265`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先通过调用 `split_qualified_name_cache()` 获取一个线程安全的静态缓存对象，并尝试以共享锁查找 `qualified_name` 的已存储结果；若缓存命中则直接返回。缓存未命中时，函数手动解析字符串：它遍历字符，维护一个 `template_depth` 计数器，在遇到 `<` 时递增，遇到 `>` 且深度大于零时递减；只有当 `template_depth` 为零且遇到连续两个冒号 `::` 时，才会将当前累积的片段压入结果向量并清空缓冲区。解析完成后，若结果的首片段为空字符串（例如输入以 `::` 开头时的前导空片段）则将其移除。最后，在获得结果向量后，函数以唯一锁更新缓存：若缓存条目数达到 `kSplitQualifiedNameCacheMaxEntries` 则清空整个缓存，然后插入或覆盖当前结果并返回。该实现依赖内部缓存来避免重复解析，同时利用 `template_depth` 正确跳过模板参数内部的嵌套作用域分隔符，确保仅在顶层作用域边界分割名称。

#### Side Effects

- Modifies global cache `split_qualified_name_cache` by inserting new entries or clearing it when size exceeds `kSplitQualifiedNameCacheMaxEntries`.

#### Reads From

- `qualified_name` parameter
- global `split_qualified_name_cache` (read via find)

#### Writes To

- global `split_qualified_name_cache` (`insert_or_assign`, clear)

#### Usage Patterns

- Parsing qualified names during symbol extraction
- Splitting names for namespace or module resolution
- Caching parsed components to improve performance

### `clore::extract::symbol_kind_name`

Declaration: `extract/model.cppm:26`

Definition: `extract/model.cppm:244`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::symbol_kind_name` 的实现基于一个直接覆盖 `SymbolKind` 所有枚举成员的 switch 语句，将每个枚举值映射到一个对应的 `std::string_view` 字面量。控制流完全线性：根据传入的 `kind` 决定返回值，每个 case 都显式处理，末尾的 default 分支返回 `"unknown"` 作为安全回退。该函数没有依赖任何外部数据结构或模型，其算法本质是一个全量的编译期映射表，保证了在任意有效输入下的常量时间查找。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- the `kind` parameter (`SymbolKind`)

#### Usage Patterns

- used to obtain a string representation of a symbol kind for display or logging
- used when serializing symbol information to human-readable formats

## Internal Structure

`extract:model` 是提取子系统的核心数据模型模块，定义了整个分析流程中所有结构化信息的类型体系。它导入标准库和 `support` 工具模块，声明了 `ProjectModel`、`SymbolInfo`、`ModuleUnit`、`FileInfo` 以及相关的 `SymbolKind` 枚举、位置/范围类型和符号标识符等公共接口，同时也通过匿名命名空间封装内部实现细节。模块内部按职责划分为三层：最外层是外部可访问的数据结构与查询函数（如 `lookup_symbol`、`find_module_by_name`）；中间层是 `resolve_source_snippet` 等按需加载逻辑；内层则是线程安全的缓存机制（`SplitQualifiedNameCache`、条带状互斥体）与哈希辅助函数，用于优化限定名拆分和源片段解析的性能与内存占用。

在实现结构上，该模块采用延迟填充（lazy population）策略：符号的源片段（source snippet）不会在首次提取时全部读入内存，而是仅记录磁盘偏移与长度，仅在需要时才通过 `resolve_source_snippet` 从文件读取并缓存。为确保并发安全，源片段解析使用基于符号哈希模素数的条带状互斥锁进行细粒度保护。多数公共函数接受 `const ProjectModel &` 引用，以只读方式访问模型数据；内部缓存（如拆分后限定名的结果）被声明为函数静态变量，减少全局状态泄露。整体设计强调模型与索引分离，并依赖 `support` 模块提供的路径规范化、文本处理等底层能力。

## Related Pages

- [Module support](../support/index.md)

