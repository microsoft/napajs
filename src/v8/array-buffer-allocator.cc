// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "array-buffer-allocator.h"

#include <v8.h>
#include <cstring>

using namespace napa::v8_extensions;

void* ArrayBufferAllocator::Allocate(size_t length) {
    void* data = AllocateUninitialized(length);
    return std::memset(data, 0, length);
}

void* ArrayBufferAllocator::AllocateUninitialized(size_t length) {
    return malloc(length);
}

void ArrayBufferAllocator::Free(void* data, size_t length) {
    free(data);
}