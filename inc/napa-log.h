#pragma once

#include <napa/providers/logging.h>

#define LOG(section, level, traceId, format, ...) do {                       \
    auto& logger = napa::providers::GetLoggingProvider();                    \
    if (logger.IsLogEnabled(section, level)) {                               \
        logger.Log(section, level, traceId, format, __VA_ARGS__);            \
    }                                                                        \
} while (0)


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
