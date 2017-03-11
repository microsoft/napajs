#pragma once

#include <napa/module/module-internal.h>

#include <functional>
#include <memory>

namespace napa {
namespace module {

    /// <summary>
    /// It follows node.js's module resolution algorithm, https://nodejs.org/api/modules.html#modules_all_together except,
    /// - Napa module is created in thread-safe way.
    /// - Napa native module uses '.napa' extension instead of '.node'.
    /// - It supports require(name, string), which loads a module from a given script string.
    /// </summary>
    class ModuleLoader {
    public:

        /// <summary> Default constructor. </summary>
        ModuleLoader();

        /// <summary> Non-copyable and Non-movable. </summary>
        ModuleLoader(const ModuleLoader&) = delete;
        ModuleLoader& operator=(const ModuleLoader&) = delete;
        ModuleLoader(ModuleLoader&&) = delete;
        ModuleLoader& operator=(ModuleLoader&&) = delete;

        /// <summary> Initialize built-in modules without require(). </summary>
        /// <param name="context"> V8 context for running javascript. </param>
        static void InitializeBuiltIns(v8::Local<v8::Context> context);

        /// <summary> Register a module that can be loaded by require. </summary>
        /// <param name="moduleName"> Module name. </param>
        /// <param name="register"> Function to register built-in module to module context. </param>
        static void RegisterCoreModule(const char* moduleName,
                                       const std::function<void (v8::Local<v8::Object>)>& initializer);

        /// <summary> Get module root directory. </summary>
        /// <returns> Module root directory. </returns>
        static const std::string& GetModuleRootDirectory();

        /// <summary> Get current directory under running Javascript context. </summary>
        /// <returns> Directory of running JavaScript. </returns>
        static std::string GetCurrentDirectory();

        /// <summary> Get directory of calling Javascript context. </summary>
        /// <returns> Directory of calling JavaScript. If called from external string, module root directory will be returned. </returns>
        static std::string GetCallingDirectory();

        #define INIT_BUILTIN_MODULES napa::module::ModuleLoader::InitializeBuiltIns
        #define REGISTER_CORE_MODULE napa::module::ModuleLoader::RegisterCoreModule

        #define MODULE_ROOT_DIR napa::module::ModuleLoader::GetModuleRootDirectory
        #define CURRENT_DIR napa::module::ModuleLoader::GetCurrentDirectory
        #define CALLING_DIR napa::module::ModuleLoader::GetCallingDirectory

    private:

        /// <summary> Create and initialize module loader. </summary>
        static ModuleLoader* CreateAndInitialize(bool fromJS);

        /// <summary> Global callback function for require(). </summary>
        /// <param name="args"> V8 argument to return module object. </param>
        static void RequireCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Return directory of a JavaScript context. </summary>
        /// <param name="context"> V8 context. </param>
        /// <returns> Directory of context. If called from external string, module root directory will be returned. </returns>
        static std::string GetContextDirectory(const v8::Local<v8::Context>& context);

        class ModuleLoaderImpl;
        std::unique_ptr<ModuleLoaderImpl> _impl;
    };

}
}
