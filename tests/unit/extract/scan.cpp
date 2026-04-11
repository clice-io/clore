#include "eventide/zest/zest.h"

import clore.extract;

using namespace clore::extract;

TEST_SUITE(scan) {
    TEST_CASE(topological_order_simple) {
        DependencyGraph graph;
        graph.files = {"a.h", "b.h", "c.cpp"};
        graph.edges = {
            {.from = "c.cpp", .to = "b.h"},
            {.from = "b.h", .to = "a.h"},
        };

        auto order_result = topological_order(graph);
        ASSERT_TRUE(order_result.has_value());
        auto& order = *order_result;
        ASSERT_EQ(order.size(), 3u);

        // a.h should come before b.h, b.h should come before c.cpp
        std::size_t pos_a = 0, pos_b = 0, pos_c = 0;
        for(std::size_t i = 0; i < order.size(); ++i) {
            if(order[i] == "a.h") pos_a = i;
            if(order[i] == "b.h") pos_b = i;
            if(order[i] == "c.cpp") pos_c = i;
        }
        EXPECT_LT(pos_a, pos_b);
        EXPECT_LT(pos_b, pos_c);
    }

    TEST_CASE(topological_order_no_edges) {
        DependencyGraph graph;
        graph.files = {"a.cpp", "b.cpp", "c.cpp"};

        auto order_result = topological_order(graph);
        ASSERT_TRUE(order_result.has_value());
        EXPECT_EQ(order_result->size(), 3u);
    }

    TEST_CASE(topological_order_with_cycle) {
        DependencyGraph graph;
        graph.files = {"a.h", "b.h"};
        graph.edges = {
            {.from = "a.h", .to = "b.h"},
            {.from = "b.h", .to = "a.h"},
        };

        // A cycle must be reported as an error — not silently swallowed.
        auto order_result = topological_order(graph);
        EXPECT_FALSE(order_result.has_value());
        EXPECT_NE(order_result.error().message.find("cycle"), std::string::npos);
    }

    TEST_CASE(topological_order_diamond) {
        DependencyGraph graph;
        graph.files = {"base.h", "left.h", "right.h", "top.cpp"};
        graph.edges = {
            {.from = "left.h", .to = "base.h"},
            {.from = "right.h", .to = "base.h"},
            {.from = "top.cpp", .to = "left.h"},
            {.from = "top.cpp", .to = "right.h"},
        };

        auto order_result = topological_order(graph);
        ASSERT_TRUE(order_result.has_value());
        auto& order = *order_result;
        ASSERT_EQ(order.size(), 4u);

        // base.h should come before left.h and right.h
        // left.h and right.h should come before top.cpp
        std::size_t pos_base = 0, pos_left = 0, pos_right = 0, pos_top = 0;
        for(std::size_t i = 0; i < order.size(); ++i) {
            if(order[i] == "base.h") pos_base = i;
            if(order[i] == "left.h") pos_left = i;
            if(order[i] == "right.h") pos_right = i;
            if(order[i] == "top.cpp") pos_top = i;
        }
        EXPECT_LT(pos_base, pos_left);
        EXPECT_LT(pos_base, pos_right);
        EXPECT_LT(pos_left, pos_top);
        EXPECT_LT(pos_right, pos_top);
    }
};
