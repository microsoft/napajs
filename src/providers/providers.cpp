#include "providers.h"

#include "console-logging-provider.h"
#include "nop-logging-provider.h"
#include "nop-metric-provider.h"

#include <napa-log.h>

#include <boost/dll.hpp>

#include <iostream>
#include <string>


using namespace napa::providers;

// Forward declarations.
static LoggingProvider* LoadLoggingProvider(const std::string& providerName);
static MetricProvider* LoadMetricProvider(const std::string& providerName);

// Providers.
static LoggingProvider* _loggingProvider = nullptr;
static MetricProvider* _metricProvider = nullptr;


bool napa::providers::Initialize(const napa::PlatformSettings& settings) {
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
    NAPA_ASSERT(_loggingProvider, "logging provider not set");
    return *_loggingProvider;
}

MetricProvider& napa::providers::GetMetricProvider() {
    NAPA_ASSERT(_metricProvider, "metric provider not set");
    return *_metricProvider;
}

template <typename ProviderType>
static ProviderType* LoadProvider(const std::string& providerName, const std::string& functionName) {
    // TODO @asib: resolve path to shared library given the provider name, need to use module loader APIs.
    auto createProviderFunc = boost::dll::import<ProviderType*()>(providerName, functionName);
    return createProviderFunc();
}

static LoggingProvider* LoadLoggingProvider(const std::string& providerName) {
    if (providerName.empty() || providerName == "nop") {
        return new NopLoggingProvider();
    }

    if (providerName == "console") {
        return new ConsoleLoggingProvider();
    }

    return LoadProvider<LoggingProvider>(providerName, "CreateLoggingProvider");
}

static MetricProvider* LoadMetricProvider(const std::string& providerName) {
    if (providerName.empty()) {
        return new NopMetricProvider();
    }

    return LoadProvider<MetricProvider>(providerName, "CreateMetricProvider");;
}
