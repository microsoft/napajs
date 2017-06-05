#include "providers.h"

#include "console-logging-provider.h"
#include "nop-logging-provider.h"
#include "nop-metric-provider.h"

#include "module/module-resolver.h"

#include <napa-log.h>

#include <boost/dll.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <string>


using namespace napa::providers;

// Forward declarations.
static LoggingProvider* LoadLoggingProvider(const std::string& providerName);
static MetricProvider* LoadMetricProvider(const std::string& providerName);

// Providers - Initilally assigned to defaults.
static LoggingProvider* _loggingProvider = LoadLoggingProvider("");
static MetricProvider* _metricProvider = LoadMetricProvider("");


bool napa::providers::Initialize(const napa::PlatformSettings& settings) {
    _loggingProvider = LoadLoggingProvider(settings.loggingProvider);
    _metricProvider = LoadMetricProvider(settings.metricProvider);

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
    return *_loggingProvider;
}

MetricProvider& napa::providers::GetMetricProvider() {
    return *_metricProvider;
}

template <typename ProviderType>
static ProviderType* LoadProvider(
    const std::string& providerName,
    const std::string& jsonProperyPath,
    const std::string& functionName) {

    napa::module::ModuleResolver moduleResolver;

    // Resolve the provider module information
    auto moduleInfo = moduleResolver.Resolve(providerName.c_str());
    NAPA_ASSERT(!moduleInfo.packageJsonPath.empty(), "missing package.json in provider '%s'", providerName.c_str());

    // Full path to the root of the provider module
    auto modulePath = boost::filesystem::path(moduleInfo.packageJsonPath).parent_path();

    // Extract relative path to provider dll from package.json
    boost::property_tree::ptree package;
    boost::property_tree::json_parser::read_json(moduleInfo.packageJsonPath, package);
    auto providerRelativePath = package.get_optional<std::string>(jsonProperyPath);
    NAPA_ASSERT(
        providerRelativePath.is_initialized(),
        "missing property '%s' in '%s'",
        jsonProperyPath.c_str(),
        moduleInfo.packageJsonPath.c_str());

    // Full path to provider dll
    auto providerPath = (modulePath / providerRelativePath.get()).normalize().make_preferred();

    // boost::dll unloads dll when a reference object is gone.
    // Keep a static instance for each provider type (each template type will have its own static variable).
    static auto createProviderFunc = boost::dll::import<ProviderType*()>(providerPath, functionName);

    return createProviderFunc();
}

static LoggingProvider* LoadLoggingProvider(const std::string& providerName) {
    if (providerName.empty() || providerName == "console") {
        static auto consoleLoggingProvider = std::make_unique<ConsoleLoggingProvider>();
        return consoleLoggingProvider.get();
    }

    if (providerName == "nop") {
        static auto nopLoggingProvider = std::make_unique<NopLoggingProvider>();
        return nopLoggingProvider.get();
    }

    return LoadProvider<LoggingProvider>(providerName, "providers.logging", "CreateLoggingProvider");
}

static MetricProvider* LoadMetricProvider(const std::string& providerName) {
    if (providerName.empty()) {
        static auto nopMetricProvider = std::make_unique<NopMetricProvider>();
        return nopMetricProvider.get();
    }

    return LoadProvider<MetricProvider>(providerName, "providers.metric", "CreateMetricProvider");;
}
