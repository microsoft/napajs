// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <napa/zone/napa-async-runner.h>

#include <zone/async-complete-task.h>
#include <zone/worker-context.h>
#include <zone/napa-zone.h>

using namespace napa;
using namespace napa::zone;

namespace {

/// <summary> Prepare asynchronous work. </summary>
/// <param name="jsCallback"> Javascript callback. </summary>
/// <param name="asyncWork"> Function to run asynchronously in separate thread. </param>
/// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous callback completes. </param>
/// <returns> AsyncContext instance. </summary>
std::shared_ptr<AsyncContext>
PrepareAsyncWork(v8::Local<v8::Function> jsCallback, AsyncWork asyncWork, AsyncCompleteCallback asyncCompleteCallback);

} // End of anonymous namespace.

/// <summary> It runs a synchronous function in the separate thread and posts a completion into the current V8 execution
/// loop. </summary>
/// <param name="jsCallback"> Javascript callback. </summary>
/// <param name="asyncWork"> Function to run asynchronously in separate thread. </param>
/// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous callback completes. </param>
void napa::zone::PostAsyncWork(v8::Local<v8::Function> jsCallback,
                               AsyncWork asyncWork,
                               AsyncCompleteCallback asyncCompleteCallback) {
    auto context = PrepareAsyncWork(jsCallback, std::move(asyncWork), std::move(asyncCompleteCallback));
    if (context == nullptr) {
        return;
    }

    context->future =
        std::async(std::launch::async,
                   [context]() {
                       context->result = context->asyncWork();

                       auto asyncCompleteTask = std::make_shared<AsyncCompleteTask>(context);
                       context->zone->GetScheduler()->ScheduleOnWorker(context->workerId, asyncCompleteTask);
                   });
}

/// <summary> It runs an asynchronous function and post a completion into the current V8 execution loop. </summary>
/// <param name="jsCallback"> Javascript callback. </summary>
/// <param name="asyncWork"> Function to wrap async-supporting function. </param>
/// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous function completes. </param>
void napa::zone::DoAsyncWork(v8::Local<v8::Function> jsCallback,
                             const CompletionWork& asyncWork,
                             AsyncCompleteCallback asyncCompleteCallback) {
    auto context = PrepareAsyncWork(jsCallback, nullptr, std::move(asyncCompleteCallback));
    if (context == nullptr) {
        return;
    }

    asyncWork([context](void* result) {
        context->result = result;

        auto asyncCompleteTask = std::make_shared<AsyncCompleteTask>(context);
        context->zone->GetScheduler()->ScheduleOnWorker(context->workerId, asyncCompleteTask);
    });
}

namespace {

std::shared_ptr<AsyncContext>
PrepareAsyncWork(v8::Local<v8::Function> jsCallback, AsyncWork asyncWork, AsyncCompleteCallback asyncCompleteCallback) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = std::make_shared<zone::AsyncContext>();

    context->zone = reinterpret_cast<NapaZone*>(WorkerContext::Get(WorkerContextItem::ZONE));
    if (context->zone == nullptr) {
        return nullptr;
    }

    context->scheduler = context->zone->GetScheduler();
    context->workerId =
        static_cast<WorkerId>(reinterpret_cast<uintptr_t>(WorkerContext::Get(WorkerContextItem::WORKER_ID)));

    context->jsCallback.Reset(isolate, jsCallback);
    context->asyncWork = std::move(asyncWork);
    context->asyncCompleteCallback = std::move(asyncCompleteCallback);

    return context;
}

} // End of anonymous namespace.
