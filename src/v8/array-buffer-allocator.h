// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>
#include <cstring>

namespace napa {
namespace v8_extensions {

    ///<summary> Allocator that V8 uses to allocate |ArrayBuffer|'s memory. </summary>
    class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
    public:

        /// <see> v8::ArrayBuffer::Allocator::Allocate </see>
        virtual void* Allocate(size_t length) override {
            void* data = AllocateUninitialized(length);
            return std::memset(data, 0, length);
        }

        /// <see> v8::ArrayBuffer::Allocator::AllocateUninitialized </see>
        virtual void* AllocateUninitialized(size_t length) override {
            return malloc(length);
        }

        /// <see> v8::ArrayBuffer::Allocator::Free </see>
        virtual void Free(void* data, size_t length) override {
            free(data);
        }
    };
}
}
