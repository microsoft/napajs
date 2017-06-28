#pragma once

#include "module-file-loader.h"

#include <string>

namespace napa {
namespace module {

    // forward declaration.
    class ModuleCache;

    /// <summary> It loads a module from javascript file. </summary>
    class JavascriptModuleLoader : public ModuleFileLoader {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="builtsInSetter"> Built-in modules registerer. </param>
        /// <param name="moduleCache"> Cache for all modules. </param>
        JavascriptModuleLoader(BuiltInModulesSetter builtInModulesSetter, ModuleCache& moduleCache);

        /// <summary> It loads a module from javascript file. </summary>
        /// <param name="path"> Module path called by require(). </param>
        /// <param name="module"> Loaded javascript module if successful. </param>
        /// <returns> True if the javascript module is loaded, false otherwise. </returns>
        bool TryGet(const std::string& path, v8::Local<v8::Object>& module) override;

    private:

        /// Built-in modules registerer.
        BuiltInModulesSetter _builtInModulesSetter;

        /// Module cache instance.
        ModuleCache& _moduleCache;
    };

}   // End of namespace module.
}   // End of namespace napa.