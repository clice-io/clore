---
title: 'Module extract:model'
description: 'extract:model 模块定义并管理 C++ 项目提取结果的核心数据模型。它提供了 ProjectModel、SymbolInfo、ModuleUnit、FileInfo、NamespaceInfo 等类型，用于表示符号、模块单元、文件、命名空间及其层级关系；同时包括 SymbolKind 枚举、SymbolID 唯一标识符、SourceLocation/SourceRange 位置类型等基础组件。该模块还公开了一系列查找和解析函数（如 find_symbol、find_module_by_name、resolve_source_snippet、split_top_level_qualified_name），用于在模型上执行符号定位、模块查询、源片段按需加载以及限定名称操作。extract:model 是构建高级分析（如依赖图、符号浏览）的基础，其公共接口覆盖了项目模型中所有结构化数据的访问与查询入口。'
layout: doc
template: doc
---

# Module `extract:model`

## Summary

`extract:model` 模块定义并管理 C++ 项目提取结果的核心数据模型。它提供了 `ProjectModel`、`SymbolInfo`、`ModuleUnit`、`FileInfo`、`NamespaceInfo` 等类型，用于表示符号、模块单元、文件、命名空间及其层级关系；同时包括 `SymbolKind` 枚举、`SymbolID` 唯一标识符、`SourceLocation`/`SourceRange` 位置类型等基础组件。该模块还公开了一系列查找和解析函数（如 `find_symbol`、`find_module_by_name`、`resolve_source_snippet`、`split_top_level_qualified_name`），用于在模型上执行符号定位、模块查询、源片段按需加载以及限定名称操作。`extract:model` 是构建高级分析（如依赖图、符号浏览）的基础，其公共接口覆盖了项目模型中所有结构化数据的访问与查询入口。

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

`clore::extract::FileInfo` 是一个纯数据聚合体，用于打包单个源文件的提取结果。它的三个字段 `path`、`symbols` 和 `includes` 分别保存该文件的路径字符串、该文件中定义的所有符号标识符（`SymbolID`）向量，以及该文件直接包含的头文件路径向量。该结构体不维护任何运行时不变式；所有字段均可在构造后直接修改，且 `symbols` 与 `includes` 的顺序与实现依赖的提取算法有关，不保证语义上的排序或唯一性。

#### Invariants

- Each `symbols` entry corresponds to a distinct symbol extracted from the file
- `path` uniquely identifies the source file within the extraction context
- `includes` lists the immediate file dependencies as seen by the extractor

#### Key Members

- `path`
- `symbols`
- `includes`

#### Usage Patterns

- Used to represent the result of extracting symbols and dependencies from a single file
- Collected into lists or maps keyed by file path for downstream analysis
- Consumed by tools that transform or visualize include hierarchies and symbol definitions

### `clore::extract::ModuleUnit`

Declaration: `extract/model.cppm:135`

