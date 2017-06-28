#include "catch.hpp"

#include "napa-initialization-guard.h"

#include <napa.h>

#include <future>

// Make sure Napa is initialized exactly once.
static NapaInitializationGuard _guard;

TEST_CASE("zone apis", "[api]") {

    SECTION("create zone") {
        napa::Zone zone("zone1");

        REQUIRE(zone.GetId() == "zone1");
    }

    SECTION("broadcast valid javascript") {
        napa::Zone zone("zone1");

        auto result = zone.BroadcastSync("var i = 3 + 5;");

        REQUIRE(result == NAPA_RESULT_SUCCESS);
    }

    SECTION("broadcast illegal javascript") {
        napa::Zone zone("zone1");

        auto result = zone.BroadcastSync("var i = 3 +");

        REQUIRE(result == NAPA_RESULT_BROADCAST_SCRIPT_ERROR);
    }

    SECTION("broadcast and execute javascript") {
        napa::Zone zone("zone1");

        auto resultCode = zone.BroadcastSync("function func(a, b) { return Number(a) + Number(b); }");
        REQUIRE(resultCode == NAPA_RESULT_SUCCESS);

        napa::FunctionSpec spec;
        spec.function = NAPA_STRING_REF("func");
        spec.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };

        auto result = zone.ExecuteSync(spec);
        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "5");
    }

    SECTION("broadcast and execute javascript async") {
        napa::Zone zone("zone1");

        std::promise<napa::Result> promise;
        auto future = promise.get_future();

        zone.Broadcast("function func(a, b) { return Number(a) + Number(b); }", [&promise, &zone](napa::ResultCode) {
            napa::FunctionSpec spec;
            spec.function = NAPA_STRING_REF("func");
            spec.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };
            
            zone.Execute(spec, [&promise](napa::Result result) {
                promise.set_value(std::move(result));
            });
        });

        auto result = future.get();
        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "5");
    }

    SECTION("broadcast and execute javascript without timing out") {
        napa::Zone zone("zone1");

        std::promise<napa::Result> promise;
        auto future = promise.get_future();

        // Warmup to avoid loading napajs on first call
        zone.BroadcastSync("require('napajs');");

        zone.Broadcast("function func(a, b) { return Number(a) + Number(b); }", [&promise, &zone](napa::ResultCode) {
            napa::FunctionSpec spec;
            spec.function = NAPA_STRING_REF("func");
            spec.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };
            spec.options.timeout = 100;

            zone.Execute(spec, [&promise](napa::Result result) {
                promise.set_value(std::move(result));
            });
        });

        auto result = future.get();
        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "5");
    }

    SECTION("broadcast and execute javascript with exceeded timeout") {
        napa::Zone zone("zone1");

        std::promise<napa::Result> promise;
        auto future = promise.get_future();

        // Warmup to avoid loading napajs on first call
        zone.BroadcastSync("require('napajs');");

        zone.Broadcast("function func() { while(true) {} }", [&promise, &zone](napa::ResultCode) {
            napa::FunctionSpec spec;
            spec.function = NAPA_STRING_REF("func");
            spec.options.timeout = 200;

            zone.Execute(spec, [&promise](napa::Result result) {
                promise.set_value(std::move(result));
            });
        });

        auto result = future.get();
        REQUIRE(result.code == NAPA_RESULT_TIMEOUT);
        REQUIRE(result.errorMessage == "Terminated due to timeout");
    }

    SECTION("execute 2 javascript functions, one succeeds and one times out") {
        napa::Zone zone("zone1");

        // Warmup to avoid loading napajs on first call
        zone.BroadcastSync("require('napajs');");

        auto res = zone.BroadcastSync("function f1(a, b) { return Number(a) + Number(b); }");
        REQUIRE(res == NAPA_RESULT_SUCCESS);
        res = zone.BroadcastSync("function f2() { while(true) {} }");
        REQUIRE(res == NAPA_RESULT_SUCCESS);

        std::promise<napa::Result> promise1;
        auto future1 = promise1.get_future();

        std::promise<napa::Result> promise2;
        auto future2 = promise2.get_future();

        napa::FunctionSpec request1;
        request1.function = NAPA_STRING_REF("f1");
        request1.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };
        request1.options.timeout = 100;

        napa::FunctionSpec request2;
        request2.function = NAPA_STRING_REF("f2");
        request2.options.timeout = 100;

        zone.Execute(request1, [&promise1](napa::Result result) {
            promise1.set_value(std::move(result));
        });

        zone.Execute(request2, [&promise2](napa::Result result) {
            promise2.set_value(std::move(result));
        });

        auto result = future1.get();
        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "5");

        result = future2.get();
        REQUIRE(result.code == NAPA_RESULT_TIMEOUT);
        REQUIRE(result.errorMessage == "Terminated due to timeout");
    }

    SECTION("broadcast javascript requiring a module") {
        napa::Zone zone("zone1");

        auto resultCode = zone.BroadcastSync("var path = require('path'); function func() { return path.extname('test.txt'); }");
        REQUIRE(resultCode == NAPA_RESULT_SUCCESS);

        napa::FunctionSpec spec;
        spec.function = NAPA_STRING_REF("func");

        auto result = zone.ExecuteSync(spec);
        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "\".txt\"");
    }

    SECTION("execute function in a module") {
        napa::Zone zone("zone1");

        napa::FunctionSpec spec;
        spec.module = NAPA_STRING_REF("path");
        spec.function = NAPA_STRING_REF("extname");
        spec.arguments = { NAPA_STRING_REF("\"test.txt\"") };

        auto result = zone.ExecuteSync(spec);
        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "\".txt\"");
    }
}
