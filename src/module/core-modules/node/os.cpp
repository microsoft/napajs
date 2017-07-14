#include "os.h"

#include <napa-module.h>
#include <platform/os.h>

using namespace napa;
using namespace napa::module;

void TypeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, platform::GetOSType()));
}

void os::Init(v8::Local<v8::Object> exports) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = isolate->GetCurrentContext();

    NAPA_SET_METHOD(exports, "type", TypeCallback);
}