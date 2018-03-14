// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/assert.h>
#include <napa/exports.h>
#include <napa/v8-helpers/maybe.h>
#include <napa/v8-helpers/string.h>
#include <napa/v8-helpers/flow.h>

#include <v8.h>

namespace napa {
namespace module {
namespace binding {

    /// <summary> Get 'module' object of napa binding, which is napa-binding.node in Node.JS isolate or napa-binding from core-modules in Napa isolate. </summary>
    /// <returns> 'module' object for napa binding (napajs/bin/napa-binding.node or napa.dll) </returns>
    NAPA_BINDING_API v8::Local<v8::Object> GetModule();

    /// <summary> Get 'module.exports' from napa binding. </summary>
    /// <returns> 'module.exports' object for napa binding (napajs/bin/napa-binding.node or napa.dll) </returns>
    inline v8::Local<v8::Object> GetBinding() {
        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);

        auto bindingModule = GetModule();        
        auto binding = bindingModule->Get(napa::v8_helpers::MakeV8String(isolate, "exports"));
        NAPA_ASSERT(!binding.IsEmpty() && binding->IsObject(), "\"exports\" is not available or not object type.");

        return scope.Escape(v8::Local<v8::Object>::Cast(binding));
    }

    /// <summary> It calls 'module.require' from context of napa binding in C++. </summary> 
    /// <param name="moduleName"> Module name in node 'require' convention. </summary>
    /// <returns> Object if success, or an empty handle with exception thrown. </summary>
    /// <remarks> 
    ///  1) 'napajs' must be required from JS first before Require can be used. 
    ///  2) Base directory calling 'require' is from 'napajs/bin'. 
    /// </remarks>
    inline v8::MaybeLocal<v8::Object> Require(const char* moduleName) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);
        
        //auto bindingModule = GetModule();
        //auto require = bindingModule->Get(napa::v8_helpers::MakeV8String(isolate, "require"));
        //NAPA_ASSERT(!require.IsEmpty() && require->IsFunction(), "Function \"require\" is not available from module object");

        auto context = isolate->GetCurrentContext();
        auto require = context->Global()->Get(napa::v8_helpers::MakeV8String(isolate, "require"));
        NAPA_ASSERT(!require.IsEmpty() && require->IsFunction(), "Function \"require\" is not available from module object");

        v8::Local<v8::Value> argv[] = { napa::v8_helpers::MakeV8String(isolate, moduleName)};
        return scope.Escape(
            napa::v8_helpers::ToLocal<v8::Object>(
                v8::Local<v8::Function>::Cast(require)->Call(isolate->GetCurrentContext(), v8::Null(isolate), 1, argv)));
    }

    /// <summary> Create a new instance of a wrap type exported from napa binding. </summary>
    inline v8::MaybeLocal<v8::Object> NewInstance(const char* wrapType, int argc, v8::Local<v8::Value> argv[]) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);

        auto binding = GetBinding();
        auto constructor = binding->Get(napa::v8_helpers::MakeV8String(isolate, wrapType));
        JS_ENSURE_WITH_RETURN(
            isolate, 
            !constructor.IsEmpty() && constructor->IsFunction(), 
            v8::MaybeLocal<v8::Object>(), 
            "Wrap type \"%s\" is not found in napa binding.",
            wrapType);

        return scope.Escape(
            v8::Local<v8::Function>::Cast(constructor)->NewInstance(isolate->GetCurrentContext(), argc, argv)
                .FromMaybe(v8::Local<v8::Object>()));
    }

    /// <summary> Call a function from a module under current context </summary>
    /// <param name="moduleName"> Module name. </param>
    /// <param name="functionName"> Function from module. </param>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> Actual arguments. </param>
    /// <returns> Return value of function, or an empty handle if exception is thrown. </returns>
    /// <remarks> moduleName should take 'napajs/bin' as base directory. </remarks>
    inline v8::MaybeLocal<v8::Object> NewInstance(
        const char* moduleName, 
        const char* className, 
        int argc = 0, 
        v8::Local<v8::Value> argv[] = nullptr) {

        auto isolate = v8::Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();
        v8::EscapableHandleScope scope(isolate);

        auto moduleHandle = Require(moduleName);
        RETURN_VALUE_ON_PENDING_EXCEPTION(moduleHandle, v8::MaybeLocal<v8::Object>());
        
        auto module = moduleHandle.ToLocalChecked();
        auto constructor = v8::Local<v8::Function>::Cast(
            module->Get(v8_helpers::MakeV8String(isolate, className)));

        JS_ENSURE_WITH_RETURN(
            isolate,
            !constructor.IsEmpty(),
            v8::MaybeLocal<v8::Object>(),
            "Class \"%s\" is not found in \"%s\".",
            className,
            moduleName);

        return scope.Escape(
            constructor->NewInstance(context, argc, argv)
                .FromMaybe(v8::Local<v8::Object>()));
    }

    /// <summary> Call a function from a module under current context </summary>
    /// <param name="moduleName"> Module name. </param>
    /// <param name="functionName"> Function from module. </param>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> Actual arguments. </param>
    /// <returns> Return value of function, or an empty handle if exception is thrown. </returns>
    /// <remarks> moduleName should take 'napajs/bin' as base directory. </remarks>
    inline v8::MaybeLocal<v8::Value> Call(
        const char* moduleName,
        const char* functionName,
        int argc = 0,
        v8::Local<v8::Value> argv[] = nullptr) {

        auto isolate = v8::Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();
        v8::EscapableHandleScope scope(isolate);

        auto moduleHandle = Require(moduleName);
        RETURN_VALUE_ON_PENDING_EXCEPTION(moduleHandle, v8::MaybeLocal<v8::Value>());

        auto module = moduleHandle.ToLocalChecked();
        auto function = module->Get(v8_helpers::MakeV8String(isolate, functionName));

        JS_ENSURE_WITH_RETURN(
            isolate,
            !function.IsEmpty() && function->IsFunction(),
            v8::MaybeLocal<v8::Value>(),
            "Function \"%s\" is not found in module \"%s\".",
            functionName,
            moduleName);

        return scope.Escape(
            v8::Local<v8::Function>::Cast(function)->Call(context, module, argc, argv)
                .FromMaybe(v8::Local<v8::Value>()));
    }
}
}
}