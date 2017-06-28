#pragma once

#include "module-file-loader.h"

#include <string>

namespace napa {
namespace module {

    /// <summary> It loads an object from json file. </summary>
    class JsonModuleLoader : public ModuleFileLoader {
    public:

        /// <summary> It loads an object from json file. </summary>
        /// <param name="path"> Module path called by require(). </param>
        /// <param name="module"> Loaded object if successful. </param>
        /// <returns> True if the object is loaded, false otherwise. </returns>
        bool TryGet(const std::string& path, v8::Local<v8::Object>& module) override;
    };

}   // End of namespace module.
}   // End of namespace napa.