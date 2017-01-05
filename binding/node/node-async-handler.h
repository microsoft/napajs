#ifndef NAPA_BINDING_NODE_ASYNC_HANDLER_H
#define NAPA_BINDING_NODE_ASYNC_HANDLER_H

#include <uv.h>

#include <vector>
#include <functional>
#include <utility>


namespace napa
{
namespace binding
{
    /// <summary>
    ///     Helper class to facilitate dispatcing async calls fron node addons.
    ///     The handler stores the provided V8 callback function persistently which enables calling
    ///     it when the underlying execution callback is called.
    ///     The dispatcing of the V8 callback happend on the node main thread.
    /// </summary>
    template <typename ResponseType>
    class NodeAsyncHandler
    {
    public:

        typedef std::function<std::vector<v8::Local<v8::Value>>(const ResponseType&)> CallbackArgsGeneratorFunction;

        /// <summary>Non copyable and non movable. </summary>
        NodeAsyncHandler(const NodeAsyncHandler&) = delete;
        NodeAsyncHandler& operator=(const NodeAsyncHandler&) = delete;
        NodeAsyncHandler(NodeAsyncHandler&&) = delete;
        NodeAsyncHandler& operator=(NodeAsyncHandler&&) = delete;

        /// <summary>
        ///     Factory method to create the handler.
        ///     Releasing the handler resources happens automatically when dispatcing is done.
        ///     In cases when an error occurs before a call to DispatchCallback was made, the user
        ///     should invoke the Release method.
        /// </summary>
        static NodeAsyncHandler<ResponseType>* New(
            v8::Isolate* isolate,
            const v8::Local<v8::Function>& callback,
            CallbackArgsGeneratorFunction argsGeneratorFunc);

        /// <summary>Releasing handler resources </summary>
        static void Release(NodeAsyncHandler<ResponseType>* handler);

        /// <summary>Invoking the stored V8 callback on node main thread. </summary>
        void DispatchCallback(ResponseType&& response);

    private:

        NodeAsyncHandler() {}

        // Prevent users from deleting the handler.
        ~NodeAsyncHandler() {}

        static void AsyncCompletionCallback(uv_async_t* asyncHandle);

        v8::Isolate* _isolate;
        v8::Persistent<v8::Function> _callback;
        CallbackArgsGeneratorFunction _argsGeneratorFunc;
        
        uv_async_t _asyncHandle;
        ResponseType _response;
    };

    template <typename ResponseType>
    NodeAsyncHandler<ResponseType>* NodeAsyncHandler<ResponseType>::New(
        v8::Isolate* isolate,
        const v8::Local<v8::Function>& callback,
        CallbackArgsGeneratorFunction argsGeneratorFunc)
    {
        auto handler = new NodeAsyncHandler<ResponseType>();

        handler->_isolate = isolate;

        // Store the callback in a persistent function.
        handler->_callback.Reset(isolate, callback);

        // Store the argument generator function.
        handler->_argsGeneratorFunc = std::move(argsGeneratorFunc);

        // Store the handler to enable retrieving it in the uv callbacks.
        handler->_asyncHandle.data = handler;

        // Initialize the uv async handle and set the callback function
        // that will be executed on the node main thread.
        uv_async_init(uv_default_loop(), &(handler->_asyncHandle), AsyncCompletionCallback);

        return handler;
    }

    template <typename ResponseType>
    void NodeAsyncHandler<ResponseType>::Release(NodeAsyncHandler<ResponseType>* handler)
    {
        // Free the persistent function callback.
        handler->_callback.Reset();

        // De-allocate the memory.
        delete handler;
    }

    template <typename ResponseType>
    void NodeAsyncHandler<ResponseType>::DispatchCallback(ResponseType&& response)
    {
        _response = std::forward<ResponseType>(response);

        // Defer JS callback to the node main thread.
        uv_async_send(&_asyncHandle);
    }

    template <typename ResponseType>
    void NodeAsyncHandler<ResponseType>::AsyncCompletionCallback(uv_async_t* asyncHandle)
    {
        auto handler = reinterpret_cast<NodeAsyncHandler<ResponseType>*>(asyncHandle->data);

        auto isolate = handler->_isolate;
        v8::HandleScope scope(isolate);
        auto context = isolate->GetCurrentContext();

        auto callback = v8::Local<v8::Function>::New(isolate, handler->_callback);

        auto args = handler->_argsGeneratorFunc(handler->_response);

        // Call the user provided Javascript callback.
        callback->Call(context, context->Global(), static_cast<int>(args.size()), args.data());

        // Cleanup
        uv_close(reinterpret_cast<uv_handle_t*>(asyncHandle), [](uv_handle_t* asyncHandle) {
            auto handler = reinterpret_cast<NodeAsyncHandler<ResponseType>*>(asyncHandle->data);

            Release(handler);
        });
    }

}
}

#endif // NAPA_BINDING_NODE_ASYNC_HANDLER_H
