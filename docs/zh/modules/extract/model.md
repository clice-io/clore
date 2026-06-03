---
title: 'Module extract:model'
description: '模块 extract::model 定义了提取流程的核心数据结构，包括 ProjectModel、SymbolInfo、FileInfo、ModuleUnit、NamespaceInfo、SymbolID、SourceLocation 和 SourceRange 等类型。这些类型共同描述了从 C++ 项目中提取出的符号（函数、类、模块单元等）及其层级关系、位置信息与引用关系，构成后续分析、生成文档或证据的基础数据模型。'
layout: doc
template: doc
---

# Module `extract:model`

## Summary

模块 `extract::model` 定义了提取流程的核心数据结构，包括 `ProjectModel`、`SymbolInfo`、`FileInfo`、`ModuleUnit`、`NamespaceInfo`、`SymbolID`、`SourceLocation` 和 `SourceRange` 等类型。这些类型共同描述了从 C++ 项目中提取出的符号（函数、类、模块单元等）及其层级关系、位置信息与引用关系，构成后续分析、生成文档或证据的基础数据模型。

该模块公开了丰富的查找与操作接口，如 `find_symbol`、`find_symbols`、`find_module_by_name`、`find_module_by_source`、`lookup_symbol` 以及辅助函数 `split_top_level_qualified_name`、`join_qualified_name_parts`、`namespace_prefix_from_qualified_name`、`symbol_kind_name` 和 `resolve_source_snippet`。这些接口允许使用者按限定名称、签名、源文件路径或 `SymbolID` 高效定位符号与模块单元，并支持延迟解析源代码片段。模块依赖底层 `support` 模块提供的通用工具，专注于为提取系统提供一致的数据模型和查询抽象。

## Imports

- [`support`](../support/index.md)

## Imported By

- [`extract:ast`](ast.md)
- [`extract:merge`](merge.md)

## Types

### `clore::extract::FileInfo`

Declaration: `src/extract/model.cppm:139`

