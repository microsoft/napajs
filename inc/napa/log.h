// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/assert.h>
#include <napa/providers/logging.h>

#include <stdarg.h>

/// <summary> The maximum string length of a single log call. Anything over will be truncated. </summary>
const size_t LOG_MAX_SIZE = 512;

inline void LogFormattedMessage(
    napa::providers::LoggingProvider& logger,
    const char* section,
    napa::providers::LoggingProvider::Verboseness level,
    const char* traceId,
    const char* file,
    int line,
    const char* format, ...) {

    char message[LOG_MAX_SIZE];
    va_list args;
    va_start(args, format);
    int size = vsnprintf(message, LOG_MAX_SIZE, format, args);
    va_end(args);

    NAPA_ASSERT(size >= 0, "Log formatting error, probably wrong format encoding");
    logger.LogMessage(section, level, traceId, file, line, message);
}

#ifndef NAPA_LOG_DISABLED

#define LOG(section, level, traceId, format, ...) do {                                                   \
    auto& logger = napa::providers::GetLoggingProvider();                                                \
    if (logger.IsLogEnabled(section, level)) {                                                           \
        LogFormattedMessage(logger, section, level, traceId, __FILE__, __LINE__, format, ##__VA_ARGS__); \
    }                                                                                                    \
} while (false)

#else

// Do nothing without generating any "unreferenced variable" warnings.
template <typename... Types> inline void LOG(Types&&...) {}

#endif

#define LOG_ERROR(section, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Error, "", format, ##__VA_ARGS__)

#define LOG_ERROR_WITH_TRACEID(section, traceId, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Error, traceId, format, ##__VA_ARGS__)

#define LOG_WARNING(section, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Warning, "", format, ##__VA_ARGS__)

#define LOG_WARNING_WITH_TRACEID(section, traceId, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Warning, traceId, format, ##__VA_ARGS__)

#define LOG_INFO(section, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Info, "", format, ##__VA_ARGS__)

#define LOG_INFO_WITH_TRACEID(section, traceId, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Info, traceId, format, ##__VA_ARGS__)

#define LOG_DEBUG(section, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Debug, "", format, ##__VA_ARGS__)

#define LOG_DEBUG_WITH_TRACEID(section, traceId, format, ...) \
    LOG(section, napa::providers::LoggingProvider::Verboseness::Debug, traceId, format, ##__VA_ARGS__)

// Macro NAPA_DEBUG is used to help debugging Napa source code.
#if defined(NAPA_DEBUG_ENABLED)

#define NAPA_DEBUG(section, format, ...) LOG_DEBUG(section, format, ##__VA_ARGS__)

#else

// Do nothing without generating any "unreferenced variable" warnings.
#define NAPA_DEBUG(section, format, ...) ((void)0)

#endif
