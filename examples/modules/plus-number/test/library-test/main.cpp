// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "plus-number.h"

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

using namespace napa::demo;

TEST_CASE("PlusNumber: add the given value", "[add]") {
    PlusNumber plusNumber(3);

    REQUIRE(plusNumber.Add(2) == 5);
    REQUIRE(plusNumber.Add(3) != 7);
}