Definition: `extract/model.cppm:135`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::ModuleUnit` 将单个 C++20 模块单元（接口或分区）建模为一组紧密关联的字段。`name` 保存完整的模块名，例如 `"foo"` 或 `"foo:bar"`，而 `is_interface` 语义上标明该单元是否以 `export module` 声明开头（为 `true`）或是纯 `module` 分区。`source_file` 存储经过规范化的文件路径，作为该单元的物理来源。两个容器字段 `imports` 和 `symbols` 分别记录了模块导入列表和在该单元中声明的所有符号标识符 (`SymbolID`)；它们共同构成了从源码到依赖与导出符号的映射，且所有字段在构造后应保持不可变状态，以确保对模块单元的查询是一致的。

#### Invariants

- `name`是完整的模块名，例如"foo"或"foo:bar"
- `is_interface`为真表示导出模块，为假表示非导出模块
- `source_file`是归一化后的源文件路径
- `imports`列出该模块单元的导入
- `symbols`列出在该模块单元中声明的`SymbolID`

#### Key Members

- `name`
- `is_interface`
- `source_file`
- `imports`
- `symbols`

#### Usage Patterns

- 用于在提取过程中表示每个模块单元
- 作为模块信息的元数据容器

### `clore::extract::NamespaceInfo`

Declaration: `extract/model.cppm:128`

Definition: `extract/model.cppm:128`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该结构体用于表示提取过程中的单个命名空间信息。它的三个字段共同建模了一个命名空间的层次及其包含的符号：`name` 保存命名空间的完整限定名称；`symbols` 存储该命名空间内直接声明的符号标识符列表；`children` 记录直接嵌入的子命名空间名称（非限定形式）。在填充数据时，`children` 中的每一项应能与另一个 `NamespaceInfo` 实例的 `name` 匹配，从而重构嵌套关系。不存在跨此结构体的额外不变量，所有字段均在构造时初始化并在后续逐步填入。

#### Invariants

- `name` is the fully qualified or relative namespace name
- `symbols` contains `SymbolIDs` of entities directly declared in this namespace
- `children` lists the names of immediately nested namespaces

#### Key Members

- `name` identifies the namespace
- `symbols` stores direct symbol references
- `children` holds child namespace names

#### Usage Patterns

- Populated by extraction code to represent namespace hierarchy
- Consumed by documentation generators to produce namespace pages

### `clore::extract::ProjectModel`

Declaration: `extract/model.cppm:143`

Definition: `extract/model.cppm:143`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::ProjectModel` 将整个项目的解析结果聚合为一个扁平的、可检索的数据模型，其内部由若干无序映射和向量组成。核心字段包括 `symbols`（以 `SymbolID` 为键的符号信息）、`files`（以规范化文件路径为键的文件信息）、`namespaces`（以命名空间名为键的命名空间信息）以及 `modules`（以规范化源文件路径为键的模块单元）。辅助查找映射 `symbol_ids_by_qualified_name` 和 `module_name_to_sources` 分别用于从完整限定名到可能重载的 `SymbolID` 列表、以及从模块名到源文件路径列表的高效正向查询。`file_order` 按分析顺序记录文件路径，`uses_modules` 标记是否至少有一个模块单元被解析。

重要不变性包括：`file_order` 中的每个路径必须同时存在于 `files` 中；`uses_modules` 为 `true` 当且仅当 `modules` 非空。`symbol_ids_by_qualified_name` 和 `module_name_to_sources` 的键类型均采用 `clore::support::TransparentStringHash` 和 `TransparentStringEqual`，支持透明字符串查找以避免临时 `std::string` 构造。`symbol_ids_by_qualified_name` 在代码生成和证据构建中负责精确的限定名解析，而 `module_name_to_sources` 则用于跨模块链接。`modules` 映射本身使用相同的透明哈希策略，确保以 `std::string_view` 进行高效查找。

#### Invariants

- `symbols` 中的每个 `SymbolID` 唯一标识一个符号。
- `files` 和 `modules` 的键都是规范化的源文件路径。
- `file_order` 保持源文件的解析顺序。
- `symbol_ids_by_qualified_name` 允许一个限定名对应多个重载。
- `uses_modules` 仅在检测到至少一个模块声明时为 true。

#### Key Members

- `symbols`
- `files`
- `namespaces`
- `file_order`
- `modules`
- `symbol_ids_by_qualified_name`
- `module_name_to_sources`
- `uses_modules`

#### Usage Patterns

- 提取阶段填充模型，后续生成代码或证据时查询。
- 通过 `symbol_ids_by_qualified_name` 进行符号的精确限定名查找。
- 通过 `modules` 和 `module_name_to_sources` 处理 C++20 模块依赖。
- `uses_modules` 用于判断是否采用模块化编译策略。

### `clore::extract::SourceLocation`

Declaration: `extract/model.cppm:64`

