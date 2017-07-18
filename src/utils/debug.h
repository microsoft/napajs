#pragma once

#include "napa-log.h"

#if defined(DEBUG_NAPA)
    #define NAPA_DEBUG(section, format, ...) LOG_DEBUG(section, format, ##__VA_ARGS__)
#else
    #define NAPA_DEBUG
#endif
