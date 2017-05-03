#pragma once

#include <node.h>
#include <uv.h>

#include <functional>
#include <memory>
#include <vector>

namespace napa {
namespace module {

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

    /// <summary> Class holding asynchonous callbacks and libuv request. </summary>
    struct AsyncContext {
        /// <summary> libuv request. </summary>
        uv_work_t work;

        /// <summary> Javascript callback. </summary>
        v8::Persistent<v8::Function> jsCallback;

        /// <summary> Function to run asynchronously in separate thread. </summary>
        AsyncWork asyncWork;

        /// <summary> Result from asynchronous work. </summary>
        void* result = nullptr;

        /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
        AsyncCompleteCallback asyncCompleteCallback;
    };

    /// <summary> Class holding completion callback and libuv request. </summary>
    struct CompletionContext {
        /// <summary> libuv request. </summary>
        uv_async_t work;

        /// <summary> Javascript callback. </summary>
        v8::Persistent<v8::Function> jsCallback;

        /// <summary> Result from asynchronous work. </summary>
        void* result = nullptr;

        /// <summary> Callback running in V8 isolate after asynchronous callback completes. </summary>
        AsyncCompleteCallback asyncCompleteCallback;
    };

    /// <summary> Callback run asynchronously in separate thread. </summary>
    /// <param name="work"> libuv request holding asynchronous callbacks. </summary>
    inline void RunAsyncWork(uv_work_t* work) {
        auto context = static_cast<AsyncContext*>(work->data);
        context->result = context->asyncWork();
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
        context->asyncCompleteCallback(jsCallback, context->result);
    }

    /// <summary> Callback run in node event loop. </summary>
    /// <param name="work"> libuv request holding asynchronous callbacks. </summary>
    inline void RunCompletionCallback(uv_async_t* work) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto context = static_cast<CompletionContext*>(work->data);

        auto jsCallback = v8::Local<v8::Function>::New(isolate, context->jsCallback);
        context->asyncCompleteCallback(jsCallback, context->result);

        uv_close(reinterpret_cast<uv_handle_t*>(work), [](auto work) {
            auto context = static_cast<CompletionContext*>(work->data);
            context->jsCallback.Reset();
            delete context;
        });
    }

    /// <summary> It runs a synchronous function in a separate thread and posts a completion into the current V8 execution loop. </summary>
    /// <param name="jsCallback"> Javascript callback. </summary>
    /// <param name="asyncWork"> Function to run asynchronously in separate thread. </param>
    /// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous callback completes. </param>
    /// <remarks> Return value from 'asyncWork' will be the input to 'asyncCompleteCallback'. </remarks>
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

    /// <summary> It runs an asynchronous function and post a completion into the current V8 execution loop. </summary>
    /// <param name="jsCallback"> Javascript callback. </summary>
    /// <param name="asyncWork"> Function to wrap async-supporting function. </param>
    /// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous function completes. </param>
    /// <remarks> Argument at 'asyncWork' completion callback will be the input to 'asyncCompleteCallback'. </remarks>
    inline void DoAsyncWork(v8::Local<v8::Function> jsCallback,
                            const CompletionWork& asyncWork,
                            AsyncCompleteCallback asyncCompleteCallback) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto context = new CompletionContext();

        context->work.data = context;
        context->jsCallback.Reset(isolate, jsCallback);
        context->asyncCompleteCallback = std::move(asyncCompleteCallback);

        uv_async_init(uv_default_loop(), &context->work, RunCompletionCallback);

        asyncWork([context](void* result) {
            context->result = result;

            uv_async_send(&context->work);
        });
    }

}   // End of namespace module.
}   // End of namespace napa.
