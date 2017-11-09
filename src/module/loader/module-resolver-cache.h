// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "module-resolver.h"

#include <memory>

namespace napa {
namespace module {

    /// <summary>
    /// The Module resolver cache helps to reduce the overhead when ModuleResolver
    /// is trying to traversal the file system.
    /// </summary>
    class ModuleResolverCache {
    public:

        /// <summary> Constructor. </summary>
        ModuleResolverCache();

        /// <summary> Default destructor. </summary>
        ~ModuleResolverCache();

        /// <summary> Non-copyable. </summary>
        ModuleResolverCache(const ModuleResolverCache&) = delete;
        ModuleResolverCache& operator=(const ModuleResolverCache&) = delete;

        /// <summary> Movable. </summary>
        ModuleResolverCache(ModuleResolverCache&&) = default;
        ModuleResolverCache& operator=(ModuleResolverCache&&) = default;

        /// <summary> Lookup the module info. </summary>
        /// <param name="name"> Module name or path. </param>
        /// <param name="path"> Current context path. If nullptr, it'll be current path. </param>
        /// <returns> Module resolution information. </returns>
        ModuleInfo Lookup(const char* name, const char* path);

        /// <summary> Insert the specific module info into the cache. </summary>
        /// <param name="name"> Module name or path. </param>
        /// <param name="path"> Current context path. If nullptr, it'll be current path. </param>
        void Insert(const char* name, const char* path, const ModuleInfo& moduleInfo);

    private:
        class ModuleResolverCacheImpl;
        std::unique_ptr<ModuleResolverCacheImpl> _impl;
    };
    
}   // End of namespace module.
}   // End of namespace napa.