Definition: `extract/model.cppm:64`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::SourceLocation` 的核心由三个字段组成：`file`（存储源文件名）、`line`（行号，默认 `0`）和 `column`（列号，默认 `0`）。其关键不变式是 `line == 0` 代表位置未知，而有效源行从 `1` 开始计数。成员 `is_known()` 的实现直接检查 `line != 0`，这是判定位置是否已知的唯一依据；当位置未知时，`column` 通常也为 `0`，但该不变式并未被显式强制。`file` 即使对于已知位置也可能为空，这取决于调用方提供的信息。该结构体通过哨兵值 `0` 避免使用额外的布尔标志，使常见的位置未知状态与默认初始化语义一致。

#### Invariants

- `line == 0` indicates the source location is unknown; otherwise `line >= 1`
- `file` and `column` have no guaranteed constraints (may be empty/0)

#### Key Members

- `file` field
- `line` field
- `column` field
- `is_known()` method

#### Usage Patterns

- Returned or accepted by extraction `APIs` to describe where source content was found
- Checked via `is_known()` before using the location

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

结构体 `clore::extract::SourceRange` 以聚合方式建模一个连续的源代码区域，内部仅包含两个 `SourceLocation` 成员 `begin` 和 `end`。不变量要求 `begin` 所代表的源位置在 `end` 之前或相同，以保证区域逻辑有效性。由于该结构体为平凡聚合类型，字段均直接公开访问，无特殊构造、赋值或比较操作实现，所有语义依赖外部对 `SourceLocation` 的比较能力。

### `clore::extract::SymbolID`

Declaration: `extract/model.cppm:28`

Definition: `extract/model.cppm:28`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::SymbolID` 使用一个 64 位的 `hash` 字段和一个 32 位的 `signature` 字段作为其内部表示。`hash` 值为 0 是哨兵值，表示无效 ID；所有有效 ID 的 `hash` 均非零，这是核心不变量。`signature` 派生自相同的 USR 源，用于在极罕见的 64 位哈希碰撞时进一步消歧。`is_valid` 成员直接通过 `hash != 0` 判断有效性。两个比较运算符 `operator==` 和 `operator<=>` 均采用默认实现，编译器会对所有成员进行逐字段比较和字典序排列。

#### Invariants

- `hash` 为 0 表示无效标识符，有效标识符的 `hash` 必为非零
- `signature` 用于在极小概率的 64 位哈希碰撞时进行区分
- 默认比较操作符比较所有字段（先 `hash`，后 `signature`）

#### Key Members

- `hash`
- `signature`
- `is_valid()`
- `operator==`
- `operator<=>`

#### Usage Patterns

- 作为符号的唯一标识符被其他代码使用
- 通过 `is_valid()` 检查标识符是否有效
- 利用默认比较操作符进行排序或集合操作

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

`clore::extract::SymbolInfo` 是提取模型中唯一描述一个符号的结构体。它通过 `id` 全局标识，并存储 `kind`、`name`、`qualified_name`、`enclosing_namespace` 和 `signature` 等基本描述。关系层使用 `parent`、`children`、`bases` 及 `derived` 构建树型父子与继承关系，使用 `calls`、`called_by`、`references` 和 `referenced_by` 构建跨符号的引用网络——所有关系均以 `SymbolID` 向量表示，保证图一致且可遍历。源码片段存储采用两级策略：当 `source_snippet` 非空时直接携带文本；否则通过 `source_snippet_offset`、`source_snippet_length`、`source_snippet_file_size` 和 `source_snippet_hash` 四个字段记录源文件中片段的位置与校验值，支持按需加载并避免冗余内存占用。`is_template` 与 `template_params` 记录模板信息；`declaration_location` 与可选的 `definition_location` 记录编译期位置；`access` 与 `doc_comment` 分别携带访问控制符和文档注释；`lexical_parent_name` 与 `lexical_parent_kind` 以字符串形式保留词法父级信息，独立于关系中的 `parent` 字段，用于快速输出或序列化。

#### Invariants

- `id` uniquely identifies the symbol
- `declaration_location` is always set
- `source_snippet` may be empty, in which case `source_snippet_offset`, `source_snippet_length`, `source_snippet_file_size`, and `source_snippet_hash` reference the raw source text in `declaration_location.file`
- `parent` and `children` form a tree hierarchy
- `bases` and `derived` reflect inheritance relationships
- `calls` and `called_by` are symmetric directed edges from call graph
- `references` and `referenced_by` are symmetric directed edges from reference graph

