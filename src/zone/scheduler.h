// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "simple-thread-pool.h"
#include "task.h"
#include "worker.h"

#include <settings/settings.h>
#include <utils/debug.h>

#include <napa/log.h>

#include <atomic>
#include <list>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

namespace napa {
namespace zone {

    /// <summary> The scheduler is responsible for assigning tasks to workers. </summary>
    template <typename WorkerType>
    class SchedulerImpl {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="settings"> A settings object. </param>
        /// <param name="workerSetupCallback"> Callback to setup the isolate after worker created its isolate. </param>
        SchedulerImpl(const settings::ZoneSettings& settings, std::function<void(WorkerId)> workerSetupCallback);

        /// <summary> Destructor. Waits for all tasks to finish. </summary>
        ~SchedulerImpl();

        /// <summary> Schedules the task on a single worker. </summary>
        /// <param name="task"> Task to schedule. </param>
        void Schedule(std::shared_ptr<Task> task);

        /// <summary> Schedules the task on a specific worker. </summary>
        /// <param name="workerId"> The id of the worker. </param>
        /// <param name="task"> Task to schedule. </param>
        /// <remarks>
        /// By design, it enqueues a task immediately,
        /// so the task will have higher priority than ones called by Schedule().
        /// </remarks>
        void ScheduleOnWorker(WorkerId workerId, std::shared_ptr<Task> task);

        /// <summary> Schedules immediate task on a specific worker. </summary>
        /// <param name="workerId"> The id of the worker. </param>
        /// <param name="task"> Task to schedule. </param>
        /// <remarks>
        /// By design, it enqueues a task immediately before any non-immedidate task,
        /// but after any existing immediate task.
        /// </remarks>
        void ScheduleImmediateOnWorker(WorkerId workerId, std::shared_ptr<Task> task);

        /// <summary> Schedules the task on all workers. </summary>
        /// <param name="task"> Task to schedule. </param>
        /// <remarks>
        /// By design, it enqueues a task immediately,
        /// so the task will have higher priority than ones called by Schedule().
        /// </remarks>
        void ScheduleOnAllWorkers(std::shared_ptr<Task> task);

    private:

        /// <summary> The logic invoked when a worker is idle. </summary>
        void IdleWorkerNotificationCallback(WorkerId workerId);

        /// <summary> The workers that are used for running the tasks. </summary>
        std::vector<WorkerType> _workers;

        /// <summary> New tasks that weren't assigned to a specific worker. </summary>
        std::queue<std::shared_ptr<Task>> _nonScheduledTasks;

        /// <summary> List of idle workers, used when assigning non scheduled tasks. </summary>
        std::list<WorkerId> _idleWorkers;

        /// <summary> Flags to indicate that a worker is in the idle list. </summary>
        std::vector<std::list<WorkerId>::iterator> _idleWorkersFlags;

        /// <summary> Uses a single thread to synchronize task queuing and posting. </summary>
        std::unique_ptr<SimpleThreadPool> _synchronizer;

        /// <summary> A flag to signal that scheduler is stopping. </summary>
        std::atomic<bool> _shouldStop;

        /// <summary> Tasks being scheduled but not yet dispatched to worker or put into non-scheduled queue. </summary>
        std::atomic<size_t> _beingScheduled;
    };

    typedef SchedulerImpl<Worker> Scheduler;

    template <typename WorkerType>
    SchedulerImpl<WorkerType>::SchedulerImpl(const settings::ZoneSettings& settings, std::function<void(WorkerId)> workerSetupCallback) :
        _idleWorkersFlags(settings.workers),
        _synchronizer(std::make_unique<SimpleThreadPool>(1)),
        _shouldStop(false),
        _beingScheduled(0) {

        _workers.reserve(settings.workers);

        for (WorkerId i = 0; i < settings.workers; i++) {
            _workers.emplace_back(i, settings, workerSetupCallback, [this](WorkerId workerId) {
                IdleWorkerNotificationCallback(workerId);
            });
            _workers[i].Start();

            // All workers are idle initially.
            auto iter = _idleWorkers.emplace(_idleWorkers.end(), i);
            _idleWorkersFlags[i] = iter;
        }
    }

    template <typename WorkerType>
    SchedulerImpl<WorkerType>::~SchedulerImpl() {
        NAPA_DEBUG("Scheduler", "Shutting down: Start draining unscheduled tasks...");

        // Wait for all tasks to be scheduled.
        while (_beingScheduled > 0 || !_nonScheduledTasks.empty()) {
            std::this_thread::yield();
        }

        // Signal scheduler callbacks to not process anymore tasks.
        _shouldStop = true;

        // Wait for synchronizer to finish his book-keeping.
        _synchronizer = nullptr;

        // Wait for all workers to finish processing remaining tasks.
        _workers.clear();

        NAPA_DEBUG("Scheduler", "Shutdown completed");
    }

