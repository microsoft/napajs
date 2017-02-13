#pragma once

#include "task.h"
#include "settings/settings.h"

#include <functional>
#include <memory>


namespace napa {
namespace scheduler {

    // Represent the core id type.
    using CoreId = uint32_t;

    /// <summary> Represents a single execution unit for running tasks. </summary>
    class Core {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="id"> The task id. </param>
        /// <param name="settings"> Settings object. </param>
        /// <param name="idleNotificationCallback"> Triggers when the core becomes idle. </param>
        Core(CoreId id, const Settings &settings, std::function<void(CoreId)> idleNotificationCallback);

        /// <summary> Destructor. </summary>
        /// <note> This will block until all pending tasks are completed. </note>
        ~Core();

        /// <summary> Non-copyable. </summary>
        Core(const Core&) = delete;
        Core& operator=(const Core&) = delete;

        /// <summary> Moveable. </summary>
        Core(Core&&);
        Core& operator=(Core&&);

        /// <summary> Schedules a task on this core. </summary>
        /// <param name="task"> Task to schedule. </param>
        /// <note> Same task instance may run on multiple cores, hence the use of shared_ptr. </node>
        void Schedule(std::shared_ptr<Task> task);

    private:

        /// <summary> The core thread logic. </summary>
        void CoreThreadFunc(const Settings& settings);

        struct Impl;
        std::unique_ptr<Impl> _impl;
    };

}
}