#### Key Members

- `id`
- `name`
- `qualified_name`
- `kind`
- `declaration_location`
- `parent`
- `children`
- `bases`
- `derived`
- `calls`
- `called_by`
- `references`
- `referenced_by`

#### Usage Patterns

- Serialized as part of extracted symbol database
- Consumed by tools for code navigation, dependency analysis, and documentation generation
- Populated by the extraction pass that processes AST nodes
- Queried via `SymbolID` to traverse relationships

### `clore::extract::SymbolKind`

Declaration: `extract/model.cppm:8`

Definition: `extract/model.cppm:8`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::SymbolKind` 是一个以 `std::uint8_t` 为底层类型的 `enum class`。其枚举值按符号类别在 C++ 抽象语法树中的层次顺序声明：先放置命名空间与类型定义（`Namespace`、`Class`、`Struct`、`Union`、`Enum`），随后是类型成员（`EnumMember`、`Function`、`Method`、`Variable`、`Field`、`TypeAlias`），接着是编译期构造（`Macro`、`Template`、`Concept`），最后以 `Unknown` 作为未识别或未匹配类别的兜底值。这一固定顺序与底层的小型整数类型共同保证了枚举值可以紧凑地存储、直接用于位图或数组索引，同时 `enum class` 的强作用域特性杜绝了隐式整数转换。`Unknown` 成员的存在使得下游处理逻辑无需为每个新类别调整边界检查，只需在最后集中处理未知情况即可维持实现的一致性。

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

该函数首先委托给 `clore::extract::find_modules_by_name` 获取与给定名称匹配的所有 `ModuleUnit*` 指针的集合。若集合为空，立即返回 `nullptr`；若仅有一个元素，則直接返回该元素。对于多个候选模块，函数遍历集合以统计 `ModuleUnit::is_interface` 为 `true` 的条目数量：恰好一个接口单元时返回该接口单元；零个接口单元时记录警告并返回第一个候选作为回退；两个及以上接口单元时记录歧义警告并返回 `nullptr`。此算法依赖 `clore::extract::find_modules_by_name` 的查询结果以及 `ModuleUnit::is_interface` 属性，同时通过 `logging::warn` 报告二义性或缺失接口单元的情况。

#### Side Effects

- Logs warnings via `logging::warn` when no interface unit exists or when multiple interface units are found.

#### Reads From

- `model` parameter
- `module_name` parameter
- `ModuleUnit::is_interface` field on each candidate

#### Writes To

- logging output

#### Usage Patterns

- Resolve a single module by name for symbol extraction
- Disambiguate between multiple modules with the same name by preferring interface units
- Fail gracefully when ambiguous module names exist

### `clore::extract::find_module_by_source`

Declaration: `extract/model.cppm:194`

Definition: `extract/model.cppm:449`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::find_module_by_source` 的实现依赖 `ProjectModel::modules` 这一关联容器（其键类型为源文件路径，值类型为 `ModuleUnit`）。函数的核心算法是对该容器执行一次 `std::find` 操作，以传入的 `source_file` 参数作为查找键。若查找成功，则返回指向对应 `ModuleUnit` 实例的常量指针；否则返回 `nullptr`。整个控制流仅含一个分支（检查迭代器是否抵达 `end`），无额外循环或复杂逻辑。该函数仅依赖 `ProjectModel` 的 `modules` 成员及其 `find` 方法，不调用其他内部函数或模块。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model`.modules`
- `source_file`

#### Usage Patterns

- Used to retrieve a module unit given its source file path
- Often called after scanning or building the project model

### `clore::extract::find_modules_by_name`

Declaration: `extract/model.cppm:191`

Definition: `extract/model.cppm:395`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先在 `model.module_name_to_sources` 中执行一次哈希查找。若 `it` 等于末尾迭代器或 `it->second` 为空，则提前返回空 `std::vector`。否则，它为结果预留容量，然后依次对 `it->second` 中的每个 `source` 调用 `find_module_by_source`，将返回的非空指针压入 `modules`。最后对所有收集到的 `ModuleUnit*` 按 `source_file` 成员进行稳定排序，并返回该向量。

