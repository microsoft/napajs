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

        auto response = zone.BroadcastSync("var i = 3 + 5;");

        REQUIRE(response == NAPA_RESPONSE_SUCCESS);
    }

    SECTION("broadcast illegal javascript") {
        napa::Zone zone("zone1");

        auto response = zone.BroadcastSync("var i = 3 +");

        REQUIRE(response == NAPA_RESPONSE_BROADCAST_SCRIPT_ERROR);
    }

    SECTION("broadcast and execute javascript") {
        napa::Zone zone("zone1");

        auto responseCode = zone.BroadcastSync("function func(a, b) { return Number(a) + Number(b); }");
        REQUIRE(responseCode == NAPA_RESPONSE_SUCCESS);

        napa::ExecuteRequest request;
        request.function = NAPA_STRING_REF("func");
        request.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };

        auto response = zone.ExecuteSync(request);
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    SECTION("broadcast and execute javascript async") {
        napa::Zone zone("zone1");

        std::promise<napa::ExecuteResponse> promise;
        auto future = promise.get_future();

        zone.Broadcast("function func(a, b) { return Number(a) + Number(b); }", [&promise, &zone](napa::ResponseCode) {
            napa::ExecuteRequest request;
            request.function = NAPA_STRING_REF("func");
            request.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };
            
            zone.Execute(request, [&promise](napa::ExecuteResponse response) {
                promise.set_value(std::move(response));
            });
        });

        auto response = future.get();
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    SECTION("broadcast and execute javascript without timing out") {
        napa::Zone zone("zone1");

        std::promise<napa::ExecuteResponse> promise;
        auto future = promise.get_future();

        // Warmup to avoid loading napajs on first call
        zone.BroadcastSync("require('napajs');");

        zone.Broadcast("function func(a, b) { return Number(a) + Number(b); }", [&promise, &zone](napa::ResponseCode) {
            napa::ExecuteRequest request;
            request.function = NAPA_STRING_REF("func");
            request.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };
            request.options.timeout = 100;

            zone.Execute(request, [&promise](napa::ExecuteResponse response) {
                promise.set_value(std::move(response));
            });
        });

        auto response = future.get();
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    SECTION("broadcast and execute javascript with exceeded timeout") {
        napa::Zone zone("zone1");

        std::promise<napa::ExecuteResponse> promise;
        auto future = promise.get_future();

        // Warmup to avoid loading napajs on first call
        zone.BroadcastSync("require('napajs');");

        zone.Broadcast("function func() { while(true) {} }", [&promise, &zone](napa::ResponseCode) {
            napa::ExecuteRequest request;
            request.function = NAPA_STRING_REF("func");
            request.options.timeout = 200;

            zone.Execute(request, [&promise](napa::ExecuteResponse response) {
                promise.set_value(std::move(response));
            });
        });

        auto response = future.get();
        REQUIRE(response.code == NAPA_RESPONSE_TIMEOUT);
        REQUIRE(response.errorMessage == "Execute exceeded timeout");
    }

    SECTION("execute 2 javascript functions, one succeeds and one times out") {
        napa::Zone zone("zone1");

        // Warmup to avoid loading napajs on first call
        zone.BroadcastSync("require('napajs');");

        auto res = zone.BroadcastSync("function f1(a, b) { return Number(a) + Number(b); }");
        REQUIRE(res == NAPA_RESPONSE_SUCCESS);
        res = zone.BroadcastSync("function f2() { while(true) {} }");
        REQUIRE(res == NAPA_RESPONSE_SUCCESS);

        std::promise<napa::ExecuteResponse> promise1;
        auto future1 = promise1.get_future();

        std::promise<napa::ExecuteResponse> promise2;
        auto future2 = promise2.get_future();

        napa::ExecuteRequest request1;
        request1.function = NAPA_STRING_REF("f1");
        request1.arguments = { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") };
        request1.options.timeout = 100;

        napa::ExecuteRequest request2;
        request2.function = NAPA_STRING_REF("f2");
        request2.options.timeout = 100;

        zone.Execute(request1, [&promise1](napa::ExecuteResponse response) {
            promise1.set_value(std::move(response));
        });

        zone.Execute(request2, [&promise2](napa::ExecuteResponse response) {
            promise2.set_value(std::move(response));
        });

        auto response = future1.get();
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");

        response = future2.get();
        REQUIRE(response.code == NAPA_RESPONSE_TIMEOUT);
        REQUIRE(response.errorMessage == "Execute exceeded timeout");
    }

    SECTION("broadcast javascript requiring a module") {
        napa::Zone zone("zone1");

        auto responseCode = zone.BroadcastSync("var path = require('path'); function func() { return path.extname('test.txt'); }");
        REQUIRE(responseCode == NAPA_RESPONSE_SUCCESS);

        napa::ExecuteRequest request;
        request.function = NAPA_STRING_REF("func");

        auto response = zone.ExecuteSync(request);
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "\".txt\"");
    }

    SECTION("execute function in a module") {
        napa::Zone zone("zone1");

        napa::ExecuteRequest request;
        request.module = NAPA_STRING_REF("path");
        request.function = NAPA_STRING_REF("extname");
        request.arguments = { NAPA_STRING_REF("\"test.txt\"") };

        auto response = zone.ExecuteSync(request);
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "\".txt\"");
    }
}
