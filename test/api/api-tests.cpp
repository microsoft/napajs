#include "catch.hpp"

#include "napa.h"

#include <future>

std::once_flag flag;

TEST_CASE("container apis", "[api]") {
    // Only call napa::Initialize once per process.
    std::call_once(flag, []() {
        REQUIRE(napa::Initialize() == NAPA_RESPONSE_SUCCESS);
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

        auto response = container.RunSync("func", { CREATE_NAPA_STRING_REF("2"), CREATE_NAPA_STRING_REF("3") });
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    SECTION("load and run javascript async") {
        napa::Container container;

        std::promise<napa::Response> prom;
        auto fut = prom.get_future();

        container.Load("function func(a, b) { return Number(a) + Number(b); }", [&prom, &container](NapaResponseCode) {
            container.Run("func", { CREATE_NAPA_STRING_REF("2"), CREATE_NAPA_STRING_REF("3") },
                [&prom](napa::Response response) {
                    prom.set_value(std::move(response));
                }
            );
        });

        auto response = fut.get();
        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "5");
    }

    /// <note>
    ///     This must be the last test in this test suite, since napa initialize and shutdown
    ///     can only run once per process.
    /// </note>
    SECTION("shutdown napa") {
        REQUIRE(napa::Shutdown() == NAPA_RESPONSE_SUCCESS);
    }
}