整个实现核心依赖 `ProjectModel` 的 `module_name_to_sources` 映射提供的反向索引，以及 `find_module_by_source` 从源文件查找具体模块单元的能力。排序步骤保证了同一名称下的多个模块单元（如分区与主模块）按源文件路径有序输出。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- model`.module_name_to_sources`
- model (via `find_module_by_source`)
- `ModuleUnit::source_file`

#### Usage Patterns

- retrieve modules by name from project model
- look up module units by module name

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:179`

Definition: `extract/model.cppm:371`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::find_symbol` 基于对 `clore::extract::find_symbols` 的调用实现。它传入 `qualified_name` 获取所有候选符号，然后检查返回向量的 `size` 是否为 1。如果恰好有一个匹配项，函数返回该 `SymbolInfo` 指针；否则返回 `nullptr`。该实现依赖于 `find_symbols` 的内部查找逻辑（包括对 `ProjectModel::symbols` 的遍历和按 `SymbolInfo::qualified_name` 的匹配），本身不包含额外算法，仅作为确保唯一查找结果的简化接口。

#### Side Effects

No observable side effects are evident from the extracted code.

### `clore::extract::find_symbol`

Declaration: `extract/model.cppm:181`

Definition: `extract/model.cppm:379`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

This overload of `clore::extract::find_symbol` first checks if the given `signature` is empty; if so, it delegates directly to the single‑parameter overload `clore::extract::find_symbol(model, qualified_name)`. Otherwise, it obtains all candidates via `clore::extract::find_symbols(model, qualified_name)`, then iterates through the returned symbol pointers comparing each symbol’s `SymbolInfo::signature` field to the requested `signature`. The first match is returned immediately; if no match is found, the function returns `nullptr`. The core control flow is linear: a fast path for the common case of an empty signature, followed by a linear scan over the set of symbols that share the same qualified name, relying on the `SymbolInfo::signature` member for disambiguation.

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` parameter
- `qualified_name` parameter
- `signature` parameter
- symbols returned by `clore::extract::find_symbols`

#### Usage Patterns

- locate a specific symbol by both name and signature
- fall back to name-only lookup when signature is not provided

### `clore::extract::find_symbols`

Declaration: `extract/model.cppm:185`

Definition: `extract/model.cppm:354`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先在 `model.symbol_ids_by_qualified_name` 中执行映射查找以定位与传入的 `qualified_name` 对应的所有 `SymbolID` 条目。如果未找到任何条目，则立即返回一个空向量；否则，会预先分配足够的空间，然后对每个 `SymbolID` 调用 `lookup_symbol` 以解析出对应的 `SymbolInfo` 指针。有效指针被收集到结果向量中，而通过 `lookup_symbol` 无法解析的 ID 则被忽略。该实现完全依赖于 `ProjectModel` 中的符号索引和全局查找函数 `lookup_symbol`，不涉及递归搜索、命名空间限定或模块隔离逻辑。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.symbol_ids_by_qualified_name`
- `lookup_symbol(model, symbol_id)`

#### Usage Patterns

- Retrieve all symbols matching a given fully qualified name during extraction.

### `clore::extract::join_qualified_name_parts`

Declaration: `extract/model.cppm:59`

Definition: `extract/model.cppm:328`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过将 `count` 与 `parts.size()` 取最小值得到安全迭代次数 `safe_count`，避免越界访问。随后从索引 0 开始逐项遍历 `parts`，当索引大于 0 时在结果字符串 `joined` 后追加 `"::"` 分隔符，再追加当前元素。循环结束后返回 `joined`。其内部流程完全依赖标准库 `std::vector` 的大小查询与 `std::string` 的拼接操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `parts` (const `std::vector<std::string>`&)
- `count` (`std::size_t`)

#### Writes To

- the returned `std::string` object

#### Usage Patterns

- Reconstruct qualified names from a list of namespace or name parts

### `clore::extract::lookup_symbol`

Declaration: `extract/model.cppm:177`

