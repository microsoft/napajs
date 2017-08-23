// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/exports.h>

#include <array>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <v8.h>

namespace napa {
namespace module {

    /// <summary> Napa module version. </summary>
    static const int32_t MODULE_VERSION = 2;

    /// <summary> Variable name to export for module registration. </summary>
    static const char* NAPA_MODULE_EXPORT = "_napa_module";

    /// <summary> Function pointer to initialize a module. It's called after a module is loaded. </summary>
    typedef void (*ModuleInitializer)(
        v8::Local<v8::Object> exports,
        v8::Local<v8::Value> module);

    /// <summary> Module information. </summary>
    struct NapaModule {
        /// <summary> Current module version. </summary>
        int32_t version;

        /// <summary> Module name. </summary>
        const char* moduleName;

        /// <summary> Function pointer to initialize a module. </summary>
        ModuleInitializer initializer;
    };

    /// <summary> It binds the method name with V8 function. </summary>
    /// <param name="exports"> V8 object to bind with the given callback function. </param>
    /// <param name="name"> Method name. </param>
    /// <param name="callback"> Binding V8 function object. </param>
    template <typename T>
    void SetMethod(const T& exports,
        const char* name,
        v8::FunctionCallback callback) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);

        auto functionTemplate = v8::FunctionTemplate::New(isolate, callback);
        auto function = functionTemplate->GetFunction();
        auto functionName = v8::String::NewFromUtf8(isolate, name);
        function->SetName(functionName);

        exports->Set(functionName, function);
    }

    /// <summary> It binds the method name with V8 prototype function object. </summary>
    /// <param name="functionTemplate"> V8 function template object to bind with the given callback function. </param>
    /// <param name="name"> Method name. </param>
    /// <param name="callback"> Binding V8 function object. </param>
    inline void SetPrototypeMethod(v8::Local<v8::FunctionTemplate> functionTemplate,
        const char* name,
        v8::FunctionCallback callback) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);

        auto signature = v8::Signature::New(isolate, functionTemplate);
        functionTemplate->PrototypeTemplate()->Set(
            v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kNormal).ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, callback, v8::Local<v8::Value>(), signature));
    }

#define NAPA_REGISTER_MODULE(name, function)                          \
    extern "C" {                                                      \
    DLL_EXPORT napa::module::NapaModule _napa_module = {              \
        napa::module::MODULE_VERSION,                                 \
        #name,                                                        \
        reinterpret_cast<napa::module::ModuleInitializer>(function)}; \
    }

    /// <summary> It sets the persistent constructor at the current V8 isolate. </summary>
    /// <param name="name"> Unique constructor name. It's recommended to use the same name as module. </param>
    /// <param name="constructor"> V8 persistent function to constructor V8 object. </param>
    NAPA_API void SetPersistentConstructor(const char* name, v8::Local<v8::Function> constructor);

    /// <summary> It gets the given persistent constructor from the current V8 isolate. </summary>
    /// <param name="name"> Unique constructor name given at SetPersistentConstructor() call. </param>
    /// <returns> V8 local function object. </returns>
    NAPA_API v8::Local<v8::Function> GetPersistentConstructor(const char* name);

} // namespace module
} // namespace napa