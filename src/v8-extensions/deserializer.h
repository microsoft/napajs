// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "serialized-data.h"

#include <v8.h>

namespace napa {
namespace v8_extensions {

    /// <summary>
    /// Deserializer is used to deserialize a SerializedData instance to a JavaScript object.
    /// It is derived from v8::ValueDeserializer::Delegate, whose interface is implemented
    /// to handle ShardArrayBuffer specially as below.
    ///   For each of the SharedArrayBuffer in the input SerializedData, 
    ///   1). create a SharedArrayBuffer instance from its SharedArrayBuffer::Contents stored in SerializedData.
    ///   2). generate a ShareableWrap of ExternalizedContents, and attach it to the SharedArrayBuffer instance.
    /// </summary>
    class Deserializer : public v8::ValueDeserializer::Delegate {
    public:
        Deserializer(v8::Isolate* isolate, std::shared_ptr<SerializedData> data);

        v8::MaybeLocal<v8::Value> ReadValue();

        static Deserializer* NewDeserializer(v8::Isolate* isolate, std::shared_ptr<SerializedData> data);

    private:
        v8::Isolate* _isolate;
        v8::ValueDeserializer _deserializer;
        std::shared_ptr<SerializedData> _data;

        Deserializer(const Deserializer&) = delete;
        Deserializer& operator=(const Deserializer&) = delete;
    };
}
}
