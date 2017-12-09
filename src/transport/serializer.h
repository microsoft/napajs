// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "serialized-data.h"

#include <napa/module/binding/wraps.h>

namespace napa {
namespace transport {

    using namespace v8;

    typedef std::pair<SharedArrayBuffer::Contents, std::shared_ptr<ExternalizedContents>> SharedArrayBufferExternalization;

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
            Local<SharedArrayBuffer> shared_array_buffer
        ) override;

        void* ReallocateBufferMemory(void* old_buffer, size_t size, size_t* actual_size) override;

        void FreeBufferMemory(void* buffer) override;

    private:
        SharedArrayBufferExternalization MaybeExternalize(Local<SharedArrayBuffer> shared_array_buffer);

        Maybe<bool> FinalizeTransfer();

        Isolate* _isolate;
        ValueSerializer _serializer;
        std::shared_ptr<SerializedData> _data;
        std::vector<Global<SharedArrayBuffer>> _shared_array_buffers;

        Serializer(const Serializer&) = delete;
        Serializer& operator=(const Serializer&) = delete;
    };


    Maybe<bool> Serializer::WriteValue(Local<Value> value) {
        bool ok;
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
        Local<SharedArrayBuffer> shared_array_buffer
    ) {
        for (size_t index = 0; index < _shared_array_buffers.size(); ++index) {
            if (_shared_array_buffers[index] == shared_array_buffer) {
                return Just<uint32_t>(static_cast<uint32_t>(index));
            }
        }

        size_t index = _shared_array_buffers.size();
        _shared_array_buffers.emplace_back(_isolate, shared_array_buffer);
        return Just<uint32_t>(static_cast<uint32_t>(index));
    }

    void* Serializer::ReallocateBufferMemory(void* old_buffer, size_t size, size_t* actual_size) {
        void* result = realloc(old_buffer, size);
        *actual_size = result ? size : 0;
        return result;
    }

    void Serializer::FreeBufferMemory(void* buffer) { free(buffer); }

    SharedArrayBufferExternalization
    Serializer::MaybeExternalize(Local<SharedArrayBuffer> shared_array_buffer) {
        Local<Context> context = _isolate->GetCurrentContext();
        Local<String> key = v8_helpers::MakeV8String(_isolate, "_externalized");
        bool ok;
        if (shared_array_buffer->IsExternal()
            && shared_array_buffer->Has(context, key).To(&ok)) {
            Local<Value> value;
            if (shared_array_buffer->Get(context, key).ToLocal(&value)) {
                auto sharedPtrWrap = NAPA_OBJECTWRAP::Unwrap<napa::module::SharedPtrWrap>(Local<Object>::Cast(value));
                auto externalizedContents = sharedPtrWrap->Get<ExternalizedContents>();
                return std::make_pair(shared_array_buffer->GetContents(), externalizedContents);
            }
            return std::make_pair(shared_array_buffer->GetContents(), nullptr);
        } else {
            auto contents = shared_array_buffer->Externalize();
            auto externalizedContents = std::make_shared<ExternalizedContents>(contents);
            auto sharedPtrWrap = napa::module::binding::CreateShareableWrap(externalizedContents);
            shared_array_buffer->CreateDataProperty(context, key, sharedPtrWrap);
            return std::make_pair(contents, externalizedContents);
        }
    }

    Maybe<bool> Serializer::FinalizeTransfer() {
        for (const auto& global_shared_array_buffer : _shared_array_buffers) {
            Local<SharedArrayBuffer> shared_array_buffer =
                Local<SharedArrayBuffer>::New(_isolate, global_shared_array_buffer);
            _data->_shared_array_buffer_contents.push_back(MaybeExternalize(shared_array_buffer));
        }

        return Just(true);
    }
}
}
