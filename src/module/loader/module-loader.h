// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <memory>

namespace napa {
namespace module {

    /// <summary>
    /// It follows node.js's module resolution algorithm, https://nodejs.org/api/modules.html#modules_all_together except,
    /// - Napa module is created in thread-safe way.
    /// - Napa native module uses '.napa' extension instead of '.node'.
    /// </summary>
    class ModuleLoader {
    public:
        /// <summary> It creates a module loader. One thread can have only one module loader. </summary>
        static void CreateModuleLoader();

/// <summary>
/// A helper macro to create a module loader instance at current thread.
/// This must be called before calling 'require'.
/// </summary>
#define CREATE_MODULE_LOADER napa::module::ModuleLoader::CreateModuleLoader

        /// <summary> Non-copyable and Non-movable. </summary>
        ModuleLoader(const ModuleLoader&) = delete;
        ModuleLoader& operator=(const ModuleLoader&) = delete;
        ModuleLoader(ModuleLoader&&) = delete;
        ModuleLoader& operator=(ModuleLoader&&) = delete;

    private:
        /// <summary> Constructor. </summary>
        ModuleLoader();

        /// <summary> Default destructor. </summary>
        ~ModuleLoader();

        class ModuleLoaderImpl;
        std::unique_ptr<ModuleLoaderImpl> _impl;
    };

} // namespace module
} // namespace napa