Definition: `src/extract/model.cppm:139`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::FileInfo` 是一个聚合类型，用于存储单个源文件的提取结果。其内部包含三个数据成员：`path` 记录文件路径，`symbols` 保存该文件中定义的所有符号标识符（`SymbolID`），`includes` 列出文件直接引用的包含路径。该结构体不维护任何超越其成员默认初始化的不变性——例如，`symbols` 和 `includes` 的顺序由填充逻辑自由决定，且没有强制要求元素唯一性。唯一隐含的约定是 `path` 应当指向一个实际存在的源文件，但此约束由构造或填充 `FileInfo` 的代码负责，而非在结构体内部检查。

### `clore::extract::ModuleUnit`

Declaration: `src/extract/model.cppm:152`

Definition: `src/extract/model.cppm:152`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

内部结构由五个核心字段组成：`name` 存储模块的全限定名称（例如 `"foo"` 或 `"foo:bar"`），`is_interface` 区分该单元是否为导出接口（`export module`）还是内部实现单元（`module`），`source_file` 记录规范化后的源文件路径，`imports` 列出本单元直接导入的所有模块名称，`symbols` 则按声明顺序存储该单元内出现的符号标识符。关键不变式是 `is_interface` 必须与单元声明中的 `export` 关键字一致，且 `name` 永远不会为空（即使是未命名模块也有特定占位符表示）。所有 vector 成员保持声明的先后顺序，`imports` 中不会出现重复的模块名称，`source_file` 在整个提取管道中始终以操作系统原生路径形式存储，用于后续诊断和查询。这些字段共同构成了模块提取模型中最基础的数据单元，描述了模块的声明属性、物理位置以及它所引入和导出的符号依赖关系。

#### Invariants

- `name` 表示完整的模块标识符
- `is_interface` 为 `true` 时该单元为接口单元
- `source_file` 是经过规范化的文件路径
- `imports` 列出本单元直接导入的所有模块
- `symbols` 包含本单元内声明的符号 ID

#### Key Members

- `name`
- `is_interface`
- `source_file`
- `imports`
- `symbols`

#### Usage Patterns

- 此结构体由模块提取器填充，用于封装解析后的单个模块单元信息
- 其他组件可通过 `name` 和 `imports` 分析模块间的依赖关系
- `symbols` 用于关联模块单元与其内部声明的符号定义
- `source_file` 可用于定位或索引原始源文件

### `clore::extract::NamespaceInfo`

Declaration: `src/extract/model.cppm:145`

Definition: `src/extract/model.cppm:145`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`NamespaceInfo` 是一个聚合结构体，其内部直接暴露三个公共数据成员。`name` 存储命名空间的限定名；`symbols` 维护该命名空间内直接声明的符号 ID 序列，用于快速查询当前作用域下的实体；`children` 则列出所有直接子命名空间的名称字符串，以支持递归命名空间树遍历。该结构体不包含构造函数或成员函数，因此不强制任何不变量——调用方可以直接修改各成员，依赖调用方在组装数据时保证 `name` 非空、`symbols` 和 `children` 无重复等语义约束。作为值类型，其拷贝和移动均按默认逐成员处理，仅用于在提取阶段临时保存解析结果。

#### Invariants

- `name` 成员用于标识命名空间的唯一名称
- `symbols` 和 `children` 成员可以为空，表示没有符号或子命名空间

#### Key Members

- `name`：命名空间名称
- `symbols`：命名空间内符号的标识符列表
- `children`：子命名空间名称列表

#### Usage Patterns

- 外部代码通过访问 `name`、`symbols` 和 `children` 来获取命名空间的层级结构
- 通常被其他提取逻辑填充并用于构建命名空间树或符号索引

### `clore::extract::ProjectModel`

Declaration: `src/extract/model.cppm:160`

Definition: `src/extract/model.cppm:160`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

struct `clore::extract::ProjectModel` 是提取阶段的核心数据容器，用于持有整个项目的解析结果。其内部由若干无序映射（`std::unordered_map`）构成，分别按不同键索引：`symbols` 以 `SymbolID` 索引所有符号信息；`files` 以规范化源文件路径索引 `FileInfo`；`namespaces` 以名称索引命名空间信息；`modules` 以路径索引 `ModuleUnit`（仅当项目使用 C++20 模块时有效）；`symbol_ids_by_qualified_name` 提供精确限定名称到 `SymbolID` 向量的反向查找（允许一个名称对应多个重载）；`module_name_to_sources` 将模块名映射到其所属的源文件路径列表。此外，`file_order` 按处理顺序保存所有文件路径，确保遍历顺序的确定性。`uses_modules` 布尔标志作为一个整体不变性，指示项目中是否至少存在一条模块声明，从而影响模块相关容器的解释方式。各容器之间通过键的规范化（如文件路径和模块名）和透明散列/等价谓词维护一致性，支撑后续的代码生成与证据构建。

#### Invariants

- `file_order` 按解析顺序存储文件路径，保证遍历顺序稳定
- `symbol_ids_by_qualified_name` 中的值向量至少包含一个元素，且可能包含多个表示重载
- `module_name_to_sources` 中的值向量可能包含多个源文件路径（当模块跨文件分区时）
- `uses_modules` 为 `true` 当且仅当至少一个模块单元存在

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

- 用于文档生成阶段遍历所有符号和文件
- 通过 `symbol_ids_by_qualified_name` 实现精确的限定名查找，以处理链接和引用
- 通过 `module_name_to_sources` 将模块名映射到源文件，用于跨模块的交叉引用
- 通过 `file_order` 确定文件的处理顺序
- `uses_modules` 被其他组件用来调整生成策略（例如是否启用模块相关格式）

### `clore::extract::SourceLocation`

Declaration: `src/extract/model.cppm:81`

Definition: `src/extract/model.cppm:81`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::SourceLocation` 的内部布局由三个字段组成：字符串型 `file`、32 位无符号整型 `line`（默认值为 `0`）和 32 位无符号整型 `column`（默认值为 `0`）。其核心不变量是：`line` 等于 `0` 时标识位置未知，有效行号从 `1` 开始。关键成员函数 `is_known()` 通过 `return line != 0` 实现，直接利用了该不变量，因此当 `line` 非零时返回 `true`，否则返回 `false`。

#### Invariants

- `line == 0` 表示位置未知，有效行号从 1 开始

#### Key Members

- `file`
- `line`
- `column`
- `is_known`

#### Usage Patterns

- 在 `clore::extract` 命名空间内作为源代码位置的载体
- 通过 `is_known` 方法判断位置是否已确定，以便进行后续处理

