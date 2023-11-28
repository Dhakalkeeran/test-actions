#include "solver/symbolicexpressionparser.h"
#include "catch2/catch_test_macros.hpp"

TEST_CASE("SymbolicExpressionParser parses strings to S-Expressions") {
    auto parser = SymbolicExpressionParser();

    SECTION("should parse empty list") {
        auto list = parser.ParseSymbolList("()");
        REQUIRE(list->children.empty());
    }

    SECTION("should parse list of variables") {
        auto list = parser.ParseSymbolList("(x y z)");
        REQUIRE(dynamic_cast<Symbol*>(list->children[0])->content == "x");
        REQUIRE(dynamic_cast<Symbol*>(list->children[1])->content == "y");
        REQUIRE(dynamic_cast<Symbol*>(list->children[2])->content == "z");
    }

    SECTION("should parse nested list") {
        auto list = parser.ParseSymbolList("(())");
        REQUIRE(dynamic_cast<SymbolList*>(list->children[0])->children.empty());
    }

    SECTION("should parse mixed list with symbols and nested lists") {
        auto list = parser.ParseSymbolList("(x (y z) w)");
        REQUIRE(dynamic_cast<Symbol*>(list->children[0])->content == "x");
        REQUIRE(dynamic_cast<Symbol*>(list->children[2])->content == "w");

        auto nestedList = dynamic_cast<SymbolList*>(list->children[1]);
        REQUIRE(dynamic_cast<Symbol*>(nestedList->children[0])->content == "y");
        REQUIRE(dynamic_cast<Symbol*>(nestedList->children[1])->content == "z");
    }

    SECTION("should parse list of variables with escaped characters") {
        auto list = parser.ParseSymbolList("(|q(1,2)| true)");
        REQUIRE(dynamic_cast<Symbol*>(list->children[0])->content == "|q(1,2)|");
        REQUIRE(dynamic_cast<Symbol*>(list->children[1])->content == "true");
    }
}
