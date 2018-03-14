// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "worker.h"

#include <utils/debug.h>

#include <napa/log.h>

#include <v8.h>

#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <thread>

#include <v8-extensions/v8-extensions-macros.h>
#if !(V8_VERSION_CHECK_FOR_ARRAY_BUFFER_ALLOCATOR)
    #include <v8-extensions/array-buffer-allocator.h>
#endif

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

    /// <summary> Queue for tasks scheduled on this worker. </summary>
    std::queue<std::shared_ptr<Task>> immediateTasks;
    
    /// <summary> Condition variable to indicate if there are more tasks to consume. </summary>
    std::condition_variable hasTaskEvent;

    /// <summary> Lock for task queue and immediate task queue. </summary>
    std::mutex queueLock;

    /// <summary> V8 isolate associated with this worker. </summary>
    v8::Isolate* isolate;

    /// <summary> A callback function to setup the isolate after worker created its isolate. </summary>
    std::function<void(WorkerId)> setupCallback;

    /// <summary> A callback function that is called when worker becomes idle. </summary>
    std::function<void(WorkerId)> idleNotificationCallback;

    /// <summary> The zone settings for the current worker. </summary>
    settings::ZoneSettings settings;
};

Worker::Worker(WorkerId id,
               const settings::ZoneSettings& settings,
               std::function<void(WorkerId)> setupCallback,
               std::function<void(WorkerId)> idleNotificationCallback)
    : _impl(std::make_unique<Worker::Impl>()) {

    _impl->id = id;
    _impl->setupCallback = std::move(setupCallback);
    _impl->idleNotificationCallback = std::move(idleNotificationCallback);
    _impl->settings = settings;
}

Worker::~Worker() {
    // Signal the thread loop that it should stop processing tasks.
    Enqueue(nullptr, SchedulePhase::DefaultPhase);
    NAPA_DEBUG("Worker", "(id=%u) Shutting down: Start draining task queue.", _impl->id);
    
    _impl->workerThread.join();

    if (_impl->isolate != nullptr) {
        _impl->isolate->Dispose();
    }
    NAPA_DEBUG("Worker", "(id=%u) Shutdown complete.", _impl->id);
}

Worker::Worker(Worker&&) = default;
Worker& Worker::operator=(Worker&&) = default;

void Worker::Start() {
    _impl->workerThread = std::thread(&Worker::WorkerThreadFunc, this, _impl->settings);
}

void Worker::Schedule(std::shared_ptr<Task> task, SchedulePhase phase) {
    NAPA_ASSERT(task != nullptr, "Task should not be null");
    Enqueue(task, phase);
    NAPA_DEBUG("Worker", "(id=%u) Task queued.", _impl->id);
}

void Worker::Enqueue(std::shared_ptr<Task> task, SchedulePhase phase) {
    {
        std::unique_lock<std::mutex> lock(_impl->queueLock);
        if (phase == SchedulePhase::ImmediatePhase && task != nullptr) {
            _impl->immediateTasks.emplace(std::move(task));
        }
        else {
            _impl->tasks.emplace(std::move(task));
        }
    }
    _impl->hasTaskEvent.notify_one();
}

void Worker::WorkerThreadFunc(const settings::ZoneSettings& settings) {
    
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

    NAPA_DEBUG("Worker", "(id=%u) V8 Isolate created.", _impl->id);

    // Setup worker after isolate creation.
    _impl->setupCallback(_impl->id);
    NAPA_DEBUG("Worker", "(id=%u) Setup completed.", _impl->id);

    while (true) {
        std::shared_ptr<Task> task;

        {
            // Logically one merged task queue is the concatenation of immediate queue and
            // the normal queue. The logically merged queue is treated as empty only when both queues
            // are empty. And when not empty, immediate tasks will be handled prior to normal tasks.
            // Inside each single queue (immediate or normal), tasks are first in first out.
            std::unique_lock<std::mutex> lock(_impl->queueLock);
            if (_impl->tasks.empty() && _impl->immediateTasks.empty()) {
                _impl->idleNotificationCallback(_impl->id);

                // Wait until new tasks come.
                _impl->hasTaskEvent.wait(
                    lock, 
                    [this]() { return !(_impl->tasks.empty() && _impl->immediateTasks.empty()); });
            }

            if (_impl->immediateTasks.empty()) {
                task = _impl->tasks.front();
                _impl->tasks.pop();
            }
            else {
                task = _impl->immediateTasks.front();
                _impl->immediateTasks.pop();
            }
        }

        // A null task means that the worker needs to shutdown.
        if (task == nullptr) {
            NAPA_DEBUG("Worker", "(id=%u) Finish serving tasks.", _impl->id);
            break;
        }

        // Resume execution capabilities if isolate was previously terminated.
        _impl->isolate->CancelTerminateExecution();

        task->Execute();
    }
}

static v8::Isolate* CreateIsolate(const settings::ZoneSettings& settings) {
    v8::Isolate::CreateParams createParams;

    // The allocator is a global V8 setting.
#if V8_VERSION_CHECK_FOR_ARRAY_BUFFER_ALLOCATOR
    static std::unique_ptr<v8::ArrayBuffer::Allocator> defaultArrayBufferAllocator(v8::ArrayBuffer::Allocator::NewDefaultAllocator());
    createParams.array_buffer_allocator = defaultArrayBufferAllocator.get();
#else
    static napa::v8_extensions::ArrayBufferAllocator commonAllocator;
    createParams.array_buffer_allocator = &commonAllocator;
#endif

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
