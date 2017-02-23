#pragma once

#include <napa/providers/logging.h>

#include <iostream>
#include <stdarg.h>

/// <summary> The maximum string length of a single log call. Anything over will be truncated. </summary>
const size_t LOG_MAX_SIZE = 512;

#define NAPA_ASSERT(condition, message) do {            \
    if (!(condition)) {                                 \
        std::cerr << "Assertion failed: `"              \
                  << #condition                         \
                  << "`, file "                         \
                  << __FILE__                           \
                  << ", line "                          \
                  << __LINE__                           \
                  << " : "                              \
                  << message                            \
                  << "."                                \
                  << std::endl;                         \
        std::terminate();                               \
    }                                                   \
} while (false)


inline void LogFormattedMessage(
    napa::providers::LoggingProvider& logger,
    const char* section,
    napa::providers::Verboseness level,
    const char* traceId,
    const char* format, ...) {

    char message[LOG_MAX_SIZE];
    va_list args;
    va_start(args, format);
    int size = vsnprintf(message, LOG_MAX_SIZE, format, args);
    va_end(args);

    NAPA_ASSERT(size >= 0, "Log formatting error, probably wrong format encoding");
    logger.LogMessage(section, level, traceId, message);
}


#define LOG(section, level, traceId, format, ...) do {                               \
    auto& logger = napa::providers::GetLoggingProvider();                            \
    if (logger.IsLogEnabled(section, level)) {                                       \
        LogFormattedMessage(logger, section, level, traceId, format, __VA_ARGS__);   \
    }                                                                                \
} while (false)


#define LOG_ERROR(section, format, ...) \
    LOG(section, napa::providers::Verboseness::Error, "", format, __VA_ARGS__);

#define LOG_ERROR_WITH_TRACEID(section, traceId,  format, ...) \
    LOG(section, napa::providers::Verboseness::Error, traceId, format, __VA_ARGS__);

#define LOG_WARNING(section, format, ...) \
    LOG(section, napa::providers::Verboseness::Warning, "", format, __VA_ARGS__);

#define LOG_WARNING_WITH_TRACEID(section, traceId, format, ...) \
    LOG(section, napa::providers::Verboseness::Warning, traceId, format, __VA_ARGS__);

#define LOG_INFO(section, format, ...) \
    LOG(section, napa::providers::Verboseness::Info, "", format, __VA_ARGS__);

#define LOG_INFO_WITH_TRACEID(section, traceId, format, ...) \
    LOG(section, napa::providers::Verboseness::Info, traceId, format, __VA_ARGS__);

#define LOG_DEBUG(section, format, ...) \
    LOG(section, napa::providers::Verboseness::Debug, "", format, __VA_ARGS__);

#define LOG_DEBUG_WITH_TRACEID(section, traceId, format, ...) \
    LOG(section, napa::providers::Verboseness::Debug, traceId, format, __VA_ARGS__);
