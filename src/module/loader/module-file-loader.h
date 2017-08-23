// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module/module-internal.h>

#include <functional>
#include <string>

namespace napa {
namespace module {

    using BuiltInModulesSetter = std::function<void(v8::Local<v8::Context> context)>;

    /// <summary> Interface to load a module from file. </summary>
    class ModuleFileLoader {
    public:
        virtual ~ModuleFileLoader() = default;

        /// <summary> It loads a module from file. </summary>
        /// <param name="path"> Module path called by require(). </param>
        /// <param name="arg"> Optional argument for loading module file. Passed through as arg1 from require. </param>
        /// <param name="module"> Loaded module if successful. </param>
        /// <returns> True if the module was loaded, false otherwise. </returns>
        virtual bool TryGet(const std::string& path, v8::Local<v8::Value> arg, v8::Local<v8::Object>& module) = 0;
    };
} // namespace module
} // namespace napa