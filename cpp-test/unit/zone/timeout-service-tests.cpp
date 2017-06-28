#include "catch.hpp"

#include "zone/timeout-service.h"

#include <atomic>
#include <future>

using namespace napa::zone;
using namespace std::chrono;
using namespace std::chrono_literals;


TEST_CASE("timeout service calls callback on time", "[timeout-service][!mayfail]") {
    std::promise<void> promise;
    auto future = promise.get_future();
    
    auto startTime = high_resolution_clock::now();
    high_resolution_clock::time_point callbackTime;
    
    auto token = TimeoutService::Instance().Register(50ms, [&promise, &callbackTime]() {
        callbackTime = high_resolution_clock::now();
        promise.set_value();
    });

    auto status = future.wait_for(100ms);
    REQUIRE(status != std::future_status::timeout);

    milliseconds delta = duration_cast<milliseconds>(callbackTime - startTime) - 50ms;
    REQUIRE(delta.count() < 2);
}

TEST_CASE("timeout service doesn't call callback if cancel was called", "[timeout-service]") {
    std::promise<void> promise;
    auto future = promise.get_future();

    auto startTime = high_resolution_clock::now();
    high_resolution_clock::time_point callbackTime;

    auto token = TimeoutService::Instance().Register(50ms, [&promise, &callbackTime]() {
        callbackTime = high_resolution_clock::now();
        promise.set_value();
    });

    std::this_thread::sleep_for(20ms);
    token->Cancel();

    auto status = future.wait_for(80ms);
    REQUIRE(status == std::future_status::timeout);
}
