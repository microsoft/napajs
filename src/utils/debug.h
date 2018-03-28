// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "napa/log.h"

#if defined(DEBUG_NAPA)

#define NAPA_DEBUG(section, format, ...) LOG_DEBUG(section, format, ##__VA_ARGS__)

#else

// Do nothing without generating any "unreferenced variable" warnings.
#define NAPA_DEBUG(section, format, ...) ((void)0)

#endif
