// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/exports.h>
#include <v8.h>

namespace napa {
namespace v8_extensions {

    class SerializedData;

    class NAPA_API Utils {
        public:
        static std::shared_ptr<SerializedData>
        SerializeValue(v8::Isolate* isolate, v8::Local<v8::Value> value);
        
        static v8::MaybeLocal<v8::Value>
        DeserializeValue(v8::Isolate* isolate, std::shared_ptr<SerializedData>& data);
    };
}
}
