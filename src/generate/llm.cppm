module;

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <format>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <curl/curl.h>

#include "llvm/Support/JSON.h"

export module generate:llm;

import support;

export namespace clore::generate {

struct LLMError {
    std::string message;
};

namespace detail {

auto build_request_json(std::string_view model, std::string_view system_prompt,
                        std::string_view prompt) -> std::string;

auto parse_response(std::string_view json) -> std::expected<std::string, LLMError>;

}  // namespace detail

auto call_llm(std::string_view model, std::string_view system_prompt,
              std::string_view prompt)
    -> std::expected<std::string, LLMError>;

/// Fully non-blocking LLM client using curl_multi.
///
/// Usage:
///   LLMClient client(model, system_prompt, max_concurrent);
///   client.submit(tag, prompt);
///   client.submit(tag, prompt);
///   client.run([](uint64_t tag, auto result) { ... });
///
/// The on_complete callback may call submit() to add more work. The event loop
/// continues until no pending or in-flight requests remain.
class LLMClient {
public:
    using Callback = std::function<void(std::uint64_t tag,
                                        std::expected<std::string, LLMError> result)>;

    LLMClient(std::string_view model, std::string_view system_prompt,
              std::uint32_t max_concurrent);

    ~LLMClient();

    LLMClient(const LLMClient&) = delete;
    auto operator=(const LLMClient&) -> LLMClient& = delete;
    LLMClient(LLMClient&&) = delete;
    auto operator=(LLMClient&&) -> LLMClient& = delete;

    /// Submit a prompt. `tag` is returned in the callback to identify the result.
    /// May be called before run() or from within the on_complete callback.
    auto submit(std::uint64_t tag, std::string prompt) -> std::expected<void, LLMError>;

    /// Run the event loop. Calls on_complete for each completed request.
    /// Returns when no more pending or in-flight requests remain.
    auto run(Callback on_complete) -> std::expected<void, LLMError>;

private:
    struct InFlightRequest {
        CURL* easy = nullptr;
        curl_slist* headers = nullptr;
        std::string request_body;
        std::string response_body;
        std::uint64_t tag = 0;
    };

    auto try_launch_pending() -> std::expected<void, LLMError>;
    auto launch_one(std::uint64_t tag, std::string prompt) -> std::expected<void, LLMError>;
    auto process_completed(Callback& on_complete) -> void;
    auto cleanup_request(CURL* easy) -> void;

