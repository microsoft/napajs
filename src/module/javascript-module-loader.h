#pragma once

#include "module-cache.h"
#include "module-file-loader.h"

#include <string>

namespace napa {
namespace module {

    /// <summary> It loads a module from javascript file. </summary>
    class JavascriptModuleLoader : public ModuleFileLoader {
    public:

        /// <summary> Constructor. </summary>
        JavascriptModuleLoader(BuiltInsSetter builtInsSetter, ModuleCache& cache);

        /// <summary> It loads a module from javascript file. </summary>
        /// <param name="path"> Module path called by require(). </param>
        /// <param name="module"> Loaded javascript module if successful. </param>
        /// <returns> True if the javascript module is loaded, false otherwise. </returns>
        bool TryGet(const std::string& path, v8::Local<v8::Object>& module) override;

    private:

        /// Built-in modules registerer.
        BuiltInsSetter _builtInsSetter;

        /// Module cache instance.
        ModuleCache& _cache;
    };

}   // End of namespace module.
}   // End of namespace napa.