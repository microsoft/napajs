#pragma once

#include "module-file-loader.h"
#include "platform/dll.h"

#include <memory>
#include <string>
#include <vector>

namespace napa {
namespace module {

    /// <summary> It loads a module from binary file. </summary>
    class BinaryModuleLoader : public ModuleFileLoader {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="builtsInSetter"> Built-in modules registerer. </param>
        BinaryModuleLoader(BuiltInModulesSetter builtInModulesSetter);

        /// <summary> It loads a module from binary file. </summary>
        /// <param name="path"> Module path called by require(). </param>
        /// <param name="module"> Loaded binary module if successful. </param>
        /// <returns> True if binary module is loaded, false otherwise. </returns>
        bool TryGet(const std::string& path, v8::Local<v8::Object>& module) override;

    private:

        /// Built-in modules registerer.
        BuiltInModulesSetter _builtInModulesSetter;

        /// <summary> It keeps binary modules loaded. </summary>
        std::vector<std::shared_ptr<dll::SharedLibrary>> _modules;
    };

}   // End of namespace module.
}   // End of namespace napa.