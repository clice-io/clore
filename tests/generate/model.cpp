#include "generate/prelude.h"

import config;
import extract;
import generate;

#include "generate/support.h"

TEST_SUITE(generate_model) {

TEST_CASE(symbol_target_key_round_trips_length_prefixed_encoding) {
    auto symbol =
        make_function_symbol(77, "run", "demo::runner", "void run(int\tmode)", "demo.cpp", "demo");

    auto key = make_symbol_target_key(symbol);
    auto parsed = parse_symbol_target_key(key);

    EXPECT_NE(key.find(":"), std::string::npos);
    EXPECT_EQ(parsed.qualified_name, "demo::runner");
    EXPECT_EQ(parsed.signature, "void run(int\tmode)");
}

};  // TEST_SUITE(generate_model)
