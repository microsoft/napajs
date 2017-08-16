// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "core-module-loader.h"
#include "javascript-module-loader.h"
#include "module-cache.h"
#include "module-loader-helpers.h"

#include <platform/filesystem.h>

#include <sstream>

using namespace napa;
using namespace napa::module;

namespace {

const std::string CORE_MODULE_EXTENSION = ".js";
const std::string CORE_MODULE_DIRECTORY = "lib\\core";

} // End of anonymous namespace.

CoreModuleLoader::CoreModuleLoader(BuiltInModulesSetter builtInModulesSetter,
                                   ModuleCache& moduleCache,
                                   ModuleCache& bindingCache)
    : JavascriptModuleLoader(std::move(builtInModulesSetter), moduleCache), _bindingCache(bindingCache) {}

bool CoreModuleLoader::TryGet(const std::string& name, v8::Local<v8::Object>& module) {
    filesystem::Path basePath(module_loader_helpers::GetNapaRuntimeDirectory());
    auto fileName = name + CORE_MODULE_EXTENSION;

    // Check ./lib or ../lib directory only
    auto fullPath = basePath / CORE_MODULE_DIRECTORY / fileName;
    if (filesystem::IsRegularFile(fullPath)) {
        // Load javascript core module from a file at ./lib directory.
        return JavascriptModuleLoader::TryGet(fullPath.String(), module);
    }

    fullPath = (basePath.Parent() / CORE_MODULE_DIRECTORY / fileName).Normalize();
    if (filesystem::IsRegularFile(fullPath)) {
        // Load javascript core module from a file at ../lib directory.
        return JavascriptModuleLoader::TryGet(fullPath.String(), module);
    }

    // Return binary core module if exists.
    return _bindingCache.TryGet(name, module);
}