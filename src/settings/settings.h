#ifndef NAPA_SETTINGS_H
#define NAPA_SETTINGS_H

#include <algorithm>
#include <string>
#include <thread>
#include <vector>


namespace napa {
namespace runtime {
namespace internal {

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
    };

}
}
}

#endif //NAPA_SETTINGS_H
