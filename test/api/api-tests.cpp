#include "catch.hpp"

#include "napa.h"

#include <future>

std::once_flag flag;

TEST_CASE("container apis", "[api]") {
    // Only call napa::Initialize once per process.
    std::call_once(flag, []() {
        REQUIRE(napa::Initialize("--loggingProvider console") == NAPA_RESPONSE_SUCCESS);
    });

    SECTION("load valid javascript") {
        napa::Container container;

        auto response = container.LoadSync("var i = 3 + 5;");

        REQUIRE(response == NAPA_RESPONSE_SUCCESS);
    }

    SECTION("load illegal javascript") {
        napa::Container container;

        auto response = container.LoadSync("var i = 3 +");

        REQUIRE(response == NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
    }

    SECTION("load valid javascript file") {
        napa::Container container;

        auto response = container.LoadFileSync("test.js");

        REQUIRE(response == NAPA_RESPONSE_SUCCESS);
    }

    SECTION("load valid non existing javascript file") {
        napa::Container container;

        auto response = container.LoadFileSync("non-existing-file.js");

        REQUIRE(response == NAPA_RESPONSE_LOAD_FILE_ERROR);
    }

    SECTION("load and run javascript") {
        napa::Container container;

        auto responseCode = container.LoadSync("function func(a, b) { return Number(a) + Number(b); }");
        REQUIRE(responseCode == NAPA_RESPONSE_SUCCESS);

        auto response = container.RunSync("func", { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") });
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    SECTION("load and run javascript async") {
        napa::Container container;

        std::promise<napa::Response> promise;
        auto future = promise.get_future();

        container.Load("function func(a, b) { return Number(a) + Number(b); }", [&promise, &container](NapaResponseCode) {
            container.Run("func", { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") },
                [&promise](napa::Response response) {
                    promise.set_value(std::move(response));
                }
            );
        });

        auto response = future.get();
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    SECTION("load and run javascript without timing out") {
        napa::Container container;

        std::promise<napa::Response> promise;
        auto future = promise.get_future();

        container.Load("function func(a, b) { return Number(a) + Number(b); }", [&promise, &container](NapaResponseCode) {
            container.Run("func", { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") },
                [&promise](napa::Response response) {
                    promise.set_value(std::move(response));
                },
                30);
        });

        auto response = future.get();
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    SECTION("load and run javascript with exceeded timeout") {
        napa::Container container;

        std::promise<napa::Response> promise;
        auto future = promise.get_future();

        container.Load("function func() { while(true) {} }", [&promise, &container](NapaResponseCode) {
            container.Run("func", {},
                [&promise](napa::Response response) {
                    promise.set_value(std::move(response));
                },
                30);
        });

        auto response = future.get();
        REQUIRE(response.code == NAPA_RESPONSE_TIMEOUT);
        REQUIRE(response.errorMessage == "Run exceeded timeout");
    }

    SECTION("run 2 javascript functions, one succeeds and one times out") {
        napa::Container container;

        auto res = container.LoadSync("function f1(a, b) { return Number(a) + Number(b); }");
        REQUIRE(res == NAPA_RESPONSE_SUCCESS);
        res = container.LoadSync("function f2() { while(true) {} }");
        REQUIRE(res == NAPA_RESPONSE_SUCCESS);

        std::promise<napa::Response> promise1;
        auto future1 = promise1.get_future();

        std::promise<napa::Response> promise2;
        auto future2 = promise2.get_future();

        container.Run("f1", { NAPA_STRING_REF("2"), NAPA_STRING_REF("3") }, [&promise1](napa::Response response) {
            promise1.set_value(std::move(response));
        }, 30);

        container.Run("f2", {}, [&promise2](napa::Response response) {
            promise2.set_value(std::move(response));
        }, 30);

        auto response = future1.get();
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");

        response = future2.get();
        REQUIRE(response.code == NAPA_RESPONSE_TIMEOUT);
        REQUIRE(response.errorMessage == "Run exceeded timeout");
    }

    /// <note>
    ///     This must be the last test in this test suite, since napa initialize and shutdown
    ///     can only run once per process.
    /// </note>
    SECTION("shutdown napa") {
        REQUIRE(napa::Shutdown() == NAPA_RESPONSE_SUCCESS);
    }
}
