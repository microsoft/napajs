#include "core-module-loader.h"
#include "javascript-module-loader.h"
#include "module-cache.h"
#include "module-loader-helpers.h"

#include <boost/filesystem.hpp>

#include <sstream>

using namespace napa;
using namespace napa::module;

namespace {

    const std::string CORE_MODULE_EXTENSION = ".js";
    const std::string CORE_MODULE_DIRECTORY = "lib\\core";

}   // End of anonymous namespace.

CoreModuleLoader::CoreModuleLoader(BuiltInModulesSetter builtInModulesSetter,
                                   ModuleCache& moduleCache,
                                   ModuleCache& bindingCache)
    : JavascriptModuleLoader(std::move(builtInModulesSetter), moduleCache), _bindingCache(bindingCache) {}
                                
bool CoreModuleLoader::TryGet(const std::string& name, v8::Local<v8::Object>& module) {
    boost::filesystem::path basePath(module_loader_helpers::GetModuleRootDirectory());
    auto fileName = name + CORE_MODULE_EXTENSION;

    // Check ./lib or ../lib directory only
    auto fullPath = basePath / CORE_MODULE_DIRECTORY / fileName;
    if (boost::filesystem::is_regular_file(fullPath)) {
        // Load javascript core module from a file at ./lib directory.
        return JavascriptModuleLoader::TryGet(fullPath.string(), module);
    }

    fullPath = basePath.parent_path() / CORE_MODULE_DIRECTORY / fileName;
    if (boost::filesystem::is_regular_file(fullPath)) {
        // Load javascript core module from a file at ../lib directory.
        return JavascriptModuleLoader::TryGet(fullPath.string(), module);
    }

    // Return binary core module if exists.
    return _bindingCache.TryGet(name, module);
}