// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <catch/catch.hpp>

#include "zone/timer.h"

#include <atomic>
#include <future>

#include <iostream>

using namespace napa::zone;
using namespace std::chrono;
using namespace std::chrono_literals;

TEST_CASE("timer is triggered after provided timeout", "[timer][!mayfail]") {
    std::promise<high_resolution_clock::time_point> promise;
    auto future = promise.get_future();

    Timer timer([&promise]() { promise.set_value(high_resolution_clock::now()); }, 50ms);

    auto startTime = high_resolution_clock::now();
    timer.Start();

    auto status = future.wait_for(100ms);
    REQUIRE(status != std::future_status::timeout);

    REQUIRE(future.get() - startTime >= 50ms);
}

TEST_CASE("timer is not called if stopped", "[timer]") {
    std::promise<void> promise;
    auto future = promise.get_future();

    Timer timer([&promise]() { promise.set_value(); }, 50ms);

    timer.Start();
    std::this_thread::sleep_for(20ms);
    timer.Stop();

    auto status = future.wait_for(80ms);
    REQUIRE(status == std::future_status::timeout);
}

TEST_CASE("timers are called by order", "[timer]") {
    std::atomic<int> callOrder(0);

    std::promise<int> promise1;
    auto future1 = promise1.get_future();
    Timer timer1([&promise1, &callOrder]() { promise1.set_value(++callOrder); }, 100ms);

    std::promise<int> promise2;
    auto future2 = promise2.get_future();
    Timer timer2([&promise2, &callOrder]() { promise2.set_value(++callOrder); }, 50ms);

    std::promise<int> promise3;
    auto future3 = promise3.get_future();
    Timer timer3([&promise3, &callOrder]() { promise3.set_value(++callOrder); }, 20ms);

    timer1.Start();
    timer2.Start();
    timer3.Start();

    // The order of which the timer callbacks are expected.
    REQUIRE(future1.get() == 3);
    REQUIRE(future2.get() == 2);
    REQUIRE(future3.get() == 1);
}