#include <catch.hpp>

#include <module/module-loader.h>
#include <napa/module/platform.h>
#include <napa/module/module-internal.h>
#include <napa/v8-helpers.h>
#include <scheduler/scheduler.h>
#include <zone/zone-impl.h>
#include <v8/array-buffer-allocator.h>
#include <v8/v8-common.h>

#include <boost/algorithm/string.hpp>
#include <boost/dll.hpp>
#include <boost/filesystem.hpp>

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <future>
#include <mutex>
#include <thread>

using namespace napa;
using namespace napa::module;
using namespace napa::scheduler;

class V8InitializationGuard {
public:
    V8InitializationGuard() {
        static V8Initialization initialization;
    }

private:
    class V8Initialization {
    public:
        V8Initialization() {
            v8_common::Initialize();
        }

        ~V8Initialization() {
            v8_common::Shutdown();
        }
    };
};

// Make sure V8 it initialized exactly once.
static V8InitializationGuard _guard;

bool RunScript(const std::string& input, std::function<bool(v8::Local<v8::Value>)> verifier) {
    v8_extensions::ArrayBufferAllocator allocator;
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = &allocator;

    auto isolate = v8::Isolate::New(params);

    std::unique_ptr<v8::Isolate, std::function<void(v8::Isolate*)>> deferred(isolate, [](auto isolate) {
        isolate->Dispose();
    });

    v8::Isolate::Scope isolateScope(isolate);
    v8::HandleScope handleScope(isolate);

    auto context = v8::Context::New(isolate);
    context->SetSecurityToken(v8::Undefined(isolate));
    v8::Context::Scope contextScope(context);

    INIT_WORKER_CONTEXT();
    CREATE_MODULE_LOADER();

    auto source = v8_helpers::MakeV8String(isolate, input);

    v8::TryCatch tryCatch;
    auto script = v8::Script::Compile(source);
    if (tryCatch.HasCaught()) {
        return false;
    }

    auto run = script->Run();
    if (tryCatch.HasCaught()) {
        return false;
    }

    return verifier(run);
}

bool RunScriptFile(const std::string& filename, std::function<bool(v8::Local<v8::Value>)> verifier) {
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();

    return RunScript(buffer.str(), verifier);
}

bool LoadJavascript(int index) {
    std::string input;
    input.append("jsmodule").append(std::to_string(index));

    std::ostringstream oss;
    oss << "var js = require('./jsmodule');"
        << "js.print('" << input << "');"
        << "js = require('./jsmodule.js');"
        << "js.print('" << input << "');"
        << "js = require('./test/../jsmodule.js');"
        << "js.print('" << input << "');";

    return RunScript(oss.str(), [&input](v8::Local<v8::Value> run) {
        std::transform(input.begin(), input.end(), input.begin(), toupper);

        v8::String::Utf8Value value(run);
        return input.compare(*value) == 0;
    });
}

bool LoadNapaModule(int index) {
    std::string input;
    input.append("sample").append(std::to_string(index));

    std::ostringstream oss;
    oss << "var sample = require('./sample.napa');"
        << "sample.print('" << input << "');"
        << "sample = require('./test/../sample.napa');"
        << "sample.print('" << input << "');";

    return RunScript(oss.str(), [&input](v8::Local<v8::Value> run) {
        v8::String::Utf8Value value(run);
        return input.compare(*value) == 0;
    });
}