Definition: `extract/model.cppm:349`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::lookup_symbol` 的核心逻辑是对 `model.symbols` 容器执行一次哈希表查找。它使用 `SymbolID` 类型的 `id` 作为键，调用 `model.symbols.find(id)` 获得迭代器 `it`，然后检查 `it` 是否等于 `model.symbols.end()`。若不等，则返回指向 `it->second`（即 `SymbolInfo` 对象）的 `const SymbolInfo*`；否则返回 `nullptr`。该实现依赖于 `ProjectModel` 内部已构建好的 `symbols` 映射结构，以及 `SymbolID` 类型的哈希和相等比较能力。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.symbols`
- `id`

#### Usage Patterns

- Checking if a `SymbolID` exists in the model
- Retrieving a symbol's details for further processing

### `clore::extract::namespace_prefix_from_qualified_name`

Declaration: `extract/model.cppm:62`

Definition: `extract/model.cppm:341`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数的实现从输入 `qualified_name` 开始，调用 `split_top_level_qualified_name` 将其拆分为若干个顶级作用域部件（例如 `"std::vector"` 会得到 `["std", "vector"]`）。如果拆分得到的部件数量小于等于 1（意味着限定名本身没有命名空间前缀，如 `"vector"` 或空字符串），则直接返回空字符串。否则，利用 `join_qualified_name_parts` 将所有部件（除最后一个表示名称本身的部件外）重新连接起来，形成完整的命名空间前缀字符串并返回。整个流程完全依赖于 `split_top_level_qualified_name` 的拆分逻辑以及 `join_qualified_name_parts` 的拼接行为。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter

#### Writes To

- return value

#### Usage Patterns

- Used to obtain the namespace prefix of a qualified name during symbol extraction and name manipulation.

### `clore::extract::resolve_source_snippet`

Declaration: `extract/model.cppm:200`

Definition: `extract/model.cppm:455`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过按需延迟读取实现源片段缓存。首先它调用 `source_snippet_mutex_for(sym)` 获取并持有 `std::lock_guard`，保证对该 `SymbolInfo` 的源片段操作的线程安全。如果 `sym.source_snippet` 非空，则直接返回 `true`（已缓存）。否则验证先决条件：`sym.source_snippet_length` 为零或 `sym.declaration_location.file` 为空则返回 `false`。接着使用 `std::filesystem::file_size` 获取文件大小，若失败则返回 `false`；若 `sym.source_snippet_file_size` 非零且与当前文件大小不符，也返回 `false`。再检查 `offset` 与 `length` 是否不超出文件范围，超出则失败。

通过验证后，以二进制模式打开文件，`seekg` 到 `offset` 位置，读取 `length` 字节到 `std::string`。若读取失败则返回 `false`。若 `sym.source_snippet_hash` 非零，则调用 `hash_source_snippet_bytes` 比对哈希值，不一致则返回 `false`。最后，对读取结果进行 `\r\n` 到 `\n` 的标准化处理，将结果移动赋值给 `sym.source_snippet` 并返回 `true`。该函数依赖 `std::filesystem`、`std::ifstream`、内部哈希函数以及 `SymbolInfo` 的字段（如 `source_snippet_offset`、`source_snippet_length`、`source_snippet_file_size`、`source_snippet_hash`、`declaration_location`）。

#### Side Effects

- Modifies the `source_snippet` field of the `SymbolInfo` argument
- Acquires and releases a mutex (`source_snippet_mutex_for(sym)`)
- Performs file I/O (reads from the filesystem via `std::ifstream`)
- Optionally calls `hash_source_snippet_bytes` to validate the snippet

#### Reads From

- `sym.source_snippet`
- `sym.source_snippet_length`
- `sym.declaration_location.file`
- `sym.source_snippet_offset`
- `sym.source_snippet_file_size`
- `sym.source_snippet_hash`
- File system metadata and content of the file specified by `declaration_location.file`

#### Writes To

- `sym.source_snippet`

#### Usage Patterns

- Called to lazily resolve the textual source snippet for a symbol during navigation or display
- Used after symbol extraction to load snippet on demand from disk

### `clore::extract::split_top_level_qualified_name`

