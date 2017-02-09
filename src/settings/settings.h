#pragma once

#include <algorithm>
#include <string>
#include <thread>
#include <vector>


namespace napa {

    /// <summary> The settings a user may set to customize napa. Initialized with defaults. </summary>
    struct Settings {

        /// <summary> The number of container cores. </summary>
        uint32_t cores = std::max(1u, std::thread::hardware_concurrency() / 2);

        /// <summary> The logging provider (cross-container). </summary>
        std::string loggingProvider = "console";

        /// <summary> The metric provider (cross-container). </summary>
        std::string metricProvider;

        /// <summary> V8 flags (cross-container). </summary>
        std::vector<std::string> v8Flags;

        /// <summary> A flag to specify whether v8 should be initialized (cross-container). </summary>
        bool initV8 = true;

        /// <summary> Isolate memory contstraint - The maximum old space size in megabytes. </summary>
        uint32_t maxOldSpaceSize = 0u;

        /// <summary> Isolate memory contstraint - The maximum semi space size in megabytes. </summary>
        uint32_t maxSemiSpaceSize = 0u;

        /// <summary> Isolate memory contstraint - The maximum executable size in megabytes. </summary>
        uint32_t maxExecutableSize = 0u;

        /// <summary> The maximum size that the isolate stack is allowed to grow in bytes. </summary>
        uint32_t maxStackSize = 500 * 1024;
    };

}