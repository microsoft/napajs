// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "lock-wrap.h"

#include <napa/module/binding/wraps.h>

#include <memory>
#include <mutex>
#include <vector>

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(napa::module::LockWrap);

void LockWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<LockWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    InitConstructorTemplate<LockWrap>(constructorTemplate);

    NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "guardSync", GuardSyncCallback);

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

    CHECK_ARG(isolate, args.Length() >= 1, "1 argument is required for calling 'guardSync'.");
    CHECK_ARG(isolate, args[0]->IsFunction(), "Argument \"func\" shall be 'Function' type.");
    CHECK_ARG(isolate, args.Length() < 2 || args[1]->IsArray(), "Argument \"params\" shall be a valid array.");

    auto context = isolate->GetCurrentContext();
    auto holder = args.Holder();
    auto thisObject = node::ObjectWrap::Unwrap<LockWrap>(args.Holder());

    v8::TryCatch tryCatch(isolate);
    try {
        auto mutex = thisObject->Get<std::mutex>();
        std::lock_guard<std::mutex> guard(*mutex);

        std::vector<v8::Local<v8::Value>> params;

        if (args.Length() >= 2) {
            auto paramsArray = v8::Local<v8::Array>::Cast(args[1]);
            int paramsLength = paramsArray->Length();
            params.reserve(paramsLength);

            for (int i = 0; i < paramsLength; i++) {
                auto item = paramsArray->Get(context, i).ToLocalChecked();
                params.emplace_back(item);
            }
        }

        auto result = v8::Local<v8::Function>::Cast(args[0])->Call(
            context,
            holder,
            static_cast<int>(params.size()),
            params.empty() ? nullptr : params.data());

        if (result.IsEmpty() || tryCatch.HasCaught()) {
            tryCatch.ReThrow();
        } else {
            args.GetReturnValue().Set(result.ToLocalChecked());
        }
    } catch (const std::system_error& ex) {
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
    }
}
