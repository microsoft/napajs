#include "simple-allocator-debugger-wrap.h"

#include <napa-memory.h>
#include <napa/memory/allocator-debugger.h>

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(SimpleAllocatorDebuggerWrap);

SimpleAllocatorDebuggerWrap::SimpleAllocatorDebuggerWrap(std::shared_ptr<napa::memory::Allocator> allocator) {
    this->_object = NAPA_MAKE_SHARED<napa::memory::SimpleAllocatorDebugger>(std::move(allocator));
}

void SimpleAllocatorDebuggerWrap::Init(){
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, ConstructorCallback);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    AllocatorWrap::InitConstructorTemplate<SimpleAllocatorDebuggerWrap>(constructorTemplate);

    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "getDebugInfo", GetDebugInfoCallback);

    auto constructor = constructorTemplate->GetFunction();
    AllocatorWrap::InitConstructor("<SimpleAllocatorDebugger>", constructor);
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
}

void SimpleAllocatorDebuggerWrap::ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    JS_ENSURE(isolate, args.IsConstructCall(), "Class \"SimpleAllocatorDebuggerWraporWrap\" allows constructor call only.");
    CHECK_ARG(isolate, args.Length() <= 1, "Class \"SimpleAllocatorDebuggerWrap\" requires 1 argument of \"allocator\" in constructor.'");

    std::shared_ptr<napa::memory::Allocator> allocator;
    if (args.Length() == 0) {
        allocator = NAPA_MAKE_SHARED<napa::memory::DefaultAllocator>();
    } 
    else {
        CHECK_ARG(isolate, args[0]->IsObject(), "Argument \"allocator\" should be \"AllocatorWrap\" type.'");
        auto allocatorWrap = NAPA_OBJECTWRAP::Unwrap<AllocatorWrap>(v8::Local<v8::Object>::Cast(args[0]));
        JS_ENSURE(isolate, allocatorWrap != nullptr,  "argument \"allocator\" must be of type \"AllocatorWrap\".");
        allocator = allocatorWrap->Get();
    }

    // It's deleted when its Javascript object is garbage collected by V8's GC.
    auto wrap = new SimpleAllocatorDebuggerWrap(allocator);
    wrap->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void SimpleAllocatorDebuggerWrap::GetDebugInfoCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args.Holder());
    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, thisObject->Get<napa::memory::AllocatorDebugger>()->GetDebugInfo()));
}
