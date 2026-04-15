# clore

![C++ Standard](https://img.shields.io/badge/C++-23-blue.svg)
[![GitHub license](https://img.shields.io/github/license/clice-io/clore)](https://github.com/clice-io/clore/blob/main/LICENSE)
[![generate-docs](https://github.com/clice-io/clore/actions/workflows/generate-docs.yml/badge.svg)](https://github.com/clice-io/clore/actions/workflows/generate-docs.yml)
[![Documentation](https://img.shields.io/badge/view-documentation-blue)](https://docs.clice.io/clore/)

clore is a document generator for humans and agents that combines LLVM and LLM technologies and is written in modern C++.

## How to build

Run `pixi run build`. If you don't have pixi installed, install it first.

## CLI usage

```bash
clore \
  --config clore.toml \
  --compile-commands build/compile_commands.json \
  --source-dir . \
  --output-dir docs/api \
  --dry-run
```

### Arguments

- `--config`: path to config file (default: `clore.toml`)
- `--compile-commands`: path to `compile_commands.json` (required)
- `--source-dir`: source root directory (required)
- `--output-dir`: output root directory (required)
- `--dry-run`: generate prompt-assembled docs without LLM calls
- `--model`: model name for online generation (mutually exclusive with `--dry-run`)
- `--rate-limit`: max concurrent LLM requests when `--model` is used (default: `16`)
- `--log-level`: override log level (`trace|debug|info|warn|error|off`)

## `clore.toml`

```toml
# Optional. Can be overridden by --log-level.
# log_level = "info"

[filter]
include = ["src/"]
exclude = []

[extract]
max_snippet_bytes = 8192

[evidence_rules]
max_callers = 5
max_callees = 5
max_siblings = 8
max_source_bytes = 4096
max_related_summaries = 3
max_top_modules = 8
max_top_namespaces = 8

[llm]
system_prompt = "You are a C++ documentation writer."
retry_count = 3
retry_initial_backoff_ms = 250
```
