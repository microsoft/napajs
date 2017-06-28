#include "catch.hpp"

#include "zone/node-zone.h"

#include <future>

using namespace napa;
using namespace napa::zone;

TEST_CASE("node zone not available before init", "[node-zone]") {
    auto zone = NodeZone::Get();
    REQUIRE(zone == nullptr);
}

TEST_CASE("node zone delegate should work after init", "[node-zone]") {
    auto broadcast = [](const std::string& source, BroadcastCallback callback){
        callback(NAPA_RESULT_SUCCESS);
    };
    
    auto execute = [](const FunctionSpec& spec, ExecuteCallback callback) {
        callback({ NAPA_RESULT_SUCCESS, "", std::string("hello world"), nullptr });
    };

    NodeZone::Init(broadcast, execute);

    auto zone = NodeZone::Get();
    REQUIRE(zone != nullptr);
    
    {
        std::promise<ResultCode> promise;
        auto future = promise.get_future();

        zone->Broadcast("", [&promise](ResultCode resultCode) {
            promise.set_value(NAPA_RESULT_SUCCESS);
        });
        
        REQUIRE(future.get() == NAPA_RESULT_SUCCESS);
    }

    {
        std::promise<Result> promise;
        auto future = promise.get_future();

        FunctionSpec spec;
        zone->Execute(spec, [&promise](Result result) {
            promise.set_value(std::move(result));
        });

        auto result = future.get();
        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "hello world");
    }
}
