#include "transport-context-wrap-impl.h"
#include <napa/module/shared-wrap.h>

using namespace napa::transport;
using namespace napa::module;
using namespace napa::v8_helpers;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(TransportContextWrapImpl);

TransportContextWrapImpl::TransportContextWrapImpl(TransportContext* context) {
    _context = context;
}

TransportContext* TransportContextWrapImpl::Get() {
    return _context;
}

void TransportContextWrapImpl::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, TransportContextWrapImpl::ConstructorCallback);
    constructorTemplate->SetClassName(MakeV8String(isolate, _exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "saveShared", SaveSharedCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "loadShared", LoadSharedCallback);
    NAPA_SET_ACCESSOR(constructorTemplate, "sharedCount", GetSharedCountCallback, nullptr);

    NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, constructorTemplate->GetFunction());
}

void TransportContextWrapImpl::GetSharedCountCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args){
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<TransportContextWrapImpl>(args.Holder());
    JS_ENSURE(isolate, thisObject != nullptr, "Invalid object to get property \"sharedCount\".");

    args.GetReturnValue().Set(thisObject->_context->GetSharedCount());
}

void TransportContextWrapImpl::ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    JS_ENSURE(isolate, args.IsConstructCall(), 
        "class \"TransportContextWrap\" allows constructor call only.");

    CHECK_ARG(isolate, args.Length() <= 1, 
        "class \"TransportContextWrap\" accept no arguments or 1 argument of 'handle' as constructor.'");

    // It's deleted when its Javascript object is garbage collected by V8's GC.
    // TODO: use replacement new to allocate memory from napa.dll
    TransportContext* context = nullptr;
    
    if (args.Length() == 0 || args[0]->IsUndefined()) {
        context = new TransportContext();
    } else {
        auto result = v8_helpers::V8ValueToPtr<TransportContext>(isolate, args[0]);
        JS_ENSURE(isolate, result.second, 
            "argument 'handle' must be of type [number, number].");
        context = result.first;
    }
    auto wrap = new TransportContextWrapImpl(context);
    wrap->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void TransportContextWrapImpl::SaveSharedCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 arguments are required for \"saveShared\".");
    CHECK_ARG(isolate, args[0]->IsObject(), "Argument \"object\" shall be 'SharedWrap' type.");

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<TransportContextWrap>(args.Holder());
    auto sharedWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(v8::Local<v8::Object>::Cast(args[0]));
    thisObject->Get()->SaveShared(sharedWrap->Get<void>());
}

void TransportContextWrapImpl::LoadSharedCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 arguments are required for \"saveShared\".");

    auto result = v8_helpers::V8ValueToUintptr(isolate, args[0]);
    JS_ENSURE(isolate, result.second, "Unable to cast \"handle\" to pointer. Please check if it's in valid format.");

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<TransportContextWrap>(args.Holder());
    auto object = thisObject->Get()->LoadShared<void>(result.first);

    args.GetReturnValue().Set(SharedWrap::NewInstance(object));
}
