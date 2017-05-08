#pragma once

#include <napa/v8-helpers/string.h>
#include <v8.h>
#include <unordered_map>

namespace napa {
namespace v8_helpers {
    /// <summary> Call a function from a module under current context </summary>
    /// <param name="moduleName"> Module name. </param>
    /// <param name="functionName"> Function from module. </param>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> Actual arguments. </param>
    /// <returns> Return value of function, or an empty handle if exception is thrown. </returns>
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

    /// <summary> Convert a V8 object to an map. </summary>
    template <typename ValueType>
    inline std::unordered_map<std::string, ValueType> V8ObjectToMap(
        v8::Isolate* isolate,
        const v8::Local<v8::Object>& obj) {

        auto context = isolate->GetCurrentContext();
        std::unordered_map<std::string, ValueType> res;

        auto maybeProps = obj->GetOwnPropertyNames(context);
        if (!maybeProps.IsEmpty()) {
            auto props = maybeProps.ToLocalChecked();
            res.reserve(props->Length());

            for (uint32_t i = 0; i < props->Length(); i++) {
                auto key = props->Get(context, i).ToLocalChecked();
                auto value = obj->Get(context, key).ToLocalChecked();

                v8::String::Utf8Value keyString(key->ToString(context).ToLocalChecked());
                res.emplace(*keyString, V8ValueTo<ValueType>(value));
            }
        }
        return res;
    }
}
}