#### Member Functions

##### `clore::extract::SourceLocation::is_known`

Declaration: `src/extract/model.cppm:87`

Definition: `src/extract/model.cppm:87`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_known() const noexcept {
        return line != 0;
    }
```

### `clore::extract::SourceRange`

Declaration: `src/extract/model.cppm:92`

Definition: `src/extract/model.cppm:92`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::SourceRange` 是一个聚合结构体，内部仅包含两个 `SourceLocation` 类型的公开数据成员：`begin` 和 `end`。它未定义任何自定义构造函数、析构函数或成员函数，完全依赖默认的聚合初始化与逐成员复制语义，因此其存储布局是平凡的。该类型的使用者负责维护 `begin` 应代表范围的逻辑起始、`end` 代表逻辑结束这一不变性——例如，假设 `SourceLocation` 存在全序关系，则通常要求 `begin` 不大于 `end`，但该约束并未通过结构体自身的成员函数或断言来强制实施。作为仅含两个字段的 POD 式类型，`SourceRange` 在实现中常被直接拷贝或作为函数参数传递，其实现开销仅等同于两次 `SourceLocation` 的复制。

#### Invariants

- `begin` 和 `end` 通常用于表示一个连续的范围
- 范围语义上期望 `begin` 不晚于 `end`（但未强制检查）

#### Key Members

- `begin`：范围的起始位置
- `end`：范围的结束位置

#### Usage Patterns

- 其他代码通过访问 `begin` 和 `end` 来获取范围边界
- 可能用于传递或存储源代码片段的定位信息

### `clore::extract::SymbolID`

Declaration: `src/extract/model.cppm:45`

Definition: `src/extract/model.cppm:45`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

结构体 `clore::extract::SymbolID` 内部由两个字段组成：一个 64 位的无符号整数 `hash` 和一个 32 位的无符号整数 `signature`。其核心不变式是：有效标识符的 `hash` 必须非零，零值作为全局无效哨兵（由 `is_valid()` 检查 `hash != 0` 实现），而 `signature` 字段则源自同一 USR 来源，专门用于在极其罕见的情况下进一步消除 64 位哈希碰撞带来的歧义。

成员实现方面，`operator==` 和 `operator<=>` 均使用默认合成，因此比较基于 `hash` 和 `signature` 的字典序，同时生成 `std::strong_ordering` 全序关系。`is_valid()` 仅依赖 `hash` 的零值判断，不要求 `signature` 一定为特定值，这保持了简单的有效性验证。

#### Invariants

- A valid `SymbolID` has a non-zero `hash`.
- The `hash` field is zero for the invalid/null sentinel.
- Both `hash` and `signature` are derived from the same USR source.

#### Key Members

- `hash` field
- `signature` field
- `is_valid()` method
- `operator==`
- `operator<=>`

#### Usage Patterns

- Serves as the primary identifier for symbols in the extraction pipeline.
- Used as a key or value in containers that support default comparison and ordering.
- Returned by functions that need to refer to a symbol without exposing its full name.

#### Member Functions

##### `clore::extract::SymbolID::is_valid`

Declaration: `src/extract/model.cppm:52`

Definition: `src/extract/model.cppm:52`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
[[nodiscard]] bool is_valid() const noexcept {
        return hash != 0;
    }
```

##### `clore::extract::SymbolID::operator<=>`

Declaration: `src/extract/model.cppm:57`

Definition: `src/extract/model.cppm:57`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
auto operator<=>(const SymbolID&) const = default
```

##### `clore::extract::SymbolID::operator==`

Declaration: `src/extract/model.cppm:56`

