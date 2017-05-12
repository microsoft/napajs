#pragma once

#include <napa/v8-helpers/flow.h>
#include <napa/v8-helpers/maybe.h>
#include <napa/v8-helpers/string.h>

#include <v8.h>

namespace napa {
namespace v8_helpers {
    /// <summary> Call a function under current context </summary>
    /// <param name="functionName"> Function under current context. </param>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> Actual arguments. </param>
    /// <returns> Return value of function, or an empty handle if exception is thrown. </returns>
    inline v8::MaybeLocal<v8::Value> Call(
        const char* functionName, 
        int argc = 0, 
        v8::Local<v8::Value> argv[] = nullptr) {

        auto isolate = v8::Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();
        v8::EscapableHandleScope scope(isolate);
        
        auto function = v8::Local<v8::Function>::Cast(context->Global()->Get(v8_helpers::MakeV8String(isolate, functionName)));

        JS_ENSURE_WITH_RETURN(
            isolate,
            !function.IsEmpty(),
            v8::MaybeLocal<v8::Value>(),
            "Function \"%s\" is not found in current context.",
            functionName);

        return scope.Escape(
            function->Call(context, context->Global(), argc, argv)
                .FromMaybe(v8::Local<v8::Value>()));
    }

    /// <summary> It imports a module. </summary> 
    /// <param name="moduleName"> Module name in node 'require' convention. </summary>
    /// <returns> Object if success, or an empty handle with exception thrown. </summary>
    inline v8::MaybeLocal<v8::Object> Require(const char* moduleName) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);

        constexpr int argc = 1;
        v8::Local<v8::Value> argv[argc] = { v8_helpers::MakeV8String(isolate, moduleName)};
        return scope.Escape(LocalCast<v8::Object>(Call("require", argc, argv)));
    }

    /// <summary> Call a function from a module under current context </summary>
    /// <param name="moduleName"> Module name. </param>
    /// <param name="functionName"> Function from module. </param>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> Actual arguments. </param>
    /// <returns> Return value of function, or an empty handle if exception is thrown. </returns>
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
        auto function = v8::Local<v8::Function>::Cast(
            module->Get(v8_helpers::MakeV8String(isolate, functionName)));

        JS_ENSURE_WITH_RETURN(
            isolate,
            !function.IsEmpty(),
            v8::MaybeLocal<v8::Value>(),
            "Function \"%s\" is not found in \"%s\".",
            functionName,
            moduleName);

        return scope.Escape(
            function->Call(context, module, argc, argv)
                .FromMaybe(v8::Local<v8::Value>()));

    }

    /// <summary> Call a member function of an object </summary>
    /// <param name="object"> JavaScript object. </param>
    /// <param name="functionName"> Member function name. </param>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> Actual arguments. </param>
    /// <returns> Return value of function, or empty handle if exception is thrown. </returns>
    inline v8::MaybeLocal<v8::Value> Call(
        v8::Local<v8::Object> object,
        const char* functionName,
        int argc = 0,
        v8::Local<v8::Value> argv[] = nullptr) {

        auto isolate = v8::Isolate::GetCurrent();
        auto context = isolate->GetCurrentContext();
        v8::EscapableHandleScope scope(isolate);

        auto function = v8::Local<v8::Function>::Cast(
            object->Get(v8_helpers::MakeV8String(isolate, functionName)));

        JS_ENSURE_WITH_RETURN(
            isolate,
            !function.IsEmpty(),
            v8::MaybeLocal<v8::Value>(),
            "Function \"%s\" is not found in object.",
            functionName);

        return scope.Escape(
            function->Call(context, object, argc, argv)
                .FromMaybe(v8::Local<v8::Value>()));
    }
}
}