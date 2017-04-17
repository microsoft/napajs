#include "process.h"

// This is not a module extension, so define this macro to use V8 common macros.
#define NAPA_MODULE_EXTENSION
#include <napa-module.h>
#include <napa/module/platform.h>

#include <boost/filesystem.hpp>

#include <chrono>
#include <iostream>
#include <sstream>


using namespace napa;
using namespace napa::module;

namespace {

    /// <summary> Callback to cwd(). </summary>
    void CwdCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to chdir(). </summary>
    void ChdirCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to exit process. </summary>
    void ExitCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to hrtime. </summary>
    void HrtimeCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to umask. </summary>
    void UmaskCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Environment variable getter. </summary>
    void EnvGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>&);

    /// <summary> Environment variable setter. </summary>
    void EnvSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value>, const v8::PropertyCallbackInfo<v8::Value>&);

}   // End of anonymous namespace.

void process::Init(v8::Local<v8::Object> exports) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    NAPA_SET_METHOD(exports, "cwd", CwdCallback);
    NAPA_SET_METHOD(exports, "chdir", ChdirCallback);
    NAPA_SET_METHOD(exports, "exit", ExitCallback);
    NAPA_SET_METHOD(exports, "hrtime", HrtimeCallback);
    NAPA_SET_METHOD(exports, "umask", UmaskCallback);

    auto arguments = v8::Array::New(isolate, platform::argc);
    for (int i = 0; i < platform::argc; ++i) {
        (void)arguments->CreateDataProperty(context, i, v8_helpers::MakeV8String(isolate, platform::argv[i]));
    }
    (void)exports->CreateDataProperty(context, v8_helpers::MakeV8String(isolate, "argv"), arguments);

    auto envObjectTemplate = v8::ObjectTemplate::New(isolate);
    envObjectTemplate->SetNamedPropertyHandler(EnvGetterCallback, EnvSetterCallback);
    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "env"),
                                      envObjectTemplate->NewInstance());

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "platform"),
                                      v8_helpers::MakeV8String(isolate, platform::PLATFORM));

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "version"),
                                      v8_helpers::MakeV8String(isolate, std::to_string(MODULE_VERSION)));

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "execPath"),
                                      v8_helpers::MakeV8String(isolate, platform::GetExecPath()));

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "pid"),
                                      v8::Integer::New(isolate, platform::Getpid()));
}

namespace {

    void CwdCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, 
            boost::filesystem::current_path().make_preferred().string().c_str()));
    }

    void ChdirCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1 && args[0]->IsString(),
            "process.chdir requires a string parameter.");

        v8::String::Utf8Value dirname(args[0]);
        boost::filesystem::path dir(*dirname);
        JS_ENSURE(isolate, boost::filesystem::exists(dir), dir.string() + "doesn't exists");

        boost::filesystem::current_path(dir);

        args.GetReturnValue().SetUndefined();
    }

    void ExitCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        CHECK_ARG(isolate,
            args.Length() == 1,
            "process.exit requires 1 int32 parameter as exit code.");

        CHECK_ARG(isolate,
            args[0]->IsInt32(),
            "Exit code must be integer.");

        std::exit(args[0]->Int32Value());
    }

    void HrtimeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        const static uint32_t NANOS_PER_SECOND = 1000000000;

        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);
        auto context = isolate->GetCurrentContext();

        // Returns the current time in nanoseconds
        uint64_t time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

        if (args.Length() == 1) {
            CHECK_ARG(isolate, args[0]->IsArray(), "process.hrtime only accepts an Array tuple");

            auto arr = v8::Local<v8::Array>::Cast(args[0]);
            CHECK_ARG(isolate, arr->Length() == 2, "process.hrtime only accepts an Array tuple of size 2");

            uint64_t prev = (static_cast<uint64_t>(arr->Get(0)->Uint32Value()) * NANOS_PER_SECOND)
                + arr->Get(1)->Uint32Value();

            // Calculate the delta
            time -= prev;
        }

        v8::Local<v8::Array> res = v8::Array::New(isolate, 2);
        (void)res->CreateDataProperty(
            context,
            0,
            v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(time / NANOS_PER_SECOND)));

        (void)res->CreateDataProperty(
            context,
            1,
            v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(time % NANOS_PER_SECOND)));

        args.GetReturnValue().Set(res);
    }

    void UmaskCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        if (args.Length() == 0 || args[0]->IsUndefined()) {
            auto old = platform::Umask(0);
            platform::Umask(old);
            args.GetReturnValue().Set(old);
            return;
        }
        
        CHECK_ARG(isolate,
            args[0]->IsInt32() || args[0]->IsString(),
            "Argument must be an integer or octal string");

        int32_t value = 0;
        if (args[0]->IsInt32()) {
            // Integer.
            value = args[0]->Int32Value();
        } else {
            // Octal string.
            v8::String::Utf8Value arg(args[0]);
            value = std::strtol(*arg, nullptr, 8);
        }

        auto old = platform::Umask(value);
        args.GetReturnValue().Set(old);
    }

    void EnvGetterCallback(v8::Local<v8::String> propertyKey, const v8::PropertyCallbackInfo<v8::Value>& info) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        v8::String::Utf8Value key(propertyKey);
        auto value = platform::GetEnv(*key);

        if (value.empty()) {
            info.GetReturnValue().SetUndefined();
        } else {
            info.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, value));
        }
    }

    void EnvSetterCallback(v8::Local<v8::String> propertyKey,
                           v8::Local<v8::Value> propertyValue,
                           const v8::PropertyCallbackInfo<v8::Value>& info) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        v8::String::Utf8Value key(propertyKey);
        v8::String::Utf8Value value(propertyValue);
        platform::SetEnv(*key, *value);

        info.GetReturnValue().Set(propertyValue);
    }

}   // End of anonymous namespace.