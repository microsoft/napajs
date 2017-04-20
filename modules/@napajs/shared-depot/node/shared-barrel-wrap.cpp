#include "shared-barrel-wrap.h"

#include <napa-log.h>
#include <napa/module/shared-wrap.h>

using namespace napa::module;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(SharedBarrelWrap)

void SharedBarrelWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();

    // Prepare constructor template.
    auto functionTemplate = v8::FunctionTemplate::New(isolate, NewCallback);
    functionTemplate->SetClassName(v8_helpers::MakeV8String(isolate, _exportName));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // Set prototype method.
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "id", GetId);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "exists", Exists);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "set", Set);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "get", Get);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "remove", Remove);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "count", Count);

    // Set persistent constructor into V8.
    NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
}

v8::Local<v8::Object> SharedBarrelWrap::Create(std::shared_ptr<SharedBarrel> barrel) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto instance = NewInstance();
    auto wrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(instance);
    wrap->_barrel = std::move(barrel);

    return scope.Escape(instance);
}

std::shared_ptr<SharedBarrel> SharedBarrelWrap::Get() {
    return _barrel;
}

void SharedBarrelWrap::NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    JS_ASSERT(isolate, args.IsConstructCall(), "Only constructor call is allowed");

    auto wrap = new SharedBarrelWrap();
    wrap->Wrap(args.This());

    args.GetReturnValue().Set(args.This());
}

v8::Local<v8::Object> SharedBarrelWrap::NewInstance() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);

    auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName, SharedBarrelWrap);
    auto context = isolate->GetCurrentContext();
    auto instance = constructor->NewInstance(context).ToLocalChecked();

    return scope.Escape(instance);
}

void SharedBarrelWrap::GetId(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    auto barrelWrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(args.This());
    JS_ASSERT(isolate, barrelWrap != nullptr, "Can't unwrap a shared barrel wrapper.");

    args.GetReturnValue().Set(v8::Uint32::NewFromUnsigned(isolate, barrelWrap->_barrel->GetId()));
}

void SharedBarrelWrap::Exists(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && args[0]->IsString(),
        "Shared object name must be given as argument.");

    v8::String::Utf8Value key(args[0]->ToString());

    auto barrelWrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(args.This());
    JS_ASSERT(isolate, barrelWrap != nullptr, "Can't unwrap a shared barrel wrapper.");

    auto exists = barrelWrap->_barrel->Exists(*key);
    args.GetReturnValue().Set(exists);
}

void SharedBarrelWrap::Set(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 2,
        "Two arguments are required.");

    CHECK_ARG(isolate,
        args[0]->IsString(),
        "The first argument must be a shared object name.");

    CHECK_ARG(isolate,
        args[1]->IsObject(),
        "The second argument must be a SharedWrap instance.");

    v8::String::Utf8Value key(args[0]->ToString());

    auto objectWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args[1]->ToObject());
    JS_ASSERT(isolate, objectWrap != nullptr, "Can't unwrap a shared object wrapper.");
    auto object = objectWrap->Get();

    auto barrelWrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(args.This());
    JS_ASSERT(isolate, barrelWrap != nullptr, "Can't unwrap a shared barrel wrapper.");

    auto succeeded = barrelWrap->_barrel->Set(*key, object);
    args.GetReturnValue().Set(succeeded);
}

void SharedBarrelWrap::Get(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && args[0]->IsString(),
        "Shared object name must be given as argument.");

    v8::String::Utf8Value key(args[0]->ToString());

    auto barrelWrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(args.This());
    JS_ASSERT(isolate, barrelWrap != nullptr, "Can't unwrap a shared barrel wrapper.");

    auto object = barrelWrap->_barrel->Get<void>(*key);
    if (object == nullptr) {
        args.GetReturnValue().SetNull();
        return;
    }

    auto objectWrap = SharedWrap::Create(std::move(object));
    args.GetReturnValue().Set(objectWrap);
}

void SharedBarrelWrap::Remove(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && args[0]->IsString(),
        "Shared object name must be given as argument.");

    v8::String::Utf8Value key(args[0]->ToString());

    auto barrelWrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(args.This());
    JS_ASSERT(isolate, barrelWrap != nullptr, "Can't unwrap a shared barrel wrapper.");

    barrelWrap->_barrel->Remove(*key);

    args.GetReturnValue().SetUndefined();
}

void SharedBarrelWrap::Count(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    auto barrelWrap = NAPA_OBJECTWRAP::Unwrap<SharedBarrelWrap>(args.This());
    JS_ASSERT(isolate, barrelWrap != nullptr, "Can't unwrap a shared barrel wrapper.");

    uint32_t count = static_cast<uint32_t>(barrelWrap->_barrel->GetCount());
    args.GetReturnValue().Set(v8::Uint32::NewFromUnsigned(isolate, count));
}