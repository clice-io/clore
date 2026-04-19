#include <cstdlib>

#include "generate/prelude.h"
#include "kota/async/async.h"

import agent;
import config;
import extract;
import generate;

#include "generate/support.h"

namespace fs = std::filesystem;

TEST_SUITE(agent_loop) {

TEST_CASE(run_agent_rejects_missing_provider_environment) {
    ScopedTempDir temp("agent_invalid");
    fs::create_directories(temp.path / "src");

    ::unsetenv("ANTHROPIC_BASE_URL");
    ::unsetenv("ANTHROPIC_API_KEY");
    ::unsetenv("OPENAI_BASE_URL");
    ::unsetenv("OPENAI_API_KEY");

    auto config = make_config(temp.path);

    auto model = make_basic_model(temp.path);

    auto result = clore::agent::run_agent(config, model, "test-model", temp.path.generic_string());

    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message.find("no supported llm provider environment found"),
              std::string::npos);
}

TEST_CASE(run_agent_async_rejects_missing_provider_environment) {
    ScopedTempDir temp("agent_invalid_async");
    fs::create_directories(temp.path / "src");

    ::unsetenv("ANTHROPIC_BASE_URL");
    ::unsetenv("ANTHROPIC_API_KEY");
    ::unsetenv("OPENAI_BASE_URL");
    ::unsetenv("OPENAI_API_KEY");

    auto config = make_config(temp.path);

    auto model = make_basic_model(temp.path);

    kota::event_loop loop;
    auto task = clore::agent::run_agent_async(config,
                                              model,
                                              "test-model",
                                              temp.path.generic_string(),
                                              loop);
    loop.schedule(task);
    loop.run();

    auto result = task.result();
    EXPECT_TRUE(result.has_error());
    EXPECT_NE(result.error().message.find("no supported llm provider environment found"),
              std::string::npos);
}

};  // TEST_SUITE(agent_loop)