    template <typename WorkerType>
    void SchedulerImpl<WorkerType>::Schedule(std::shared_ptr<Task> task) {
        NAPA_ASSERT(task, "task is null");
        _beingScheduled++;
        _synchronizer->Execute([this, task]() {
            if (_idleWorkers.empty()) {
                NAPA_DEBUG("Scheduler", "All workers are busy, putting task to non-scheduled queue.");

                // If there is no idle worker, put the task into the non-scheduled queue.
                _nonScheduledTasks.emplace(std::move(task));
            } else {
                // Pop the worker id from the idle workers list.
                auto workerId = _idleWorkers.front();
                _idleWorkers.pop_front();
                _idleWorkersFlags[workerId] = _idleWorkers.end();

                // Schedule task on worker
                _workers[workerId].Schedule(std::move(task));

                NAPA_DEBUG("Scheduler", "Scheduled task on worker %u.", workerId);
            }
            _beingScheduled--;
        });
        
    }

    template <typename WorkerType>
    void SchedulerImpl<WorkerType>::ScheduleOnWorker(WorkerId workerId, std::shared_ptr<Task> task) {
        NAPA_ASSERT(workerId < _workers.size(), "worker id out of range");

        _synchronizer->Execute([workerId, this, task]() {
            // If the worker is idle, change it's status.
            if (_idleWorkersFlags[workerId] != _idleWorkers.end()) {
                _idleWorkers.erase(_idleWorkersFlags[workerId]);
                _idleWorkersFlags[workerId] = _idleWorkers.end();
            }

            // Schedule task on worker
            _workers[workerId].Schedule(std::move(task));

            NAPA_DEBUG("Scheduler", "Explicitly scheduled task on worker %u.", workerId);
        });
    }

    template <typename WorkerType>
    void SchedulerImpl<WorkerType>::ScheduleImmediateOnWorker(WorkerId workerId, std::shared_ptr<Task> task) {
        NAPA_ASSERT(workerId < _workers.size(), "worker id out of range");

        _synchronizer->Execute([workerId, this, task]() {
            // If the worker is idle, change it's status.
            if (_idleWorkersFlags[workerId] != _idleWorkers.end()) {
                _idleWorkers.erase(_idleWorkersFlags[workerId]);
                _idleWorkersFlags[workerId] = _idleWorkers.end();
            }

            // Schedule task on worker
            _workers[workerId].ScheduleImmediate(std::move(task));

            NAPA_DEBUG("Scheduler", "Explicitly scheduled task on worker %u.", workerId);
        });
    }

    template <typename WorkerType>
    void SchedulerImpl<WorkerType>::ScheduleOnAllWorkers(std::shared_ptr<Task> task) {
        NAPA_ASSERT(task, "task is null");

        _synchronizer->Execute([this, task]() {
            // Clear all idle workers.
            _idleWorkers.clear();
            for (auto& flag : _idleWorkersFlags) {
                flag = _idleWorkers.end();
            }

            // Schedule the task on all workers.
            for (auto& worker : _workers) {
                worker.Schedule(task);
            }
            NAPA_DEBUG("Scheduler", "Scheduled task on all workers");
        });
    }

    template <typename WorkerType>
    void SchedulerImpl<WorkerType>::IdleWorkerNotificationCallback(WorkerId workerId) {
        NAPA_ASSERT(workerId < _workers.size(), "worker id out of range");

        if (_shouldStop) {
            return;
        }

        _synchronizer->Execute([this, workerId]() {
            if (!_nonScheduledTasks.empty()) {
                // If there is a non scheduled task, schedule it on the idle worker.
                auto task = _nonScheduledTasks.front();
                _nonScheduledTasks.pop();
                _workers[workerId].Schedule(std::move(task));

                NAPA_DEBUG("Scheduler", "Worker %u fetched a task from non-scheduled queue", workerId);
            } else {
                // Put worker in idle list.
                if (_idleWorkersFlags[workerId] == _idleWorkers.end()) {
                    auto iter = _idleWorkers.emplace(_idleWorkers.end(), workerId);
                    _idleWorkersFlags[workerId] = iter;

                    NAPA_DEBUG("Scheduler", "Worker %u becomes idle", workerId);
                }
            }
        });
    }
}
}
