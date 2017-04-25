#include "worker.h"

#include "module/module-loader.h"
#include "v8/array-buffer-allocator.h"
#include "zone/zone-impl.h"

#include <napa-log.h>
#include <napa/module/worker-context.h>

// Disable third party library warnnings
#pragma warning(push)
#pragma warning(disable: 4127 4458 4068)
#include <moodycamel/blockingconcurrentqueue.h>
#pragma warning(pop)

#include <v8.h>

#include <cstdlib>
#include <thread>

using namespace napa;
using namespace napa::scheduler;

// Forward declaration
static v8::Isolate* CreateIsolate(const ZoneSettings& settings);
static void ConfigureIsolate(v8::Isolate* isolate, const ZoneSettings& settings);

struct Worker::Impl {

    /// <summary> The worker id. </summary>
    WorkerId id;

    /// <summary> The thread that executes the tasks. </summary>
    std::thread workerThread;

    /// <summary> Queue for tasks scheduled on this worker. </summary>
    moodycamel::BlockingConcurrentQueue<std::shared_ptr<Task>> tasks;

    /// <summary> V8 isolate associated with this worker. </summary>
    v8::Isolate* isolate;

    /// <summary> A callback function that is called when a worker becomes idle. </summary>
    std::function<void(WorkerId)> idleNotificationCallback;
};

Worker::Worker(WorkerId id,
               const ZoneSettings& settings,
               std::function<void(WorkerId)> idleNotificationCallback)
    : _impl(std::make_unique<Worker::Impl>()) {

    _impl->id = id;
    _impl->idleNotificationCallback = std::move(idleNotificationCallback);
    _impl->workerThread = std::thread(&Worker::WorkerThreadFunc, this, settings);
}

Worker::~Worker() {
    // Signal the thread loop that it should stop processing tasks.
    _impl->tasks.enqueue(nullptr);

    _impl->workerThread.join();

    if (_impl->isolate != nullptr) {
        _impl->isolate->Dispose();
    }
}

Worker::Worker(Worker&&) = default;
Worker& Worker::operator=(Worker&&) = default;

void Worker::Schedule(std::shared_ptr<Task> task) {
    NAPA_ASSERT(task, "task was null");

    _impl->tasks.enqueue(std::move(task));
}

void Worker::WorkerThreadFunc(const ZoneSettings& settings) {
    // Zone instance into TLS.
    module::WorkerContext::Set(module::WorkerContextItem::ZONE,
                               reinterpret_cast<void*>(ZoneImpl::Get(settings.id).get()));

    // Worker Id into TLS.
    module::WorkerContext::Set(module::WorkerContextItem::WORKER_ID,
                               reinterpret_cast<void*>(static_cast<uintptr_t>(_impl->id)));

    _impl->isolate = CreateIsolate(settings);

    // If any user of the v8.dll uses a locker on any isolate, all isolates must be locked before use.
    // Since we are 1-1 with threads and isolates, a top level lock that is never released is ok.
    v8::Locker locker(_impl->isolate);

    ConfigureIsolate(_impl->isolate, settings);

    v8::Isolate::Scope isolateScope(_impl->isolate);
    v8::HandleScope handleScope(_impl->isolate);
    v8::Local<v8::Context> context = v8::Context::New(_impl->isolate);

    // We set an empty security token so callee can access caller's context.
    context->SetSecurityToken(v8::Undefined(_impl->isolate));
    v8::Context::Scope contextScope(context);

    // Load module loader and built-in modules of require, console and etc.
    CREATE_MODULE_LOADER();

    while (true) {
        std::shared_ptr<Task> task;
        
        // Retrieve a task to execute. Wait if non exists.
        if (!_impl->tasks.try_dequeue(task)) {
            _impl->idleNotificationCallback(_impl->id);
            _impl->tasks.wait_dequeue(task);
        }

        // A null task means that the worker needs to shutdown.
        if (task == nullptr) {
            break;
        }

        // Resume execution capabilities if isolate was previously terminated.
        _impl->isolate->CancelTerminateExecution();

        task->Execute();
    }
}

static v8::Isolate* CreateIsolate(const ZoneSettings& settings) {
    // The allocator is a global V8 setting.
    static napa::v8_extensions::ArrayBufferAllocator commonAllocator;

    v8::Isolate::CreateParams createParams;
    createParams.array_buffer_allocator = &commonAllocator;

    // Set the maximum V8 heap size.
    createParams.constraints.set_max_old_space_size(settings.maxOldSpaceSize);
    createParams.constraints.set_max_semi_space_size(settings.maxSemiSpaceSize);
    createParams.constraints.set_max_executable_size(settings.maxExecutableSize);

    return v8::Isolate::New(createParams);
}

static void ConfigureIsolate(v8::Isolate* isolate, const ZoneSettings& settings) {
    isolate->SetFatalErrorHandler([](const char* location, const char* message) {
        LOG_ERROR("V8", "V8 Fatal error at %s. Error: %s", location, message);
    });

    // Prevent V8 from aborting on uncaught exception.
    isolate->SetAbortOnUncaughtExceptionCallback([](v8::Isolate*) {
        LOG_ERROR("V8", "V8 uncaught exception was thrown.");
        return false;
    });

    // V8 takes a pointer to the minimum (x86 stack grows down) allowed stack address
    // so, capture the current top of the stack and calculate minimum allowed
    uint32_t currentStackAddress;
    auto limit = (reinterpret_cast<uintptr_t>(&currentStackAddress - settings.maxStackSize / sizeof(uint32_t*)));
    isolate->SetStackLimit(limit);
}
