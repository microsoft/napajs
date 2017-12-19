// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "v8-extensions-macros.h"

#if V8_VERSION_CHECK_FOR_BUILT_IN_TYPE_TRANSPORTER

#include "externalized-contents.h"

#include <stdlib.h>

using namespace napa::v8_extensions;
using namespace v8;

ExternalizedContents::ExternalizedContents(const SharedArrayBuffer::Contents& contents) :
    _data(contents.Data()),
    _size(contents.ByteLength()) {}

ExternalizedContents::ExternalizedContents(ExternalizedContents&& other) :
    _data(other._data),
    _size(other._size) {
    other._data = nullptr;
    other._size = 0;
}

ExternalizedContents& ExternalizedContents::operator=(ExternalizedContents&& other) {
    if (this != &other) {
        _data = other._data;
        _size = other._size;
        other._data = nullptr;
        other._size = 0;
    }
    return *this;
}

ExternalizedContents::~ExternalizedContents() {
    // TODO #146: Get array_buffer_allocator to free ExternalizedContents.
    free(_data);
}

#endif
