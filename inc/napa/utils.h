// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/assert.h>

#include <cstdio>
#include <stdarg.h>

namespace napa {
namespace utils {

    /// <summary> Format message with truncation. </summary>
    inline void FormatMessageWithTruncation(char* buffer, size_t bufferSize, const char* format, ...) {
        va_list args;
        va_start(args, format);
        int size = vsnprintf(buffer, bufferSize, format, args);
        va_end(args);

        NAPA_ASSERT(size >= 0, "Format message error, probably wrong format encoding");
    }
}
}