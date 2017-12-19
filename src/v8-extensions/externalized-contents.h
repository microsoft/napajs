// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>

namespace napa {
namespace v8_extensions {

    /// <summary> 
    /// 1. ExternalizedContents holds the externalized memory of a SharedArrayBuffer once it is serialized.
    /// 2. Only 1 instance of ExternalizedContents would be generated for each SharedArrayBuffer.
    ///    If a SharedArrayBuffer had been externalized, it will reuse the ExternalizedContents instance
    ///    created before in napa::v8_extensions::Utils::SerializeValue().
    /// </summary>
    class ExternalizedContents {
    public:
        explicit ExternalizedContents(const v8::SharedArrayBuffer::Contents& contents);

        ExternalizedContents(ExternalizedContents&& other);

        ExternalizedContents& operator=(ExternalizedContents&& other);

        ~ExternalizedContents();

    private:
        void* _data;
        size_t _size;

        ExternalizedContents(const ExternalizedContents&) = delete;
        ExternalizedContents& operator=(const ExternalizedContents&) = delete;
    };
}
}
