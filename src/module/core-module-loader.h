#pragma once

#include "module-cache.h"
#include "module-file-loader.h"

#include <string>

namespace napa {
namespace module {

    /// <summary> It loads a core module. </summary>
    class CoreModuleLoader : public ModuleFileLoader {
    public:

        /// <summary> Constructor. </summary>
        CoreModuleLoader(ModuleCache& cache);

        /// <summary> It loads a core module. </summary>
        /// <param name="name"> Core module name. </param>
        /// <param name="module"> Loaded core module if successful. </param>
        /// <returns> True if core module is loaded, false otherwise. </returns>
        bool TryGet(const std::string& name, v8::Local<v8::Object>& module) override;

    private:

        /// Module cache instance.
        ModuleCache& _cache;
    };

}   // End of namespace module.
}   // End of namespace napa.