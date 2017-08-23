// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "module-cache.h"

#include <unordered_map>

using namespace napa;
using namespace napa::module;

using PersistentModule = v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object>>;
using PersistentModuleCache = std::unordered_map<std::string, PersistentModule>;

struct ModuleCache::ModuleCacheImpl {
    /// <summary> Module cache to avoid module loading overhead. </summary>
    PersistentModuleCache moduleCache;
};

ModuleCache::ModuleCache() :
    _impl(std::make_unique<ModuleCache::ModuleCacheImpl>()) {}

ModuleCache::~ModuleCache() = default;

std::string NormalizeCacheKey(const std::string& path) {
#ifdef _WIN32
    // Remove UNC prefix if present.
    if (path.substr(0, 4) == "\\\\?\\") {
        return path.substr(4);
    }
#endif
    return path;
}

void ModuleCache::Upsert(const std::string& path, v8::Local<v8::Object> module) {
    if (module.IsEmpty()) {
        return;
    }

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto key = NormalizeCacheKey(path);
    auto iter = _impl->moduleCache.find(key);
    if (iter != _impl->moduleCache.end()) {
        // If exists, reset it and override with new module.
        iter->second.Reset();
        iter->second = PersistentModule(isolate, module);
    } else {
        _impl->moduleCache.emplace(std::piecewise_construct,
            std::forward_as_tuple(key),
            std::forward_as_tuple(isolate, module));
    }
}

bool ModuleCache::TryGet(const std::string& path, v8::Local<v8::Object>& module) const {
    auto isolate = v8::Isolate::GetCurrent();

    auto key = NormalizeCacheKey(path);
    auto iter = _impl->moduleCache.find(key);
    if (iter == _impl->moduleCache.end()) {
        return false;
    }

    module = v8::Local<v8::Object>::New(isolate, iter->second);
    return true;
}