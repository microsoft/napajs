#pragma once

#include <algorithm>
#include <string>
#include <thread>
#include <vector>


namespace napa {

    /// <summary> Zone specific settings. </summary>
    struct ZoneSettings {

        /// <summary> The zone id. </summary>
        std::string id;

        /// <summary> The number of zone workers. </summary>
        uint32_t workers = std::max(1u, std::thread::hardware_concurrency() / 2);

        /// <summary> A file that will be loaded on all zone workers in global scope. </summary>
        std::string bootstrapFile;

        /// <summary> Isolate memory constraint - The maximum old space size in megabytes. </summary>
        uint32_t maxOldSpaceSize = 0u;

        /// <summary> Isolate memory constraint - The maximum semi space size in megabytes. </summary>
        uint32_t maxSemiSpaceSize = 0u;

        /// <summary> Isolate memory constraint - The maximum executable size in megabytes. </summary>
        uint32_t maxExecutableSize = 0u;

        /// <summary> The maximum size that the isolate stack is allowed to grow in bytes. </summary>
        uint32_t maxStackSize = 500 * 1024;
    };

    /// <summary> Platform settings. These are cross-zone settings as well as default settings for zones. </summary>
    struct PlatformSettings : ZoneSettings {

        /// <summary> The logging provider. </summary>
        std::string loggingProvider = "console";

        /// <summary> The metric provider. </summary>
        std::string metricProvider;

        /// <summary> V8 flags. </summary>
        std::vector<std::string> v8Flags;

        /// <summary> A flag to specify whether v8 should be initialized. </summary>
        bool initV8 = true;
    };
}