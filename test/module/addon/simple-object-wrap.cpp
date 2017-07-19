// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "simple-object-wrap.h"

#include <napa/zone.h>
#include <napa/assert.h>
#include <napa/async.h>
#include <napa/v8-helpers.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace napa::module;
using namespace napa::test;
using namespace napa::v8_helpers;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(SimpleObjectWrap);

void SimpleObjectWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();

    // Prepare constructor template.
    auto functionTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<SimpleObjectWrap>);
    functionTemplate->SetClassName(MakeV8String(isolate, exportName));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototypes.
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "getValue", GetValue);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "setValue", SetValue);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "doIncrementWork", DoIncrementWork);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "postIncrementWork", PostIncrementWork);

    // Set persistent constructor into V8.
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, functionTemplate->GetFunction());
}

void SimpleObjectWrap::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(exportName, ZoneWrap);
    args.GetReturnValue().Set(constructor->NewInstance(context).ToLocalChecked());
}

void SimpleObjectWrap::GetValue(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    auto wrap = ObjectWrap::Unwrap<SimpleObjectWrap>(args.Holder());

    args.GetReturnValue().Set(wrap->value);
}

void SimpleObjectWrap::SetValue(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    CHECK_ARG(isolate, args[0]->IsUint32(), "first argument to setValue must be a uint32");

    auto wrap = ObjectWrap::Unwrap<SimpleObjectWrap>(args.Holder());
    wrap->value = args[0]->Uint32Value();
}

void SimpleObjectWrap::DoIncrementWork(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    CHECK_ARG(isolate, args.Length() == 1 && args[0]->IsFunction(), "It requires a callback as arguments");

    auto wrap = ObjectWrap::Unwrap<SimpleObjectWrap>(args.Holder());

    napa::zone::DoAsyncWork(
        v8::Local<v8::Function>::Cast(args[0]),
        [wrap](auto complete) {
            // This runs at the same thread.
            auto newValue = ++wrap->value;

            complete(reinterpret_cast<void*>(static_cast<uintptr_t>(newValue)));
        },
        [](auto jsCallback, void* result) {
            // This runs at the same thread as one DoIncrementWork() is called.
            auto isolate = v8::Isolate::GetCurrent();

            int32_t argc = 1;
            v8::Local<v8::Value> argv[] = {
                v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(result)))
            };

            jsCallback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        }
    );
}

void SimpleObjectWrap::PostIncrementWork(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();

    CHECK_ARG(isolate, args.Length() == 1 && args[0]->IsFunction(), "It requires a callback as arguments");

    auto wrap = ObjectWrap::Unwrap<SimpleObjectWrap>(args.Holder());

    napa::zone::PostAsyncWork(
        v8::Local<v8::Function>::Cast(args[0]),
        [wrap]() {
            #ifdef _WIN32
                Sleep(10);
            #else
                // Sleep 10 ms.
                usleep(10 * 1000);
            #endif

            // This runs at the separate thread.
            auto newValue = ++wrap->value;

            return reinterpret_cast<void*>(static_cast<uintptr_t>(newValue));
        },
        [](auto jsCallback, void* result) {
            // This runs at the same thread as one PostIncrementWork() is called.
            auto isolate = v8::Isolate::GetCurrent();

            int32_t argc = 1;
            v8::Local<v8::Value> argv[] = { 
                v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(result)))
            };

            jsCallback->Call(isolate->GetCurrentContext()->Global(), argc, argv);
        }
    );
}
