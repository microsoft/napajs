// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "napa/napa-binding.h"

#include "node/console.h"
#include "node/file-system.h"
#include "node/os.h"
#include "node/path.h"
#include "node/process.h"
#include "node/tty-wrap.h"


#define INITIALIZE_CORE_MODULE(registerer, name, built_in, init)                             \
    do {                                                                                     \
        napa::module::ModuleInitializer initializer = [](auto exports, auto module) {        \
            return reinterpret_cast<napa::module::ModuleInitializer>(init)(exports, module); \
        };                                                                                   \
        registerer(name, built_in, initializer);                                             \
    } while (false)

// <remarks> napa-binding needs to be put at bottom since it may access other core modules. </remarks>
// macro arguments: registerer, core module name, is built-in, core module initialization function.
#define INITIALIZE_CORE_MODULES(registerer)                                \
    INITIALIZE_CORE_MODULE(registerer, "console", true, console::Init);    \
    INITIALIZE_CORE_MODULE(registerer, "fs", false, file_system::Init);    \
    INITIALIZE_CORE_MODULE(registerer, "os", false, os::Init);             \
    INITIALIZE_CORE_MODULE(registerer, "path", false, path::Init);         \
    INITIALIZE_CORE_MODULE(registerer, "process", true, process::Init);    \
    INITIALIZE_CORE_MODULE(registerer, "tty_wrap", false, tty_wrap::Init); \
    INITIALIZE_CORE_MODULE(registerer, "napa-binding", false, binding::Init);
