// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "serialized-data.h"

#include <napa/module/binding/wraps.h>

namespace napa {
namespace transport {

    using namespace v8;

    class Serializer : public ValueSerializer::Delegate {
    public:
        explicit Serializer(Isolate* isolate) :
            _isolate(isolate),
            _serializer(isolate, this) {}

        Maybe<bool> WriteValue(Local<Value> value);

        std::shared_ptr<SerializedData> Release() { return _data; }

    protected:
        void ThrowDataCloneError(Local<String> message) override;

        Maybe<uint32_t> GetSharedArrayBufferId(
            Isolate* isolate,
            Local<SharedArrayBuffer> sharedArrayBuffer
        ) override;

        void* ReallocateBufferMemory(void* oldBuffer, size_t size, size_t* actualSize) override;

        void FreeBufferMemory(void* buffer) override;

    private:
        ExternalizedSharedArrayBufferContents MaybeExternalize(Local<SharedArrayBuffer> sharedArrayBuffer);

        Maybe<bool> FinalizeTransfer();

        Isolate* _isolate;
        ValueSerializer _serializer;
        std::shared_ptr<SerializedData> _data;
        std::vector<Global<SharedArrayBuffer>> _sharedArrayBuffers;

        Serializer(const Serializer&) = delete;
        Serializer& operator=(const Serializer&) = delete;
    };


    Maybe<bool> Serializer::WriteValue(Local<Value> value) {
        bool ok = false;
        _data.reset(new SerializedData);
        _serializer.WriteHeader();

        Local<Context> context = _isolate->GetCurrentContext();
        if (!_serializer.WriteValue(context, value).To(&ok)) {
            _data.reset();
            return Nothing<bool>();
        }

        if (!FinalizeTransfer().To(&ok)) {
            return Nothing<bool>();
        }

        std::pair<uint8_t*, size_t> pair = _serializer.Release();
        _data->_data.reset(pair.first);
        _data->_size = pair.second;
        return Just(true);
    }

    void Serializer::ThrowDataCloneError(Local<String> message) {
        _isolate->ThrowException(Exception::Error(message));
    }

    Maybe<uint32_t> Serializer::GetSharedArrayBufferId(
        Isolate* isolate,
        Local<SharedArrayBuffer> sharedArrayBuffer
    ) {
        for (size_t index = 0; index < _sharedArrayBuffers.size(); ++index) {
            if (_sharedArrayBuffers[index] == sharedArrayBuffer) {
                return Just<uint32_t>(static_cast<uint32_t>(index));
            }
        }

        size_t index = _sharedArrayBuffers.size();
        _sharedArrayBuffers.emplace_back(_isolate, sharedArrayBuffer);
        return Just<uint32_t>(static_cast<uint32_t>(index));
    }

    void* Serializer::ReallocateBufferMemory(void* oldBuffer, size_t size, size_t* actualSize) {
        void* result = realloc(oldBuffer, size);
        *actualSize = result ? size : 0;
        return result;
    }

    void Serializer::FreeBufferMemory(void* buffer) { free(buffer); }

    ExternalizedSharedArrayBufferContents
    Serializer::MaybeExternalize(Local<SharedArrayBuffer> sharedArrayBuffer) {
        Local<Context> context = _isolate->GetCurrentContext();
        Local<String> key = v8_helpers::MakeV8String(_isolate, "_externalized");
        bool ok = false;
        if (sharedArrayBuffer->IsExternal()
            && sharedArrayBuffer->Has(context, key).To(&ok)) {
            Local<Value> value;
            // If the SharedArrayBuffer has been externalized, just get its Contents without externalizing it again,
            // and get its ExternalizedContents which has been stored in the '_externalized' property of the SharedArrayBuffer.
            if (sharedArrayBuffer->Get(context, key).ToLocal(&value)) {
                auto sharedPtrWrap = NAPA_OBJECTWRAP::Unwrap<napa::module::SharedPtrWrap>(Local<Object>::Cast(value));
                auto externalizedContents = sharedPtrWrap->Get<ExternalizedContents>();
                return std::make_pair(sharedArrayBuffer->GetContents(), externalizedContents);
            }
            return std::make_pair(sharedArrayBuffer->GetContents(), nullptr);
        } else {
            // If the SharedArrayBuffer has not been externalized,
            // externalize it and get its Contents and ExternalizedContents at first,
            // then store its ExternalizedContents in the '_externalized' property of the original SharedArrayBuffer.
            auto contents = sharedArrayBuffer->Externalize();
            auto externalizedContents = std::make_shared<ExternalizedContents>(contents);
            auto sharedPtrWrap = napa::module::binding::CreateShareableWrap(externalizedContents);
            sharedArrayBuffer->CreateDataProperty(context, key, sharedPtrWrap);
            return std::make_pair(contents, externalizedContents);
        }
    }

    Maybe<bool> Serializer::FinalizeTransfer() {
        for (const auto& globalSharedArrayBuffer : _sharedArrayBuffers) {
            Local<SharedArrayBuffer> sharedArrayBuffer =
                Local<SharedArrayBuffer>::New(_isolate, globalSharedArrayBuffer);
            // Externalize the SharedArrayBuffer if it hasn't been done before,
            // and store it's ExternalizedContents which will be used when deserializing it in deserializer.
            _data->_externalizedSharedArrayBufferContents.push_back(MaybeExternalize(sharedArrayBuffer));
        }

        return Just(true);
    }
}
}
