// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace napa {
namespace settings {

    /// <summary> Platform settings - setting that affect all zones. </summary>
    struct PlatformSettings {

        /// <summary> The logging provider. </summary>
        std::string loggingProvider = "console";

        /// <summary> The metric provider. </summary>
        std::string metricProvider;
    };

    /// <summary> Zone specific settings. </summary>
    struct ZoneSettings {

        /// <summary> Recycle mode determines a Zone's behavior when it is collected by GC. </summary>
        enum class RecycleMode {
            Auto,
            Manual
        };

        /// <summary> The zone id. </summary>
        std::string id;

        /// <summary> The number of zone workers. </summary>
        uint32_t workers = 2;

        /// <summary> Set default recycle mode to AUTO. </summary>
        RecycleMode recycle = RecycleMode::Auto;

        /// <summary> Isolate memory constraint - The maximum old space size in megabytes. </summary>
        uint32_t maxOldSpaceSize = 0u;

        /// <summary> Isolate memory constraint - The maximum semi space size in megabytes. </summary>
        uint32_t maxSemiSpaceSize = 0u;

        /// <summary> Isolate memory constraint - The maximum executable size in megabytes. </summary>
        uint32_t maxExecutableSize = 0u;

        /// <summary> The maximum size that the isolate stack is allowed to grow in bytes. </summary>
        uint32_t maxStackSize = 500 * 1024;
    };
}
}