#include "catch.hpp"

#include "module/loader/module-loader.h"
#include "providers/providers.h"
#include "zone/eval-task.h"
#include "zone/call-task.h"
#include "zone/task-decorators.h"
#include "zone/worker-context.h"
#include "settings/settings.h"
#include "v8/array-buffer-allocator.h"
#include "napa-initialization-guard.h"

#include "v8.h"

#include <vector>

using namespace napa;
using namespace napa::zone;
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
    context->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(context);

    INIT_WORKER_CONTEXT();
    CREATE_MODULE_LOADER();

    EvalTask("require('../lib/index');").Execute();

    SECTION("load valid javascript") {
        ResultCode loadResponseCode;
        EvalTask("var i = 3 + 5;", "", [&loadResponseCode](ResultCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESULT_SUCCESS);
    }

    SECTION("load fails when javascript is malformed") {
        ResultCode loadResponseCode;
        EvalTask("var j = 3 +", "", [&loadResponseCode](ResultCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESULT_BROADCAST_SCRIPT_ERROR);
    }

    SECTION("load fails when javascript exception is thrown") {
        ResultCode loadResponseCode;
        EvalTask("throw Error('error');", "", [&loadResponseCode](ResultCode code) {
            loadResponseCode = code;
        }).Execute();

        REQUIRE(loadResponseCode == NAPA_RESULT_BROADCAST_SCRIPT_ERROR);
    }

    SECTION("execute succeeds with a valid and existing function") {
        EvalTask("function foo(a, b) { return a + b; }").Execute();

        FunctionSpec spec;
        spec.function = NAPA_STRING_REF("foo");
        spec.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        Result result;
        CallTask(std::make_shared<CallContext>(spec, [&](Result res) {
            result = std::move(res);
        })).Execute();

        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "8");
    }

    SECTION("execute fails for non-existing function") {
        FunctionSpec spec;
        spec.function = NAPA_STRING_REF("bar");
        spec.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        Result result;
        CallTask(std::make_shared<CallContext>(spec, [&](Result res) {
            result = std::move(res);
        })).Execute();

        REQUIRE(result.code == NAPA_RESULT_EXECUTE_FUNC_ERROR);
    }

    SECTION("execute fails when function throws exception") {
        EvalTask("function f1(a, b) { throw 'an error' }").Execute();

        FunctionSpec spec;
        spec.function = NAPA_STRING_REF("f1");
        spec.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        Result result;
        CallTask(std::make_shared<CallContext>(spec, [&](Result res) {
            result = std::move(res);
        })).Execute();

        REQUIRE(result.code == NAPA_RESULT_EXECUTE_FUNC_ERROR);
        REQUIRE(result.errorMessage == "an error");
    }

    SECTION("execute succeeds when timeout was not exceeded") {
        EvalTask("function f2(a, b) { return a + b; }").Execute();

        FunctionSpec spec;
        spec.function = NAPA_STRING_REF("f2");
        spec.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        Result result;
        TimeoutTaskDecorator<CallTask>(500ms, std::make_shared<CallContext>(spec, [&](Result res) {
            result = std::move(res);
        })).Execute();

        REQUIRE(result.code == NAPA_RESULT_SUCCESS);
        REQUIRE(result.returnValue == "8");
    }

    SECTION("execute fails when timeout exceeded") {
        EvalTask("function f3() { while(true) {} }").Execute();

        FunctionSpec spec;
        spec.function = NAPA_STRING_REF("f3");

        Result result;
        TimeoutTaskDecorator<CallTask>(500ms, std::make_shared<CallContext>(spec, [&](Result res) {
            result = std::move(res);
        })).Execute();

        REQUIRE(result.code == NAPA_RESULT_TIMEOUT);
        REQUIRE(result.errorMessage == "Terminated due to timeout");
    }

    SECTION("execute succeeds after a failed task") {
        EvalTask("function f4() { while(true) {} }").Execute();
        EvalTask("function f5(a, b) { return Number(a) + Number(b); }").Execute();

        FunctionSpec request1;
        request1.function = NAPA_STRING_REF("f4");

        Result response1;
        TimeoutTaskDecorator<CallTask>(500ms, std::make_shared<CallContext>(request1, [&](Result res) {
            response1 = std::move(res);
        })).Execute();

        REQUIRE(response1.code == NAPA_RESULT_TIMEOUT);
        REQUIRE(response1.errorMessage == "Terminated due to timeout");

        FunctionSpec request2;
        request2.function = NAPA_STRING_REF("f5");
        request2.arguments = { NAPA_STRING_REF("3"), NAPA_STRING_REF("5") };

        Result response2;
        TimeoutTaskDecorator<CallTask>(500ms, std::make_shared<CallContext>(request2, [&](Result res) {
            response2 = std::move(res);
        })).Execute();

        REQUIRE(response2.code == NAPA_RESULT_SUCCESS);
        REQUIRE(response2.returnValue == "8");
    }
}
