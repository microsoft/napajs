#include "catch.hpp"

#include "module/global-store.h"


TEST_CASE("Global store returns nullptr for non existing key", "[global-store]") {
    REQUIRE(napa::module::global_store::GetValue("NonExisting") == nullptr);
}

TEST_CASE("Global store gets value for existing key", "[global-store]") {
    int i = 5;
    
    REQUIRE(napa::module::global_store::SetValue("key", &i));
    
    auto res = napa::module::global_store::GetValue("key");
    REQUIRE(*static_cast<int*>(res) == 5);
}

TEST_CASE("Global store fails to set an existing key", "[global-store]") {
    int i = 5;

    REQUIRE(napa::module::global_store::SetValue("key1", &i));
    REQUIRE(napa::module::global_store::SetValue("key1", &i) == false);
}