Definition: `src/extract/model.cppm:56`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
bool operator==(const SymbolID&) const = default
```

### `clore::extract::SymbolInfo`

Declaration: `src/extract/model.cppm:97`

Definition: `src/extract/model.cppm:97`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

实现层面的 `clore::extract::SymbolInfo` 是一个扁平的聚合体，集中存储单个符号的完整提取元数据。其字段按职责分为若干逻辑组：符号标识与签名（ `id`、`kind`、`name`、`qualified_name`、`enclosing_namespace`、`signature`）、位置信息（ `declaration_location` 与可选的 `definition_location`）、文档与源码快照（ `doc_comment`、`source_snippet` 以及后备的偏移/长度/文件大小/哈希四元组）、层次与父级关系（ `parent`、`children` 以及 `lexical_parent_name`/`lexical_parent_kind`）、继承关系（ `bases`、`derived`）、调用关系（ `calls`、`called_by`）、引用关系（ `references`、`referenced_by`），以及访问控制和模板信息（ `access`、`is_template`、`template_params`）。一个关键不变量是：当 `source_snippet` 为空字符串时，该符号的源码片段并不直接驻留内存，而是通过 `declaration_location` 所引用的文件，结合 `source_snippet_offset`、`source_snippet_length`、`source_snippet_file_size` 和 `source_snippet_hash` 这四条信息按需懒加载，从而在不损失精确性的前提下降低大规模符号集的内存占用。所有关系字段（ `parent`、`children`、`bases`、`derived`、`calls`、`called_by`、`references`、`referenced_by`）均使用 `SymbolID` 进行跨符号引用，保证了关系图的无环性与查找效率。

#### Invariants

- `id` is unique per symbol within an extraction session.
- `kind` must be a valid `SymbolKind` enumerator.
- `source_snippet_hash` is computed from the snippet content when `source_snippet` is non-empty.
- `parent` is `std::nullopt` if the symbol has no parent in the extracted hierarchy.
- `children` excludes the symbol itself; relationships are directed.
- `bases` and `derived` are non-empty only for class/struct types.
- `calls` and `called_by` capture call graph edges; symmetry is not guaranteed.
- `references` and `referenced_by` capture use relations; symmetry not guaranteed.

#### Key Members

- `id`
- `kind`
- `name`
- `qualified_name`
- `declaration_location`
- `definition_location`
- `parent`
- `children`
- `bases`
- `derived`
- `calls`
- `called_by`
- `references`
- `referenced_by`
- `source_snippet`
- `doc_comment`
- `access`
- `is_template`
- `template_params`

#### Usage Patterns

- Returned by extraction functions to represent a single symbol.
- Consumed by documentation generators to produce symbol documentation pages.
- Populated by tools that walk the AST and fill in fields like `id`, `kind`, `parent`, and `children`.
- Used to build relationship graphs via `bases`/`derived`, `calls`/`called_by`, and `references`/`referenced_by`.
- On-demand snippet resolution when `source_snippet` is empty: consumers read `source_snippet_offset` and siblings to load from file.

### `clore::extract::SymbolKind`

Declaration: `src/extract/model.cppm:25`

Definition: `src/extract/model.cppm:25`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::SymbolKind` 是一个基于底层类型 `std::uint8_t` 的枚举类，编译器会为其成员自动分配整数值（从 0 开始）。成员按语义类别分组排列：首先是位于命名空间或类型作用域的实体（`Namespace`、`Class`、`Struct`、`Union`、`Enum`、`EnumMember`），接着是函数与成员函数（`Function`、`Method`），然后是数据成员（`Variable`、`Field`），再是类型别名、宏及模板或概念相关（`TypeAlias`、`Macro`、`Template`、`Concept`），最后以 `Unknown` 结束。这种分组顺序并非强制要求，但直观反映了各类 C++ 符号的层级关系。`Unknown` 成员作为兜底值，用于处理无法分类或尚未识别的符号，保证了枚举空间的完备性。整个枚举实例仅占用一个字节，适合嵌入其他结构或作为紧凑的标签类型使用。

#### Invariants

- Each symbol has exactly one `SymbolKind` value
- Enum values are mutually exclusive and uniquely identify a symbol's category
- The `Unknown` value serves as a default or error sentinel
- Underlying type is `std::uint8_t` for fixed-width storage

#### Key Members

- `Namespace`
- `Class`
- `Function`
- `Method`
- `Variable`
- `Unknown`

#### Usage Patterns

- Used to tag extracted symbol nodes with their semantic category
- Code switches on `SymbolKind` to dispatch handling logic
- Symbol filtering and grouping operations depend on this enumeration
- Probably compared or serialized in diagnostics and reports

#### Member Variables

##### `clore::extract::SymbolKind::Class`

Declaration: `src/extract/model.cppm:27`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Class
```

##### `clore::extract::SymbolKind::Concept`

Declaration: `src/extract/model.cppm:39`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Concept
```

##### `clore::extract::SymbolKind::Enum`

