// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "node-zone-delegates.h"

#include <zone/call-task.h>
#include <zone/eval-task.h>

#include <uv.h>

struct AsyncContext {
    /// <summary> libuv request. </summary>
    uv_async_t work;

    /// <summary> Callback that will be running in Node event loop. </summary>
    std::function<void()> callback;
};

/// <summary> Run an async work item in Node. </summary>
void Run(uv_async_t* work) {
    auto context = static_cast<AsyncContext*>(work->data);

    context->callback();

    uv_close(reinterpret_cast<uv_handle_t*>(work), [](auto work) {
        auto context = static_cast<AsyncContext*>(work->data);
        delete context;
    });
}

/// <summary> Schedule a function in Node event loop. </summary>
void ScheduleInNode(std::function<void()> callback) {
    auto context = new AsyncContext();
    context->work.data = context;
    context->callback = std::move(callback);

    uv_async_init(uv_default_loop(), &context->work, Run);
    uv_async_send(&context->work);
}

void napa::node_zone::Broadcast(const napa::FunctionSpec& spec, napa::BroadcastCallback callback) {
    auto requestContext = std::make_shared<napa::zone::CallContext>(spec, callback);
    ScheduleInNode([requestContext = std::move(requestContext)]() {
        napa::zone::CallTask task(std::move(requestContext));
        task.Execute();
    });
}

void napa::node_zone::Execute(const napa::FunctionSpec& spec, napa::ExecuteCallback callback) {
    auto requestContext = std::make_shared<napa::zone::CallContext>(spec, callback);
    ScheduleInNode([requestContext = std::move(requestContext)]() {
        napa::zone::CallTask task(std::move(requestContext));
        task.Execute();
    });
}
