// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <napa/module.h>

#include "simple-object-wrap.h"

using namespace napa;
using namespace napa::test;
using namespace napa::module;


void GetModuleName(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, "simple-napa-addon"));
}

void CreateSimpleObjectWrap(const v8::FunctionCallbackInfo<v8::Value>& args) {
    SimpleObjectWrap::NewInstance(args);
}

void Init(v8::Local<v8::Object> exports) {
    SimpleObjectWrap::Init();

    NODE_SET_METHOD(exports, "getModuleName", GetModuleName);
    NODE_SET_METHOD(exports, "createSimpleObjectWrap", CreateSimpleObjectWrap);
}

NODE_MODULE(addon, Init);
