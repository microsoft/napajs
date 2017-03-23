#pragma once

#include "console.h"
#include "file-system.h"
#include "path.h"
#include "process.h"

#define INITIALIZE_CORE_MODULE(registerer, name, built_in, init)                        \
    do {                                                                                \
        napa::module::ModuleInitializer initializer = [](auto exports, auto module) {   \
            return init(exports);                                                       \
        };                                                                              \
        registerer(name, built_in, initializer);                                        \
    } while (false)

// registerer, core module name, is built-in, core module initialization function.
#define INITIALIZE_CORE_MODULES(registerer)                                             \
    INITIALIZE_CORE_MODULE(registerer, "console", true, console::Init);                 \
    INITIALIZE_CORE_MODULE(registerer, "fs", false, file_system::Init);                 \
    INITIALIZE_CORE_MODULE(registerer, "path", false, path::Init);                      \
    INITIALIZE_CORE_MODULE(registerer, "process", true, process::Init);
