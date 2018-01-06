// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "store-wrap.h"
#include <napa/transport.h>

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(StoreWrap);
    
void StoreWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();
    auto constructorTemplate = v8::FunctionTemplate::New(isolate, DefaultConstructorCallback<StoreWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "set", SetCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "get", GetCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "has", HasCallback);
    NAPA_SET_PROTOTYPE_METHOD(constructorTemplate, "delete", DeleteCallback);
    NAPA_SET_ACCESSOR(constructorTemplate, "id", GetIdCallback, nullptr);
    NAPA_SET_ACCESSOR(constructorTemplate, "size", GetSizeCallback, nullptr);

    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructorTemplate->GetFunction());
}

v8::Local<v8::Object> StoreWrap::NewInstance(std::shared_ptr<napa::store::Store> store) {
    auto object = napa::module::NewInstance<StoreWrap>().ToLocalChecked();
    auto wrap = NAPA_OBJECTWRAP::Unwrap<StoreWrap>(object);
    wrap->_store = std::move(store);
    return object;
}

napa::store::Store& StoreWrap::Get() {
    return *_store;
}

void StoreWrap::SetCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    
    CHECK_ARG(isolate, args.Length() == 2, "2 arguments are required for \"set\".");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument \"key\" must be string.");

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<StoreWrap>(args.Holder());
    auto& store = thisObject->Get();

    // Marshall value object into payload.
    napa::transport::TransportContext transportContext;
    auto payload = napa::transport::Marshall(args[1], &transportContext);
    
    RETURN_ON_PENDING_EXCEPTION(payload);
    
    store.Set(
        v8_helpers::V8ValueTo<std::string>(args[0]).c_str(),
        std::make_shared<napa::store::Store::ValueType>(napa::store::Store::ValueType {
            v8_helpers::V8ValueTo<std::u16string>(payload.ToLocalChecked()),
            std::move(transportContext)
        }));
}

void StoreWrap::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument are required for \"get\".");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'key' must be string.");

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<StoreWrap>(args.Holder());
    auto& store = thisObject->Get();

    // Marshall value object into payload.
    auto key = v8_helpers::V8ValueTo<std::string>(args[0]);
    auto storeValue = store.Get(key.c_str());
    if (storeValue != nullptr) {
        auto value = napa::transport::Unmarshall(
            v8_helpers::MakeExternalV8String(isolate, storeValue->payload), 
            &(storeValue->transportContext));

        RETURN_ON_PENDING_EXCEPTION(value);
        args.GetReturnValue().Set(value.ToLocalChecked());
    }
}

void StoreWrap::HasCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    
    CHECK_ARG(isolate, args.Length() == 1, "1 argument are required for \"has\".");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'key' must be string.");

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<StoreWrap>(args.Holder());
    auto& store = thisObject->Get();

    args.GetReturnValue().Set(store.Has(v8_helpers::V8ValueTo<std::string>(args[0]).c_str()));
}

void StoreWrap::DeleteCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    
    CHECK_ARG(isolate, args.Length() == 1, "1 argument are required for \"delete\".");
    CHECK_ARG(isolate, args[0]->IsString(), "Argument 'key' must be string.");

    auto thisObject = NAPA_OBJECTWRAP::Unwrap<StoreWrap>(args.Holder());
    auto& store = thisObject->Get();

    store.Delete(v8_helpers::V8ValueTo<std::string>(args[0]).c_str());
}

void StoreWrap::GetIdCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<StoreWrap>(args.Holder());
    auto& store = thisObject->Get();

    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, store.GetId()));
}

void StoreWrap::GetSizeCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    
    auto thisObject = NAPA_OBJECTWRAP::Unwrap<StoreWrap>(args.Holder());
    auto& store = thisObject->Get();

    args.GetReturnValue().Set(static_cast<uint32_t>(store.Size()));
}