    CURLM* multi_ = nullptr;
    std::string url_;
    std::string api_key_;
    std::string model_;
    std::string system_prompt_;
    std::uint32_t max_concurrent_;
    std::uint32_t in_flight_ = 0;
    std::queue<std::pair<std::uint64_t, std::string>> pending_;
    std::unordered_map<CURL*, InFlightRequest> requests_;
};

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

constexpr std::string_view kOpenAIBaseUrlEnv = "OPENAI_BASE_URL";
constexpr std::string_view kOpenAIApiKeyEnv = "OPENAI_API_KEY";

auto read_required_env(std::string_view name) -> std::expected<std::string, LLMError> {
    auto* value = std::getenv(std::string{name}.c_str());
    if(value == nullptr || value[0] == '\0') {
        return std::unexpected(LLMError{
            .message = std::format("required environment variable {} is not set", name)});
    }
    return std::string(value);
}

auto build_chat_completions_url(std::string_view api_base) -> std::string {
    std::string url(api_base);
    if(!url.empty() && url.back() != '/') {
        url += '/';
    }
    url += "chat/completions";
    return url;
}

auto escape_json_string(std::string_view s) -> std::string {
    std::string out;
    out.reserve(s.size() + 16);
    for(char c : s) {
        switch(c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if(static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
                    out += buf;
                } else {
                    out += c;
                }
                break;
        }
    }
    return out;
}

auto ensure_curl_global_init() -> std::expected<void, LLMError> {
    static std::once_flag once;
    static std::optional<LLMError> init_error;

    std::call_once(once, [] {
        auto code = curl_global_init(CURL_GLOBAL_DEFAULT);
        if(code != CURLE_OK) {
            init_error = LLMError{
                .message = std::format("curl_global_init failed: {}",
                                       curl_easy_strerror(code))};
        }
    });

    if(init_error.has_value()) {
        return std::unexpected(*init_error);
    }

    return {};
}

auto write_response_body(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
    -> std::size_t {
    auto* response = static_cast<std::string*>(userdata);
    auto bytes = size * nmemb;
    response->append(ptr, bytes);
    return bytes;
}

}  // anonymous namespace

namespace detail {

auto build_request_json(std::string_view model, std::string_view system_prompt,
                        std::string_view prompt) -> std::string {
    return std::format(
        R"({{"model":"{}","messages":[{{"role":"system","content":"{}"}},{{"role":"user","content":"{}"}}]}})",
        escape_json_string(model), escape_json_string(system_prompt),
        escape_json_string(prompt));
}

auto parse_response(std::string_view json) -> std::expected<std::string, LLMError> {
    auto parsed = llvm::json::parse(json);
    if(!parsed) {
        return std::unexpected(LLMError{
            .message = std::format("failed to parse LLM response JSON: {}",
                                   llvm::toString(parsed.takeError()))});
    }

    auto* root = parsed->getAsObject();
    if(!root) {
        return std::unexpected(LLMError{.message = "LLM response is not a JSON object"});
    }

    if(auto* error = root->getObject("error")) {
        auto msg = error->getString("message").value_or("unknown error");
        return std::unexpected(LLMError{
            .message = std::format("LLM API error: {}", std::string_view(msg))});
    }

    auto* choices = root->getArray("choices");
    if(!choices || choices->empty()) {
        return std::unexpected(LLMError{.message = "LLM response has no choices"});
    }

    auto* first = (*choices)[0].getAsObject();
    if(!first) {
        return std::unexpected(LLMError{.message = "LLM response choice is not an object"});
    }

    auto* message = first->getObject("message");
    if(!message) {
        return std::unexpected(LLMError{.message = "LLM response choice has no message"});
    }

    auto content = message->getString("content");
    if(!content) {
        return std::unexpected(LLMError{.message = "LLM response message has no content"});
    }

    return std::string(*content);
}

}  // namespace detail

// ── call_llm (synchronous single call, used by tests) ──────────────

auto call_llm(std::string_view model, std::string_view system_prompt,
              std::string_view prompt)
    -> std::expected<std::string, LLMError> {
    auto api_base_result = read_required_env(kOpenAIBaseUrlEnv);
    if(!api_base_result.has_value()) {
        return std::unexpected(std::move(api_base_result.error()));
    }

    auto api_key_result = read_required_env(kOpenAIApiKeyEnv);
    if(!api_key_result.has_value()) {
        return std::unexpected(std::move(api_key_result.error()));
    }

    auto curl_init = ensure_curl_global_init();
    if(!curl_init.has_value()) {
        return std::unexpected(std::move(curl_init.error()));
    }

    auto url = build_chat_completions_url(*api_base_result);

    CURL* easy = curl_easy_init();
    if(!easy) {
        return std::unexpected(LLMError{.message = "curl_easy_init failed"});
    }

    auto request_json = detail::build_request_json(model, system_prompt, prompt);
    std::string response_body;

    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    auto auth = std::format("Authorization: Bearer {}", *api_key_result);
    headers = curl_slist_append(headers, auth.c_str());

    curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
    curl_easy_setopt(easy, CURLOPT_POST, 1L);
    curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(easy, CURLOPT_POSTFIELDS, request_json.c_str());
    curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE_LARGE,
                     static_cast<curl_off_t>(request_json.size()));
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, &write_response_body);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1L);

    logging::info("calling LLM: {} model={}", url, model);

    auto code = curl_easy_perform(easy);

    curl_slist_free_all(headers);

    if(code != CURLE_OK) {
        auto msg = std::string(curl_easy_strerror(code));
        curl_easy_cleanup(easy);
        return std::unexpected(LLMError{.message = std::move(msg)});
    }

    long http_status = 0;
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_status);
    curl_easy_cleanup(easy);

    if(response_body.empty()) {
        return std::unexpected(LLMError{.message = "empty response from LLM"});
    }

    if(http_status >= 400) {
        return std::unexpected(LLMError{
            .message = std::format("LLM request failed with HTTP {}: {}",
                                   http_status, response_body.substr(0, 200))});
    }

    return detail::parse_response(response_body);
}

