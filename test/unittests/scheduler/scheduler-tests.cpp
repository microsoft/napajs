#include "catch.hpp"

#include "scheduler/scheduler.h"

#include <atomic>
#include <future>

using namespace napa;
using namespace napa::scheduler;


class TestTask : public Task {
public:
    TestTask(std::function<void()> callback = []() {}) : 
        numberOfExecutions(0),
        lastExecutedCoreId(99),
        _callback(std::move(callback)) {}

    void SetCurrentCoreId(CoreId id) {
        lastExecutedCoreId = id;
    }

    virtual void Execute() override
    {
        numberOfExecutions++;
        _callback();
    }

    std::atomic<uint32_t> numberOfExecutions;
    std::atomic<CoreId> lastExecutedCoreId;

private:
    std::function<void()> _callback;
};


template <uint32_t I>
class TestCore {
public:

    TestCore(CoreId id, const Settings &settings) : _id(id) {
        numberOfCores++;
    }

    ~TestCore() {
        for (auto& fut : _futures) {
            fut.get();
        }
    }

    void Schedule(std::shared_ptr<Task> task) {
        auto testTask = std::dynamic_pointer_cast<TestTask>(task);
        testTask->SetCurrentCoreId(_id);

        _futures.emplace_back(std::async(std::launch::async, [this, task]() {
            task->Execute();
            _idleNotificationCallback(_id);
        }));
    }

    void SubscribeForIdleNotifications(std::function<void(CoreId)> callback) {
        _idleNotificationCallback = std::move(callback);
    }

    static uint32_t numberOfCores;

private:
    CoreId _id;
    std::vector<std::shared_future<void>> _futures;
    std::function<void(CoreId)> _idleNotificationCallback;
};

template <uint32_t I>
uint32_t TestCore<I>::numberOfCores = 0;


TEST_CASE("scheduler creates correct number of cores", "[scheduler]") {
    Settings settings;
    settings.cores = 3;

    auto scehduler = std::make_unique<SchedulerImpl<TestCore<1>>>(settings);

    REQUIRE(TestCore<1>::numberOfCores == settings.cores);
}

TEST_CASE("scheduler assigns tasks correctly", "[scheduler]") {
    Settings settings;
    settings.cores = 3;

    auto scehduler = std::make_unique<SchedulerImpl<TestCore<2>>>(settings);
    auto task = std::make_shared<TestTask>();

    SECTION("schedules on exactly one core") {
        scehduler->Schedule(task);
        scehduler = nullptr; // force draining all scheduled tasks

        REQUIRE(task->numberOfExecutions == 1);
    }

    SECTION("schedule on a specific core") {
        scehduler->ScheduleOnCore(2, task);
        scehduler = nullptr; // force draining all scheduled tasks

        REQUIRE(task->numberOfExecutions == 1);
        REQUIRE(task->lastExecutedCoreId == 2);
    }

    SECTION("schedule on all cores") {
        scehduler->ScheduleOnAllCores(task);
        scehduler = nullptr; // force draining all scheduled tasks

        REQUIRE(task->numberOfExecutions == settings.cores);
    }
}

TEST_CASE("scheduler distributes and schedules all tasks", "[scheduler]") {
    Settings settings;
    settings.cores = 4;

    auto scehduler = std::make_unique<SchedulerImpl<TestCore<3>>>(settings);

    std::vector<std::shared_ptr<TestTask>> tasks;
    for (size_t i = 0; i < 1000; i++) {
        auto task = std::make_shared<TestTask>();
        tasks.push_back(task);
        scehduler->Schedule(task);
    }

    scehduler = nullptr; // force draining all scheduled tasks

    std::vector<bool> scheduledCoresFlags = { false, false, false, false };
    for (size_t i = 0; i < 1000; i++) {
        // Make sure that each task was executed once
        REQUIRE(tasks[i]->numberOfExecutions == 1);
        scheduledCoresFlags[tasks[i]->lastExecutedCoreId] = true;
    }

    // Make sure that all cores were participating
    for (auto flag: scheduledCoresFlags) {
        REQUIRE(flag);
    }
}
