#pragma once

#include <napa/providers/logging.h>
#include <napa/providers/metric.h>

#include "settings/settings.h"


namespace napa {
namespace providers {

    /// <summary> Initializes and loads all providers based on the provided settings. </summary>
    bool Initialize(const napa::runtime::internal::Settings& settings);

    /// <summary> Clean up and destroy all loaded providers. </summary>
    void Shutdown();

    /// <summary> Returns the logging provider. </summary>
    LoggingProvider& GetLoggingProvider();

    /// <summary> Returns the metric provider. </summary>
    MetricProvider& GetMetricProvider();
}
}
