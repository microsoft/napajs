#include "catch.hpp"

#include "zone/worker.h"
#include "napa-initialization-guard.h"

#include "v8.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace napa;
using namespace napa::settings;
using namespace napa::zone;



class TestTask : public Task {
public:
    TestTask(std::function<std::string(void)> func = []() { return ""; })
        : numberOfExecutions(0), _func(std::move(func)) {}

    virtual void Execute() override {
        result = _func();
        numberOfExecutions++;
    }

    std::atomic<uint32_t> numberOfExecutions;
    std::string result;
private:
    std::function<std::string(void)> _func;
};

// Make sure V8 it initialized exactly once.
static NapaInitializationGuard _guard;

TEST_CASE("worker runs scheduled task", "[scheduler-worker]") {
    auto worker = std::make_unique<Worker>(0, ZoneSettings(), [](WorkerId) {});
    
    auto task = std::make_shared<TestTask>();
    worker->Schedule(task);

    worker = nullptr; // Wait for worker to complete all tasks.

    REQUIRE(task->numberOfExecutions == 1);
}

TEST_CASE("worker notifies idle condition", "[scheduler-worker]") {
    std::mutex mutex;
    std::condition_variable cv;

    auto worker = std::make_unique<Worker>(0, ZoneSettings(), [&cv](WorkerId) {
        cv.notify_one();
    });

    worker->Schedule(std::make_shared<TestTask>());

    std::unique_lock<std::mutex> lock(mutex);
    bool idleNotificationReceived = (cv.wait_for(lock, std::chrono::milliseconds(1000)) == std::cv_status::no_timeout);

    REQUIRE(idleNotificationReceived == true);
}

TEST_CASE("worker runs all tasks before shutting down", "[scheduler-worker]") {
    auto worker = std::make_unique<Worker>(0, ZoneSettings(), [](WorkerId) {});

    auto task = std::make_shared<TestTask>();

    for (size_t i = 0; i < 100; i++) {
        worker->Schedule(task);
    }

    worker = nullptr; // Wait for worker to complete all tasks.

    REQUIRE(task->numberOfExecutions == 100);
}

TEST_CASE("worker runs javascript task", "[scheduler-worker]") {
    auto worker = std::make_unique<Worker>(0, ZoneSettings(), [](WorkerId) {});

    auto task = std::make_shared<TestTask>([]() {
        auto isolate = v8::Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();

        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "'Hello' + ', World!'",
            v8::NewStringType::kNormal).ToLocalChecked();

        auto script = v8::Script::Compile(context, source).ToLocalChecked();
        
        auto result = script->Run(context).ToLocalChecked();

        v8::String::Utf8Value utf8(result);

        return std::string(*utf8);
    });

    worker->Schedule(task);

    worker = nullptr; // Wait for worker to complete all tasks.

    REQUIRE(task->result == "Hello, World!");
}

TEST_CASE("worker runs javascript with stack overflow", "[scheduler-worker]") {
    auto worker = std::make_unique<Worker>(0, ZoneSettings(), [](WorkerId) {});

    auto task = std::make_shared<TestTask>([]() {
        auto isolate = v8::Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();

        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "function test() { test(); }; test();",
            v8::NewStringType::kNormal).ToLocalChecked();

        auto script = v8::Script::Compile(context, source).ToLocalChecked();

        v8::TryCatch tryCatch(isolate);
        script->Run(context);

        if (tryCatch.HasCaught()) {
            auto exception = tryCatch.Exception();
            v8::String::Utf8Value errorMessage(exception);

            return std::string(*errorMessage);
        }

        return std::string();
    });

    worker->Schedule(task);

    worker = nullptr; // Wait for worker to complete all tasks.

    REQUIRE(task->result == "RangeError: Maximum call stack size exceeded");
}
