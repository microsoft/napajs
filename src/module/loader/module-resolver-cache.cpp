// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "module-resolver-cache.h"

#include <mutex>
#include <unordered_map>

using namespace napa;
using namespace napa::module;

struct ModuleResolverCacheKey {
    std::string _name;
    std::string _path;

    bool operator==(const ModuleResolverCacheKey& other) const {
        return _name == other._name && _path == other._path;
    }
};

namespace std {
    template<>
    struct hash<ModuleResolverCacheKey> {
        std::size_t operator()(ModuleResolverCacheKey key) const {
            return std::hash<std::string>{}(key._name) ^ std::hash<std::string>{}(key._path);
        }
    };
}

class ModuleResolverCache::ModuleResolverCacheImpl {
public:
    ModuleInfo Lookup(const char* name, const char* path);
    void Insert(const char* name, const char* path, const ModuleInfo& moduleInfo);
private:
    std::mutex _lock;
    std::unordered_map<ModuleResolverCacheKey, ModuleInfo> _resolvedModules;
};

ModuleResolverCache::ModuleResolverCache() : _impl(std::make_unique<ModuleResolverCache::ModuleResolverCacheImpl>()) {}

ModuleResolverCache::~ModuleResolverCache() = default;

ModuleInfo ModuleResolverCache::Lookup(const char* name, const char* path) {
    return _impl->Lookup(name, path);
}

void ModuleResolverCache::Insert(const char* name, const char* path, const ModuleInfo& moduleInfo) {
    _impl->Insert(name, path, moduleInfo);
}

ModuleInfo ModuleResolverCache::ModuleResolverCacheImpl::Lookup(const char* name, const char* path) {
    ModuleResolverCacheKey key{name, path};

    std::lock_guard<std::mutex> lock(_lock);

    auto result = _resolvedModules.find(key);
    if (result != _resolvedModules.end()) {
        return result->second;
    }

    return ModuleInfo{ModuleType::NONE, name, path};
}

void ModuleResolverCache::ModuleResolverCacheImpl::Insert(const char* name, const char* path, const ModuleInfo& moduleInfo) {
    ModuleResolverCacheKey key{name, path};
    
    std::lock_guard<std::mutex> lock(_lock);

    _resolvedModules.emplace(std::make_pair(key, moduleInfo));
}
