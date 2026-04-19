#include "generate/prelude.h"

import generate;
import network;

namespace fs = std::filesystem;

TEST_SUITE(generate_cache) {

TEST_CASE(make_prompt_response_cache_key_deterministic) {
    clore::net::PromptRequest request{
        .prompt = "Summarize this function.",
        .output_contract = clore::net::PromptOutputContract::Markdown,
    };

    auto key1 =
        clore::generate::cache::make_prompt_response_cache_key("function_analysis:demo::foo",
                                                               "You are a writer.",
                                                               request);
    auto key2 =
        clore::generate::cache::make_prompt_response_cache_key("function_analysis:demo::foo",
                                                               "You are a writer.",
                                                               request);

    ASSERT_TRUE(key1.has_value());
    ASSERT_TRUE(key2.has_value());
    EXPECT_EQ(*key1, *key2);
}

TEST_CASE(make_prompt_response_cache_key_differentiates_prompt) {
    clore::net::PromptRequest request_a{
        .prompt = "A",
        .output_contract = clore::net::PromptOutputContract::Markdown,
    };
    clore::net::PromptRequest request_b{
        .prompt = "B",
        .output_contract = clore::net::PromptOutputContract::Markdown,
    };

    auto key_a = clore::generate::cache::make_prompt_response_cache_key("k", "sys", request_a);
    auto key_b = clore::generate::cache::make_prompt_response_cache_key("k", "sys", request_b);

    ASSERT_TRUE(key_a.has_value());
    ASSERT_TRUE(key_b.has_value());
    EXPECT_NE(*key_a, *key_b);
}

TEST_CASE(find_cached_response_hit_and_miss) {
    clore::generate::cache::CacheIndex index;
    index.entries["key1"] = "response1";

    auto hit = clore::generate::cache::find_cached_response(index, "key1");
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(*hit, "response1");

    auto miss = clore::generate::cache::find_cached_response(index, "key2");
    EXPECT_FALSE(miss.has_value());
}

TEST_CASE(save_and_load_cache_index_roundtrip) {
    auto temp_dir = fs::temp_directory_path() /
                    std::format("clore_gen_cache_{}",
                                std::chrono::steady_clock::now().time_since_epoch().count());
    fs::create_directories(temp_dir);

    auto save_result =
        clore::generate::cache::save_cache_entry(temp_dir.generic_string(), "mykey", "myresponse");
    ASSERT_TRUE(save_result.has_value());

    auto load_result = clore::generate::cache::load_cache_index(temp_dir.generic_string());
    ASSERT_TRUE(load_result.has_value());

    auto hit = clore::generate::cache::find_cached_response(*load_result, "mykey");
    ASSERT_TRUE(hit.has_value());
    EXPECT_EQ(*hit, "myresponse");

    std::error_code ec;
    fs::remove_all(temp_dir, ec);
}

TEST_CASE(load_cache_index_empty_for_missing_dir) {
    auto result = clore::generate::cache::load_cache_index("/nonexistent/path/for/sure/clore");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->entries.empty());
}

};  // TEST_SUITE(generate_cache)
