---
title: 'API Reference'
description: '本项目是一个名为 clore 的 C++ 库，旨在利用大型语言模型（LLM）来自动探索代码库并生成结构化指南文档。其核心子系统包括：智能代理（agent 模块）驱动 LLM 交互与工具调用的主循环；LLM 通信层（client、network、http、protocol 模块）提供与 OpenAI 和 Anthropic 等提供者的异步请求、响应解析及协议适配；文档生成（generate、extract 模块）负责从源代码提取符号、分析依赖并生成最终文档页面；配置与 schema（config、schema 模块）管理应用设置并自动为 C++ 类型生成兼容的 JSON Schema；以及 支持工具（support 模块）提供 Unicode 文本处理、文件读写、路径规范化等基础能力。读者可将该库视为一个端到端的文档自动编写框架，各模块围绕“代码理解→LLM 调用→文档产出”这一主线解耦协作，上层可灵活组合协议层与生成策略以适配不同 LLM 服务与输出格式。'
layout: doc
template: doc
---

# API Reference

## Overview

本项目是一个名为 `clore` 的 C++ 库，旨在利用大型语言模型（LLM）来自动探索代码库并生成结构化指南文档。其核心子系统包括：**智能代理**（`agent` 模块）驱动 LLM 交互与工具调用的主循环；**LLM 通信层**（`client`、`network`、`http`、`protocol` 模块）提供与 `OpenAI` 和 Anthropic 等提供者的异步请求、响应解析及协议适配；**文档生成**（`generate`、`extract` 模块）负责从源代码提取符号、分析依赖并生成最终文档页面；**配置与 schema**（`config`、`schema` 模块）管理应用设置并自动为 C++ 类型生成兼容的 JSON Schema；以及 **支持工具**（`support` 模块）提供 Unicode 文本处理、文件读写、路径规范化等基础能力。读者可将该库视为一个端到端的文档自动编写框架，各模块围绕“代码理解→LLM 调用→文档产出”这一主线解耦协作，上层可灵活组合协议层与生成策略以适配不同 LLM 服务与输出格式。

## Modules

- [`agent`](modules/agent/index.md)
- [`agent:tools`](modules/agent/tools.md)
- [`anthropic`](modules/anthropic/index.md)
- [`client`](modules/client/index.md)
- [`config`](modules/config/index.md)
- [`config:load`](modules/config/load.md)
- [`config:normalize`](modules/config/normalize.md)
- [`config:schema`](modules/config/schema.md)
- [`config:validate`](modules/config/validate.md)
- [`extract`](modules/extract/index.md)
- [`extract:ast`](modules/extract/ast.md)
- [`extract:cache`](modules/extract/cache.md)
- [`extract:compiler`](modules/extract/compiler.md)
- [`extract:filter`](modules/extract/filter.md)
- [`extract:merge`](modules/extract/merge.md)
- [`extract:model`](modules/extract/model.md)
- [`extract:scan`](modules/extract/scan.md)
- [`generate`](modules/generate/index.md)
- [`generate:analysis`](modules/generate/analysis.md)
- [`generate:cache`](modules/generate/cache.md)
- [`generate:common`](modules/generate/common.md)
- [`generate:diagram`](modules/generate/diagram.md)
- [`generate:dryrun`](modules/generate/dryrun.md)
- [`generate:evidence`](modules/generate/evidence.md)
- [`generate:markdown`](modules/generate/markdown.md)
- [`generate:model`](modules/generate/model.md)
- [`generate:page`](modules/generate/page.md)
- [`generate:planner`](modules/generate/planner.md)
- [`generate:scheduler`](modules/generate/scheduler.md)
- [`generate:symbol`](modules/generate/symbol.md)
- [`http`](modules/http/index.md)
- [`network`](modules/network/index.md)
- [`openai`](modules/openai/index.md)
- [`protocol`](modules/protocol/index.md)
- [`provider`](modules/provider/index.md)
- [`schema`](modules/schema/index.md)
- [`support`](modules/support/index.md)

## Namespaces

- [`clore`](namespaces/clore/index.md)
- [`clore::agent`](namespaces/clore/agent/index.md)
- [`clore::config`](namespaces/clore/config/index.md)
- [`clore::extract`](namespaces/clore/extract/index.md)
- [`clore::extract::cache`](namespaces/clore/extract/cache/index.md)
- [`clore::generate`](namespaces/clore/generate/index.md)
- [`clore::generate::cache`](namespaces/clore/generate/cache/index.md)
- [`clore::logging`](namespaces/clore/logging/index.md)
- [`clore::net`](namespaces/clore/net/index.md)
- [`clore::net::anthropic`](namespaces/clore/net/anthropic/index.md)
- [`clore::net::anthropic::detail`](namespaces/clore/net/anthropic/detail/index.md)
- [`clore::net::anthropic::protocol`](namespaces/clore/net/anthropic/protocol/index.md)
- [`clore::net::anthropic::protocol::detail`](namespaces/clore/net/anthropic/protocol/detail/index.md)
- [`clore::net::anthropic::schema`](namespaces/clore/net/anthropic/schema/index.md)
- [`clore::net::detail`](namespaces/clore/net/detail/index.md)
- [`clore::net::openai`](namespaces/clore/net/openai/index.md)
- [`clore::net::openai::detail`](namespaces/clore/net/openai/detail/index.md)
- [`clore::net::openai::protocol`](namespaces/clore/net/openai/protocol/index.md)
- [`clore::net::openai::protocol::detail`](namespaces/clore/net/openai/protocol/detail/index.md)
- [`clore::net::openai::schema`](namespaces/clore/net/openai/schema/index.md)
- [`clore::net::openai::schema::detail`](namespaces/clore/net/openai/schema/detail/index.md)
- [`clore::net::protocol`](namespaces/clore/net/protocol/index.md)
- [`clore::net::schema`](namespaces/clore/net/schema/index.md)
- [`clore::support`](namespaces/clore/support/index.md)

## Module Dependency Diagram

```mermaid
graph LR
    M0["agent"]
    M1["anthropic"]
    M2["client"]
    M3["config"]
    M4["extract"]
    M5["generate"]
    M6["http"]
    M7["network"]
    M8["openai"]
    M9["protocol"]
    M10["provider"]
    M11["schema"]
    M12["support"]
    M3 --> M0
    M4 --> M0
    M5 --> M0
    M7 --> M0
    M9 --> M0
    M11 --> M0
    M12 --> M0
    M2 --> M1
    M6 --> M1
    M9 --> M1
    M10 --> M1
    M11 --> M1
    M12 --> M1
    M6 --> M2
    M9 --> M2
    M11 --> M2
    M12 --> M2
    M12 --> M3
    M3 --> M4
    M12 --> M4
    M3 --> M5
    M4 --> M5
    M6 --> M5
    M7 --> M5
    M9 --> M5
    M12 --> M5
    M12 --> M6
    M2 --> M8
    M6 --> M8
    M9 --> M8
    M10 --> M8
    M11 --> M8
    M12 --> M8
    M6 --> M9
    M12 --> M9
    M6 --> M10
    M9 --> M10
    M11 --> M10
    M6 --> M11
    M9 --> M11
    M12 --> M11
```

