// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "call-context-wrap.h"
#include "transport-context-wrap-impl.h"

#include <napa/transport.h>

using namespace napa;
using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(CallContextWrap);

void CallContextWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<CallContextWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    InitConstructorTemplate<CallContextWrap>(constructorTemplate);
    
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "resolve", ResolveCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "reject", RejectCallback);
    NAPA_SET_ACCESSOR(constructorTemplate, "finished", IsFinishedCallback, nullptr);
    NAPA_SET_ACCESSOR(constructorTemplate, "elapse", GetElapseCallback, nullptr);
    NAPA_SET_ACCESSOR(constructorTemplate, "module", GetModuleCallback, nullptr);
    NAPA_SET_ACCESSOR(constructorTemplate, "function", GetFunctionCallback, nullptr);
    NAPA_SET_ACCESSOR(constructorTemplate, "args", GetArgumentsCallback, nullptr);
    NAPA_SET_ACCESSOR(constructorTemplate, "transportContext", GetTransportContextCallback, nullptr);
    NAPA_SET_ACCESSOR(constructorTemplate, "options", GetOptionsCallback, nullptr);

    auto constructor = constructorTemplate->GetFunction();
    InitConstructor("<CallContextWrap>", constructor);
    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructor);
}

v8::Local<v8::Object> CallContextWrap::NewInstance(std::shared_ptr<zone::CallContext> call) {
    return ShareableWrap::NewInstance<CallContextWrap>(call);
}

zone::CallContext& CallContextWrap::GetRef() {
    return ShareableWrap::GetRef<zone::CallContext>();
}

void CallContextWrap::ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument of 'result' is required for \"resolve\".");
    
    v8::String::Utf8Value result(args[0]);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());
    auto success = thisObject->GetRef().Resolve(std::string(*result, result.length()));

    JS_ENSURE(isolate, success, "Resolve call failed: Already finished.");
}

void CallContextWrap::RejectCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1 || args.Length() == 2, "at least 1 argument of 'reason' is required for \"reject\".");

    napa::ResultCode code = NAPA_RESULT_EXECUTE_FUNC_ERROR;
    v8::Local<v8::Value> reason;

    if (args.Length() == 1) {
        reason = args[0];
    } else {
        CHECK_ARG(isolate, args[0]->IsUint32(), "arg 'resultCode' should be a number type.");
        code = static_cast<napa::ResultCode>(args[0]->Uint32Value());

        reason = args[1];
    }
    
    v8::String::Utf8Value reasonStr(reason);

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());
    auto success = thisObject->GetRef().Reject(code, std::string(*reasonStr, reasonStr.length()));
    JS_ENSURE(isolate, success, "Reject call failed: Already finished.");
}

void CallContextWrap::IsFinishedCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args){
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());
    args.GetReturnValue().Set(thisObject->GetRef().IsFinished());
}

void CallContextWrap::GetElapseCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args){
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());
    args.GetReturnValue().Set(v8_helpers::HrtimeToV8Uint32Array(isolate, thisObject->GetRef().GetElapse().count()));
}

void CallContextWrap::GetModuleCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());
    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, thisObject->GetRef().GetModule()));
}

void CallContextWrap::GetFunctionCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());
    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, thisObject->GetRef().GetFunction()));
}

void CallContextWrap::GetArgumentsCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());

    auto& cppArgs = thisObject->GetRef().GetArguments();
    auto jsArgs = v8::Array::New(isolate, static_cast<int>(cppArgs.size()));
    for (size_t i = 0; i < cppArgs.size(); ++i) {
        // TODO: Switch to 2-bytes external string.
        (void)jsArgs->CreateDataProperty(context, static_cast<uint32_t>(i), v8_helpers::MakeV8String(isolate, cppArgs[i]));
    }
    args.GetReturnValue().Set(jsArgs);
}

void CallContextWrap::GetTransportContextCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<CallContextWrap>(args.Holder());

    auto& transportContext = thisObject->GetRef().GetTransportContext();
    // Create a non-owning transport context wrap, since transport context is always owned by call context. 
    auto wrap = TransportContextWrapImpl::NewInstance(false, &transportContext);
    args.GetReturnValue().Set(wrap);
}

void CallContextWrap::GetOptionsCallback(v8::Local<v8::String> /*propertyName*/, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    
    // Prepare execute options.
    // NOTE: export necessary fields from CallContext.GetOptions to jsOptions object here. Now it's empty.
    auto jsOptions = v8::Object::New(isolate);

    args.GetReturnValue().Set(jsOptions);
}

