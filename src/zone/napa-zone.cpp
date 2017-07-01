#include "napa-zone.h"

#include <platform/dll.h>
#include <utils/string.h>
#include <zone/eval-task.h>
#include <zone/call-task.h>
#include <zone/call-context.h>
#include <zone/task-decorators.h>

#include <napa-log.h>

#include <boost/filesystem.hpp>

using namespace napa;
using namespace napa::zone;

// Static members initialization
std::mutex NapaZone::_mutex;
std::unordered_map<std::string, std::weak_ptr<NapaZone>> NapaZone::_zones;

std::shared_ptr<NapaZone> NapaZone::Create(const settings::ZoneSettings& settings) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto iter = _zones.find(settings.id);
    if (iter != _zones.end() && !iter->second.expired()) {
        LOG_ERROR("Zone", "Failed to create zone '%s': a zone with this name already exists.", settings.id.c_str());
        return nullptr;
    }

    // An helper class to enable make_shared of NapaZone
    struct MakeSharedEnabler : public NapaZone {
        MakeSharedEnabler(const settings::ZoneSettings& settings) : NapaZone(settings) {}
    };

    std::shared_ptr<NapaZone> zone = std::make_shared<MakeSharedEnabler>(settings);
    _zones[settings.id] = zone;
    try {
        zone->Init();
    } catch (const std::exception& ex) {
        LOG_ERROR("Zone", "Failed to initialize zone '%s': %s", settings.id.c_str(), ex.what());
        return nullptr;
    }
    return zone;
}

std::shared_ptr<NapaZone> NapaZone::Get(const std::string& id) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto iter = _zones.find(id);
    if (iter == _zones.end()) {
        return nullptr;
    }

    auto zone = iter->second.lock();
    if (zone == nullptr) {
        LOG_WARNING("Zone", "Zone '%s' was already deleted.", id.c_str());

        // Use this chance to clean up the map
        _zones.erase(id);
    }

    return zone;
}

NapaZone::NapaZone(const settings::ZoneSettings& settings) : _settings(settings) {
}

/// <summary> Load 'napajs' module during bootstrap. We use relative path to decouple from how module will be published.  </summary>
static const std::string NAPAJS_MODULE_PATH = boost::filesystem::path(dll::ThisLineLocation()).parent_path().parent_path().string();
static const std::string BOOTSTRAP_SOURCE = "require('" + utils::string::ReplaceAllCopy(NAPAJS_MODULE_PATH, "\\", "\\\\") + "');";

void NapaZone::Init() {
    // Create the zone's scheduler.
    _scheduler = std::make_unique<Scheduler>(_settings);

    // Bootstrap after zone is created.
    Broadcast(BOOTSTRAP_SOURCE, [](ResultCode code){ 
        NAPA_ASSERT(code == NAPA_RESULT_SUCCESS, "Bootstrap Napa zone failed.");
    });
}

const std::string& NapaZone::GetId() const {
    return _settings.id;
}

void NapaZone::Broadcast(const std::string& source, BroadcastCallback callback) {
    // Makes sure the callback is only called once, after all workers finished running the broadcast task.
    auto counter = std::make_shared<std::atomic<uint32_t>>(_settings.workers);
    auto callOnce = [callback = std::move(callback), counter](napa_result_code code) {
        if (--(*counter) == 0) {
            callback(code);
        }
    };

    auto broadcastTask = std::make_shared<EvalTask>(source, "", std::move(callOnce));

    _scheduler->ScheduleOnAllWorkers(std::move(broadcastTask));
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

    _scheduler->Schedule(std::move(task));
}

const settings::ZoneSettings& NapaZone::GetSettings() const {
    return _settings;
}

std::shared_ptr<Scheduler> NapaZone::GetScheduler() {
    return _scheduler;
}
