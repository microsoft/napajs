#include "providers.h"

#include "console-logging-provider.h"
#include "nop-logging-provider.h"
#include "nop-metric-provider.h"

#include <assert.h>
#include <iostream>
#include <string>


using namespace napa::runtime::internal;
using namespace napa::providers;

// Forward declarations.
static LoggingProvider* LoadLoggingProvider(const std::string& providerName);
static MetricProvider* LoadMetricProvider(const std::string& providerName);

// Providers.
static LoggingProvider* _loggingProvider;
static MetricProvider* _metricProvider;


bool napa::providers::Initialize(const Settings& settings) {
    try {
        _loggingProvider = LoadLoggingProvider(settings.loggingProvider);
        _metricProvider = LoadMetricProvider(settings.metricProvider);
    } catch (std::exception& ex) {
        std::cerr << "Error occurred while loading providers: " << ex.what() << "\n";
        return false;
    }

    return true;
}

void napa::providers::Shutdown() {
    if (_loggingProvider != nullptr) {
        _loggingProvider->Destroy();
    }
    
    if (_metricProvider != nullptr) {
        _metricProvider->Destroy();
    }
}

LoggingProvider& napa::providers::GetLoggingProvider() {
    assert(_loggingProvider);
    return *_loggingProvider;
}

MetricProvider& napa::providers::GetMetricProvider() {
    assert(_metricProvider);
    return *_metricProvider;
}

static LoggingProvider* LoadLoggingProvider(const std::string& providerName) {
    if (providerName.empty()) {
        return new NopLoggingProvider();
    }

    if (providerName == "console") {
        return new ConsoleLoggingProvider();
    }

    // TODO @asib: add cross platform loading implementation (BOOST.Dll?)

    return nullptr;
}

static MetricProvider* LoadMetricProvider(const std::string& providerName) {
    if (providerName.empty()) {
        return new NopMetricProvider();
    }
    // TODO @asib: add cross platform loading implementation (BOOST.Dll?)

    return nullptr;
}
