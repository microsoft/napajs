// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "javascript-module-loader.h"

#include <string>

namespace napa {
namespace module {

    /// <summary> It loads a core module. </summary>
    class CoreModuleLoader : public JavascriptModuleLoader {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="builtInSetter"> Built-in modules registerer. </param>
        /// <param name="moduleCache"> Cache for all modules. </param>
        /// <param name="bindingCache"> Cache for binding core binary modules. </param>
        CoreModuleLoader(BuiltInModulesSetter builtInModulesSetter,
                         ModuleCache& moduleCache,
                         ModuleCache& bindingCache);

        /// <summary> It loads a core module. </summary>
        /// <param name="name"> Core module name. </param>
        /// <param name="arg"> Argument for loading the file. Passed through as arg1 from require. </param>
        /// <param name="module"> Loaded core module if successful. </param>
        /// <returns> True if core module is loaded, false otherwise. </returns>
        bool TryGet(const std::string& name, v8::Local<v8::Value> arg, v8::Local<v8::Object>& module) override;

    private:

        /// Cache instance for binding binary core modules.
        ModuleCache& _bindingCache;
    };

}   // End of namespace module.
}   // End of namespace napa.