// ── LLMClient (non-blocking curl_multi) ─────────────────────────────

LLMClient::LLMClient(std::string_view model, std::string_view system_prompt,
                      std::uint32_t max_concurrent)
    : model_(model), system_prompt_(system_prompt), max_concurrent_(max_concurrent) {}

LLMClient::~LLMClient() {
    // Cleanup any remaining in-flight requests
    for(auto& [easy, req] : requests_) {
        if(multi_) {
            curl_multi_remove_handle(multi_, easy);
        }
        if(req.headers) {
            curl_slist_free_all(req.headers);
        }
        curl_easy_cleanup(easy);
    }
    requests_.clear();

    if(multi_) {
        curl_multi_cleanup(multi_);
    }
}

auto LLMClient::submit(std::uint64_t tag, std::string prompt)
    -> std::expected<void, LLMError> {
    pending_.push({tag, std::move(prompt)});
    return {};
}

auto LLMClient::run(Callback on_complete) -> std::expected<void, LLMError> {
    // Nothing to do
    if(pending_.empty() && in_flight_ == 0) {
        return {};
    }

    // Initialize environment
    auto api_base_result = read_required_env(kOpenAIBaseUrlEnv);
    if(!api_base_result.has_value()) {
        return std::unexpected(std::move(api_base_result.error()));
    }
    auto api_key_result = read_required_env(kOpenAIApiKeyEnv);
    if(!api_key_result.has_value()) {
        return std::unexpected(std::move(api_key_result.error()));
    }
    auto curl_init = ensure_curl_global_init();
    if(!curl_init.has_value()) {
        return std::unexpected(std::move(curl_init.error()));
    }

    url_ = build_chat_completions_url(*api_base_result);
    api_key_ = std::move(*api_key_result);

    multi_ = curl_multi_init();
    if(!multi_) {
        return std::unexpected(LLMError{.message = "curl_multi_init failed"});
    }

    // Event loop: launch pending, poll, process completed, repeat
    while(in_flight_ > 0 || !pending_.empty()) {
        // Launch as many pending requests as concurrency allows
        if(auto r = try_launch_pending(); !r.has_value()) {
            return std::unexpected(std::move(r.error()));
        }

        if(in_flight_ == 0) break;

        // Drive transfers
        int still_running = 0;
        auto mcode = curl_multi_perform(multi_, &still_running);
        if(mcode != CURLM_OK) {
            return std::unexpected(LLMError{
                .message = std::format("curl_multi_perform failed: {}",
                                       curl_multi_strerror(mcode))});
        }

        // Process completed transfers
        process_completed(on_complete);

        // Wait for activity (up to 1 second)
        if(in_flight_ > 0) {
            curl_multi_poll(multi_, nullptr, 0, 1000, nullptr);
        }
    }

    curl_multi_cleanup(multi_);
    multi_ = nullptr;

    return {};
}

auto LLMClient::try_launch_pending() -> std::expected<void, LLMError> {
    while(!pending_.empty() && in_flight_ < max_concurrent_) {
        auto [tag, prompt] = std::move(pending_.front());
        pending_.pop();
        if(auto r = launch_one(tag, std::move(prompt)); !r.has_value()) {
            return std::unexpected(std::move(r.error()));
        }
    }
    return {};
}

