#pragma once

#include <node.h>
#include <uv.h>

#include <functional>
#include <memory>
#include <vector>

namespace napa {
namespace module {

    /// <summary> Function to run asynchronously in separate thread. </summary>
    using AsyncWork = std::function<void()>;

    /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
    /// <remarks> It's called inside v8::HandleScopoe. </summary>
    using AsyncCompleteCallback = std::function<void(v8::Local<v8::Function>)>;

    /// <summary> Class holding asynchonous callbacks and libuv request. </summary>
    struct AsyncContext {
        /// <summary> libuv request. </summary>
        uv_work_t work;

        /// <summary> Javascript callback. </summary>
        v8::Persistent<v8::Function> jsCallback;

        /// <summary> Function to run asynchronously in separate thread. </summary>
        AsyncWork asyncWork;

        /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
        AsyncCompleteCallback asyncCompleteCallback;
    };

    /// <summary> Callback run asynchronously in separate thread. </summary>
    /// <param name="work"> libuv request holding asynchronous callbacks. </summary>
    inline void RunAsyncWork(uv_work_t* work) {
        auto context = static_cast<AsyncContext*>(work->data);
        context->asyncWork();
    }

    /// <summary> Callback run in V8 isolate after asynchronous callback completes. </summary>
    /// <param name="work"> libuv request holding asynchronous callbacks. </summary>
    /// <param name="status"> O if asynchronous work is successful. </summary>
    inline void RunAsyncCompleteCallback(uv_work_t* work, int status) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto context = static_cast<AsyncContext*>(work->data);

        std::unique_ptr<AsyncContext, std::function<void(AsyncContext*)>> deferred(context, [](auto context) {
            context->jsCallback.Reset();
            delete context;
        });

        if (status != 0) {
            return;
        }

        auto jsCallback = v8::Local<v8::Function>::New(isolate, context->jsCallback);
        context->asyncCompleteCallback(jsCallback);
    }

    /// <summary> It posts asynchronous work. </summary>
    /// <param name="jsCallback"> Javascript callback. </summary>
    /// <param name="asyncWork"> Function to run asynchronously in separate thread. </param>
    /// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous callback completes. </param>
    inline void PostAsyncWork(v8::Local<v8::Function> jsCallback,
                              AsyncWork asyncWork,
                              AsyncCompleteCallback asyncCompleteCallback) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto context = new AsyncContext();

        context->work.data = context;
        context->jsCallback.Reset(isolate, jsCallback);
        context->asyncWork = std::move(asyncWork);
        context->asyncCompleteCallback = std::move(asyncCompleteCallback);

        uv_queue_work(uv_default_loop(), &context->work, RunAsyncWork, RunAsyncCompleteCallback);
    }

}   // End of namespace module.
}   // End of namespace napa.