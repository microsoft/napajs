// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "v8-extensions-macros.h"

#if V8_VERSION_CHECK_FOR_BUILT_IN_TYPE_TRANSPORTER

#include "serialized-data.h"

#include <stdlib.h>

using namespace napa::v8_extensions;
using namespace v8;

SerializedData::SerializedData() : _size(0) {}

const uint8_t* SerializedData::GetData() const { return _data.get(); }

size_t SerializedData::GetSize() const { return _size; }

const std::vector<ExternalizedSharedArrayBufferContents>&
SerializedData::GetExternalizedSharedArrayBufferContents() const {
    return _externalizedSharedArrayBufferContents;
}

void SerializedData::DataDeleter::operator()(uint8_t* p) const { free(p); }

#endif
