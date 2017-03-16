#pragma once

#include <memory>
#include <string>

namespace napa {
namespace module {

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
        /// <returns> Full module path. </returns>
        std::string Resolve(const char* name, const char* path = nullptr);

        /// <summary> It registers built-in or core modules, so they are resolved first. </summary>
        /// <param name="name"> Module name. </param>
        /// <returns>
        /// True if it successfully adds a module.
        /// If it fails or there is a duplication, return false.
        /// </returns>
        bool SetAsBuiltInOrCoreModule(const char* name);

    private:

        /// <summary> Implementation of module resolver. </summary>
        class ModuleResolverImpl;
        std::unique_ptr<ModuleResolverImpl> _impl;
    };

}   // End of namespace module.
}   // End of namespace napa.