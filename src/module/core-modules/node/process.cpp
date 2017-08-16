// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "process.h"

#include <napa/module.h>
#include <platform/filesystem.h>
#include <platform/os.h>
#include <platform/process.h>

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

} // End of anonymous namespace.

void process::Init(v8::Local<v8::Object> exports) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    NAPA_SET_METHOD(exports, "cwd", CwdCallback);
    NAPA_SET_METHOD(exports, "chdir", ChdirCallback);
    NAPA_SET_METHOD(exports, "exit", ExitCallback);
    NAPA_SET_METHOD(exports, "hrtime", HrtimeCallback);
    NAPA_SET_METHOD(exports, "umask", UmaskCallback);

    auto argc = platform::GetArgc();
    auto argv = platform::GetArgv();
    auto arguments = v8::Array::New(isolate, argc);
    for (int i = 0; i < argc; ++i) {
        (void)arguments->CreateDataProperty(context, i, v8_helpers::MakeV8String(isolate, argv[i]));
    }
    (void)exports->CreateDataProperty(context, v8_helpers::MakeV8String(isolate, "argv"), arguments);

    auto envObjectTemplate = v8::ObjectTemplate::New(isolate);
    envObjectTemplate->SetNamedPropertyHandler(EnvGetterCallback, EnvSetterCallback);
    (void)exports->CreateDataProperty(
        context, v8_helpers::MakeV8String(isolate, "env"), envObjectTemplate->NewInstance());

    (void)exports->CreateDataProperty(
        context, v8_helpers::MakeV8String(isolate, "platform"), v8_helpers::MakeV8String(isolate, platform::PLATFORM));

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "version"),
                                      v8_helpers::MakeV8String(isolate, std::to_string(MODULE_VERSION)));

    (void)exports->CreateDataProperty(context,
                                      v8_helpers::MakeV8String(isolate, "execPath"),
                                      v8_helpers::MakeV8String(isolate, filesystem::ProgramPath().String()));

    (void)exports->CreateDataProperty(
        context, v8_helpers::MakeV8String(isolate, "pid"), v8::Integer::New(isolate, platform::Getpid()));
}

namespace {

void CwdCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, filesystem::CurrentDirectory().String()));
}

void ChdirCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1 && args[0]->IsString(), "process.chdir requires a string parameter.");

    v8::String::Utf8Value dirname(args[0]);
    filesystem::Path dir(*dirname);
    JS_ENSURE(isolate, filesystem::IsDirectory(dir), "Directory \"%s\" doesn't exist", dir.c_str());

    filesystem::SetCurrentDirectory(dir);

    args.GetReturnValue().SetUndefined();
}

void ExitCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "process.exit requires 1 int32 parameter as exit code.");

    CHECK_ARG(isolate, args[0]->IsInt32(), "Exit code must be integer.");

    std::exit(args[0]->Int32Value());
}

void HrtimeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    // Returns the current time in nanoseconds
    uint64_t time = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    if (args.Length() == 1) {
        auto result = v8_helpers::V8Uint32ArrayToHrtime(isolate, args[0]);
        JS_ENSURE(isolate, result.second, "The 1st argument of hrtime must be a two-element uint32 array.");

        // Calculate the delta
        time -= result.first;
    }
    args.GetReturnValue().Set(v8_helpers::HrtimeToV8Uint32Array(isolate, time));
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

    CHECK_ARG(isolate, args[0]->IsInt32() || args[0]->IsString(), "Argument must be an integer or octal string");

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

} // End of anonymous namespace.