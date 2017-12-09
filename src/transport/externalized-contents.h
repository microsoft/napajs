// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>

namespace napa {
namespace transport {

    using namespace v8;

    class ExternalizedContents {
    public:
        explicit ExternalizedContents(const SharedArrayBuffer::Contents& contents) :
            _data(contents.Data()),
            _size(contents.ByteLength()) {}

        ExternalizedContents(ExternalizedContents&& other) :
            _data(other._data),
            _size(other._size) {
            other._data = nullptr;
            other._size = 0;
        }

        ExternalizedContents& operator=(ExternalizedContents&& other) {
            if (this != &other) {
                _data = other._data;
                _size = other._size;
                other._data = nullptr;
                other._size = 0;
            }
            return *this;
        }

        ~ExternalizedContents();

    private:
        void* _data;
        size_t _size;

        ExternalizedContents(const ExternalizedContents&) = delete;
        ExternalizedContents& operator=(const ExternalizedContents&) = delete;
    };

    ExternalizedContents::~ExternalizedContents() {
        // TODO #146: Get array_buffer_allocator to free ExternalizedContents.
        free(_data);
    }
}
}