Declaration: `src/extract/model.cppm:30`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Enum
```

##### `clore::extract::SymbolKind::EnumMember`

Declaration: `src/extract/model.cppm:31`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
EnumMember
```

##### `clore::extract::SymbolKind::Field`

Declaration: `src/extract/model.cppm:35`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Field
```

##### `clore::extract::SymbolKind::Function`

Declaration: `src/extract/model.cppm:32`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Function
```

##### `clore::extract::SymbolKind::Macro`

Declaration: `src/extract/model.cppm:37`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Macro
```

##### `clore::extract::SymbolKind::Method`

Declaration: `src/extract/model.cppm:33`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Method
```

##### `clore::extract::SymbolKind::Namespace`

Declaration: `src/extract/model.cppm:26`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Namespace
```

##### `clore::extract::SymbolKind::Struct`

Declaration: `src/extract/model.cppm:28`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Struct
```

##### `clore::extract::SymbolKind::Template`

Declaration: `src/extract/model.cppm:38`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Template
```

##### `clore::extract::SymbolKind::TypeAlias`

Declaration: `src/extract/model.cppm:36`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
TypeAlias
```

##### `clore::extract::SymbolKind::Union`

Declaration: `src/extract/model.cppm:29`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Union
```

##### `clore::extract::SymbolKind::Unknown`

Declaration: `src/extract/model.cppm:40`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Unknown
```

##### `clore::extract::SymbolKind::Variable`

Declaration: `src/extract/model.cppm:34`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

###### Implementation

```cpp
Variable
```

## Functions

### `clore::extract::find_module_by_name`

Declaration: `src/extract/model.cppm:205`

Definition: `src/extract/model.cppm:433`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过委托给 `find_modules_by_name` 获取匹配 `module_name` 的所有 `ModuleUnit` 指针来启动。若结果为空则立即返回 `nullptr`；若仅有一个匹配则直接返回该指针。对于多个匹配的情况，函数遍历所有候选模块，统计其中 `is_interface` 为 `true` 的单元数量并记录最后一个接口单元。若恰好存在一个接口单元则将其返回；若不存在任何接口单元，则通过 `logging::warn` 记录一条警告（指出拥有多个实现单元但缺少接口单元），并回退返回第一个候选模块；若存在多个接口单元，则同样记录歧义警告并返回 `nullptr`。

该实现依赖 `find_modules_by_name` 提供初步的模块列表，依赖 `ModuleUnit::is_interface` 区分接口与实现单元，并利用 `logging::warn` 报告非唯一或缺失接口的情况。返回值在接口唯一时优先于实现单元，在无接口时采用保守回退策略。

#### Side Effects

- Logs a warning when no interface unit exists for the module
- Logs a warning when multiple interface units are found

#### Reads From

- `model` parameter (passed to `find_modules_by_name`)
- `module_name` parameter
- `module->is_interface` for each module in the candidate list

#### Usage Patterns

- Resolve a module name to a unique module unit during extraction or analysis
- Handle module name lookup with disambiguation and logging

### `clore::extract::find_module_by_source`

Declaration: `src/extract/model.cppm:211`

Definition: `src/extract/model.cppm:466`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数通过直接查询传入的 `ProjectModel` 实例中的 `modules` 成员（其类型为关联容器），实现基于源文件名到模块单元的映射查找。它使用 `model.modules.find(source_file)` 进行一次容器查找，若找到则返回对应 `ModuleUnit` 对象的指针，否则返回 `nullptr`。整个实现仅依赖 `ProjectModel.modules` 这一数据结构，不涉及任何遍历、排序或额外的函数调用，因此算法的时间复杂度完全由该容器的查找复杂度决定。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.modules` (the map from source file paths to `ModuleUnit` objects)
- `source_file` (the lookup key)

#### Usage Patterns

- Used to obtain a module unit given a source file path during project model queries
- Expected to be called when resolving modules by source location

### `clore::extract::find_modules_by_name`

Declaration: `src/extract/model.cppm:208`

Definition: `src/extract/model.cppm:412`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先通过 `model.module_name_to_sources`（一个从模块名到源文件路径列表的映射）查找给定的 `module_name`。如果映射不存在或对应的值列表为空，则立即返回一个空的 `std::vector<const ModuleUnit*>`。否则，预先分配足够的容量，并依次对列表中的每个 `source` 调用 `find_module_by_source` 来获取对应的 `ModuleUnit` 指针，将其存入结果向量。最后，使用 `std::sort` 按 `ModuleUnit::source_file` 的字典序对所有找到的模块指针进行排序，以确保输出的顺序稳定且可预测。

内部控制流完全依赖于 `ProjectModel` 中预先构建的 `module_name_to_sources` 索引以及 `find_module_by_source` 辅助函数。排序步骤保证了即使在同一模块名对应多个源文件（例如接口和分区）时，结果也能按文件路径升序排列。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.module_name_to_sources`
- `find_module_by_source` (reads from `model`)
- `ModuleUnit::source_file` for sorting

