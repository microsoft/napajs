// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "worker.h"

#include <utils/debug.h>

#include <napa/log.h>

#include <v8.h>
#include <platform/dll.h>
#include <platform/filesystem.h>
#include <utils/string.h>
#include <node.h>

#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <atomic>

#include <v8-extensions/v8-extensions-macros.h>
#if !(V8_VERSION_CHECK_FOR_ARRAY_BUFFER_ALLOCATOR)
    #include <v8-extensions/array-buffer-allocator.h>
#endif

using namespace napa;
using namespace napa::zone;

static const std::string NAPA_WORKER_INIT_PATH = 
    utils::string::ReplaceAllCopy(
        filesystem::Path(dll::ThisLineLocation()).Parent().Parent().Normalize().String(), "\\", "\\\\"
    ) + "/lib/zone/napa-worker-init.js";
// Forward declaration
static v8::Isolate* CreateIsolate(const settings::ZoneSettings& settings);
static void ConfigureIsolate(v8::Isolate* isolate, const settings::ZoneSettings& settings);

struct Worker::Impl {

    uv_thread_t tId;

    uv_loop_t loop;

    v8::TaskRunner* foregroundTaskRunner;

    v8::TaskRunner* backgroundTaskRunner;

    /// <summary> The worker id. </summary>
    WorkerId id;

    std::atomic<int> numberOfTasksRunning;

    /// <summary> The thread that executes the tasks. </summary>
    // std::thread workerThread;

    /// <summary> Queue for tasks scheduled on this worker. </summary>
    // std::queue<std::shared_ptr<Task>> tasks;
    
    /// <summary> Condition variable to indicate if there are more tasks to consume. </summary>
    // std::condition_variable hasTaskEvent;

    /// <summary> Lock for task queue. </summary>
    // std::mutex queueLock;

    /// <summary> V8 isolate associated with this worker. </summary>
    v8::Isolate* isolate;

    /// <summary> A callback function to setup the isolate after worker created its isolate. </summary>
    std::function<void(WorkerId, v8::TaskRunner*, v8::TaskRunner*)> setupCallback;

    /// <summary> A callback function that is called when worker becomes idle. </summary>
    std::function<void(WorkerId)> idleNotificationCallback;

    /// <summary> The zone settings for the current worker. </summary>
    settings::ZoneSettings settings;
};

Worker::Worker(WorkerId id,
               const settings::ZoneSettings& settings,
               std::function<void(WorkerId, v8::TaskRunner*, v8::TaskRunner*)> setupCallback,
               std::function<void(WorkerId)> idleNotificationCallback)
    : _impl(std::make_unique<Worker::Impl>()) {
    NAPA_ASSERT(uv_loop_init(&_impl->loop) == 0, "Worker (id=%u) failed to initialize its loop.", id);

    _impl->numberOfTasksRunning = 0;
    _impl->id = id;
    _impl->setupCallback = std::move(setupCallback);
    _impl->idleNotificationCallback = std::move(idleNotificationCallback);
    _impl->settings = settings;
}

Worker::~Worker() {
    // TODO::Stop gracefully.
    // Signal the thread loop that it should stop processing tasks.
    // Enqueue(nullptr);
    NAPA_DEBUG("Worker", "(id=%u) Shutting down: Start draining task queue.", _impl->id);

    uv_stop(&_impl->loop);
    uv_thread_join(&_impl->tId);
    uv_loop_close(&_impl->loop);

    if (_impl->isolate != nullptr) {
        _impl->isolate->Dispose();
    }
    NAPA_DEBUG("Worker", "(id=%u) Shutdown complete.", _impl->id);
}

Worker::Worker(Worker&&) = default;
Worker& Worker::operator=(Worker&&) = default;

void Worker::Start() {
    int result = uv_thread_create(&_impl->tId, [](void* arg){
        Worker* worker = static_cast<Worker*>(arg);
        worker->WorkerThreadFunc(worker->_impl->settings);
    }, static_cast<void*>(this));
    NAPA_ASSERT(result == 0, "Worker (id=%u) failed to start.", _impl->id);
}

WorkerId Worker::GetWorkerId() const {
    return _impl->id;
}

void Worker::OnTaskFinish() {
    NAPA_DEBUG("Worker", "Worker (id=%u) finished one task.", _impl->id);
    _impl->numberOfTasksRunning--;
    if (_impl->numberOfTasksRunning == 0) {
        NAPA_DEBUG("Worker", "Worker (id=%u) has no running task, calling idle callback...", _impl->id);
        _impl->idleNotificationCallback(_impl->id);
    }
    else if (_impl->numberOfTasksRunning < 0) {
        throw std::runtime_error("numberOfTaskRunning must not be less than zero!");
    }
}

class WorkerTask : public v8::Task {
public:
    WorkerTask(std::shared_ptr<napa::zone::Task> napaTask, napa::zone::Worker& napaWorker);
    virtual void Run() override;
private:
    std::shared_ptr<napa::zone::Task> _napaTask;
    napa::zone::Worker& _napaWorker;
};

WorkerTask::WorkerTask(std::shared_ptr<napa::zone::Task> napaTask, napa::zone::Worker& napaWorker) :
    _napaTask(napaTask),
    _napaWorker(napaWorker) {
}

void WorkerTask::Run() {
    try {
        _napaTask->Execute();
    }
    catch(...) {
        _napaWorker.OnTaskFinish();
        throw;  
    }
    _napaWorker.OnTaskFinish();
}

void Worker::Schedule(std::shared_ptr<Task> task) {
    NAPA_ASSERT(task != nullptr, "Task should not be null");
    NAPA_ASSERT(_impl->foregroundTaskRunner != nullptr, "ForegroundTaskRunner should not be null");
    _impl->numberOfTasksRunning++;
    auto workerTask = std::make_unique<WorkerTask>(std::move(task), *this);
    _impl->foregroundTaskRunner->PostTask(std::move(workerTask));
    NAPA_DEBUG("Worker", "(id=%u) Task queued.", _impl->id);
}

void Worker::Enqueue(std::shared_ptr<Task> task) {
}

void Worker::WorkerThreadFunc(const settings::ZoneSettings& settings) {
    int main_argc;
    char** main_argv;
    int main_exec_argc;
    const char** main_exec_argv;
    node::GetNodeMainArgments(main_argc, main_argv, main_exec_argc, main_exec_argv);

    NAPA_DEBUG("Worker", "(id=%u) Setup completed.", _impl->id);

    const char* worker_argv[4];
    worker_argv[0] = "node";
    worker_argv[1] = NAPA_WORKER_INIT_PATH.c_str();
    // zone id.
    worker_argv[2] = settings.id.c_str();
    // worker id.
    auto workId = std::to_string(_impl->id);
    worker_argv[3] = workId.c_str();

    node::Start(static_cast<void*>(&_impl->loop),
                4, worker_argv, main_exec_argc, main_exec_argv, false,
                [this](v8::TaskRunner* foregroundTaskRunner, v8::TaskRunner* backgroundTaskRunner){
                    NAPA_ASSERT(foregroundTaskRunner != nullptr, "Foreground task runner should not be null");
                    NAPA_ASSERT(backgroundTaskRunner != nullptr, "Background task runner should not be null");
                    // Setup worker after isolate creation.
                    _impl->foregroundTaskRunner = foregroundTaskRunner;
                    _impl->backgroundTaskRunner = backgroundTaskRunner;
                    _impl->setupCallback(_impl->id, _impl->foregroundTaskRunner, _impl->backgroundTaskRunner);
                    _impl->idleNotificationCallback(_impl->id);
                });

}