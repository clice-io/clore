module;

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <format>
#include <functional>
#include <limits>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
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
///   LLMClient client(model, system_prompt, max_concurrent,
///                    retry_count, retry_initial_backoff_ms);
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
              std::uint32_t max_concurrent,
              std::uint32_t retry_count,
              std::uint32_t retry_initial_backoff_ms);

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
    struct PendingRequest {
        std::uint64_t tag = 0;
        std::string prompt;
        std::uint32_t attempt = 0;
    };

    struct InFlightRequest {
        CURL* easy = nullptr;
        curl_slist* headers = nullptr;
        std::string prompt;
        std::string request_body;
        std::string response_body;
        std::uint64_t tag = 0;
        std::uint32_t attempt = 0;
    };

    auto enqueue_pending(PendingRequest request, std::chrono::milliseconds delay) -> std::expected<void, LLMError>;
    auto try_launch_pending() -> std::expected<void, LLMError>;
    auto launch_one(PendingRequest request) -> std::expected<void, LLMError>;
    auto compute_retry_delay(std::uint32_t next_attempt) const -> std::expected<std::chrono::milliseconds, LLMError>;
    auto next_pending_delay() const -> std::optional<std::chrono::milliseconds>;
    auto process_completed(Callback& on_complete) -> void;
    auto cleanup_request(CURL* easy) -> void;
    auto reset_active_state() noexcept -> void;

    CURLM* multi_ = nullptr;
    std::string url_;
    std::string api_key_;
    std::string model_;
    std::string system_prompt_;
    std::uint32_t max_concurrent_;
    std::uint32_t retry_count_;
    std::uint32_t retry_initial_backoff_ms_;
    std::uint32_t in_flight_ = 0;
    std::multimap<std::chrono::steady_clock::time_point, PendingRequest> pending_;
    std::unordered_map<CURL*, InFlightRequest> requests_;
    std::optional<LLMError> configuration_error_;
};

}  // namespace clore::generate

// ── implementation ──────────────────────────────────────────────────

namespace clore::generate {

namespace {

constexpr std::string_view kOpenAIBaseUrlEnv = "OPENAI_BASE_URL";
constexpr std::string_view kOpenAIApiKeyEnv = "OPENAI_API_KEY";
constexpr long kCurlConnectTimeoutMs = 5'000;
constexpr long kCurlRequestTimeoutMs = 120'000;

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
    // Intentionally no explicit non-empty argument validation here.
    // This transport layer forwards inputs as-is so single-call and multi-call
    // paths share the same API-side error semantics.
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
    curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT_MS, kCurlConnectTimeoutMs);
    curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, kCurlRequestTimeoutMs);
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
                     std::uint32_t max_concurrent,
                     std::uint32_t retry_count,
                     std::uint32_t retry_initial_backoff_ms)
    : model_(model),
      system_prompt_(system_prompt),
      max_concurrent_(max_concurrent),
      retry_count_(retry_count),
      retry_initial_backoff_ms_(retry_initial_backoff_ms) {
    if(max_concurrent_ == 0) {
        configuration_error_ = LLMError{
            .message = "max_concurrent must be greater than 0"};
    }
}

LLMClient::~LLMClient() {
    reset_active_state();
}

auto LLMClient::submit(std::uint64_t tag, std::string prompt)
    -> std::expected<void, LLMError> {
    if(configuration_error_.has_value()) {
        return std::unexpected(*configuration_error_);
    }

    return enqueue_pending(PendingRequest{
        .tag = tag,
        .prompt = std::move(prompt),
        .attempt = 0,
    }, std::chrono::milliseconds{0});
}

