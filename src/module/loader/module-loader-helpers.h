// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module/module-internal.h>
#include <vector>

#include <vector>

namespace napa {
namespace module {
    namespace module_loader_helpers {

        /// <summary> Core module information from 'core-modules.json'. </summary>
        struct CoreModuleInfo {
            /// <summary> Core module name. </summary>
            std::string name;

            /// <summary> True if it's a built-in module. False, otherwise. </summary>
            bool isBuiltIn;

            /// <summary> Constructor. </summary>
            CoreModuleInfo(std::string name, bool isBuiltIn) : name(std::move(name)), isBuiltIn(isBuiltIn) {}
        };

        /// <summary> It exports loaded module. </summary>
        /// <param name="object"> Loaded javascript object. </param>
        /// <param name="initializer"> Callback function to initialize a module. </param>
        /// <returns> Exported javascript object. </returns>
        v8::Local<v8::Object> ExportModule(v8::Local<v8::Object> object,
                                           const napa::module::ModuleInitializer& initializer);

        /// <summary> It returns directory of a JavaScript context. </summary>
        /// <returns> Directory of context. If called from external string, module root directory will be returned.
        /// </returns>
        std::string GetCurrentContextDirectory();

        /// <summary> It returns parent directory of module.filename. </summary>
        /// <returns> Parent directory of module.filename. </summary>
        std::string GetModuleDirectory(v8::Local<v8::Object> module);

        /// <summary> It returns the directory of napa runtime. </summary>
        /// <returns> Directory of napa runtime. </returns>
        const std::string& GetNapaRuntimeDirectory();

        /// <summary> It returns napa.dll path. </summary>
        /// <returns> Path of napa.dll. </summary>
        const std::string& GetNapaDllPath();

        /// <summary> It returns the process current working directory. </summary>
        const std::string& GetCurrentWorkingDirectory();

        /// <summary> It returns the root directory of napa lib files. </summary>
        const std::string& GetLibDirectory();

        /// <summary> It sets globals and the root module path of global context as the script path. </summary>
        void SetupTopLevelContext();

        /// <summary> It sets up a module context. </summary>
        /// <param name="parentContext"> Parent context. </param>
        /// <param name="moduleContext"> Module context. </param>
        /// <param name="path"> Module path called by require(). </param>
        /// <returns> V8 context object. </returns>
        void SetupModuleContext(v8::Local<v8::Context> parentContext,
                                v8::Local<v8::Context> moduleContext,
                                const std::string& path);

        /// <summary> It reads javascript core module information. </summary>
        std::vector<CoreModuleInfo> ReadCoreModulesJson();

        /// <summary> It reads a module file to javascript string. </summary>
        /// <param name="path"> File path to read. </param>
        /// <returns> V8 string containing file content. </returns>
        v8::Local<v8::String> ReadModuleFile(const std::string& path);

    } // End of namespace module_loader_helpers.
} // End of namespace module
} // End of namespace napa