// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "externalized-contents.h"

namespace napa {
namespace v8_extensions {

    using namespace v8;

    typedef std::pair<SharedArrayBuffer::Contents, std::shared_ptr<ExternalizedContents>> ExternalizedSharedArrayBufferContents;

    /// <summary>
    /// SerializedData holds the serialized data of a JavaScript object, and it is required during its deserialization.
    /// If the JavaScript object has properties or elements of SharedArrayBuffer or types based on SharedArrayBuffer, 
    /// like DataView and TypedArray, their ExternalizedContents will be stored in _externalizedSharedArrayBufferContents.
    /// </summary>
    class SerializedData {
    public:
        SerializedData();

        const uint8_t* GetData() const;

        size_t GetSize() const;

        const std::vector<ExternalizedSharedArrayBufferContents>& GetExternalizedSharedArrayBufferContents() const;

    private:
        struct DataDeleter {
            void operator()(uint8_t* p) const;
        };

        std::unique_ptr<uint8_t, DataDeleter> _data;
        size_t _size;
        std::vector<ExternalizedSharedArrayBufferContents> _externalizedSharedArrayBufferContents;

    private:
        friend class Serializer;

        SerializedData(const SerializedData&) = delete;
        SerializedData& operator=(const SerializedData&) = delete;
    };
}
}
