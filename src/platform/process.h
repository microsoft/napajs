// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <cstdint>
#include <string>

namespace napa {
namespace platform {

    /// <summary> Get the number of process arguments. </summary>
    int GetArgc();

    /// <summary> Get the process arguments. </summary>
    char** GetArgv();

    /// <summary> Set environment variable. </summary>
    bool SetEnv(const char* name, const char* value);

    /// <summary> Get environment variable. </summary>
    std::string GetEnv(const char* name);

    /// <summary> Set file permission mask at the current process. </summary>
    int32_t Umask(int32_t mode);

    /// <summary> Return pid. </summary>
    int32_t Getpid();

    /// <summary> Return tid. </summary>
    int32_t Gettid();

    /// <summary> Return nonzero value if a descriptor is associated with a character device. </summary>
    /// <param name="fd"> File descriptor. </param>
    int32_t Isatty(int32_t fd);

}
}