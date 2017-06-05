#pragma once

#include <iostream>
#include <exception> 

#include <stdarg.h>

/// <summary> The maximum message length of a single assert call. Anything over will be truncated. </summary>
static constexpr size_t MAX_ASSERT_MESSAGE_SIZE = 512;

inline void OutputAssertMessage(const char* condition, const char* file, int line, const char* format, ...) {
    char message[MAX_ASSERT_MESSAGE_SIZE];

    va_list args;
    va_start(args, format);
    int size = vsnprintf(message, MAX_ASSERT_MESSAGE_SIZE, format, args);
    va_end(args);

    if (size >= 0) {
        std::cerr << "Assertion failed: `" << condition << "`, file " << file << ", line " << line
            << " : " << message << "." << std::endl;
    }
}

#define NAPA_ASSERT(condition, format, ...) do {                                    \
    if (!(condition)) {                                                             \
        OutputAssertMessage(#condition, __FILE__, __LINE__, format, __VA_ARGS__);   \
        std::terminate();                                                           \
    }                                                                               \
} while (false)

#define NAPA_FAIL(message) NAPA_ASSERT(false, message)