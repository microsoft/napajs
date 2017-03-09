#include "process.h"

// This is not a module extension, so define this macro to use V8 common macros.
#define NAPA_MODULE_EXTENSION
#include <napa-module.h>
#include <napa/command-line.h>

#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>

using namespace napa;
using namespace napa::module;

namespace {

    /// <summary> Callback to cwd(). </summary>
    void CwdCallback(const v8::FunctionCallbackInfo<v8::Value>&);

    /// <summary> Callback to exit process. </summary>
    void ExitCallback(const v8::FunctionCallbackInfo<v8::Value>&);

}   // End of anonymous namespace.

void process::Init(v8::Local<v8::Context> context) {
    auto isolate = v8::Isolate::GetCurrent();

    auto process = v8::ObjectTemplate::New(isolate);

    NAPA_SET_METHOD(process, "cwd", CwdCallback);
    NAPA_SET_METHOD(process, "exit", ExitCallback);

    auto instance = process->NewInstance();

    auto arguments = v8::Array::New(isolate, command_line::argc);
    for (int i = 0; i < command_line::argc; ++i) {
        (void)arguments->CreateDataProperty(context, i, v8_helpers::MakeV8String(isolate, command_line::argv[i]));
    }
    (void)instance->CreateDataProperty(context, v8_helpers::MakeV8String(isolate, "argv"), arguments);

    (void)context->Global()->CreateDataProperty(isolate->GetCurrentContext(),
                                                v8_helpers::MakeV8String(isolate, "process"),
                                                instance);
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

}   // End of anonymous namespace.