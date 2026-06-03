---
title: 'Clore 提取模块深度解析'
description: 'Guide: Clore 提取模块深度解析'
layout: doc
template: doc
---

# 提取模块深度解析（`clore::extract`）

## 概述

`clore::extract` 是 Clore 的代码分析引擎，负责将 C++ 源代码转化为结构化的 `ProjectModel`。它通过加载 `compile_commands.json`（编译数据库），利用 Clang 工具链进行依赖扫描和 AST 解析，最终得到包含符号、文件、命名空间和模块信息的完整项目模型。

**模块文件**（8 个 C++20 模块单元）：

| 模块 | 文件 | 符号数 | 职责 |
|------|------|--------|------|
| `extract` | `src/extract/extract.cppm` | 175 | 顶层编排、缓存管理、并行 AST 提取 |
| `extract:ast` | `src/extract/ast.cppm` | 201 | Clang AST 解析、符号提取 |
| `extract:scan` | `src/extract/scan.cppm` | 117 | 依赖扫描、包含关系分析 |
| `extract:compiler` | `src/extract/compiler.cppm` | 100 | 编译数据库加载、工具链管理 |
| `extract:model` | `src/extract/model.cppm` | 190 | 数据模型（`SymbolInfo`、`ProjectModel` 等） |
| `extract:filter` | `src/extract/filter.cppm` | 43 | 路径过滤 |
| `extract:merge` | `src/extract/merge.cppm` | 94 | 符号合并、索引重建 |
| `extract:cache` | `src/extract/cache.cppm` | 222 | 缓存管理 |

---

## 核心数据模型

### `SymbolID` — 符号唯一标识

```cpp
struct SymbolID {
    std::uint64_t hash = 0;    // 基于 USR 的 64 位哈希，0 为无效 ID
    std::string signature;      // 辅助签名，用于解决哈希碰撞（概率极低）
};
```

基于 Clang 的 USR（Unified Symbol Resolution）生成哈希值，确保跨编译单元的符号一致性。

### `SymbolKind` — 符号类型枚举

涵盖 C++ 所有主要实体类型：`Namespace`、`Class`、`Struct`、`Union`、`Enum`、`EnumMember`、`Function`、`Method`、`Variable`、`Field`、`TypeAlias`、`Macro`、`Template`、`Concept`、`Unknown`。

### `SymbolInfo` — 符号完整信息

参见架构概览。关键设计点：

- **位置信息分离**：`declaration_location`（声明位置）与 `definition_location`（定义位置，可选）分开存储
- **源码片段按需加载**：`source_snippet` 可能为空；此时通过 `source_snippet_offset` / `source_snippet_length` / `source_snippet_file_size` 在需要时从磁盘读取
- **关系图**：`bases`/`derived`（继承）、`calls`/`called_by`（调用）、`references`/`referenced_by`（引用）
- **层次结构**：`parent`/`children` 表示父子关系（如类与成员）

### `ProjectModel` — 项目模型

见架构概览。关键设计点：

- **`symbol_ids_by_qualified_name`**：支持重载的全限定名查找（一个名字对应多个 `SymbolID`）
- **`file_order`**：基于依赖关系的拓扑排序结果，确保生成阶段按依赖顺序处理文件
- **`uses_modules`**：快速判断项目是否使用 C++20 模块

---

## 提取管道详解

### 阶段 1：编译数据库加载（`compiler` 模块）

```
load_compdb(path)
  └─▶ 解析 compile_commands.json
  └─▶ 对每个编译条目：
        ├─▶ normalize_entry_file() → 规范化文件路径
        ├─▶ build_compile_signature() → 构建编译签名
        ├─▶ ensure_cache_key() → 构建缓存键
        └─▶ normalize_argument_path() → 规范化参数路径
```

**`CompileEntry`** 数据结构：

| 字段 | 用途 |
|------|------|
| `file` | 源文件路径 |
| `directory` | 编译工作目录 |
| `arguments` | 编译参数列表 |
| `normalized_file` | 规范化后的文件路径 |
| `cache_key` | 缓存键（规范化路径 + 编译签名 + 源码哈希） |
| `compile_signature` | 编译签名 |
| `source_hash` | 源码文件哈希 |

### 阶段 2：依赖图构建（`scan` 模块）

```
build_dependency_graph_async(filtered_db, dep_graph, scan_cache, loop)
  └─▶ 对每个文件并行扫描：
        └─▶ scan_file() → ScanResult
              ├─▶ 读取文件内容
              ├─▶ scan_module_decl() → 快速扫描模块声明
              │     （使用 Clang 的依赖指令扫描器，无需完整预处理）
              └─▶ 解析 #include 指令 → include 列表
```

**`ScanResult`** 包含：`module_name`、`is_interface_unit`、`module_imports`、`includes`。

