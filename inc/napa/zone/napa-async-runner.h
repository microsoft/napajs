// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/exports.h>

#include <v8.h>

#include <functional>

namespace napa {
namespace zone {

    /// <summary> Function to run asynchronously in separate thread. </summary>
    /// <remarks> Return value will be the input to 'AsyncCompleteCallback'. </remarks>
    using AsyncWork = std::function<void*()>;

    /// <summary> Function to run async-supporting function in the current thread. </summary>
    /// <remarks>
    /// Completion function given as argument must be called with return values to notify asynchronous work completion.
    /// </remarks>
    using CompletionWork = std::function<void(std::function<void(void*)>)>;

    /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
    /// <remarks>
    /// It's called inside v8::HandleScopoe.
    /// Function has two arguments, Javascript callback and return value from asynchronous work.
    /// </summary>
    using AsyncCompleteCallback = std::function<void(v8::Local<v8::Function>, void*)>;

    /// <summary> It runs a synchronous function in a separate thread and posts a completion into the current V8 execution loop. </summary>
    /// <param name="jsCallback"> Javascript callback. </summary>
    /// <param name="asyncWork"> Function to run asynchronously in separate thread. </param>
    /// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous callback completes. </param>
    NAPA_API void PostAsyncWork(v8::Local<v8::Function> jsCallback,
                                AsyncWork asyncWork,
                                AsyncCompleteCallback asyncCompleteCallback);

    /// <summary> It runs an asynchronous function and post a completion into the current V8 execution loop. </summary>
    /// <param name="jsCallback"> Javascript callback. </summary>
    /// <param name="asyncWork"> Function to wrap async-supporting function. </param>
    /// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous function completes. </param>
    NAPA_API void DoAsyncWork(v8::Local<v8::Function> jsCallback,
                              const CompletionWork& asyncWork,
                              AsyncCompleteCallback asyncCompleteCallback);

}   // End of namespace module.
}   // End of namespace napa.