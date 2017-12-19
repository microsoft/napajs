// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "serialized-data.h"

#include <v8.h>

namespace napa {
namespace v8_extensions {

    /// <summary>
    /// Serializer is used to serialize a JavaScript object to a SerializedData instance.
    /// It is derived from v8::ValueSerializer::Delegate, whose interface is implemented
    /// to handle ShardArrayBuffer specially as below.
    ///   If a SharedArrayBuffer is being serialized for the first time, 
    ///   1). it will be externlized and the ExternalizedContents will be attached to its SerializedData.
    ///   2). a ShareableWrap of the ExternalizedContents will be set to the input SharedArrayBuffer.
    ///   If a SharedArrayBuffer has been serialized, the externalization will be skipped, and its ExternalizedContents
    ///   will be retrieved from the input SharedArrayBuffer and attached to its SerializedData.
    /// </summary>
    class Serializer : public v8::ValueSerializer::Delegate {
    public:
        explicit Serializer(v8::Isolate* isolate);

        v8::Maybe<bool> WriteValue(v8::Local<v8::Value> value);

        std::shared_ptr<SerializedData> Release();

    protected:
        void ThrowDataCloneError(v8::Local<v8::String> message) override;

        v8::Maybe<uint32_t> GetSharedArrayBufferId(
            v8::Isolate* isolate,
            v8::Local<v8::SharedArrayBuffer> sharedArrayBuffer
        ) override;

        void* ReallocateBufferMemory(void* oldBuffer, size_t size, size_t* actualSize) override;

        void FreeBufferMemory(void* buffer) override;

    private:
        ExternalizedSharedArrayBufferContents MaybeExternalize(v8::Local<v8::SharedArrayBuffer> sharedArrayBuffer);

        v8::Maybe<bool> FinalizeTransfer();

        v8::Isolate* _isolate;
        v8::ValueSerializer _serializer;
        std::shared_ptr<SerializedData> _data;
        std::vector<v8::Global<v8::SharedArrayBuffer>> _sharedArrayBuffers;

        Serializer(const Serializer&) = delete;
        Serializer& operator=(const Serializer&) = delete;
    };
}
}
