#include "zone-impl.h"

#include "scheduler/broadcast-task.h"
#include "scheduler/execute-task.h"
#include "scheduler/task-decorators.h"

#include <napa-log.h>

#include <boost/dll.hpp>
#include <boost/filesystem.hpp>

#include <sstream>

using namespace napa;
using namespace napa::scheduler;

// Forward declarations
static void BroadcastFromFile(const std::string& file, Scheduler& scheduler);

// Static members initialization
std::mutex ZoneImpl::_mutex;
std::unordered_map<std::string, std::weak_ptr<ZoneImpl>> ZoneImpl::_zones;

// The path to the file containing the execute main function
static const std::string ZONE_MAIN_FILE = (boost::dll::this_line_location().parent_path().parent_path() /
    "lib\\runtime\\zone-main.js").string();

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

    std::shared_ptr<ZoneImpl> zone = std::make_shared<MakeSharedEnabler>(settings);
    _zones[settings.id] = zone;
    try {
        zone->Init();
    } catch (const std::exception& ex) {
        LOG_ERROR("Zone", "Failed to initialize zone '%s': %s", settings.id.c_str(), ex.what());
        return nullptr;
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
}

void ZoneImpl::Init() {
    // Create the zone's scheduler.
    _scheduler = std::make_unique<Scheduler>(_settings);

    // Read zone main file content and broadcast it on all workers.
    BroadcastFromFile(ZONE_MAIN_FILE, *_scheduler);
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

std::shared_ptr<Scheduler> ZoneImpl::GetScheduler() {
    return _scheduler;
}

static void BroadcastFromFile(const std::string& file, Scheduler& scheduler) {
    auto filePath = boost::filesystem::path(file);
    if (filePath.is_relative()) {
        filePath = (boost::filesystem::current_path() / filePath).normalize().make_preferred();
    }

    auto filePathString = filePath.string();
    std::ifstream ifs;
    ifs.open(filePathString);

    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePathString);
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();

    auto fileContent = buffer.str();
    if (fileContent.empty()) {
        throw std::runtime_error("File content was empty: " + filePathString);
    }

    scheduler.ScheduleOnAllWorkers(std::make_shared<BroadcastTask>(std::move(fileContent)));
}
