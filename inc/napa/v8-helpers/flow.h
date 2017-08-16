// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/utils.h>
#include <v8.h>
#include <sstream>

#define THROW_IF_FAIL(isolate, expression, result, function, line, format, ...)                                        \
    \
if(!(expression)) {                                                                                                    \
        constexpr int MAX_ERROR_MESSAGE_SIZE = 512;                                                                    \
        char message[MAX_ERROR_MESSAGE_SIZE];                                                                          \
        napa::utils::FormatMessageWithTruncation(message, MAX_ERROR_MESSAGE_SIZE, format, ##__VA_ARGS__);              \
        std::stringstream temp;                                                                                        \
        temp << function << ":" << line << " -- " << message;                                                          \
        isolate->ThrowException(v8::Exception::TypeError(napa::v8_helpers::MakeV8String(isolate, temp.str())));        \
        return result;                                                                                                 \
    \
}

#define CHECK_ARG(isolate, expression, format, ...)                                                                    \
    THROW_IF_FAIL(isolate, expression, /* empty */, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define CHECK_ARG_WITH_RETURN(isolate, expression, result, format, ...)                                                \
    THROW_IF_FAIL(isolate, expression, result, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)

#define JS_ASSERT(isolate, expression, format, ...) CHECK_ARG(isolate, expression, format, ##__VA_ARGS__)

#define JS_ENSURE(isolate, expression, format, ...) CHECK_ARG(isolate, expression, format, ##__VA_ARGS__)

#define JS_FAIL(isolate, format, ...) CHECK_ARG(isolate, false, format, ##__VA_ARGS__)

#define JS_ENSURE_WITH_RETURN(isolate, expression, result, format, ...)                                                \
    CHECK_ARG_WITH_RETURN(isolate, expression, result, format, ##__VA_ARGS__)

#define SHORT_CIRCUIT_ON_PENDING_EXCEPTION(maybe, result)                                                              \
    if (maybe.IsEmpty()) {                                                                                             \
        return result;                                                                                                 \
    }

#define RETURN_ON_PENDING_EXCEPTION(maybe) SHORT_CIRCUIT_ON_PENDING_EXCEPTION(maybe, /* empty */)

#define RETURN_VALUE_ON_PENDING_EXCEPTION(maybe, result) SHORT_CIRCUIT_ON_PENDING_EXCEPTION(maybe, result)