#### Usage Patterns

- Used to retrieve all module units sharing a name.
- Typically called when resolving named modules in a project model.

### `clore::extract::find_symbol`

Declaration: `src/extract/model.cppm:198`

Definition: `src/extract/model.cppm:396`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/find-symbol.md)

当 `signature` 为空时，该函数直接委托给单参数重载 `clore::extract::find_symbol`，利用其基于 `qualified_name` 的查找逻辑。否则，它调用 `clore::extract::find_symbols` 获取所有匹配该限定名的符号列表，然后线性扫描列表中每个符号的 `signature` 字段，与传入的签名做相等比较。一旦找到匹配项即返回其指针；若遍历完毕仍未命中则返回 `nullptr`。此实现依赖 `clore::extract::find_symbols` 的集合检索能力，并依赖 `SymbolInfo::signature` 作为消歧依据。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model`
- `qualified_name`
- `signature`
- the symbols returned by `clore::extract::find_symbols`

#### Usage Patterns

- look up a symbol by qualified name only
- look up a symbol by qualified name and signature
- exact symbol resolution with optional signature matching

### `clore::extract::find_symbol`

Declaration: `src/extract/model.cppm:196`

Definition: `src/extract/model.cppm:388`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

Implementation: [Implementation](functions/find-symbol.md)

函数 `clore::extract::find_symbol` 的实现首先调用 `clore::extract::find_symbols`，传入相同的 `model` 和 `qualified_name`。若返回的匹配集合 `matches` 长度恰好为 1，则函数返回集合中的第一个元素，否则返回 `nullptr`。此实现依赖 `clore::extract::find_symbols` 完成实际的符号查找与匹配逻辑，自身仅负责将多匹配或零匹配场景统一转换为单指针返回值，从而简化调用方的歧义处理。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model` (const `ProjectModel` reference)
- `qualified_name` (`std::string_view`)

#### Usage Patterns

- Used as a convenience wrapper to retrieve a uniquely matching symbol by qualified name
- Called by the overload `clore::extract::find_symbol` that takes an additional `std::string_view` parameter

### `clore::extract::find_symbols`

Declaration: `src/extract/model.cppm:202`

Definition: `src/extract/model.cppm:371`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

该函数首先利用 `model.symbol_ids_by_qualified_name`（一个将限定名称映射到 `SymbolID` 向量的查找表）进行直接查找。如果 `qualified_name` 不存在，则立即返回空向量。否则，它会根据查找到的向量大小预分配结果空间，然后遍历每个 `symbol_id`，对每个 ID 调用 `lookup_symbol` 将标识符解析为指向 `SymbolInfo` 的指针。

在遍历过程中，`lookup_symbol` 负责根据 `SymbolID` 在 `model.symbols` 或相关索引中检索实际的符号信息。只有非空指针（即成功解析的符号）才会被追加到结果向量中。这种设计利用了符号名称到 ID 的多对一映射（因为名称重载），并通过二次查找确保返回的指针始终指向有效的、已加载的符号实例。整个流程在常量时间内完成名称查找，总复杂度取决于重载数量。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.symbol_ids_by_qualified_name`
- `lookup_symbol(model, symbol_id)` (indirectly reads model's symbol storage)

#### Usage Patterns

- retrieve all symbols with a given qualified name
- find symbols for a qualified name in the project model

### `clore::extract::join_qualified_name_parts`

Declaration: `src/extract/model.cppm:76`

Definition: `src/extract/model.cppm:345`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::join_qualified_name_parts` 实现了将限定名称各部分重新拼接为完整限定名的算法。它接收一个 `std::vector<std::string>` 类型的 `parts` 和一个 `std::size_t` 类型的 `count`，通过 `safe_count`（取 `count` 与 `parts` 大小中的最小值）确定实际参与拼接的元素个数。然后从索引 0 开始遍历至 `safe_count - 1`，在非首项前插入 `"::"` 分隔符，依次累加各部分组成最终字符串 `joined`。该函数依赖 `std::string` 的拼接操作，并内联处理边界安全，无需额外依赖其他内部函数或数据结构。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `parts` (the vector of strings)
- `count` (the number of parts to join)
- `parts.size()`