Declaration: `extract/model.cppm:57`

Definition: `extract/model.cppm:265`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::split_top_level_qualified_name` 首先通过线程安全的缓存层进行快速查找：调用 `clore::extract::(anonymous namespace)::split_qualified_name_cache` 获取 `SplitQualifiedNameCache` 实例，并施加共享锁（`std::shared_lock`）尝试在 `cache.parts_by_qualified_name` 中命中；若命中则直接返回缓存的 `parts`。否则，函数执行显式的拆分算法：它逐字符遍历 `qualified_name`，同时维护一个 `template_depth` 计数器以正确处理模板参数中的 `<>` 嵌套。仅当 `template_depth` 为零且遇到连续的两个 `:` 时，将当前累积的 `current` 字符串作为一个片段推入 `parts`，并跳过后续的第二个冒号。所有其他字符（包括普通冒号、模板括号以及命名空间内部的字符）都被追加到 `current` 中。遍历结束后，非空的 `current` 被作为最后一个片段加入 `parts`。若结果的首个片段为空（由全局作用域前缀 `::` 引起），则将其移除。最后，在独占锁（`std::unique_lock`）的保护下更新缓存：如果条目数已达到 `kSplitQualifiedNameCacheMaxEntries` 的上限则清空整个缓存，否则使用 `insert_or_assign` 将当前结果存入，并返回 `parts`。该算法的唯一外部依赖是 `split_qualified_name_cache` 函数，其返回一个内部缓存实例。

#### Side Effects

- Mutates the global `SplitQualifiedNameCache` by inserting or assigning the parsed parts
- Acquires shared and unique locks on the cache mutex

#### Reads From

- `qualified_name` parameter
- global `SplitQualifiedNameCache` returned by `split_qualified_name_cache()`

#### Writes To

- global `SplitQualifiedNameCache` (inserts or assigns entry, may clear cache if size exceeds max)

#### Usage Patterns

- Parsing qualified names for symbol resolution
- Extracting namespace prefixes
- Caching parsed results for efficiency

### `clore::extract::symbol_kind_name`

Declaration: `extract/model.cppm:26`

Definition: `extract/model.cppm:244`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::symbol_kind_name` 的实现核心是一个 `switch` 语句，它根据参数 `kind` 的 `clore::extract::SymbolKind` 枚举值直接映射到对应的可读字符串字面量。该函数穷举了 `SymbolKind` 的全部枚举成员（包括 `Namespace`、`Class`、`Struct`、`Union`、`Enum`、`EnumMember`、`Function`、`Method`、`Variable`、`Field`、`TypeAlias`、`Macro`、`Template`、`Concept` 和 `Unknown`），每个 case 返回一个描述性字符串，而默认分支则作为安全回退返回 `"unknown"`。此函数无外部依赖，不访问任何全局状态或数据结构，完全基于单一的参数进行分支并立即返回。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `SymbolKind kind` parameter

#### Usage Patterns

- Retrieving a string label for a `SymbolKind` value
- Used in logging, error messages, or display purposes

## Internal Structure

extract:model 模块定义了 Clore 提取管道中所有核心数据结构，将整个提取结果分解为 `ProjectModel`、`ModuleUnit`、`FileInfo`、`NamespaceInfo` 以及细节丰富的 `SymbolInfo` 和 `SymbolID` 等层次化类型。模块通过导入 `std` 和 `support` 获得标准库基础与文本/文件工具，内部以匿名命名空间封装了限定名称拆分缓存、源片段哈希与条纹互斥体等实现细节，这些辅助设施用于支撑延迟的源片段解析与高效名称查找。内部层次清晰：基础位置与标识类型（`SourceLocation`、`SourceRange`、`SymbolID`）位于底层，符号元数据（`SymbolInfo`）之上构建模块与文件索引，并由 `ProjectModel` 统一持有所有容器；查询函数（`find_symbol`、`find_module_by_*`、`resolve_source_snippet` 等）作为公共接口，通过符号 ID 或名称标识符快速检索模型中的对应对象。

## Related Pages

- [Module support](../support/index.md)

