// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "allocator-debugger-wrap.h"

#include <napa/memory.h>

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(AllocatorDebuggerWrap);

AllocatorDebuggerWrap::AllocatorDebuggerWrap(std::shared_ptr<napa::memory::AllocatorDebugger> allocatorDebugger) {
    this->_object = std::move(allocatorDebugger);
}

void AllocatorDebuggerWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, ConstructorCallback);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    InitConstructorTemplate<AllocatorDebuggerWrap>(constructorTemplate);

    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "allocate", AllocatorWrap::AllocateCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "deallocate", AllocatorWrap::DeallocateCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "getDebugInfo", GetDebugInfoCallback);
    NAPA_SET_ACCESSOR(constructorTemplate, "type", AllocatorWrap::GetTypeCallback, nullptr);

    auto constructor = constructorTemplate->GetFunction();
    InitConstructor("<AllocatorDebugger>", constructor);
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
}

void AllocatorDebuggerWrap::ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    JS_ENSURE(isolate, args.IsConstructCall(), "Class \"AllocatorDebuggerWrap\" allows constructor call only.");
    CHECK_ARG(isolate,
              args.Length() <= 1,
              "Class \"AllocatorDebuggerWrap\" requires 1 argument of \"allocator\" in constructor.'");

    std::shared_ptr<napa::memory::Allocator> allocator;
    if (args.Length() == 0) {
        allocator = std::shared_ptr<napa::memory::Allocator>(&napa::memory::GetDefaultAllocator(),
                                                             [](napa::memory::Allocator*) {});
    } else {
        CHECK_ARG(isolate, args[0]->IsObject(), "Argument \"allocator\" should be \"AllocatorWrap\" type.'");
        auto allocatorWrap = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(v8::Local<v8::Object>::Cast(args[0]));
        JS_ENSURE(isolate, allocatorWrap != nullptr, "argument \"allocator\" must be of type \"AllocatorWrap\".");
        allocator = allocatorWrap->Get();
    }

    // It's deleted when its Javascript object is garbage collected by V8's GC.
    auto wrap = new AllocatorDebuggerWrap(NAPA_MAKE_SHARED<napa::memory::SimpleAllocatorDebugger>(allocator));
    wrap->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void AllocatorDebuggerWrap::GetDebugInfoCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<ShareableWrap>(args.Holder());
    args.GetReturnValue().Set(
        v8_helpers::MakeV8String(isolate, thisObject->Get<napa::memory::AllocatorDebugger>()->GetDebugInfo()));
}
