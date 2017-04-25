#include <napa/module/worker-context.h>
#include <napa/module/napa-async-runner.h>
#include <scheduler/async-complete-task.h>
#include <zone/zone-impl.h>

using namespace napa;
using namespace napa::module;

/// <summary> It posts asynchronous work. </summary>
/// <param name="jsCallback"> Javascript callback. </summary>
/// <param name="asyncWork"> Function to run asynchronously in separate thread. </param>
/// <param name="asyncCompleteCallback"> Callback running in V8 isolate after asynchronous callback completes. </param>
void napa::module::PostAsyncWork(v8::Local<v8::Function> jsCallback,
                                 AsyncWork asyncWork,
                                 AsyncCompleteCallback asyncCompleteCallback) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto context = std::make_shared<AsyncContext>();

    context->zone = reinterpret_cast<ZoneImpl*>(WorkerContext::Get(WorkerContextItem::ZONE));
    if (context->zone == nullptr) {
        return;
    }

    context->workerId = static_cast<scheduler::WorkerId>(
        reinterpret_cast<uintptr_t>(module::WorkerContext::Get(WorkerContextItem::WORKER_ID)));

    context->jsCallback.Reset(isolate, jsCallback);
    context->asyncWork = std::move(asyncWork);
    context->asyncCompleteCallback = std::move(asyncCompleteCallback);

    context->future = std::async(std::launch::async, [context]() {
        context->asyncWork();

        auto asyncCompleteTask = std::make_shared<AsyncCompleteTask>(context);
        context->zone->GetScheduler()->ScheduleOnWorker(context->workerId, asyncCompleteTask);
    });
}