# clore

![C++ Standard](https://img.shields.io/badge/C++-23-blue.svg)
[![GitHub license](https://img.shields.io/github/license/clice-io/clore)](https://github.com/clice-io/clore/blob/main/LICENSE)
[![Build](https://github.com/clice-io/clore/actions/workflows/verify-build.yml/badge.svg)](https://github.com/clice-io/clore/actions/workflows/verify-build.yml)
[![Documentation](https://img.shields.io/badge/view-documentation-blue)](https://docs.clice.io/clore/)

clore is a document generator for humans and agents that combines LLVM and LLM technologies and is written in modern C++.

## How to build

Run `pixi run build`. If you don't have pixi installed, [install it first](https://pixi.sh/).

The default build path installs third-party dependencies via Conan before CMake configure.

```bash
pixi run build RelWithDebInfo
```

If you want to run each step explicitly:

```bash
pixi run install RelWithDebInfo
pixi run config RelWithDebInfo
pixi run check RelWithDebInfo
pixi run format
pixi run build RelWithDebInfo
```

Conan-managed dependencies are `spdlog`, `libcurl`, and `simdjson`.

LLVM and Clang are provided by the `pixi` environment (conda-forge packages: `clang`, `clangdev`, `llvmdev`, etc.).

## CLI usage

### Standard mode

Generate documentation from a compilation database:

```bash
clore \
  --config clore.en.toml \
  --compile-commands build/{{ type }}/compile_commands.json \
  --source-dir . \
  --output-dir docs/api \
  --dry-run
```

### Agent mode

Enable autonomous codebase exploration and guide generation:

```bash
clore \
  --config clore.en.toml \
  --source-dir . \
  --output-dir docs/guides \
  --experimental-agent-mode \
  --model gpt-4o
```

### Arguments

| Flag | Description |
| --- | --- |
| `--config` | Path to config file (default: `clore.toml`; for typical use, start from `clore.en.toml` or `clore.zh.toml` and pass that path here) |
| `--compile-commands` | Path to `compile_commands.json` |
| `--source-dir` | Source root directory for relative output paths |
| `--output-dir` | Output root directory |
| `--dry-run` | Write assembled prompts to `--output-dir` and skip LLM calls |
| `--model` | Model name for online generation (e.g. `gpt-4o`, `claude-3-5-sonnet-20241022`) |
| `--rate-limit` | Max concurrent LLM requests when `--model` is used (default: `16`) |
| `--experimental-agent-mode` | Enable agent-driven autonomous exploration and guide generation |
| `--log-level` | Override log level (`trace\|debug\|info\|warn\|error\|off`) |

## Configuration

Configuration files support localization. Use `clore.en.toml` or `clore.zh.toml` as a starting point.

### `clore.en.toml`

```toml
[filter]
include = ["src/"]
exclude = []

[llm]
system_prompt = "You are a C++ documentation writer. Please generate concise and accurate documentation for C++ code elements. Focus on the purpose, design intent, and role of each element within the codebase. The output documentation must be in Markdown format and suitable for embedding within a document page. Use English in the generated documentation."
retry_count = 10
retry_initial_backoff_ms = 5000
```

### LLM provider selection

Provider selection is automatic at the network layer:

- use Anthropic when both `ANTHROPIC_BASE_URL` and `ANTHROPIC_API_KEY` are set
- otherwise use OpenAI when both `OPENAI_BASE_URL` and `OPENAI_API_KEY` are set
