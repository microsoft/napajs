// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "napa-zone.h"

#include <module/loader/module-loader.h>
#include <platform/dll.h>
#include <platform/filesystem.h>
#include <utils/debug.h>
#include <utils/string.h>
#include <zone/eval-task.h>
#include <zone/call-task.h>
#include <zone/call-context.h>
#include <zone/task-decorators.h>
#include <zone/worker-context.h>

#include <napa/log.h>

#include <future>

using namespace napa;
using namespace napa::zone;

// Static members initialization
std::mutex NapaZone::_mutex;
std::unordered_map<std::string, std::weak_ptr<NapaZone>> NapaZone::_zones;

/// <summary> Load 'napajs' module during bootstrap. We use relative path to decouple from how module will be published.  </summary>
static const std::string NAPAJS_MODULE_PATH = filesystem::Path(dll::ThisLineLocation()).Parent().Parent().Normalize().String();
static const std::string BOOTSTRAP_SOURCE = "require('" + utils::string::ReplaceAllCopy(NAPAJS_MODULE_PATH, "\\", "\\\\") + "');";

std::shared_ptr<NapaZone> NapaZone::Create(const settings::ZoneSettings& settings) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto iter = _zones.find(settings.id);
    if (iter != _zones.end() && !iter->second.expired()) {
        NAPA_DEBUG("Zone", "Failed to create zone '%s': a zone with this name already exists.", settings.id.c_str());
        return nullptr;
    }

    // An helper class to enable make_shared of NapaZone
    struct MakeSharedEnabler : public NapaZone {
        MakeSharedEnabler(const settings::ZoneSettings& settings) : NapaZone(settings) {}
    };

    // Fail to create Napa zone is not expected, will always trigger crash.
    auto zone = std::make_shared<MakeSharedEnabler>(settings);
    _zones[settings.id] = zone;

    NAPA_DEBUG("Zone", "Napa zone \"%s\" created.", settings.id.c_str());

    return zone;
}

std::shared_ptr<NapaZone> NapaZone::Get(const std::string& id) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto iter = _zones.find(id);
    if (iter == _zones.end()) {
        NAPA_DEBUG("Zone", "Get zone \"%s\" failed due to not found.", id.c_str());
        return nullptr;
    }

    auto zone = iter->second.lock();
    if (zone == nullptr) {
        LOG_WARNING("Zone", "Zone '%s' was already deleted.", id.c_str());

        // Use this chance to clean up the map
        _zones.erase(id);
    }
    
    NAPA_DEBUG("Zone", "Get zone \"%s\" succeeded.", id.c_str());
    return zone;
}

NapaZone::NapaZone(const settings::ZoneSettings& settings) : 
    _settings(settings) {

    // Create the zone's scheduler.
    _scheduler = std::make_unique<Scheduler>(
        _settings,
        [this](WorkerId id,
               v8::TaskRunner* foregroundTaskRunner,
               v8::TaskRunner* backgroundTaskRunner) {
        // Initialize the worker context TLS data
        INIT_WORKER_CONTEXT();

        // Zone instance into TLS.
        WorkerContext::Set(WorkerContextItem::ZONE, reinterpret_cast<void*>(this));

        // Worker Id into TLS.
        WorkerContext::Set(WorkerContextItem::WORKER_ID, reinterpret_cast<void*>(static_cast<uintptr_t>(id)));

        // Set foreground task runer to TLS. </summary>
        WorkerContext::Set(WorkerContextItem::FOREGROUND_TASK_RUNNER,
                        reinterpret_cast<void*>(foregroundTaskRunner));

        // Set background task runer to TLS. </summary>
        WorkerContext::Set(WorkerContextItem::BACKGROUND_TASK_RUNNER,
                        reinterpret_cast<void*>(foregroundTaskRunner));
        });
}

const std::string& NapaZone::GetId() const {
    return _settings.id;
}

void NapaZone::Broadcast(const std::string& source, BroadcastCallback callback) {
    // Makes sure the callback is only called once, after all workers finished running the broadcast task.
    auto counter = std::make_shared<std::atomic<uint32_t>>(_settings.workers);
    auto callOnce = [this, source, callback = std::move(callback), counter](napa_result_code code) {
        if (--(*counter) == 0) {
            NAPA_DEBUG("Zone", "Finishing broadcast script \"%s\" to zone \"%s\"", source.c_str(), _settings.id.c_str());
            callback(code);
        }
    };

    auto broadcastTask = std::make_shared<EvalTask>(source, "", std::move(callOnce));

    _scheduler->ScheduleOnAllWorkers(std::move(broadcastTask));
    NAPA_DEBUG("Zone", "Scheduling broadcast script \"%s\" to zone \"%s\"", source.c_str(), _settings.id.c_str());
}

void NapaZone::Execute(const FunctionSpec& spec, ExecuteCallback callback) {
    std::shared_ptr<Task> task;

    if (spec.options.timeout > 0) {
        task = std::make_shared<TimeoutTaskDecorator<CallTask>>(
            std::chrono::milliseconds(spec.options.timeout),
            std::make_shared<CallContext>(spec, std::move(callback)));
    } else {
        task = std::make_shared<CallTask>(std::make_shared<CallContext>(spec, std::move(callback)));
    }
    
    NAPA_DEBUG("Zone", "Execute function \"%s.%s\" on zone \"%s\"", spec.module.data, spec.function.data, _settings.id.c_str());
    _scheduler->Schedule(std::move(task));
}

const settings::ZoneSettings& NapaZone::GetSettings() const {
    return _settings;
}

std::shared_ptr<Scheduler> NapaZone::GetScheduler() {
    return _scheduler;
}
