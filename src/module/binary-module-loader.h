#pragma once

#include "module-cache.h"
#include "module-file-loader.h"

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace napa {
namespace module {

    /// <summary> It loads a module from binary file. </summary>
    class BinaryModuleLoader : public ModuleFileLoader {
    public:

        /// <summary> Constructor. </summary>
        BinaryModuleLoader(BuiltInsSetter builtInsSetter);

        /// <summary> It loads a module from binary file. </summary>
        /// <param name="path"> Module path called by require(). </param>
        /// <param name="module"> Loaded binary module if successful. </param>
        /// <returns> True if binary module is loaded, false otherwise. </returns>
        bool TryGet(const std::string& path, v8::Local<v8::Object>& module) override;

    private:

        /// Built-in modules registerer.
        BuiltInsSetter _builtInsSetter;

        /// <summary> It keeps binary modules loaded. </summary>
        /// <remarks> boost::dll unload a shared library when no reference is there. </remarks>
        std::vector<boost::shared_ptr<NapaModule>> _modules;
    };

}   // End of namespace module.
}   // End of namespace napa.