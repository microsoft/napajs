#pragma once

#include "console.h"
#include "process.h"

#define INITIALIZE_BUILT_IN_MODULES(exports) \
    console::Init(exports); \
    process::Init(exports);
