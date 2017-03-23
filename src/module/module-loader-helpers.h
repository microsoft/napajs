#pragma once

#include <napa/module/module-internal.h>

namespace napa {
namespace module {
namespace module_loader_helpers {

    /// <summary> It exports loaded module. </summary>
    /// <param name="object"> Loaded javascript object. </param>
    /// <param name="initializer"> Callback function to initialize a module. </param>
    /// <returns> Exported javascript object. </returns>
    v8::Local<v8::Object> ExportModule(v8::Local<v8::Object> object,
                                       const napa::module::ModuleInitializer& initializer);

    /// <summary> It returns directory of a JavaScript context. </summary>
    /// <returns> Directory of context. If called from external string, module root directory will be returned. </returns>
    std::string GetCurrentContextDirectory();

    /// <summary> Set __dirname and __filename. </summary>
    /// <param name="exports"> Object to set modue paths. </param>
    void SetContextModulePath(v8::Local<v8::Object> exports);

    /// <summary> It creates a module context. </summary>
    /// <param name="path"> Module path called by require(). </param>
    /// <returns> V8 context object. </returns>
    v8::Local<v8::Context> SetUpModuleContext(const std::string& path);

    /// <summary> It reads a module file to javascript string. </summary>
    /// <param name="path"> File path to read. </param>
    /// <returns> V8 string containing file content. </returns>
    v8::Local<v8::String> ReadModuleFile(const std::string& path);

}   // End of namespace module_loader_helpers.
}   // End of namespace module
}   // End of namespace napa