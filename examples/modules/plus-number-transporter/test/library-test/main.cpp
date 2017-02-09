#include <plus-number-transporter.h>

#define CATCH_CONFIG_MAIN
#include <catch/catch.hpp>

using namespace napa::demo;

TEST_CASE("PlusNumberTransporter: add the given value", "[add]") {
    PlusNumberTransporter plusNumberTransporter(3);

    REQUIRE(plusNumberTransporter.Add(2) == 5);
    REQUIRE(plusNumberTransporter.Add(3) != 7);
}