auto LLMClient::run(Callback on_complete) -> std::expected<void, LLMError> {
    if(configuration_error_.has_value()) {
        return std::unexpected(*configuration_error_);
    }

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

    auto fail_run = [this](LLMError error) -> std::expected<void, LLMError> {
        reset_active_state();
        return std::unexpected(std::move(error));
    };

    // Event loop: launch pending, poll, process completed, repeat
    while(in_flight_ > 0 || !pending_.empty()) {
        // Launch as many pending requests as concurrency allows
        if(auto r = try_launch_pending(); !r.has_value()) {
            return fail_run(std::move(r.error()));
        }

        if(in_flight_ == 0) {
            auto delay = next_pending_delay();
            if(!delay.has_value()) {
                break;
            }
            if(delay->count() > 0) {
                std::this_thread::sleep_for(*delay);
            }
            continue;
        }

        // Drive transfers
        int still_running = 0;
        auto mcode = curl_multi_perform(multi_, &still_running);
        if(mcode != CURLM_OK) {
            logging::err("curl_multi_perform failed: CURLMcode={} error={}",
                         static_cast<int>(mcode), curl_multi_strerror(mcode));
            return fail_run(LLMError{
                .message = std::format("curl_multi_perform failed (CURLMcode {}): {}",
                                       static_cast<int>(mcode),
                                       curl_multi_strerror(mcode))});
        }

        // Process completed transfers
        process_completed(on_complete);

        // Wait for activity or for the next delayed retry to become ready.
        if(in_flight_ > 0 || !pending_.empty()) {
            int timeout_ms = 1000;
            if(auto delay = next_pending_delay(); delay.has_value()) {
                auto delay_ms = delay->count();
                if(delay_ms <= 0) {
                    timeout_ms = 0;
                } else if(delay_ms < static_cast<std::int64_t>(timeout_ms)) {
                    timeout_ms = static_cast<int>(delay_ms);
                }
            }
            auto poll_code = curl_multi_poll(multi_, nullptr, 0, timeout_ms, nullptr);
            if(poll_code != CURLM_OK) {
                logging::err("curl_multi_poll failed: CURLMcode={} timeout_ms={} error={}",
                             static_cast<int>(poll_code), timeout_ms,
                             curl_multi_strerror(poll_code));
                return fail_run(LLMError{
                    .message = std::format("curl_multi_poll failed (CURLMcode {}): {}",
                                           static_cast<int>(poll_code),
                                           curl_multi_strerror(poll_code))});
            }
        }
    }

    reset_active_state();

    return {};
}

auto LLMClient::enqueue_pending(PendingRequest request, std::chrono::milliseconds delay)
    -> std::expected<void, LLMError> {
    auto now = std::chrono::steady_clock::now();
    if(delay.count() < 0) {
        return std::unexpected(LLMError{
            .message = std::format("negative retry delay is not allowed: {}ms", delay.count())});
    }
    if(delay > (std::chrono::steady_clock::time_point::max() - now)) {
        return std::unexpected(LLMError{
            .message = std::format("retry delay is too large: {}ms", delay.count())});
    }
    pending_.emplace(now + delay, std::move(request));
    return {};
}

auto LLMClient::try_launch_pending() -> std::expected<void, LLMError> {
    auto now = std::chrono::steady_clock::now();
    while(!pending_.empty() && in_flight_ < max_concurrent_) {
        auto it = pending_.begin();
        if(it->first > now) {
            break;
        }

        auto request = std::move(it->second);
        pending_.erase(it);

        if(auto r = launch_one(std::move(request)); !r.has_value()) {
            return std::unexpected(std::move(r.error()));
        }
    }
    return {};
}

auto LLMClient::launch_one(PendingRequest request)
    -> std::expected<void, LLMError> {
    InFlightRequest req;
    req.tag = request.tag;
    req.prompt = std::move(request.prompt);
    req.attempt = request.attempt;

    req.request_body = detail::build_request_json(model_, system_prompt_, req.prompt);

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
            .message = std::format("duplicate LLM request handle for tag {}", request.tag)});
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
    curl_easy_setopt(stored_req.easy, CURLOPT_CONNECTTIMEOUT_MS, kCurlConnectTimeoutMs);
    curl_easy_setopt(stored_req.easy, CURLOPT_TIMEOUT_MS, kCurlRequestTimeoutMs);
    curl_easy_setopt(stored_req.easy, CURLOPT_NOSIGNAL, 1L);

    auto mcode = curl_multi_add_handle(multi_, easy);
    if(mcode != CURLM_OK) {
        cleanup_request(easy);
        return std::unexpected(LLMError{
            .message = std::format("curl_multi_add_handle failed: {}",
                                   curl_multi_strerror(mcode))});
    }

    logging::info("submitted LLM request: tag={} attempt={} url={} body_bytes={}",
                  stored_req.tag, stored_req.attempt + 1, url_, stored_req.request_body.size());
    ++in_flight_;
    return {};
}