auto LLMClient::launch_one(std::uint64_t tag, std::string prompt)
    -> std::expected<void, LLMError> {
    InFlightRequest req;
    req.tag = tag;
    req.request_body = std::move(prompt);

    // Build the actual JSON body (request_body currently holds the raw prompt)
    auto raw_prompt = std::move(req.request_body);
    req.request_body = detail::build_request_json(model_, system_prompt_, raw_prompt);

    req.easy = curl_easy_init();
    if(!req.easy) {
        return std::unexpected(LLMError{.message = "curl_easy_init failed"});
    }

    // Headers
    req.headers = nullptr;
    req.headers = curl_slist_append(req.headers, "Content-Type: application/json");
    auto auth = std::format("Authorization: Bearer {}", api_key_);
    req.headers = curl_slist_append(req.headers, auth.c_str());

    auto* easy = req.easy;
    auto [it, inserted] = requests_.emplace(easy, std::move(req));
    if(!inserted) {
        curl_easy_cleanup(easy);
        return std::unexpected(LLMError{
            .message = std::format("duplicate LLM request handle for tag {}", tag)});
    }
    auto& stored_req = it->second;

    // libcurl keeps raw pointers to the POST body and write buffer; they must point
    // at storage owned by the in-flight request, not a local temporary.
    curl_easy_setopt(stored_req.easy, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(stored_req.easy, CURLOPT_POST, 1L);
    curl_easy_setopt(stored_req.easy, CURLOPT_HTTPHEADER, stored_req.headers);
    curl_easy_setopt(stored_req.easy, CURLOPT_POSTFIELDS, stored_req.request_body.c_str());
    curl_easy_setopt(stored_req.easy, CURLOPT_POSTFIELDSIZE_LARGE,
                     static_cast<curl_off_t>(stored_req.request_body.size()));
    curl_easy_setopt(stored_req.easy, CURLOPT_WRITEFUNCTION, &write_response_body);
    curl_easy_setopt(stored_req.easy, CURLOPT_WRITEDATA, &stored_req.response_body);
    curl_easy_setopt(stored_req.easy, CURLOPT_NOSIGNAL, 1L);

    auto mcode = curl_multi_add_handle(multi_, easy);
    if(mcode != CURLM_OK) {
        cleanup_request(easy);
        return std::unexpected(LLMError{
            .message = std::format("curl_multi_add_handle failed: {}",
                                   curl_multi_strerror(mcode))});
    }

    logging::info("submitted LLM request: tag={} url={} body_bytes={}",
                  tag, url_, stored_req.request_body.size());
    ++in_flight_;
    return {};
}

auto LLMClient::process_completed(Callback& on_complete) -> void {
    CURLMsg* msg = nullptr;
    int msgs_left = 0;
    while((msg = curl_multi_info_read(multi_, &msgs_left)) != nullptr) {
        if(msg->msg != CURLMSG_DONE) continue;

        auto* easy = msg->easy_handle;
        auto it = requests_.find(easy);
        if(it == requests_.end()) continue;

        auto& req = it->second;
        auto tag = req.tag;

        std::expected<std::string, LLMError> result;

        if(msg->data.result != CURLE_OK) {
            result = std::unexpected(LLMError{
                .message = std::format("curl error: {}",
                                       curl_easy_strerror(msg->data.result))});
        } else {
            long http_status = 0;
            curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_status);
            logging::info("completed LLM request: tag={} http_status={} response_bytes={}",
                          tag, http_status, req.response_body.size());

            if(req.response_body.empty()) {
                result = std::unexpected(LLMError{.message = "empty response from LLM"});
            } else if(http_status >= 400) {
                result = std::unexpected(LLMError{
                    .message = std::format("LLM HTTP {}: {}",
                                           http_status,
                                           req.response_body.substr(0, 200))});
            } else {
                result = detail::parse_response(req.response_body);
            }
        }

        // Remove from multi before cleanup
        curl_multi_remove_handle(multi_, easy);
        cleanup_request(easy);
        --in_flight_;

        on_complete(tag, std::move(result));
    }
}

auto LLMClient::cleanup_request(CURL* easy) -> void {
    auto it = requests_.find(easy);
    if(it == requests_.end()) return;

    if(it->second.headers) {
        curl_slist_free_all(it->second.headers);
    }
    curl_easy_cleanup(easy);
    requests_.erase(it);
}

}  // namespace clore::generate
