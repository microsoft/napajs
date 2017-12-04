// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "serialized-data.h"

#include <napa/module/binding/wraps.h>

namespace napa {
namespace v8_extensions {

    using namespace v8;

    class Serializer : public ValueSerializer::Delegate {
    public:
        explicit Serializer(Isolate* isolate) :
            isolate_(isolate),
            serializer_(isolate, this) {}

        Maybe<bool> WriteValue(Local<Value> value) {
            bool ok;
            // DCHECK(!data_);
            data_.reset(new SerializedData);
            serializer_.WriteHeader();

            Local<Context> context = isolate_->GetCurrentContext();
            if (!serializer_.WriteValue(context, value).To(&ok)) {
                data_.reset();
                return Nothing<bool>();
            }

            if (!FinalizeTransfer().To(&ok)) {
                return Nothing<bool>();
            }

            std::pair<uint8_t*, size_t> pair = serializer_.Release();
            data_->data_.reset(pair.first);
            data_->size_ = pair.second;
            return Just(true);
        }

        std::shared_ptr<SerializedData> Release() { return std::move(data_); }

    protected:
        // Implements ValueSerializer::Delegate.
        void ThrowDataCloneError(Local<String> message) override {
            isolate_->ThrowException(Exception::Error(message));
        }

        Maybe<uint32_t> GetSharedArrayBufferId(
            Isolate* isolate,
            Local<SharedArrayBuffer> shared_array_buffer
        ) override {
            // DCHECK_NOT_NULL(data_);
            for (size_t index = 0; index < shared_array_buffers_.size(); ++index) {
                if (shared_array_buffers_[index] == shared_array_buffer) {
                    return Just<uint32_t>(static_cast<uint32_t>(index));
                }
            }

            size_t index = shared_array_buffers_.size();
            shared_array_buffers_.emplace_back(isolate_, shared_array_buffer);
            return Just<uint32_t>(static_cast<uint32_t>(index));
        }

        void* ReallocateBufferMemory(void* old_buffer, size_t size, size_t* actual_size) override {
            void* result = realloc(old_buffer, size);
            *actual_size = result ? size : 0;
            return result;
        }

        void FreeBufferMemory(void* buffer) override { free(buffer); }

    private:
        std::pair<SharedArrayBuffer::Contents, std::shared_ptr<ExternalizedContents>>
        MaybeExternalize(Local<SharedArrayBuffer> shared_array_buffer) {
            Local<Context> context = isolate_->GetCurrentContext();
            Local<String> key = v8_helpers::MakeV8String(isolate_, "v8ExternalizedContents");
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

        Maybe<bool> FinalizeTransfer() {
            for (const auto& global_shared_array_buffer : shared_array_buffers_) {
                Local<SharedArrayBuffer> shared_array_buffer =
                    Local<SharedArrayBuffer>::New(isolate_, global_shared_array_buffer);
                data_->shared_array_buffer_contents_.push_back(MaybeExternalize(shared_array_buffer));
            }
        
            return Just(true);
        }

        Isolate* isolate_;
        ValueSerializer serializer_;
        std::shared_ptr<SerializedData> data_;
        std::vector<Global<SharedArrayBuffer>> shared_array_buffers_;

        Serializer(const Serializer&) = delete;
        Serializer& operator=(const Serializer&) = delete;
    };
}
}