bool LoadObjectWrapModule(int index) {
    std::ifstream file("sample-test.js");

    std::stringstream buffer;
    buffer << file.rdbuf();

    return RunScript(buffer.str(), [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
}

void RunAtThreads(std::function<bool(int)> tester) {
    std::vector<std::future<bool>> futures;

    for (int i = 0; i < 16; ++i) {
        futures.push_back(std::async(std::launch::async, tester, i));
    }

    for (auto& result : futures) {
        REQUIRE(result.get() == true);
    }
}

TEST_CASE("load javascript", "[module-loader]") {
    RunAtThreads(LoadJavascript);
}

TEST_CASE("load json module", "[module-loader]") {
    auto result = RunScriptFile("jsontest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("load napa module", "[module-loader]") {
    RunAtThreads(LoadNapaModule);
}

TEST_CASE("load object wrap module", "[module-loader]") {
    RunAtThreads(LoadObjectWrapModule);
}

TEST_CASE("require.resolve", "[module-loader]") {
    std::ostringstream oss;
    oss << "require.resolve('./jsmodule.js');";

    auto result = RunScript(oss.str(), [](v8::Local<v8::Value> run) {
        v8::String::Utf8Value value(run);
        auto filePath = boost::filesystem::current_path() / "jsmodule.js";
        return filePath.string().compare(*value) == 0;
    });
    REQUIRE(result);

    oss.str("");
    oss << "require.resolve('./jsmodule.napa');";

    result = RunScript(oss.str(), [](v8::Local<v8::Value> run) {
        v8::String::Utf8Value value(run);
        return value.length() == 0;
    });
    REQUIRE(result);
}

TEST_CASE("core modules", "[module-loader]") {
    auto result = RunScriptFile("core-module-tests.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("process", "[module-loader]") {
    std::ostringstream oss;
    oss << "process.argv.length;";

    auto result = RunScript(oss.str(), [](v8::Local<v8::Value> run) {
        auto argc = run->Int32Value();
        return argc > 0;
    });
    REQUIRE(result);

    oss.str("");
    oss << "process.argv[0];";

    result = RunScript(oss.str(), [](v8::Local<v8::Value> run) {
        v8::String::Utf8Value value(run);
        return boost::filesystem::exists(boost::filesystem::path(*value));
    });
    REQUIRE(result);

    oss.str("");
    oss << "process.execPath;";

    result = RunScript(oss.str(), [](v8::Local<v8::Value> run) {
        v8::String::Utf8Value value(run);
        return boost::dll::this_line_location() == boost::filesystem::path(*value);
    });
    REQUIRE(result);

    result = RunScriptFile("processtest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("assert", "[module-loader]") {
    auto result = RunScriptFile("asserttest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("file system", "[module-loader]") {
    auto result = RunScriptFile("fstest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("global", "[module-loader]") {
    auto result = RunScriptFile("globaltest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("path", "[module-loader]") {
    auto result = RunScriptFile("pathtest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("os", "[module-loader]") {
    auto result = RunScriptFile("ostest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("util", "[module-loader]") {
    auto result = RunScriptFile("utiltest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("tty", "[module-loader]") {
    auto result = RunScriptFile("ttytest.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("cycling import", "[module-loader]") {
    auto result = RunScriptFile("cycle-main.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("resolve modules", "[module-loader]") {
    auto result = RunScriptFile("module-resolution-test.js", [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

TEST_CASE("resolve full path modules", "[module-loader]") {
    auto filePath = boost::filesystem::current_path() / "tests\\sub\\sub1\\file2.js";
    auto filePathString = filePath.string();
    boost::replace_all(filePathString, "\\", "\\\\");

    std::ostringstream oss;
    oss << "var file = require('" << filePathString << "');\r\n" 
        << "(file != null);";

    auto result = RunScript(oss.str(), [](v8::Local<v8::Value> run) {
        return run->BooleanValue();
    });
    REQUIRE(result);
}

class AsyncTestTask : public Task {
public:
    AsyncTestTask(ZoneImpl* zone, std::string filename)
        : _zone(zone), _filename(std::move(filename)), _succeeded(false) {}

    void Execute() override {
        // TODO @suchoi: Remove this line after Asi checks in the separation of zone construct and init.
        WorkerContext::Set(WorkerContextItem::ZONE, reinterpret_cast<void*>(_zone));

        if (_filename.empty()) {
            return;
        }

        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        std::ifstream file(_filename);
        std::stringstream buffer;
        buffer << file.rdbuf();

        auto source = napa::v8_helpers::MakeV8String(isolate, buffer.str());

        std::unique_ptr<std::condition_variable,
                        std::function<void(std::condition_variable*)>> deferred(&_cond, [](auto cond) {
            cond->notify_one();
        });

        v8::TryCatch tryCatch;
        auto script = v8::Script::Compile(source);
        if (tryCatch.HasCaught()) {
            _succeeded = false;
            return;
        }

        auto run = script->Run();
        if (tryCatch.HasCaught()) {
            _succeeded = false;
            return;
        }

        _succeeded = true;
    }

    bool ok() {
        std::unique_lock<std::mutex> lock(_lock);
        _cond.wait(lock);

        return _succeeded;
    }

private:

    ZoneImpl* _zone;

    std::string _filename;
    bool _succeeded = false;

    std::mutex _lock;
    std::condition_variable _cond;
};

TEST_CASE("async", "[module-loader]") {
    ZoneSettings settings;
    settings.id = "zone";
    settings.workers = 1;

    auto zone = ZoneImpl::Create(settings);

    auto scheduler = zone->GetScheduler();
    scheduler->ScheduleOnAllWorkers(std::make_shared<AsyncTestTask>(zone.get(), std::string()));

    auto task = std::make_shared<AsyncTestTask>(zone.get(), "asynctest1.js");
    scheduler->Schedule(task);

    REQUIRE(task->ok());

    task = std::make_shared<AsyncTestTask>(zone.get(), "asynctest2.js");
    scheduler->Schedule(task);

    REQUIRE(task->ok());
}