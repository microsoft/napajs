#include "catch.hpp"

#include "providers/providers.h"
#include "scheduler/load-task.h"
#include "scheduler/run-task.h"
#include "settings/settings.h"
#include "v8/array-buffer-allocator.h"
#include "v8-initialization-guard.h"


#include "v8.h"


using namespace napa;
using namespace napa::scheduler;

// Make sure V8 it initialized exactly once.
static V8InitializationGuard _guard;

TEST_CASE("tasks", "[tasks]") {
    // Initialize napa providers
    Settings settings;
    settings.loggingProvider = "";
    napa::providers::Initialize(settings);

    // Create a new Isolate and make it the current one.
    napa::v8_extensions::ArrayBufferAllocator allocator;
    v8::Isolate::CreateParams createParams;
    createParams.array_buffer_allocator = &allocator;
    auto isolate = v8::Isolate::New(createParams);

    // Required scope/isolate guards
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope contextScope(context);

    SECTION("load valid javascript") {
        NapaResponseCode loadResponseCode;
        LoadTask("var i = 3 + 5;", [&loadResponseCode](NapaResponseCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESPONSE_SUCCESS);
    }

    SECTION("load fails when javascript is malformed") {
        NapaResponseCode loadResponseCode;
        LoadTask("var j = 3 +", [&loadResponseCode](NapaResponseCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
    }

    SECTION("load fails when javascript exception is thrown") {
        NapaResponseCode loadResponseCode;
        LoadTask("throw Error('error');", [&loadResponseCode](NapaResponseCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
    }

    SECTION("run succeeds with a valid and existing function") {
        LoadTask("function foo(a, b) { return Number(a) + Number(b); }").Execute();
        
        NapaResponseCode responseCode;
        std::string returnVal;
        RunTask("foo", { "3", "5" }, [&](NapaResponseCode code, NapaStringRef errorMessage, NapaStringRef returnValue) {
            responseCode = code;
            returnVal = NAPA_STRING_REF_TO_STD_STRING(returnValue);
        }).Execute();

        REQUIRE(responseCode == NAPA_RESPONSE_SUCCESS);
        REQUIRE(returnVal == "8");
    }

    SECTION("run fails for non-existing function") {
        NapaResponseCode responseCode;
        std::string error;
        RunTask("bar", { "3", "5" }, [&](NapaResponseCode code, NapaStringRef errorMessage, NapaStringRef returnValue) {
            responseCode = code;
            error = NAPA_STRING_REF_TO_STD_STRING(errorMessage);
        }).Execute();

        REQUIRE(responseCode == NAPA_RESPONSE_RUN_FUNC_ERROR);
        REQUIRE(error == "Function not defined: bar");
    }

    SECTION("run fails when function throws exception") {
        LoadTask("function f1(a, b) { throw 'an error' }").Execute();

        NapaResponseCode responseCode;
        std::string error;
        RunTask("f1", { "3", "5" }, [&](NapaResponseCode code, NapaStringRef errorMessage, NapaStringRef returnValue) {
            responseCode = code;
            error = NAPA_STRING_REF_TO_STD_STRING(errorMessage);
        }).Execute();

        REQUIRE(responseCode == NAPA_RESPONSE_RUN_FUNC_ERROR);
        REQUIRE(error == "an error");
    }
}
