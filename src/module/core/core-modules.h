#pragma once

#include "file-system.h"
#include "path.h"

#define INITIALIZE_CORE_MODULE(registerer, name, initializer) \
    registerer(name, [](auto arg) { return initializer(arg); });

#define INITIALIZE_CORE_MODULES(registerer) \
    INITIALIZE_CORE_MODULE(registerer, "fs", file_system::Init); \
    INITIALIZE_CORE_MODULE(registerer, "path", path::Init);
