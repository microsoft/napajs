// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "lock-wrap.h"

#include <napa/module/binding/wraps.h>

#include <memory>
#include <mutex>

using namespace napa::module;

namespace {
    class LockImpl {
    public:
        LockImpl(): _lock(_mutex, std::defer_lock) {}

        void Enter() {
            _lock.lock();
        }
        void Exit() {
            _lock.unlock();
        }
    private:
        std::mutex _mutex;
        std::unique_lock<std::mutex> _lock;
    };
}

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(napa::module::LockWrap)

void LockWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<LockWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    InitConstructorTemplate<LockWrap>(constructorTemplate);

    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "enter", EnterCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "exit", ExitCallback);

    auto constructor = constructorTemplate->GetFunction();
    InitConstructor("<LockWrap>", constructor);
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
}

v8::Local<v8::Object> LockWrap::NewInstance() {
    return binding::CreateShareableWrap(std::make_shared<LockImpl>(), exportName);
}

void LockWrap::EnterCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<LockWrap>(args.Holder());
    try {
        thisObject->Get<LockImpl>()->Enter();
    } catch (const std::system_error& ex) {
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
    }
}

void LockWrap::ExitCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<LockWrap>(args.Holder());
    try {
        thisObject->Get<LockImpl>()->Exit();
    } catch (const std::system_error& ex) {
        isolate->ThrowException(v8::Exception::Error(v8_helpers::MakeV8String(isolate, ex.what())));
    }
}
