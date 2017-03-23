#pragma once

#include <napa/module/module-internal.h>

#include <functional>
#include <string>
#include <vector>

namespace napa {
namespace module {

    using BuiltInsSetter = std::function<void (v8::Local<v8::Context> context)>;

    /// <summary> Interface to load a module from file. </summary>
    class ModuleFileLoader {
    public:

        virtual ~ModuleFileLoader() = default;

        /// <summary> It loads a module from file. </summary>
        /// <param name="path"> Module path called by require(). </param>
        /// <param name="module"> Loaded module if successful. </param>
        /// <returns> True if the module was loaded, false otherwise. </returns>
        virtual bool TryGet(const std::string& path, v8::Local<v8::Object>& module) = 0;
    };

}   // End of namespace module.
}   // End of namespace napa.