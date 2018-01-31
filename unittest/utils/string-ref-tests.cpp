// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <catch/catch.hpp>

#include <napa/string-ref.h>

using namespace napa;

namespace {
    // The test string contains 6 characters: U+0024 U+00A2 U+20AC U+10348 U+10437 U+24B62
    const char u8szStr[] = u8"\u0024\u00A2\u20AC\U00010348\U00010437\U00024B62";
    const std::string u8str(u8szStr);
    const char16_t u16szStr[] = u"\u0024\u00A2\u20AC\U00010348\U00010437\U00024B62";
    const std::u16string u16str(u16szStr);
}

TEST_CASE("utils - napa-string-ref", "[string-ref]") {

    SECTION("Encoding: UTF-8 to UTF-16") {
        auto strRef1 = StringRef::FromUtf8(u8str);
        auto u16Str1 = strRef1.ToU16String();
        REQUIRE(u16Str1 == u16str);
    }

    SECTION("Encoding: UTF-16 to UTF-8") {
        auto strRef1 = StringRef::FromUtf16(u16str);
        auto u8Str1 = strRef1.ToString();
        REQUIRE(u8Str1 == u8str);
    }
}