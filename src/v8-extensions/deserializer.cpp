// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "v8-extensions-macros.h"

#if V8_VERSION_CHECK_FOR_BUILT_IN_TYPE_TRANSPORTER

#include "deserializer.h"

#include <napa/module/binding/basic-wraps.h>

using namespace napa::v8_extensions;
using namespace v8;

Deserializer::Deserializer(Isolate* isolate, std::shared_ptr<SerializedData> data) :
    _isolate(isolate),
    _deserializer(isolate, data->GetData(), data->GetSize(), this),
    _data(std::move(data)) {
    _deserializer.SetSupportsLegacyWireFormat(true);
}

MaybeLocal<Value> Deserializer::ReadValue() {
    bool readHeader = false;
    Local<Context> context = _isolate->GetCurrentContext();
    if (!_deserializer.ReadHeader(context).To(&readHeader)) {
        return MaybeLocal<Value>();
    }

#if !V8_VERSION_EQUALS_TO_OR_NEWER_THAN(6, 6)

    uint32_t index = 0;
    Local<String> key = v8_helpers::MakeV8String(_isolate, "_externalized");
    for (const auto& contents : _data->GetExternalizedSharedArrayBufferContents()) {
        Local<SharedArrayBuffer> sharedArrayBuffers = SharedArrayBuffer::New(
            _isolate, contents.first.Data(), contents.first.ByteLength());
        auto shareableWrap = napa::module::binding::CreateShareableWrap(contents.second);

        // After deserialization of a SharedArrayBuffer from its SerializedData,
        // set its '_externalized' property to a ShareableWrap of its ExternalizedContents.
        // This extends the lifecycle of the ExternalizedContents by the lifetime of the restored SharedArrayBuffer object.
        sharedArrayBuffers->CreateDataProperty(context, key, shareableWrap);
        _deserializer.TransferSharedArrayBuffer(index++, sharedArrayBuffers);
    }

#endif

    return _deserializer.ReadValue(context);
}

#if V8_VERSION_EQUALS_TO_OR_NEWER_THAN(6, 6)

MaybeLocal<SharedArrayBuffer> Deserializer::GetSharedArrayBufferFromId(
    Isolate* isolate, uint32_t cloneId) {
    if (_data && cloneId < _data->GetExternalizedSharedArrayBufferContents().size()) {
        auto externalizedSharedArrayBufferContents = _data->GetExternalizedSharedArrayBufferContents().at(cloneId);
        SharedArrayBuffer::Contents contents = externalizedSharedArrayBufferContents.first;
        auto sharedArrayBuffer = SharedArrayBuffer::New(isolate, contents.Data(), contents.ByteLength());

        // After deserialization of a SharedArrayBuffer from its SerializedData,
        // set its '_externalized' property to a ShareableWrap of its ExternalizedContents.
        // This extends the lifecycle of the ExternalizedContents
        // by the lifetime of the restored SharedArrayBuffer object.
        Local<Context> context = _isolate->GetCurrentContext();
        Local<String> key = v8_helpers::MakeV8String(_isolate, "_externalized");
        auto shareableWrap = napa::module::binding::CreateShareableWrap(externalizedSharedArrayBufferContents.second);
        sharedArrayBuffer->CreateDataProperty(context, key, shareableWrap);
        return sharedArrayBuffer;
    }
    else {
        return MaybeLocal<SharedArrayBuffer>();
    }
}

#endif

Deserializer* Deserializer::NewDeserializer(Isolate* isolate, std::shared_ptr<SerializedData> data) {
    return new Deserializer(isolate, data);
}

#endif
