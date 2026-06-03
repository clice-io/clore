---
title: 'clore::generate::escapemermaidlabel'
description: '函数 clore::generate::escape_mermaid_label 的实现在一个简单的字符遍历循环中完成。它预先为 escaped 字符串分配与输入 text 相同大小的容量（通过 reserve），然后对每个字符 ch 执行 switch 语句，将反斜杠 \\ 替换为 "\\\\"，双引号 " 替换为 "\\\""，换行符 \n 和回车符 \r 统一替换为空格 '' ''，其余字符直接添加。这种逐字处理的策略避免了任何正则表达式或状态机，使转义逻辑保持线性且可预测。'
layout: doc
template: doc
---

# `clore::generate::escape_mermaid_label`

Owner: [Module generate:diagram](../diagram.md)

Declaration: `src/generate/render/diagram.cppm:27`

Definition: `src/generate/render/diagram.cppm:123`

Declaration: [`Namespace clore::generate`](../../../namespaces/clore/generate/index.md)

## Implementation

```cpp
auto escape_mermaid_label(std::string_view text) -> std::string {
    std::string escaped;
    escaped.reserve(text.size());
    for(auto ch: text) {
        switch(ch) {
            case '\\': escaped += "\\\\"; break;
            case '"': escaped += "\\\""; break;
            case '\n':
            case '\r': escaped += ' '; break;
            default: escaped.push_back(ch); break;
        }
    }
    return escaped;
}
```

函数 `clore::generate::escape_mermaid_label` 的实现在一个简单的字符遍历循环中完成。它预先为 `escaped` 字符串分配与输入 `text` 相同大小的容量（通过 `reserve`），然后对每个字符 `ch` 执行 `switch` 语句，将反斜杠 `\\` 替换为 `"\\\\"`，双引号 `"` 替换为 `"\\\""`，换行符 `\n` 和回车符 `\r` 统一替换为空格 `' '`，其余字符直接添加。这种逐字处理的策略避免了任何正则表达式或状态机，使转义逻辑保持线性且可预测。

该函数的核心依赖仅限于 C++ 标准库的 `std::string` 和 `std::string_view`，不涉及项目内部的其他函数或外部库。控制流完全由 `for` 循环和 `switch`-`case` 分支构成，没有嵌套或回退分支，因此实现本身既是算法也是最终形式，没有隐藏的间接调用或中间数据结构。

## Side Effects

No observable side effects are evident from the extracted code.

## Reads From

- `text` parameter of type `std::string_view`

## Writes To

- Allocates and writes to a local `std::string escaped`, which is returned

## Usage Patterns

- Called by `clore::generate::render_namespace_diagram_code` to escape Mermaid node labels

## Called By

- function `clore::generate::render_namespace_diagram_code`

