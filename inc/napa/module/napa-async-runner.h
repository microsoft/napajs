#pragma once

#include <napa/exports.h>

#include <v8.h>

#include <functional>

namespace napa {
namespace module {

    /// <summary> Function to run asynchronously in separate thread. </summary>
    using AsyncWork = std::function<void()>;

    /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
    using AsyncCompleteCallback = std::function<void(v8::Local<v8::Function>)>;

    /// <summary> It posts asynchronous work. </summary>
    /// <param name="jsCallback"> Javascript callback. </summary>
    /// <param name="asyncWork"> Callback running asynchronously in separate thread. </param>
    /// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous callback completes. </param>
    NAPA_API void PostAsyncWork(v8::Local<v8::Function> jsCallback,
                                AsyncWork asyncWork,
                                AsyncCompleteCallback asyncCompleteCallback);

}   // End of namespace module.
}   // End of namespace napa.