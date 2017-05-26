#include "catch.hpp"

#include "providers/providers.h"
#include "scheduler/broadcast-task.h"
#include "scheduler/execute-task.h"
#include "scheduler/task-decorators.h"
#include "settings/settings.h"
#include "v8/array-buffer-allocator.h"
#include "napa-initialization-guard.h"

#include "v8.h"

#include <vector>

using namespace napa;
using namespace napa::scheduler;
using namespace std::chrono_literals;

// Make sure V8 it initialized exactly once.
static NapaInitializationGuard _guard;

TEST_CASE("tasks", "[tasks]") {
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

    // Set a simple zone main function
    BroadcastTask("function __zone_function_main__(func, args) { return func.apply(this, args); }").Execute();

    SECTION("load valid javascript") {
        NapaResponseCode loadResponseCode;
        BroadcastTask("var i = 3 + 5;", "", [&loadResponseCode](NapaResponseCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESPONSE_SUCCESS);
    }

    SECTION("load fails when javascript is malformed") {
        NapaResponseCode loadResponseCode;
        BroadcastTask("var j = 3 +", "", [&loadResponseCode](NapaResponseCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESPONSE_BROADCAST_SCRIPT_ERROR);
    }

    SECTION("load fails when javascript exception is thrown") {
        NapaResponseCode loadResponseCode;
        BroadcastTask("throw Error('error');", "", [&loadResponseCode](NapaResponseCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESPONSE_BROADCAST_SCRIPT_ERROR);
    }

    SECTION("execute succeeds with a valid and existing function") {
        BroadcastTask("function foo(a, b) { return Number(a) + Number(b); }").Execute();

        ExecuteRequest request;
        request.function = NAPA_STRING_REF("foo");
        request.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        ExecuteResponse response;
        ExecuteTask(request, [&](ExecuteResponse res) {
            response = std::move(res);
        }).Execute();

        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "8");
    }

    SECTION("execute fails for non-existing function") {
        ExecuteRequest request;
        request.function = NAPA_STRING_REF("bar");
        request.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        ExecuteResponse response;
        ExecuteTask(request, [&](ExecuteResponse res) {
            response = std::move(res);
        }).Execute();

        REQUIRE(response.code == NAPA_RESPONSE_EXECUTE_FUNC_ERROR);
    }

    SECTION("execute fails when function throws exception") {
        BroadcastTask("function f1(a, b) { throw 'an error' }").Execute();

        ExecuteRequest request;
        request.function = NAPA_STRING_REF("f1");
        request.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        ExecuteResponse response;
        ExecuteTask(request, [&](ExecuteResponse res) {
            response = std::move(res);
        }).Execute();

        REQUIRE(response.code == NAPA_RESPONSE_EXECUTE_FUNC_ERROR);
        REQUIRE(response.errorMessage == "an error");
    }

    SECTION("execute succeeds when timeout was not exceeded") {
        BroadcastTask("function f2(a, b) { return Number(a) + Number(b); }").Execute();

        ExecuteRequest request;
        request.function = NAPA_STRING_REF("f2");
        request.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        ExecuteResponse response;
        TimeoutTaskDecorator<ExecuteTask>(100ms, request, [&](ExecuteResponse res) {
            response = std::move(res);
        }).Execute();

        REQUIRE(response.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response.returnValue == "8");
    }

    SECTION("execute fails when timeout exceeded") {
        BroadcastTask("function f3() { while(true) {} }").Execute();

        ExecuteRequest request;
        request.function = NAPA_STRING_REF("f3");

        ExecuteResponse response;
        TimeoutTaskDecorator<ExecuteTask>(30ms, request, [&](ExecuteResponse res) {
            response = std::move(res);
        }).Execute();

        REQUIRE(response.code == NAPA_RESPONSE_TIMEOUT);
        REQUIRE(response.errorMessage == "Execute exceeded timeout");
    }

    SECTION("execute succeeds after a failed task") {
        BroadcastTask("function f4() { while(true) {} }").Execute();
        BroadcastTask("function f5(a, b) { return Number(a) + Number(b); }").Execute();

        ExecuteRequest request1;
        request1.function = NAPA_STRING_REF("f4");

        ExecuteResponse response1;
        TimeoutTaskDecorator<ExecuteTask>(30ms, request1, [&](ExecuteResponse res) {
            response1 = std::move(res);
        }).Execute();

        REQUIRE(response1.code == NAPA_RESPONSE_TIMEOUT);
        REQUIRE(response1.errorMessage == "Execute exceeded timeout");

        ExecuteRequest request2;
        request2.function = NAPA_STRING_REF("f5");
        request2.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        ExecuteResponse response2;
        TimeoutTaskDecorator<ExecuteTask>(100ms, request2, [&](ExecuteResponse res) {
            response2 = std::move(res);
        }).Execute();

        REQUIRE(response2.code == NAPA_RESPONSE_SUCCESS);
        REQUIRE(response2.returnValue == "8");
    }
}
