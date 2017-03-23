#include "core-module-loader.h"

using namespace napa;
using namespace napa::module;

CoreModuleLoader::CoreModuleLoader(ModuleCache& cache) : _cache(cache) {}

bool CoreModuleLoader::TryGet(const std::string& name, v8::Local<v8::Object>& module) {
    return _cache.TryGet(name, module);
}