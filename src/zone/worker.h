// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include "settings/settings.h"
#include "task.h"

#include <functional>
#include <memory>


namespace napa {
namespace zone {

    // Represent the worker id type.
    using WorkerId = uint32_t;

    /// <summary> Represents an execution unit (a worker) for running tasks. </summary>
    class Worker {
    public:
        /// <summary> Constructor. </summary>
        /// <param name="id"> The task id. </param>
        /// <param name="settings"> A settings object. </param>
        /// <param name="setupCallback"> Callback to setup the isolate after worker created its isolate. </param>
        /// <param name="idleNotificationCallback"> Triggers when the worker becomes idle. </param>
        Worker(WorkerId id,
            const settings::ZoneSettings& settings,
            std::function<void(WorkerId)> setupCallback,
            std::function<void(WorkerId)> idleNotificationCallback);

        /// <summary> Destructor. </summary>
        /// <note> This will block until all pending tasks are completed. </note>
        ~Worker();

        /// <summary> Non-copyable. </summary>
        Worker(const Worker&) = delete;
        Worker& operator=(const Worker&) = delete;

        /// <summary> Moveable. </summary>
        Worker(Worker&&);
        Worker& operator=(Worker&&);

        /// <summary> Schedules a task on this worker. </summary>
        /// <param name="task"> Task to schedule. </param>
        /// <note> Same task instance may run on multiple workers, hence the use of shared_ptr. </node>
        void Schedule(std::shared_ptr<Task> task);

    private:
        /// <summary> The worker thread logic. </summary>
        void WorkerThreadFunc(const settings::ZoneSettings& settings);

        /// <summary> Enqueue a task. </summary>
        void Enqueue(std::shared_ptr<Task> task);

        struct Impl;
        std::unique_ptr<Impl> _impl;
    };
} // namespace zone
} // namespace napa
