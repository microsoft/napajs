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

ModuleCache::ModuleCache() : _impl(std::make_unique<ModuleCache::ModuleCacheImpl>()) {}

ModuleCache::~ModuleCache() = default;

void ModuleCache::Upsert(const std::string& path, v8::Local<v8::Object> module) {
    if (module.IsEmpty()) {
        return;
    }

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto iter = _impl->moduleCache.find(path);
    if (iter != _impl->moduleCache.end()) {
        // If exists, reset it and override with new module.
        iter->second.Reset();
        iter->second = PersistentModule(isolate, module);
    } else {
        _impl->moduleCache.emplace(std::piecewise_construct,
                                   std::forward_as_tuple(path),
                                   std::forward_as_tuple(isolate, module));
    }
}

bool ModuleCache::TryGet(const std::string& path, v8::Local<v8::Object>& module) const {
    auto isolate = v8::Isolate::GetCurrent();

    auto iter = _impl->moduleCache.find(path);
    if (iter == _impl->moduleCache.end()) {
        return false;
    }

    module = v8::Local<v8::Object>::New(isolate, iter->second);
    return true;
}