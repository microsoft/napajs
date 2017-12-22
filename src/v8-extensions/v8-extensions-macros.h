// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <v8.h>

#define V8_VERSION_EQUALS_TO_OR_NEWER_THAN(MAJOR, MINOR) \
    (V8_MAJOR_VERSION == (MAJOR) && V8_MINOR_VERSION >= (MINOR) || V8_MAJOR_VERSION > (MAJOR))
    
#define V8_VERSION_CHECK_FOR_ARRAY_BUFFER_ALLOCATOR \
    V8_VERSION_EQUALS_TO_OR_NEWER_THAN(5, 5)

#define V8_VERSION_CHECK_FOR_BUILT_IN_TYPE_TRANSPORTER \
    V8_VERSION_EQUALS_TO_OR_NEWER_THAN(6, 2)
