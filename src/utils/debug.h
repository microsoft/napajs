// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "napa/log.h"

// use cmake-js -CDDEBUG_NAPA to define DEBUG_NAPA when compiling
#if defined(DEBUG_NAPA)
    #define NAPA_DEBUG(section, format, ...) LOG_DEBUG(section, format, ##__VA_ARGS__)
#else
    #define NAPA_DEBUG(section, format, ...) ((void)0)
#endif
