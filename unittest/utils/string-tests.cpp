// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <catch/catch.hpp>
#include <utils/string.h>

using namespace napa;

TEST_CASE("utils", "[string]") {

    SECTION("ReplaceAll - No match") {
        std::string str = "no match at all";
        utils::string::ReplaceAll(str, "some", "other");
        REQUIRE(str == "no match at all");
    }

    SECTION("ReplaceAll - Matched") {
        std::string str = "this is a test";
        utils::string::ReplaceAll(str, "is", "at");
        REQUIRE(str == "that at a test");
    }

    SECTION("ReplaceAllCopy") {
        auto str = utils::string::ReplaceAllCopy("this is a test", "is", "at");
        REQUIRE(str == "that at a test");
    }

    SECTION("Split: no compress") {
        std::vector<std::string> items;
        utils::string::Split(" this is a  \ttest ", items, " \t", false);

        std::vector<std::string> expected = {"", "this", "is", "a", "", "", "test", ""};
        REQUIRE(items == expected);
    }

    SECTION("Split: compress") {
        std::vector<std::string> items;
        utils::string::Split(" this is a  \ttest ", items, " \t", true);

        std::vector<std::string> expected = {"this", "is", "a", "test"};
        REQUIRE(items == expected);
    }

    SECTION("Split: iterator with compress") {
        std::string str = " this is a  \ttest ";
        std::vector<std::string> items;
        utils::string::Split(str.begin() + 5, str.end(), items, " \t", true);

        std::vector<std::string> expected = {"is", "a", "test"};
        REQUIRE(items == expected);
    }

    SECTION("ToLower") {
        std::string str = "This Is A Test";
        utils::string::ToLower(str);
        REQUIRE(str == "this is a test");
    }

    SECTION("ToLowerCopy") {
        REQUIRE(utils::string::ToLowerCopy("THIs Is A TesT") == "this is a test");
    }

    SECTION("ToUpper") {
        std::string str = "This Is A Test";
        utils::string::ToUpper(str);
        REQUIRE(str == "THIS IS A TEST");
    }

    SECTION("ToUpperCopy") {
        REQUIRE(utils::string::ToUpperCopy("THIs Is A TesT") == "THIS IS A TEST");
    }

    SECTION("CaseInsensitiveCompare") {
        REQUIRE(utils::string::CaseInsensitiveCompare("abc", "abd") < 0);
        REQUIRE(utils::string::CaseInsensitiveCompare("abc", "ABc") == 0);
        REQUIRE(utils::string::CaseInsensitiveCompare("abc", "AB") > 0);
    }

    SECTION("CaseInsensitiveEquals") {
        REQUIRE(!utils::string::CaseInsensitiveEquals("abc", "abd"));
        REQUIRE(utils::string::CaseInsensitiveEquals("abc", "ABc"));
        REQUIRE(!utils::string::CaseInsensitiveEquals("abc", "AB"));
    }
}