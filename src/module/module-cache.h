#pragma once

#include <napa/module/module-internal.h>

#include <memory>

namespace napa {
namespace module {

    /// <summary> Module cache to avoid module loading overhead. </summary>
    class ModuleCache {
    public:

        /// <summary> Constructor. </summary>
        ModuleCache();

        /// <summary> Default destructor. </summary>
        ~ModuleCache();

        /// <summary> Non-copyable. </summary>
        ModuleCache(const ModuleCache&) = delete;
        ModuleCache& operator=(const ModuleCache&) = delete;

        /// <summary> Movable. </summary>
        ModuleCache(ModuleCache&&) = default;
        ModuleCache& operator=(ModuleCache&&) = default;

        /// <summary> It inserts or updates a module into cache using path as a key. </summary>
        /// <param name="path"> The module name path. </param>
        /// <param name="module"> V8 representative javascript object to be cached. </param>
        void Upsert(const std::string& path, v8::Local<v8::Object> module);

        /// <summary> A helper to load a module from cache. </summary>
        /// <param name="path"> Full path of javascript or napa module. </param>
        /// <param name="module"> Cached module if successful. </param>
        /// <returns> True if it finds successfully. False, otherwise. </returns>
        bool TryGet(const std::string& path, v8::Local<v8::Object>& module) const;

    private:

        /// <summary> Implementation of module cache. </summary>
        struct ModuleCacheImpl;
        std::unique_ptr<ModuleCacheImpl> _impl;
    };

}   // End of namespace module.
}   // End of namespace napa.