#### Usage Patterns

- Used to reconstruct qualified names from tokenized parts
- Often called when building fully qualified symbol names or paths from a sequence of identifiers

### `clore::extract::lookup_symbol`

Declaration: `src/extract/model.cppm:194`

Definition: `src/extract/model.cppm:366`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::lookup_symbol` 通过直接查询 `ProjectModel` 中的 `symbols` 容器完成查找。该容器是一个以 `SymbolID` 为键、`SymbolInfo` 为值的无序关联容器（依赖于 `SymbolID` 的哈希特性和相等比较）。函数内部调用容器的 `find` 方法，若迭代器未指向容器末尾，则返回对应元素的指针；否则返回 `nullptr`。整个流程不涉及额外的搜索或遍历，依赖关系仅限于 `ProjectModel` 的数据布局和 `SymbolID` 的哈希/等价操作。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `model.symbols`
- `id`

#### Usage Patterns

- resolving symbol by identifier
- fetching symbol details after obtaining ID
- null-safe symbol access

### `clore::extract::namespace_prefix_from_qualified_name`

Declaration: `src/extract/model.cppm:79`

Definition: `src/extract/model.cppm:358`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::namespace_prefix_from_qualified_name` 先委托给内部函数 `split_top_level_qualified_name`，将输入的 `qualified_name` 拆分为各个作用域部分。如果拆分后 `parts` 的大小不超过 1（即限定名本身不含嵌套作用域），则直接返回空字符串。否则，它调用 `join_qualified_name_parts`，传入 `parts` 和 `parts.size() - 1` 作为连接的元素个数，从而拼接出除最后一个名字（即最内层标识符）之外的所有作用域前缀。整个过程完全依赖于同一内部实现单元（`(anonymous namespace)` 或同一模块内部）中的两个辅助函数，不涉及外部查询或状态变更。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- `qualified_name` parameter

#### Usage Patterns

- Extract namespace from a qualified name for semantic analysis
- Prepare namespace context for symbol lookup or display

### `clore::extract::resolve_source_snippet`

Declaration: `src/extract/model.cppm:217`

Definition: `src/extract/model.cppm:472`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

`clore::extract::resolve_source_snippet` 通过互斥锁（由 `source_snippet_mutex_for` 分配）保护，先检查 `sym.source_snippet` 是否已填充，若已填充则直接返回 `true`。若 `sym.source_snippet_length` 为零或 `sym.declaration_location.file` 为空则早期返回 `false`。随后获取文件大小，依次验证 `sym.source_snippet_file_size` 是否与当前文件大小一致（若已记录）、偏移量与长度是否越界，并尝试以二进制模式打开文件。如果文件无法打开、定位失败或读取字节数不够，则返回 `false`。若 `sym.source_snippet_hash` 非零，则用 `hash_source_snippet_bytes` 校验读取内容的哈希，不匹配则返回 `false`。通过所有检查后，将读取内容中的 `\r\n` 换行符标准化为 `\n`，存入 `sym.source_snippet`，最终返回 `true`。

#### Side Effects

- Modifies `sym.source_snippet`
- Acquires a mutex (`source_snippet_mutex_for(sym)`)
- Reads from the filesystem (file size and file content)

#### Reads From

- `sym.source_snippet`
- `sym.source_snippet_length`
- `sym.declaration_location.file`
- `sym.source_snippet_offset`
- `sym.source_snippet_file_size`
- `sym.source_snippet_hash`
- File system: file size and content of `sym.declaration_location.file`
- `hash_source_snippet_bytes` (function call)

#### Writes To

- `sym.source_snippet`

#### Usage Patterns

- Resolve source snippet for a symbol after extraction
- Lazy loading of source text for a symbol
- Called during symbol resolution or lookup

