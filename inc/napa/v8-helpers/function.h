// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

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
    inline v8::MaybeLocal<v8::Value>
    Call(const char* functionName, int argc = 0, v8::Local<v8::Value> argv[] = nullptr) {

        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);

        auto context = isolate->GetCurrentContext();
        auto function = context->Global()->Get(v8_helpers::MakeV8String(isolate, functionName));

        JS_ENSURE_WITH_RETURN(isolate,
                              !function.IsEmpty() && function->IsFunction(),
                              v8::MaybeLocal<v8::Value>(),
                              "Function \"%s\" is not found in current context.",
                              functionName);

        return scope.Escape(v8::Local<v8::Function>::Cast(function)
                                ->Call(context, context->Global(), argc, argv)
                                .FromMaybe(v8::Local<v8::Value>()));
    }

    /// <summary> Call a member function of an object </summary>
    /// <param name="object"> JavaScript object. </param>
    /// <param name="functionName"> Member function name. </param>
    /// <param name="argc"> Number of arguments. </param>
    /// <param name="argv"> Actual arguments. </param>
    /// <returns> Return value of function, or empty handle if exception is thrown. </returns>
    inline v8::MaybeLocal<v8::Value>
    Call(v8::Local<v8::Object> object, const char* functionName, int argc = 0, v8::Local<v8::Value> argv[] = nullptr) {

        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);

        auto context = isolate->GetCurrentContext();
        auto function = object->Get(v8_helpers::MakeV8String(isolate, functionName));

        JS_ENSURE_WITH_RETURN(isolate,
                              !function.IsEmpty() && function->IsFunction(),
                              v8::MaybeLocal<v8::Value>(),
                              "Function \"%s\" is not found in object.",
                              functionName);

        return scope.Escape(v8::Local<v8::Function>::Cast(function)
                                ->Call(context, object, argc, argv)
                                .FromMaybe(v8::Local<v8::Value>()));
    }
}
}