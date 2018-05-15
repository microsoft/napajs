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

// Use well-known symbol definition
// https://github.com/nodejs/node/pull/18934
// https://github.com/nodejs/node/blob/master/test/addons/hello-world/binding.cc

#define CONCAT(a, b) CONCAT_HELPER(a, b)
#define CONCAT_HELPER(a, b) a##b
#define INITIALIZER CONCAT(node_register_module_v, NODE_MODULE_VERSION)

extern "C" NODE_MODULE_EXPORT void INITIALIZER(v8::Local<v8::Object> exports,
                                               v8::Local<v8::Value> module,
                                               v8::Local<v8::Context> context) {
    SimpleObjectWrap::Init();

    NODE_SET_METHOD(exports, "getModuleName", GetModuleName);
    NODE_SET_METHOD(exports, "createSimpleObjectWrap", CreateSimpleObjectWrap);
}
