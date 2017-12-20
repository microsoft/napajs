// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "v8-extensions-macros.h"

#if V8_VERSION_CHECK_FOR_BUILT_IN_TYPE_TRANSPORTER

#include "deserializer.h"
#include "serializer.h"
#include "v8-extensions.h"

using namespace napa;
using namespace v8;

std::shared_ptr<v8_extensions::SerializedData>
v8_extensions::Utils::SerializeValue(Isolate* isolate, Local<Value> value) {
    bool ok = false;
    Serializer serializer(isolate);
    if (serializer.WriteValue(value).To(&ok)) {
        return serializer.Release();
    }
    return nullptr;
}

MaybeLocal<Value>
v8_extensions::Utils::DeserializeValue(Isolate* isolate, std::shared_ptr<v8_extensions::SerializedData>& data) {
    Local<Value> value;
    Deserializer deserializer(isolate, data);
    return deserializer.ReadValue();
}

#endif
