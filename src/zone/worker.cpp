// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "worker.h"

#include <napa/log.h>

#include <platform/dll.h>
#include <platform/filesystem.h>
#include <utils/string.h>
#include <node.h>
#include <v8.h>

#include <atomic>
#include <mutex>

using namespace napa;
using namespace napa::zone;

static std::mutex environment_loading_mutext;

static const std::string NAPA_WORKER_INIT_PATH = 
    utils::string::ReplaceAllCopy(
        filesystem::Path(dll::ThisLineLocation()).Parent().Parent().Normalize().String(), "\\", "\\\\"
    ) + "/lib/zone/napa-worker-init.js";

struct Worker::Impl {

    uv_thread_t tId;

    uv_loop_t loop;

    v8::TaskRunner* foregroundTaskRunner;

    v8::TaskRunner* backgroundTaskRunner;

    /// <summary> The worker id. </summary>
    WorkerId id;

    std::atomic<int> numberOfTasksRunning;

    /// <summary> V8 isolate associated with this worker. </summary>
    v8::Isolate* isolate;

    /// <summary> A callback function to setup the isolate after worker created its isolate. </summary>
    std::function<void(WorkerId, uv_loop_t*)> setupCallback;

    /// <summary> A callback function that is called when worker becomes idle. </summary>
    std::function<void(WorkerId)> idleNotificationCallback;

    std::function<void(WorkerId)> exitCallback;

    /// <summary> The zone settings for the current worker. </summary>
    settings::ZoneSettings settings;
};


class WorkerTask : public v8::Task {
public:
    WorkerTask(std::shared_ptr<napa::zone::Task> napaTask, std::function<void ()> onTaskFinish) :
        _napaTask(napaTask),
        _onTaskFinish(onTaskFinish) {}

    virtual void Run() override {
        try {
            _napaTask->Execute();
        }
        catch(...) {
            _onTaskFinish();
            throw;
        }
        _onTaskFinish();
    }

private:
    std::shared_ptr<napa::zone::Task> _napaTask;
    std::function<void ()> _onTaskFinish;
};


Worker::Worker(WorkerId id,
               const settings::ZoneSettings& settings,
               std::function<void(WorkerId, uv_loop_t*)> setupCallback,
               std::function<void(WorkerId)> idleNotificationCallback,
               std::function<void(WorkerId)> exitCallback)
    : _impl(std::make_unique<Worker::Impl>()) {
    NAPA_ASSERT(uv_loop_init(&_impl->loop) == 0, "Worker (id=%u) failed to initialize its loop.", id);

    _impl->numberOfTasksRunning = 0;
    _impl->id = id;
    _impl->setupCallback = std::move(setupCallback);
    _impl->idleNotificationCallback = std::move(idleNotificationCallback);
    _impl->exitCallback = std::move(exitCallback);
    _impl->settings = settings;
}

Worker::~Worker() {
    // TODO::Stop gracefully.
    NAPA_DEBUG("Worker", "(id=%u) is Shutting down.", _impl->id);

    uv_stop(&_impl->loop);
    uv_thread_join(&_impl->tId);
    uv_loop_close(&_impl->loop);

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

void Worker::Schedule(std::shared_ptr<Task> task) {
    NAPA_ASSERT(task != nullptr, "Task should not be null");
    NAPA_ASSERT(_impl->foregroundTaskRunner != nullptr, "ForegroundTaskRunner should not be null");
    _impl->numberOfTasksRunning++;
    auto workerTask = std::make_unique<WorkerTask>(std::move(task), std::move([this](){
        NAPA_DEBUG("Worker", "Worker (id=%u) finished one task.", _impl->id);
        _impl->numberOfTasksRunning--;
        if (_impl->numberOfTasksRunning == 0) {
            NAPA_DEBUG("Worker", "Worker (id=%u) has no running task, calling idle callback...", _impl->id);
            _impl->idleNotificationCallback(_impl->id);
        }
        else if (_impl->numberOfTasksRunning < 0) {
            throw std::runtime_error("numberOfTaskRunning must not be less than zero!");
        }
    }));
    _impl->foregroundTaskRunner->PostTask(std::move(workerTask));
    NAPA_DEBUG("Worker", "(id=%u) Task queued.", _impl->id);
}

void Worker::WorkerThreadFunc(const settings::ZoneSettings& settings) {
    // Create Isolate.
    v8::Isolate::CreateParams params;
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(params);
    NAPA_ASSERT(isolate, "Failed to create v8 isolate for worker.");
    _impl->isolate = isolate;

    {
        // Get MultiIsolatePlatform.
        node::MultiIsolatePlatform* multiIsolatePlatform = node::GetMainThreadMultiIsolatePlatform();
        NAPA_ASSERT(multiIsolatePlatform, "Node MultiIsolatePlatform must exist.");

        // Create IsolateData.
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        node::IsolateData* isolate_data = node::CreateIsolateData(isolate, &_impl->loop, multiIsolatePlatform);

        // Napa releted setting.
        _impl->foregroundTaskRunner = multiIsolatePlatform->GetForegroundTaskRunner(isolate).get();
        _impl->backgroundTaskRunner = multiIsolatePlatform->GetBackgroundTaskRunner(isolate).get();
        _impl->setupCallback(_impl->id, &_impl->loop);
        _impl->idleNotificationCallback(_impl->id);
        NAPA_DEBUG("Worker", "(id=%u) Setup completed.", _impl->id);

        // Create Context.
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);

        // Create node::Environment.
        const char* worker_argv[4];
        worker_argv[0] = "node";
        worker_argv[1] = NAPA_WORKER_INIT_PATH.c_str();
        // zone id.`
        worker_argv[2] = settings.id.c_str();
        // worker id.
        auto workId = std::to_string(_impl->id);
        worker_argv[3] = workId.c_str();
        node::Environment* env = node::CreateEnvironment(isolate_data, context, 4, worker_argv, 0, nullptr);

        // Problem: it would impact relevant execution behavior because,
        // 1. this logic couples with node bootstrapping logic,
        // 2. the flag is a setting shared at process level.
        // TODO : Re-evaluate the impact and figure out a solid solution at node.js / napa.js.
        // node::LoadEnvironment need access to V8 intrinsics by flag '--allow_natives_syntax'.
        // If the flag haven't been specified when launching node.js,
        // it will be disabled again during node bootstrapping.
        {
            std::lock_guard<std::mutex> lock(environment_loading_mutext);
            const char allow_natives_syntax[] = "--allow_natives_syntax";
            v8::V8::SetFlagsFromString(allow_natives_syntax, sizeof(allow_natives_syntax) - 1);
            node::LoadEnvironment(env);
        }

        // Run uv loop.
        {
            v8::SealHandleScope seal(isolate);
            bool more;
            do {
              uv_run(&_impl->loop, UV_RUN_DEFAULT);
              multiIsolatePlatform->DrainBackgroundTasks(isolate);
              more = uv_loop_alive(&_impl->loop);
              if (more) continue;
              node::EmitBeforeExit(env);
              more = uv_loop_alive(&_impl->loop);
            } while (more);
        }

        node::RunAtExit(env);
        multiIsolatePlatform->DrainBackgroundTasks(isolate);
        multiIsolatePlatform->CancelPendingDelayedTasks(isolate);
        
        node::FreeEnvironment(env);
        node::FreeIsolateData(isolate_data);
    }

    isolate->Dispose();
    _impl->exitCallback(_impl->id);
}