auto LLMClient::compute_retry_delay(std::uint32_t next_attempt) const
    -> std::expected<std::chrono::milliseconds, LLMError> {
    if(next_attempt == 0) {
        return std::unexpected(LLMError{
            .message = "next_attempt must be greater than 0"});
    }

    std::uint64_t multiplier = 1;
    for(std::uint32_t i = 1; i < next_attempt; ++i) {
        if(multiplier > (std::numeric_limits<std::uint64_t>::max() / 2)) {
            return std::unexpected(LLMError{
                .message = std::format("retry delay overflow at attempt {}", next_attempt)});
        }
        multiplier *= 2;
    }

    if(static_cast<std::uint64_t>(retry_initial_backoff_ms_) >
       (std::numeric_limits<std::uint64_t>::max() / multiplier)) {
        return std::unexpected(LLMError{
            .message = std::format("retry delay overflow at attempt {}", next_attempt)});
    }

    auto delay_ms = static_cast<std::uint64_t>(retry_initial_backoff_ms_) * multiplier;
    if(delay_ms > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
        return std::unexpected(LLMError{
            .message = std::format("retry delay exceeds supported range: {}ms", delay_ms)});
    }

    return std::chrono::milliseconds{static_cast<std::int64_t>(delay_ms)};
}

auto LLMClient::next_pending_delay() const -> std::optional<std::chrono::milliseconds> {
    if(pending_.empty()) {
        return std::nullopt;
    }
    auto now = std::chrono::steady_clock::now();
    auto ready_at = pending_.begin()->first;
    if(ready_at <= now) {
        return std::chrono::milliseconds{0};
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(ready_at - now);
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
        auto attempt = req.attempt;

        std::expected<std::string, LLMError> result;

        if(msg->data.result != CURLE_OK) {
            result = std::unexpected(LLMError{
                .message = std::format("curl error: {}",
                                       curl_easy_strerror(msg->data.result))});
        } else {
            long http_status = 0;
            curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_status);
            logging::info("completed LLM request: tag={} attempt={} http_status={} response_bytes={}",
                          tag, attempt + 1, http_status, req.response_body.size());

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

        std::optional<PendingRequest> retry_request;
        std::optional<std::chrono::milliseconds> retry_delay;
        if(!result.has_value() && attempt < retry_count_) {
            auto next_attempt = attempt + 1;
            auto delay = compute_retry_delay(next_attempt);
            if(delay.has_value()) {
                retry_request = PendingRequest{
                    .tag = tag,
                    .prompt = req.prompt,
                    .attempt = next_attempt,
                };
                retry_delay = *delay;
                logging::warn(
                    "LLM request failed: tag={} attempt={}/{} retry_in_ms={} reason={}",
                    tag, attempt + 1, retry_count_ + 1, retry_delay->count(),
                    result.error().message);
            } else {
                result = std::unexpected(std::move(delay.error()));
            }
        }

        // Remove from multi before cleanup
        curl_multi_remove_handle(multi_, easy);
        cleanup_request(easy);
        --in_flight_;

        if(retry_request.has_value() && retry_delay.has_value()) {
            auto enqueue_result = enqueue_pending(std::move(*retry_request), *retry_delay);
            if(enqueue_result.has_value()) {
                continue;
            }
            result = std::unexpected(std::move(enqueue_result.error()));
        }

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

auto LLMClient::reset_active_state() noexcept -> void {
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
    in_flight_ = 0;

    if(multi_) {
        curl_multi_cleanup(multi_);
        multi_ = nullptr;
    }
}

}  // namespace clore::generate
