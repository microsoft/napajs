// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "lock-wrap.h"

#define DEBUG_NAPA
#include <utils/debug.h>

#include <napa/module/binding/wraps.h>

#include <memory>
#include <mutex>

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(napa::module::LockWrap)

void LockWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<LockWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    InitConstructorTemplate<LockWrap>(constructorTemplate);

    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "guardSync", GuardSyncCallback);

    auto constructor = constructorTemplate->GetFunction();
    InitConstructor("<LockWrap>", constructor);
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
}

v8::Local<v8::Object> LockWrap::NewInstance() {
    return binding::CreateShareableWrap(std::make_shared<std::mutex>(), exportName);
}

void LockWrap::GuardSyncCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument is required for calling 'guardSync'.");
    CHECK_ARG(isolate, args[0]->IsFunction(), "Argument \"func\" shall be 'Function' type.");

    auto context = isolate->GetCurrentContext();
    auto holder = args.Holder();
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<LockWrap>(args.Holder());

    v8::TryCatch tryCatch;
    try {
        auto mutex = thisObject->Get<std::mutex>();
        std::lock_guard<std::mutex> guard(*mutex);

        auto result = v8::Local<v8::Function>::Cast(args[0])->Call(context, holder, 0, nullptr);

        if (result.IsEmpty() || tryCatch.HasCaught()) {
            tryCatch.ReThrow();
        } else {
            args.GetReturnValue().Set(result.ToLocalChecked());
        }
    } catch (const std::system_error& ex) {
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
    }
}
