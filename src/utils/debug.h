#pragma once

#include "napa-log.h"

#if defined(ENABLE_NAPA_DEBUG_LOG)
    #define NAPA_DEBUG(section, format, ...) LOG_DEBUG(section, format, ##__VA_ARGS__)
#else
    #define NAPA_DEBUG
#endif
