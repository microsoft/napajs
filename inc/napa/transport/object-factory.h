// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module/binding.h>
#include <napa/v8-helpers.h>

namespace napa {
namespace objectFactory {
    inline void RegisterConstrutor(const char* const cidStr, v8::Local<v8::Function> constructor) {
        auto cid = v8_helpers::MakeV8String(v8::Isolate::GetCurrent(), cidStr);
        v8::Local<v8::Value> argv[] = { cid, constructor };
        (void)napa::module::binding::Call("../lib/transport/object-factory", "registerConstructor", sizeof(argv) / sizeof(v8::Local<v8::Value>), argv);
    }
    
    inline v8::Local<v8::Function> GetConstructor(const char* const cidStr) {
        auto cid = v8_helpers::MakeV8String(v8::Isolate::GetCurrent(), cidStr);
        v8::Local<v8::Value> argv[] = { cid };
        auto v = napa::module::binding::Call("../lib/transport/object-factory", "getConstructor", sizeof(argv) / sizeof(v8::Local<v8::Value>), argv);
    
        return v8::Local<v8::Function>::Cast(v.ToLocalChecked());
    }
    
    inline v8::MaybeLocal<v8::Value> NewInstance(const char* const cidStr) {
        auto cid = v8_helpers::MakeV8String(v8::Isolate::GetCurrent(), cidStr);
        v8::Local<v8::Value> argv[] = { cid };
        return napa::module::binding::Call("../lib/transport/object-factory", "newInstance", sizeof(argv) / sizeof(v8::Local<v8::Value>), argv);
    }
}
}