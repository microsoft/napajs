// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "serializer.h"
#include "deserializer.h"

namespace napa {
namespace transport {

    using namespace v8;

    class TransportUtils {
    public:
        static std::shared_ptr<SerializedData>
        SerializeValue(Isolate* isolate, Local<Value> value) {
            bool ok;
            Serializer serializer(isolate);
            if (serializer.WriteValue(value).To(&ok)) {
                return serializer.Release();
            }
            return nullptr;
        }
        
        static MaybeLocal<Value>
        DeserializeValue(Isolate* isolate, std::shared_ptr<SerializedData>& data) {
            Local<Value> value;
            Deserializer deserializer(isolate, data);
            return deserializer.ReadValue();
        }
    };
}
}
