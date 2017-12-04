// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "externalized-contents.h"

namespace napa {
namespace v8_extensions {

    using namespace v8;

    class SerializedData {
    public:
        SerializedData() : size_(0) {}

        uint8_t* data() { return data_.get(); }

        size_t size() { return size_; }

        const std::vector<std::pair<SharedArrayBuffer::Contents, std::shared_ptr<ExternalizedContents>>>&
        shared_array_buffer_contents() {
            return shared_array_buffer_contents_;
        }

    private:
        struct DataDeleter {
            void operator()(uint8_t* p) const { free(p); }
        };

        std::unique_ptr<uint8_t, DataDeleter> data_;
        size_t size_;
        std::vector<
            std::pair<SharedArrayBuffer::Contents, std::shared_ptr<ExternalizedContents>>
        > shared_array_buffer_contents_;

    private:
        friend class Serializer;

        SerializedData(const SerializedData&) = delete;
        SerializedData& operator=(const SerializedData&) = delete;
    };
}
}
