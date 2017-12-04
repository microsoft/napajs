// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "serialized-data.h"

#include <napa/module/binding/wraps.h>

namespace napa {
namespace v8_extensions {

    using namespace v8;

    class Deserializer : public ValueDeserializer::Delegate {
    public:
        Deserializer(Isolate* isolate, std::shared_ptr<SerializedData> data) :
            isolate_(isolate),
            deserializer_(isolate, data->data(), data->size(), this),
            data_(data) {
            deserializer_.SetSupportsLegacyWireFormat(true);
        }

        MaybeLocal<Value> ReadValue() {
            bool read_header;
            Local<Context> context = isolate_->GetCurrentContext();
            if (!deserializer_.ReadHeader(context).To(&read_header)) {
                return MaybeLocal<Value>();
            }

            uint32_t index = 0;
            Local<String> key = v8_helpers::MakeV8String(isolate_, "v8ExternalizedContent");
            for (const auto& contents : data_->shared_array_buffer_contents()) {
                Local<SharedArrayBuffer> shared_array_buffer = SharedArrayBuffer::New(
                    isolate_, contents.first.Data(), contents.first.ByteLength());
                auto sharedPtrWrap = napa::module::binding::CreateShareableWrap(contents.second);
                shared_array_buffer->CreateDataProperty(context, key, sharedPtrWrap);
                deserializer_.TransferSharedArrayBuffer(index++, shared_array_buffer);
            }
        
            return deserializer_.ReadValue(context);
        }
   
    private:
        Isolate* isolate_;
        ValueDeserializer deserializer_;
        std::shared_ptr<SerializedData> data_;

        Deserializer(const Deserializer&) = delete;
        Deserializer& operator=(const Deserializer&) = delete;
   };
}
}
