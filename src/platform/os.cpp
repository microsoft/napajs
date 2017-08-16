// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <platform/os.h>
#include <platform/platform.h>

#ifdef SUPPORT_POSIX
#include <sys/utsname.h>
#endif

#include <stdexcept>

namespace napa {
namespace platform {

    const char* GetOSType() {
#ifdef SUPPORT_POSIX
        static struct utsname info;
        if (uname(&info) < 0) {
            throw std::runtime_error("Error getting uname");
        }
        return info.sysname;
#else
        return "Windows_NT";
#endif
    }

#ifdef SUPPORT_POSIX
    const char* ENV_DELIMITER = ":";
#else
    const char* ENV_DELIMITER = ";";
#endif

#ifdef SUPPORT_POSIX
    const char* DIR_SEPARATOR = "/";
#else
    const char* DIR_SEPARATOR = "\\";
#endif
}
}
