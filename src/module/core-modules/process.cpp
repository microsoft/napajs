#include "process.h"

// This is not a module extension, so define this macro to use V8 common macros.
#define NAPA_MODULE_EXTENSION
#include <napa-module.h>
#include <napa/module/command-line.h>

#include <boost/filesystem.hpp>

#include <chrono>
#include <iostream>
#include <sstream>


using namespace napa;
using namespace napa::module;

namespace {

    /// <summary> Callback to cwd(). </summary>
    void CwdCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to exit process. </summary>
    void ExitCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to hrtime. </summary>
    void HrtimeCallback(const v8::FunctionCallbackInfo<v8::Value>&);

}   // End of anonymous namespace.

void process::Init(v8::Local<v8::Object> exports) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    NAPA_SET_METHOD(exports, "cwd", CwdCallback);
    NAPA_SET_METHOD(exports, "exit", ExitCallback);
    NAPA_SET_METHOD(exports, "hrtime", HrtimeCallback);

    auto arguments = v8::Array::New(isolate, command_line::argc);
    for (int i = 0; i < command_line::argc; ++i) {
        (void)arguments->CreateDataProperty(context, i, v8_helpers::MakeV8String(isolate, command_line::argv[i]));
    }
    (void)exports->CreateDataProperty(context, v8_helpers::MakeV8String(isolate, "argv"), arguments);
}

namespace {

    void CwdCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, 
            boost::filesystem::current_path().make_preferred().string().c_str()));
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

}   // End of anonymous namespace.