#pragma once

#include "core.h"
#include "settings/settings.h"
#include "simple-thread-pool.h"
#include "task.h"

#include <assert.h>
#include <atomic>
#include <list>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

namespace napa {
namespace scheduler {

    /// <summary> The scheduler is responsible for assigning tasks to cores. </summary>
    template <typename CoreType>
    class SchedulerImpl {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="settings"> A settings object. </param>
        SchedulerImpl(const Settings& settings);

        /// <summary> Destructor. Waits for all tasks to finish. </summary>
        ~SchedulerImpl();

        /// <summary> Schedules the task on a single core. </summary>
        /// <param name="task"> Task to schedule. </param>
        void Schedule(std::shared_ptr<Task> task);

        /// <summary> Schedules the task on a specific core. </summary>
        /// <param name="coreId"> The id of the core. </param>
        /// <param name="task"> Task to schedule. </param>
        void ScheduleOnCore(CoreId coreId, std::shared_ptr<Task> task);

        /// <summary> Schedules the task on all cores. </summary>
        /// <param name="task"> Task to schedule. </param>
        void ScheduleOnAllCores(std::shared_ptr<Task> task);

    private:

        void IdleCoreNotificationCallback(CoreId coreId);

        /// <summary> The cores that are used for running the tasks. </summary>
        std::vector<CoreType> _cores;

        /// <summary> New tasks that weren't assigned to a specific core. </summary>
        std::queue<std::shared_ptr<Task>> _nonScheduledTasks;

        /// <summary> List of idle cores, used when assigning non scheduled tasks. </summary>
        std::list<CoreId> _idleCores;

        /// <summary> Flags to indicate that a core is in the idle list. </summary>
        std::vector<std::list<CoreId>::iterator> _idleCoresFlags;

        /// <summary> Uses a single thread to synchronize task queuing and posting. </summary>
        std::unique_ptr<SimpleThreadPool> _synchronizer;

        /// <summary> A flag to signal that scheduler is stopping. </summary>
        std::atomic<bool> _shouldStop;
    };

    typedef SchedulerImpl<Core> Scheduler;

    template <typename CoreType>
    SchedulerImpl<CoreType>::SchedulerImpl(const Settings& settings) :
            _idleCoresFlags(settings.cores),
            _synchronizer(std::make_unique<SimpleThreadPool>(1)),
            _shouldStop(false) {
        _cores.reserve(settings.cores);

        for (CoreId i = 0; i < settings.cores; i++) {
            _cores.emplace_back(i, settings);

            // Register to recieve idle notifications from cores
            _cores[i].SubscribeForIdleNotifications([this](CoreId coreId) {
                IdleCoreNotificationCallback(coreId);
            });

            // All cores are idle initially.
            auto iter = _idleCores.emplace(_idleCores.end(), i);
            _idleCoresFlags[i] = iter;
        }
    }

    template <typename CoreType>
    SchedulerImpl<CoreType>::~SchedulerImpl() {
        // Wait for all tasks to be scheduled.
        while (_nonScheduledTasks.size() > 0) {
            std::this_thread::yield();
        }

        // Signal scheduler callbacks to not process anymore tasks.
        _shouldStop = true;

        // Wait for synchornizer to finish his book-keeping.
        _synchronizer = nullptr;

        // Wait for all cores to finish processing remaining tasks.
        _cores.clear();
    }

    template <typename CoreType>
    void SchedulerImpl<CoreType>::Schedule(std::shared_ptr<Task> task) {
        assert(task != nullptr);
        
        _synchronizer->Execute([this, task]() {
            if (_idleCores.empty()) {
                // If there is no idle core, put the task into the non-scheduled queue.
                _nonScheduledTasks.emplace(std::move(task));
            } else {
                // Pop the core id from the idle cores list.
                auto coreId = _idleCores.front();
                _idleCores.pop_front();
                _idleCoresFlags[coreId] = _idleCores.end();

                // Schedule task on core
                _cores[coreId].Schedule(std::move(task));
            }
        });
    }

    template <typename CoreType>
    void SchedulerImpl<CoreType>::ScheduleOnCore(CoreId coreId, std::shared_ptr<Task> task) {
        assert(coreId < _cores.size());

        _synchronizer->Execute([coreId, this, task]() {
            // If the core is idle, change it's status.
            if (_idleCoresFlags[coreId] != _idleCores.end()) {
                _idleCores.erase(_idleCoresFlags[coreId]);
                _idleCoresFlags[coreId] = _idleCores.end();
            }

            // Schedule task on core
            _cores[coreId].Schedule(std::move(task));
        });
    }

    template <typename CoreType>
    void SchedulerImpl<CoreType>::ScheduleOnAllCores(std::shared_ptr<Task> task) {
        assert(task != nullptr);

        _synchronizer->Execute([this, task]() {
            // Clear all idle cores.
            _idleCores.clear();
            for (auto& flag : _idleCoresFlags) {
                flag = _idleCores.end();
            }

            // Schedule the task on all cores.
            for (auto& core : _cores) {
                core.Schedule(task);
            }
        });
    }

    template <typename CoreType>
    void SchedulerImpl<CoreType>::IdleCoreNotificationCallback(CoreId coreId) {
        assert(coreId < _cores.size());
        
        if (_shouldStop) {
            return;
        }

        _synchronizer->Execute([this, coreId]() {
            if (!_nonScheduledTasks.empty()) {
                // If there is a non scheduled task, schedule it on the idle core.
                auto task = _nonScheduledTasks.front();
                _nonScheduledTasks.pop();

                _cores[coreId].Schedule(std::move(task));
            } else {
                // Put core in idle list.
                if (_idleCoresFlags[coreId] == _idleCores.end()) {
                    auto iter = _idleCores.emplace(_idleCores.end(), coreId);
                    _idleCoresFlags[coreId] = iter;
                }
            }
        });
    }
}
}
