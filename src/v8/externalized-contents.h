// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>

namespace napa {
namespace v8_extensions {

    using namespace v8;

    class ExternalizedContents {
    public:
        explicit ExternalizedContents(const ArrayBuffer::Contents& contents) :
            data_(contents.Data()),
            size_(contents.ByteLength()) {}
        explicit ExternalizedContents(const SharedArrayBuffer::Contents& contents) :
            data_(contents.Data()),
            size_(contents.ByteLength()) {}
        ExternalizedContents(ExternalizedContents&& other) :
            data_(other.data_),
            size_(other.size_) {
          other.data_ = nullptr;
          other.size_ = 0;
        }
        ExternalizedContents& operator=(ExternalizedContents&& other) {
            if (this != &other) {
                data_ = other.data_;
                size_ = other.size_;
                other.data_ = nullptr;
                other.size_ = 0;
            }
            return *this;
        }
        ~ExternalizedContents();

    private:
        void* data_;
        size_t size_;

        /// <summary> No copy allowed. </summary>
        ExternalizedContents(const ExternalizedContents&) = delete;
        ExternalizedContents& operator=(const ExternalizedContents&) = delete;
    };
    
    ExternalizedContents::~ExternalizedContents() {
        /// figure out the correct allocator.
        /// array_buffer_allocator->Free(data_, size_);
    }
}
}
