// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "allocator-wrap.h"

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(AllocatorWrap);

void AllocatorWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<AllocatorWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    InitConstructorTemplate<AllocatorWrap>(constructorTemplate);
    
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "allocate", AllocateCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "deallocate", DeallocateCallback);
    NAPA_SET_ACCESSOR(constructorTemplate, "type", GetTypeCallback, nullptr);

    auto constructor = constructorTemplate->GetFunction();
    InitConstructor("<AllocatorWrap>", constructor);
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
}

std::shared_ptr<napa::memory::Allocator> AllocatorWrap::Get() {
    return ShareableWrap::Get<napa::memory::Allocator>();
}

void AllocatorWrap::AllocateCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument of 'size' is required for \"allocate\".");
    CHECK_ARG(isolate, args[0]->IsUint32(), "Argument \"size\" must be a unsigned integer.");

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(args.Holder());
    auto allocator = thisObject->Get();
    JS_ENSURE(isolate, allocator != nullptr, "AllocatorWrap is not attached with any C++ allocator.");

    auto handle = v8_helpers::PtrToV8Uint32Array(isolate, allocator->Allocate(args[0]->Uint32Value()));
    args.GetReturnValue().Set(handle);
}

void AllocatorWrap::DeallocateCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 2, "2 arguments is required for \"deallocate\".");
    auto result = v8_helpers::V8ValueToUintptr(isolate, args[0]);
    JS_ENSURE(isolate, result.second, "Unable to cast \"handle\" to pointer. Please check if it's in valid handle format.");
    
    CHECK_ARG(isolate, args[1]->IsUint32(), "Argument \"sizeHint\" must be a 32-bit unsigned integer.");
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(args.Holder());
    auto allocator = thisObject->Get();
    JS_ENSURE(isolate, allocator != nullptr, "AllocatorWrap is not attached with any C++ allocator.");
    
    allocator->Deallocate(reinterpret_cast<void*>(result.first), args[1]->Uint32Value());
}

void AllocatorWrap::GetTypeCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args){
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(args.Holder());
    auto allocator = thisObject->Get();
    args.GetReturnValue().Set(
        v8_helpers::MakeV8String(
            isolate, 
            allocator != nullptr ? allocator->GetType() : "<unloaded>"));
}