**`DependencyGraph`** 包含所有文件的依赖边，然后通过 `topological_order()` 计算拓扑排序顺序。

**`ScanCache`** 用于缓存扫描结果，避免重复扫描未修改的文件。

### 阶段 3：AST 提取（`ast` 模块）

```
extract_symbols(entry, ...) → std::expected<ASTResult, ASTError>
  └─▶ 创建 Clang 编译器实例（create_compiler_instance）
  └─▶ 运行 Clang 前端，遍历 AST
  └─▶ 提取：
        ├─▶ 符号列表（SymbolInfo）
        ├─▶ 关系列表（ExtractedRelation）：继承/调用/引用
        └─▶ 依赖快照（DependencySnapshot）
```

**`ExtractedRelation`** 数据结构：

| 字段 | 用途 |
|------|------|
| `from` | 源符号 ID |
| `to` | 目标符号 ID |
| `is_call` | true = 调用关系 |
| `is_inheritance` | true = 继承关系（from=派生，to=基类） |

未标记为调用或继承的关系被视为引用关系。

### 阶段 4：缓存评估与 AST 批处理

在 `extract_project_async()` 中，核心流程：

1. **缓存评估**：对每个编译条目，检查缓存记录是否有效（编译签名 + 源码哈希比对）
2. **依赖变更检测**：`cache::dependencies_changed()` 检查 AST 依赖文件的哈希是否变化
3. **并行 AST 提取**：`extract_ast_batch_async()` 使用线程池并行执行 AST 提取
4. **缓存命中**：命中缓存的条目从缓存记录直接读取 AST 结果

### 阶段 5：符号合并与索引重建（`merge` 模块）

```
rebuild_model_indexes(config, model)
  └─▶ 建立符号的父子/命名空间层次结构
  └─▶ 构建文件→符号、命名空间→符号的映射

rebuild_lookup_maps(model)
  └─▶ symbol_ids_by_qualified_name → 全限定名查找
```

`merge_symbol_info()` 用于合并同名符号（如跨编译单元的前向声明与定义），保留所有非空信息。

### 阶段 6：模块信息构建

```
build_module_info(model, scan_cache)
  └─▶ 从扫描结果收集模块声明
  └─▶ 构建 ModuleUnit 列表
  └─▶ 建立 module_name_to_sources 映射
```

### 阶段 7：源码片段解析

```
resolve_source_snippet(symbol)
  └─▶ 若 source_snippet 为空，从磁盘文件读取偏移和长度范围内的源码
  └─▶ 使用线程池执行（kota::queue），避免阻塞事件循环
```

---

## 过滤机制（`filter` 模块）

`FilterRule` 包含 `include` 和 `exclude` 模式列表。过滤处理：

1. **根路径过滤**：`filter_root_path()` 从 `include` 模式确定项目根路径
2. **路径匹配**：`matches_filter()` 检查路径是否匹配 include/exclude 模式
3. **路径规范化**：`resolve_path_under_directory()` 和 `canonical_graph_path()` 确保路径一致性

---

## 缓存系统（`cache` 命名空间）

`clore::extract::cache` 模块（`src/extract/cache.cppm`）实现：

- **`CacheRecord`**：包含编译签名、源码哈希、AST 依赖快照、扫描结果、AST 结果
- **`DependencySnapshot`**：记录 AST 提取时依赖文件的哈希，用于后续变更检测
- **缓存持久化**：`load_caches_async()` / `save_caches_async()` 将缓存序列化到磁盘
- **Clice 缓存**：`CliceCache` 缓存 Clang 编译实例状态（PCH/PCM），加速重复编译

---

## 关键设计决策

### 为什么使用 Clang 的 USR 作为符号标识？

USR（Unified Symbol Resolution）是 Clang 提供的跨编译单元符号标识机制。基于 USR 生成哈希（`SymbolID::hash`）的优势：

- 同一符号在不同文件中保持一致的 ID
- 支持跨编译单元的符号合并
- 不依赖文件路径或行号，抗重命名

### 为什么分离 AST 提取和依赖扫描？

依赖扫描（`scan_file()`）是轻量级的，只需分析 `#include` 和 `module` 声明，不需要完整预处理。而 AST 提取（`extract_symbols()`）需要完整的 Clang 前端处理，开销较大。两者分离允许：

1. 先快速建立完整的依赖图
2. 按拓扑顺序并行提取 AST
3. 缓存扫描结果，在 AST 缓存命中时避免重复扫描

### 为什么在提取阶段就解析源码片段？

源码片段（`source_snippet`）用于生成阶段构造 LLM 提示词（prompt）。在提取阶段解析并存储，避免了生成阶段再次读取磁盘文件的延迟。但为了节省内存，采用按需加载策略——仅在首次访问时从磁盘读取。

