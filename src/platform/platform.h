// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#ifndef SUPPORT_WINDOWS
#define SUPPORT_WINDOWS
#endif

#else

#ifndef SUPPORT_POSIX
#define SUPPORT_POSIX
#endif

#endif

#if defined(__linux) || defined(__linux__) || defined(linux)
#define OS_LINUX
#elif defined(__APPLE__)
#define OS_MAC
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define OS_WINDOWS
#elif defined(__FreeBSD__) || defined(__FreeBSD)
#define OS_FREEBSD
#else
// Unknown OS
#endif

namespace napa {
namespace platform {
    /// <summary> Platform. </summary>
#if defined(OS_LINUX)
    constexpr const char* PLATFORM = "linux";
#elif defined(OS_MAC)
    constexpr const char* PLATFORM = "darwin";
#elif defined(OS_WINDOWS)
    constexpr const char* PLATFORM = "win32";
#elif defined(OS_FREEBSD)
    constexpr const char* PLATFORM = "freebsd";
#else
    constexpr const char* PLATFORM = "unknown";
#endif
} // namespace platform
} // namespace napa