### `clore::extract::split_top_level_qualified_name`

Declaration: `src/extract/model.cppm:74`

Definition: `src/extract/model.cppm:282`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

实现核心是一个手工状态解析器：逐个字符扫描输入限定名，通过一个计数器 `template_depth` 跟踪当前是否处于模板参数层级（遇到 `<` 递增、遇到 `>` 递减），仅在 `template_depth == 0` 时将连续出现的 `::` 视为顶级作用域分隔符，从而正确分割类似 `std::vector<std::pair<int, int>>::iterator` 这样的名字。解析完成后会清理掉可能因前导 `::` 产生的空字符串元素。每次调用都会先通过 `split_qualified_name_cache` 获取线程级缓存（由 `static SplitQualifiedNameCache` 维护），用 `shared_lock` 尝试查找；若未命中则执行解析，并在写入缓存前用 `unique_lock` 临界区限制并发修改，当缓存条目数达到 `kSplitQualifiedNameCacheMaxEntries` 时直接清空以限制内存增长。整个流程不依赖其他外部函数，只依赖 `split_qualified_name_cache` 提供的单例缓存结构。

#### Side Effects

- Updates the global cache `split_qualified_name_cache`

#### Reads From

- `qualified_name` parameter (`std::string_view`)
- global cache `split_qualified_name_cache`

#### Writes To

- global cache `split_qualified_name_cache`

#### Usage Patterns

- Used to decompose qualified names into their top-level components for further processing like symbol lookup or name resolution.

### `clore::extract::symbol_kind_name`

Declaration: `src/extract/model.cppm:43`

Definition: `src/extract/model.cppm:261`

Declaration: [`Namespace clore::extract`](../../namespaces/clore/extract/index.md)

函数 `clore::extract::symbol_kind_name` 的实现是一个直接的 switch 语句，根据传入的 `SymbolKind` 枚举值返回对应的 `std::string_view` 字符串。它对每个已知的枚举成员（如 `SymbolKind::Namespace`、`SymbolKind::Class`、`SymbolKind::Struct`、`SymbolKind::Union`、`SymbolKind::Enum`、`SymbolKind::EnumMember`、`SymbolKind::Function`、`SymbolKind::Method`、`SymbolKind::Variable`、`SymbolKind::Field`、`SymbolKind::TypeAlias`、`SymbolKind::Macro`、`SymbolKind::Template` 和 `SymbolKind::Concept`）硬编码了一个明确的字符串字面量。对于任何未匹配的值（包括 `SymbolKind::Unknown` 或可能的非法值），函数会返回 `"unknown"` 作为默认结果。该函数不依赖其他代码或外部状态，所有字符串字面量在编译时确定，因此它的执行时间和控制流完全可预测。

#### Side Effects

No observable side effects are evident from the extracted code.

#### Reads From

- kind

#### Usage Patterns

- Converting `SymbolKind` values to human-readable strings for display or serialization

## Internal Structure

`extract:model` 模块定义了抽取流程的核心数据模型，将符号、文件、模块单元和项目组织为清晰的数据结构。它通过 `SymbolKind` 枚举、`SymbolID`（基于哈希的紧凑标识符）以及 `SymbolInfo`、`FileInfo`、`NamespaceInfo`、`ModuleUnit` 和 `ProjectModel` 等类型封装抽取结果。内部采用匿名命名空间隐藏实现细节，例如 `SplitQualifiedNameCache` 用于缓存限定名分割结果以减少重复字符串操作，以及源片段延迟加载机制（通过 `source_snippet_offset` / `source_snippet_length` 配合 `resolve_source_snippet` 函数按需从磁盘读取文本）。线程安全性通过细粒度互斥量（如 `source_snippet_mutex_for` 基于哈希条带分配）来保证，支持并发查找而不阻塞全局。

该模块仅依赖 `support` 模块（提供路径规范化、日志等通用工具），不直接引入外部解析器。其 API 主要包括查找函数（`find_symbol`、`lookup_symbol`、`find_module_by_name` 等）和辅助函数（`split_top_level_qualified_name`、`join_qualified_name_parts`、`symbol_kind_name`），调用方无需了解内部缓存与锁机制即可使用。这种分层设计将模型定义与持久化、并发控制隔离，使模型稳定且易于测试。

## Related Pages

- [Module support](../support/index.md)

