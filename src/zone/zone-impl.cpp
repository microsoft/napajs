#include "zone-impl.h"

#include "scheduler/broadcast-task.h"
#include "scheduler/execute-task.h"
#include "scheduler/task-decorators.h"

#include <napa-log.h>

#include <boost/filesystem.hpp>

#include <sstream>

using namespace napa;
using namespace napa::scheduler;

// Static members initialization
std::mutex ZoneImpl::_mutex;
std::unordered_map<std::string, std::weak_ptr<ZoneImpl>> ZoneImpl::_zones;

static const char* _dispatchFunction =
    "function __napa_execute_dispatcher__(module, func, args) {\n"
    "    var f = require(module)[func];\n"
    "    if (!f) throw new Error(\"Cannot find function '\" + func + \"' in module '\" + module + \"'\");\n"
    "    return f.apply(this, args);\n"
    "}";

std::shared_ptr<ZoneImpl> ZoneImpl::Create(const ZoneSettings& settings) {
    std::lock_guard<std::mutex> lock(_mutex);

    auto iter = _zones.find(settings.id);
    if (iter != _zones.end() && !iter->second.expired()) {
        LOG_ERROR("Zone", "Failed to create zone '%s': a zone with this name already exists.", settings.id.c_str());
        return nullptr;
    }

    // An helper class to enable make_shared of ZoneImpl
    struct MakeSharedEnabler : public ZoneImpl {
        MakeSharedEnabler(const ZoneSettings& settings) : ZoneImpl(settings) {}
    };

    std::shared_ptr<ZoneImpl> zone;
    try {
        zone = std::make_shared<MakeSharedEnabler>(settings);
        _zones[settings.id] = zone;
    } catch (const std::exception& ex) {
        LOG_ERROR("Zone", "Failed to create zone '%s': %s", settings.id.c_str(), ex.what());
    }

    return zone;
}

std::shared_ptr<ZoneImpl> ZoneImpl::Get(const std::string& id) {
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

ZoneImpl::ZoneImpl(const ZoneSettings& settings) : _settings(settings) {
    // Create the zone's scheduler.
    _scheduler = std::make_unique<Scheduler>(_settings);

    // Set the dispatcher function on all zone workers.
    _scheduler->ScheduleOnAllWorkers(std::make_shared<BroadcastTask>(_dispatchFunction));

    // Read bootstrap file content and broadcast it on all workers.
    if (!_settings.bootstrapFile.empty()) {
        auto filePath = boost::filesystem::path(_settings.bootstrapFile);
        if (filePath.is_relative()) {
            filePath = (boost::filesystem::current_path() / filePath).normalize().make_preferred();
        }

        auto filePathString = filePath.string();
        std::ifstream ifs;
        ifs.open(filePathString);

        if (!ifs.is_open()) {
            throw std::runtime_error("Failed to open bootstrap file: " + filePathString);
        }

        std::stringstream buffer;
        buffer << ifs.rdbuf();

        auto fileContent = buffer.str();
        if (fileContent.empty()) {
            throw std::runtime_error("Bootstrap file content was empty: " + filePathString);
        }

        _scheduler->ScheduleOnAllWorkers(std::make_shared<BroadcastTask>(std::move(fileContent), filePath.string()));
    }
}

const std::string& ZoneImpl::GetId() const {
    return _settings.id;
}

void ZoneImpl::Broadcast(const std::string& source, BroadcastCallback callback) {
    // Makes sure the callback is only called once, after all workers finished running the broadcast task.
    auto counter = std::make_shared<std::atomic<uint32_t>>(_settings.workers);
    auto callOnce = [callback = std::move(callback), counter](napa_response_code code) {
        if (--(*counter) == 0) {
            callback(code);
        }
    };

    auto broadcastTask = std::make_shared<BroadcastTask>(source, "", std::move(callOnce));

    _scheduler->ScheduleOnAllWorkers(std::move(broadcastTask));
}

void ZoneImpl::Execute(const ExecuteRequest& request, ExecuteCallback callback) {
    std::shared_ptr<Task> task;

    if (request.timeout > 0) {
        task = std::make_shared<TimeoutTaskDecorator<ExecuteTask>>(
            std::chrono::milliseconds(request.timeout),
            request,
            std::move(callback));
    } else {
        task = std::make_shared<ExecuteTask>(request, std::move(callback));
    }

    _scheduler->Schedule(std::move(task));
}

const ZoneSettings& ZoneImpl::GetSettings() const {
    return _settings;
}

Scheduler* ZoneImpl::GetScheduler() {
    return _scheduler.get();
}