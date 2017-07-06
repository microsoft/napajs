#include "worker.h"

#include "worker-context.h"
#include "module/loader/module-loader.h"
#include "v8/array-buffer-allocator.h"
#include "zone/napa-zone.h"

#include <napa-log.h>

#include <v8.h>

#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <thread>

using namespace napa;
using namespace napa::zone;

// Forward declaration
static v8::Isolate* CreateIsolate(const settings::ZoneSettings& settings);
static void ConfigureIsolate(v8::Isolate* isolate, const settings::ZoneSettings& settings);

struct Worker::Impl {

    /// <summary> The worker id. </summary>
    WorkerId id;

    /// <summary> The thread that executes the tasks. </summary>
    std::thread workerThread;

    /// <summary> Queue for tasks scheduled on this worker. </summary>
    std::queue<std::shared_ptr<Task>> tasks;
    
    /// <summary> Condition variable to indicate if there are more tasks to consume. </summary>
    std::condition_variable hasTaskEvent;

    /// <summary> Lock for task queue. </summary>
    std::mutex queueLock;

    /// <summary> V8 isolate associated with this worker. </summary>
    v8::Isolate* isolate;

    /// <summary> A callback function that is called when a worker becomes idle. </summary>
    std::function<void(WorkerId)> idleNotificationCallback;
};

Worker::Worker(WorkerId id,
               const settings::ZoneSettings& settings,
               std::function<void(WorkerId)> idleNotificationCallback)
    : _impl(std::make_unique<Worker::Impl>()) {

    _impl->id = id;
    _impl->idleNotificationCallback = std::move(idleNotificationCallback);
    _impl->workerThread = std::thread(&Worker::WorkerThreadFunc, this, settings);
}

Worker::~Worker() {
    // Signal the thread loop that it should stop processing tasks.
    Enqueue(nullptr);

    _impl->workerThread.join();

    if (_impl->isolate != nullptr) {
        _impl->isolate->Dispose();
    }
}

Worker::Worker(Worker&&) = default;
Worker& Worker::operator=(Worker&&) = default;

void Worker::Schedule(std::shared_ptr<Task> task) {
    NAPA_ASSERT(task != nullptr, "Task should not be null");
    Enqueue(task);
}

void Worker::Enqueue(std::shared_ptr<Task> task) {
    {
        std::unique_lock<std::mutex> lock(_impl->queueLock);
        _impl->tasks.emplace(std::move(task));
    }
    _impl->hasTaskEvent.notify_one();
}

void Worker::WorkerThreadFunc(const settings::ZoneSettings& settings) {
    // Initialize the worker context TLS data
    INIT_WORKER_CONTEXT();

    // Zone instance into TLS.
    WorkerContext::Set(WorkerContextItem::ZONE,
                               reinterpret_cast<void*>(NapaZone::Get(settings.id).get()));

    // Worker Id into TLS.
    WorkerContext::Set(WorkerContextItem::WORKER_ID,
                               reinterpret_cast<void*>(static_cast<uintptr_t>(_impl->id)));

    _impl->isolate = CreateIsolate(settings);

    // If any user of v8 library uses a locker on any isolate, all isolates must be locked before use.
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

        {
            std::unique_lock<std::mutex> lock(_impl->queueLock);
            if (_impl->tasks.empty()) {
                _impl->idleNotificationCallback(_impl->id);

                // Wait until new tasks come.
                _impl->hasTaskEvent.wait(lock, [this]() { return !_impl->tasks.empty(); });
            }

            task = _impl->tasks.front();
            _impl->tasks.pop();
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

static v8::Isolate* CreateIsolate(const settings::ZoneSettings& settings) {
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

static void ConfigureIsolate(v8::Isolate* isolate, const settings::ZoneSettings& settings) {
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
