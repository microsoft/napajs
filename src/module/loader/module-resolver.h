#pragma once

#include <memory>
#include <string>

namespace napa {
namespace module {

    enum class ModuleType : size_t {
        /// <summary> Module is not resolved correctly. </summary>
        NONE,

        /// <summary> Built-in or core module. </summary>
        CORE,

        /// <summary> Javascript module. </summary>
        JAVASCRIPT,

        /// <summary> Module with json object. </summary>
        JSON,

        /// <summary> Binary module. </summary>
        NAPA,

        /// <summary> End of module type. </summary>
        END_OF_MODULE_TYPE
    };

    /// <summary> Module's detail information acquired at path resolution. </summary>
    struct ModuleInfo {
        /// <summary> Module type. </summary>
        ModuleType type;

        /// <summary> Full path. </summary>
        std::string fullPath;

        /// <summary> Package.json path. </summary>
        std::string packageJsonPath;
    };

    /// <summary>
    /// It resolves a module path by the algorithm described at
    /// https://nodejs.org/api/modules.html#modules_all_together.
    /// One module loader has one module resolver, so each thread has its own instance of this class.
    /// </summary>
    class ModuleResolver {
    public:

        /// <summary> Constructor. </summary>
        ModuleResolver();

        /// <summary> Default destructor. </summary>
        ~ModuleResolver();

        /// <summary> Non-copyable. </summary>
        ModuleResolver(const ModuleResolver&) = delete;
        ModuleResolver& operator=(const ModuleResolver&) = delete;

        /// <summary> Movable. </summary>
        ModuleResolver(ModuleResolver&&) = default;
        ModuleResolver& operator=(ModuleResolver&&) = default;

        /// <summary> It resolves a full module path from a given argument of require(). </summary>
        /// <param name="name"> Module name or path. </param>
        /// <param name="path"> Current context path. If nullptr, it'll be current path. </param>
        /// <returns> Module resolution information. </returns>
        ModuleInfo Resolve(const char* name, const char* path = nullptr);

        /// <summary> It registers built-in or core modules, so they are resolved first. </summary>
        /// <param name="name"> Module name. </param>
        /// <returns>
        /// True if it successfully adds a module.
        /// If it fails or there is a duplication, return false.
        /// </returns>
        bool SetAsCoreModule(const char* name);

    private:

        /// <summary> Implementation of module resolver. </summary>
        class ModuleResolverImpl;
        std::unique_ptr<ModuleResolverImpl> _impl;
    };

}   // End of namespace module.
}   // End of namespace napa.