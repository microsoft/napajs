// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "externalized-contents.h"

namespace napa {
namespace transport {

    using namespace v8;

    typedef std::pair<SharedArrayBuffer::Contents, std::shared_ptr<ExternalizedContents>> ExternalizedSharedArrayBufferContents;

    class SerializedData {
    public:
        SerializedData() : _size(0) {}

        uint8_t* GetData() { return _data.get(); }

        size_t GetSize() { return _size; }

        const std::vector<ExternalizedSharedArrayBufferContents>&
        GetExternalizedSharedArrayBufferContents() {
            return _externalizedSharedArrayBufferContents;
        }

    private:
        struct DataDeleter {
            void operator()(uint8_t* p) const { free(p); }
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
