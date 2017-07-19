// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>
#include <napa/stl/string.h>

namespace napa {
namespace v8_helpers {

    static const uint32_t NANOS_PER_SECOND = 1000000000;

    /// <summary> Make a v8 array from high-resolution time. </summary>
    inline v8::Local<v8::Array> HrtimeToV8Uint32Array(v8::Isolate* isolate, uint64_t time) {
        v8::EscapableHandleScope scope(isolate);
        auto context = isolate->GetCurrentContext();
        
        v8::Local<v8::Array> res = v8::Array::New(isolate, 2);
        (void)res->CreateDataProperty(
            context,
            0,
            v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(time / NANOS_PER_SECOND)));

        (void)res->CreateDataProperty(
            context,
            1,
            v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(time % NANOS_PER_SECOND)));

        return scope.Escape(res);
    }

    /// <summary> Convert a 2-element v8 array to high-resolution time in nano-seconds. </summary>
    inline std::pair<uint64_t, bool> V8Uint32ArrayToHrtime(v8::Isolate* isolate, v8::Local<v8::Value> value) {
        v8::EscapableHandleScope scope(isolate);
        auto context = isolate->GetCurrentContext();
        
        if (value.IsEmpty() || !value->IsArray()) {
            return std::make_pair(0, false);
        }

        auto array = v8::Local<v8::Array>::Cast(value);
        if (array->Length() != 2) {
            return std::make_pair(0, false);
        }
        return std::make_pair(static_cast<uint64_t>(array->Get(0)->Uint32Value()) * NANOS_PER_SECOND + array->Get(1)->Uint32Value(), true);
    }
}
}