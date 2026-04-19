module;

#include "kota/async/async.h"
#include "kota/codec/json/json.h"

#include "llvm/Support/xxhash.h"

export module agent;

export import :tools;

import std;
import config;
import extract;
import generate;
import network;
import support;

export namespace clore::agent {

struct AgentError {
    std::string message;
};

/// Run the agent loop: explore the codebase via tool calls and produce
/// guide documents under ${output_root}/guides/.
auto run_agent(const config::TaskConfig& config,
               const extract::ProjectModel& model,
               std::string_view llm_model,
               std::string output_root) -> std::expected<std::size_t, AgentError>;

/// Asynchronous agent loop that runs on the provided event loop.
/// Callers must schedule the returned task on the loop and run it.
auto run_agent_async(const config::TaskConfig& config,
                     const extract::ProjectModel& model,
                     std::string llm_model,
                     std::string output_root,
                     kota::event_loop& loop) -> kota::task<std::size_t, AgentError>;

}  // namespace clore::agent

namespace clore::agent {

namespace json = kota::codec::json;

namespace {

constexpr std::size_t kMaxAgentTurns = 64;

constexpr std::string_view kAgentSystemPrompt =
    R"(You are an expert software architecture analyst exploring a C++ codebase through tool calls.

Your goal is to thoroughly understand the project architecture and produce accurate, evidence-based guide documents.

EXPLORATION STRATEGY (follow this order):

Phase 1 - Survey: Start with project_overview to understand scale, modules, namespaces, and symbol counts.

Phase 2 - Structure: Explore top-level namespaces and modules. Use list_namespaces, list_modules, and list_files to understand the organizational boundary.

Phase 3 - Core abstractions: Identify the most important classes and functions by examining symbol counts and cross-references. Use get_symbol on key symbols and get_dependencies to understand both what they call AND what calls them.

Phase 4 - Data flow: Trace key operations from entry points through to implementation. Follow calls and called_by relationships systematically.

Phase 5 - Verification: Before writing any guide, list your key findings as bullet points. For each finding, note which symbol or module supports it. If you are unsure about a relationship, use get_dependencies again to verify.

GUIDE CREATION STRATEGY:

Create guides in this order:
1. architecture-overview (FIRST): High-level project structure, core modules/namespaces, key abstractions, and data flow.
2. module-deep-dive or namespace-deep-dive: Detailed guides for the most important modules or namespaces.
3. developer-guide: Getting started, key patterns, important conventions.

VERIFICATION REQUIREMENT:
Before calling create_guide, you MUST:
- List 3-5 key architectural findings in your reasoning
- For each finding, state the exact symbol or module name that supports it
- If you cannot find supporting evidence for a claim, do not include it in the guide

Use the create_guide tool with a kebab-case name (e.g. "architecture-overview"). The content should be complete, well-structured markdown.

When you have finished creating all useful guides, respond with a final text message summarizing what you produced.)";

struct ToolCallResult {
    std::string tool_call_id;
    std::string content;
};

auto run_tool_call(const clore::net::ToolCall& tool_call,
                   const extract::ProjectModel& model,
                   std::string_view project_root,
                   std::string_view output_root) -> ToolCallResult {
    ToolCallResult result{
        .tool_call_id = tool_call.id,
        .content = {},
    };

    auto dispatch_result =
        dispatch_tool_call(tool_call.name, tool_call.arguments, model, project_root, output_root);
    if(!dispatch_result.has_value()) {
        result.content = std::format("Tool error: {}", dispatch_result.error().message);
    } else {
        result.content = std::move(*dispatch_result);
    }

    return result;
}

auto hash_messages(const std::vector<clore::net::Message>& messages) -> std::uint64_t {
    std::string concat;
    for(const auto& msg: messages) {
        std::visit(
            [&](const auto& m) {
                using T = std::remove_cvref_t<decltype(m)>;
                if constexpr(std::same_as<T, clore::net::SystemMessage>) {
                    concat += 'S';
                    concat += m.content;
                    concat += '\0';
                } else if constexpr(std::same_as<T, clore::net::UserMessage>) {
                    concat += 'U';
                    concat += m.content;
                    concat += '\0';
                } else if constexpr(std::same_as<T, clore::net::AssistantMessage>) {
                    concat += 'A';
                    concat += m.content;
                    concat += '\0';
                } else if constexpr(std::same_as<T, clore::net::AssistantToolCallMessage>) {
                    concat += 'A';
                    if(m.content.has_value()) {
                        concat += *m.content;
                    }
                    concat += '\0';
                    for(const auto& tc: m.tool_calls) {
                        concat += 'T';
                        concat += tc.name;
                        concat += '\0';
                        concat += tc.arguments_json;
                        concat += '\0';
                    }
                } else if constexpr(std::same_as<T, clore::net::ToolResultMessage>) {
                    concat += 'R';
                    concat += m.tool_call_id;
                    concat += '\0';
                    concat += m.content;
                    concat += '\0';
                }
            },
            msg);
    }
    return llvm::xxh3_64bits(concat);
}

auto make_agent_cache_key(const std::vector<clore::net::Message>& messages) -> std::string {
    auto hash = hash_messages(messages);
    return std::format("agent\t{}", hash);
}

auto serialize_completion_response(const clore::net::CompletionResponse& response)
    -> std::expected<std::string, clore::net::LLMError> {
    auto root = clore::net::detail::make_empty_object("agent cache serialization");
    if(!root.has_value()) {
        return std::unexpected(std::move(root.error()));
    }

    if(response.message.text.has_value()) {
        if(auto status = root->insert("t", std::string_view(*response.message.text));
           !status.has_value()) {
            return std::unexpected(clore::net::LLMError("failed to serialize agent cache text"));
        }
    }
    if(response.message.refusal.has_value()) {
        if(auto status = root->insert("r", std::string_view(*response.message.refusal));
           !status.has_value()) {
            return std::unexpected(clore::net::LLMError("failed to serialize agent cache refusal"));
        }
    }
    if(!response.message.tool_calls.empty()) {
        auto calls = clore::net::detail::make_empty_array("agent cache tool_calls");
        if(!calls.has_value()) {
            return std::unexpected(std::move(calls.error()));
        }
        for(const auto& call: response.message.tool_calls) {
            auto call_obj = clore::net::detail::make_empty_object("agent cache tool_call");
            if(!call_obj.has_value()) {
                return std::unexpected(std::move(call_obj.error()));
            }
            if(auto status = call_obj->insert("i", std::string_view(call.id));
               !status.has_value()) {
                return std::unexpected(clore::net::LLMError("failed to serialize tool call id"));
            }
            if(auto status = call_obj->insert("n", std::string_view(call.name));
               !status.has_value()) {
                return std::unexpected(clore::net::LLMError("failed to serialize tool call name"));
            }
            auto parsed_args = kota::codec::json::Value::parse(call.arguments_json);
            if(!parsed_args.has_value()) {
                return std::unexpected(clore::net::LLMError("failed to parse tool call arguments"));
            }
            if(auto status = call_obj->insert("a", std::move(*parsed_args)); !status.has_value()) {
                return std::unexpected(
                    clore::net::LLMError("failed to insert tool call arguments"));
            }
            if(auto status = calls->push_back(std::move(*call_obj)); !status.has_value()) {
                return std::unexpected(clore::net::LLMError("failed to append tool call"));
            }
        }
        if(auto status = root->insert("c", std::move(*calls)); !status.has_value()) {
            return std::unexpected(
                clore::net::LLMError("failed to serialize agent cache tool_calls"));
        }
    }

    auto encoded = root->to_json_string();
    if(!encoded.has_value()) {
        return std::unexpected(clore::net::LLMError("failed to encode agent cache response"));
    }
    return *encoded;
}

auto deserialize_completion_response(std::string_view raw_json)
    -> std::expected<clore::net::CompletionResponse, clore::net::LLMError> {
    auto parsed = kota::codec::json::Object::parse(raw_json);
    if(!parsed.has_value()) {
        return std::unexpected(clore::net::LLMError("failed to parse cached agent response"));
    }

    clore::net::AssistantOutput output;

    if(auto text_value = parsed->get("t"); text_value.has_value()) {
        auto text = text_value->get_string();
        if(text.has_value()) {
            output.text = std::string(*text);
        }
    }
    if(auto refusal_value = parsed->get("r"); refusal_value.has_value()) {
        auto refusal = refusal_value->get_string();
        if(refusal.has_value()) {
            output.refusal = std::string(*refusal);
        }
    }
    if(auto calls_value = parsed->get("c"); calls_value.has_value()) {
        auto calls = clore::net::detail::expect_array(*calls_value, "cached tool_calls");
        if(!calls.has_value()) {
            return std::unexpected(std::move(calls.error()));
        }
        for(auto call_value: *calls) {
            auto call_obj = clore::net::detail::expect_object(call_value, "cached tool_call");
            if(!call_obj.has_value()) {
                return std::unexpected(std::move(call_obj.error()));
            }
            auto id_value = call_obj->get("i");
            if(!id_value.has_value()) {
                return std::unexpected(clore::net::LLMError("missing tool_call.id"));
            }
            auto id = clore::net::detail::expect_string(*id_value, "tool_call.id");
            if(!id.has_value()) {
                return std::unexpected(std::move(id.error()));
            }
            auto name_value = call_obj->get("n");
            if(!name_value.has_value()) {
                return std::unexpected(clore::net::LLMError("missing tool_call.name"));
            }
            auto name = clore::net::detail::expect_string(*name_value, "tool_call.name");
            if(!name.has_value()) {
                return std::unexpected(std::move(name.error()));
            }
            auto args_value = call_obj->get("a");
            if(!args_value.has_value()) {
                return std::unexpected(clore::net::LLMError("missing tool_call.arguments"));
            }
            auto args_copy = kota::codec::json::Value::copy_of(*args_value);
            if(!args_copy.has_value()) {
                return std::unexpected(clore::net::LLMError("failed to copy tool_call.arguments"));
            }
            auto args_string = args_copy->to_json_string();
            if(!args_string.has_value()) {
                return std::unexpected(
                    clore::net::LLMError("failed to serialize tool_call.arguments"));
            }

            output.tool_calls.push_back(clore::net::ToolCall{
                .id = std::string(*id),
                .name = std::string(*name),
                .arguments_json = std::string(*args_string),
                .arguments = std::move(*args_copy),
            });
        }
    }

    return clore::net::CompletionResponse{
        .id = "cached",
        .model = "cached",
        .message = std::move(output),
        .raw_json = std::string(raw_json),
    };
}

auto list_existing_guide_filenames(std::string_view output_root) -> std::vector<std::string> {
    namespace fs = std::filesystem;

    std::vector<std::string> filenames;
    auto guides_dir = fs::path(std::string(output_root)) / "guides";

    std::error_code ec;
    if(!fs::exists(guides_dir, ec) || ec || !fs::is_directory(guides_dir, ec)) {
        return filenames;
    }

    for(const auto& entry: fs::directory_iterator(guides_dir, ec)) {
        if(ec) {
            break;
        }
        if(!entry.is_regular_file(ec) || ec) {
            continue;
        }
        auto path = entry.path();
        if(path.extension() != ".md") {
            continue;
        }
        filenames.push_back(path.filename().string());
    }

    std::ranges::sort(filenames);
    return filenames;
}

auto run_agent_loop(const config::TaskConfig& config,
                    const extract::ProjectModel& model,
                    std::string_view llm_model,
                    std::string_view output_root,
                    clore::generate::cache::CacheIndex& cache_index,
                    kota::event_loop& loop) -> kota::task<std::size_t, AgentError> {
    auto tool_defs_result = build_tool_definitions();
    if(!tool_defs_result.has_value()) {
        co_await kota::fail(
            AgentError{.message = std::format("failed to build tool definitions: {}",
                                              tool_defs_result.error().message)});
    }

    auto& tool_defs = *tool_defs_result;

    // Build initial messages
    std::vector<clore::net::Message> messages;
    std::string system_prompt = std::string(kAgentSystemPrompt);
    if(!config.llm.system_prompt.empty()) {
        system_prompt += "\n\n";
        system_prompt += config.llm.system_prompt;
    }
    messages.push_back(clore::net::SystemMessage{
        .content = std::move(system_prompt),
    });

    auto existing_guides = list_existing_guide_filenames(output_root);
    std::string initial_prompt =
        "Explore the codebase and create architectural guide documents. Start by calling "
        "project_overview.";
    if(existing_guides.empty()) {
        initial_prompt +=
            "\n\nThere are currently no existing guide files under guides/.";
    } else {
        initial_prompt +=
            "\n\nExisting guide files already present under guides/ are listed below. Treat "
            "them as existing project artifacts. Read them with the read_guide tool before "
            "rewriting, extending, or duplicating them.\n";
        for(const auto& filename: existing_guides) {
            initial_prompt += std::format("- {}\n", filename);
        }
    }

    messages.push_back(clore::net::UserMessage{
        .content = std::move(initial_prompt),
    });

    std::size_t guides_created = 0;

    for(std::size_t turn = 0; turn < kMaxAgentTurns; ++turn) {
        auto cache_key = make_agent_cache_key(messages);

        // Try cache first
        auto cached = clore::generate::cache::find_cached_response(cache_index, cache_key);
        clore::net::CompletionResponse response;
        bool cache_hit = false;

        if(cached.has_value()) {
            auto parsed = deserialize_completion_response(*cached);
            if(parsed.has_value()) {
                response = std::move(*parsed);
                cache_hit = true;
                logging::debug("agent cache hit for turn {} ({} messages)", turn, messages.size());
            }
        }

        if(!cache_hit) {
            clore::net::CompletionRequest request{
                .model = std::string(llm_model),
                .messages = messages,
                .tools = tool_defs,
                .tool_choice = clore::net::ToolChoiceAuto{},
            };

            auto response_result = co_await clore::net::call_completion_async(config.llm.provider,
                                                                              std::move(request),
                                                                              loop)
                                       .catch_cancel();

            if(response_result.is_cancelled()) {
                co_await kota::fail(AgentError{.message = "agent LLM request cancelled"});
            }
            if(response_result.has_error()) {
                co_await kota::fail(
                    AgentError{.message = std::format("agent LLM failed: {}",
                                                      std::move(response_result).error().message)});
            }

            response = std::move(*response_result);

            // Save to cache in provider-independent format
            auto serialized = serialize_completion_response(response);
            if(serialized.has_value()) {
                auto save_result = co_await clore::generate::cache::save_cache_entry_async(
                                       std::string(config.workspace_root),
                                       cache_key,
                                       *serialized,
                                       loop)
                                       .catch_cancel();
                if(save_result.is_cancelled()) {
                    logging::warn("agent cache save cancelled for turn {}", turn);
                } else if(save_result.has_error()) {
                    logging::warn("agent cache save failed for turn {}: {}",
                                  turn,
                                  save_result.error().message);
                } else {
                    cache_index.entries.insert_or_assign(std::move(cache_key),
                                                         std::string(*serialized));
                }
            }
        }

        // If no tool calls, the agent is done
        if(response.message.tool_calls.empty()) {
            break;
        }

        // Add assistant message to conversation
        clore::net::AssistantToolCallMessage assistant_msg;
        assistant_msg.content = response.message.text;
        assistant_msg.tool_calls = response.message.tool_calls;
        messages.push_back(std::move(assistant_msg));

        // Process tool calls in parallel while preserving output order.
        std::vector<kota::task<ToolCallResult, kota::error>> tasks;
        tasks.reserve(response.message.tool_calls.size());

        auto project_root = std::string(config.project_root);
        auto output_root_str = std::string(output_root);

        for(auto& tool_call: response.message.tool_calls) {
            logging::debug("agent tool call: {} ({})", tool_call.name, tool_call.id);
            tasks.push_back(kota::queue(
                [tool_call, &model, project_root, output_root_str]() {
                    return run_tool_call(tool_call, model, project_root, output_root_str);
                },
                loop));
        }

        auto all_result = co_await kota::when_all(std::move(tasks));

        if(all_result.has_error()) {
            co_await kota::fail(AgentError{
                .message = std::format("tool call failed: error {}", all_result.error().value())});
        }

        auto& tool_results = *all_result;
        for(std::size_t i = 0; i < response.message.tool_calls.size(); ++i) {
            if(response.message.tool_calls[i].name == "create_guide") {
                guides_created++;
            }
            auto& tool_result = tool_results[i];

            messages.push_back(clore::net::ToolResultMessage{
                .tool_call_id = std::move(tool_result.tool_call_id),
                .content = std::move(tool_result.content),
            });
        }
    }

    co_return guides_created;
}

}  // namespace

auto run_agent_async(const config::TaskConfig& config,
                     const extract::ProjectModel& model,
                     std::string llm_model,
                     std::string output_root,
                     kota::event_loop& loop) -> kota::task<std::size_t, AgentError> {
    auto cache_result = clore::generate::cache::load_cache_index(config.workspace_root);
    clore::generate::cache::CacheIndex cache_index;
    if(cache_result.has_value()) {
        cache_index = std::move(*cache_result);
        logging::info("loaded agent cache: {} entries", cache_index.entries.size());
    } else {
        logging::warn("failed to load agent cache: {}", cache_result.error().message);
    }

    co_return co_await run_agent_loop(config, model, llm_model, output_root, cache_index, loop);
}

auto run_agent(const config::TaskConfig& config,
               const extract::ProjectModel& model,
               std::string_view llm_model,
               std::string output_root) -> std::expected<std::size_t, AgentError> {
    kota::event_loop loop;
    auto task = run_agent_async(config, model, std::string(llm_model), std::move(output_root), loop);
    loop.schedule(task);
    loop.run();

    auto result = task.result();
    if(result.has_error()) {
        return std::unexpected(std::move(result).error());
    }

    return *result;
}

}  // namespace clore::agent
