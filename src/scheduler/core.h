#ifndef NAPA_CORE_H
#define NAPA_CORE_H

#include "task.h"
#include "settings.h"

#include <functional>
#include <memory>


namespace napa {
namespace runtime {
namespace internal {

    // Represent the core id type.
    using CoreId = uint32_t;

    /// <summary> Represents a single execution unit for running tasks. </summary>
    class Core {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="id"> The task id. </param>
        /// <param name="settings"> Settings object. </param>
        Core(CoreId id, const Settings &settings);

        /// <summary> Destructor. </summary>
        /// <note> This will block until all pending tasks are completed. </note>
        ~Core();

        /// <summary> Schedules a task on this core. </summary>
        /// <param name="task"> Task to schedule. </param>
        /// <note> Same task instance may run on multiple cores, hence the use of shared_ptr. </node>
        void Schedule(std::shared_ptr<Task> task);

        /// <summary> Subscribe to a notification when the core becomes idle. </summary>
        /// <param name="callback"> The callback. </param>
        void SubscribeForIdleNotifications(std::function<void(CoreId)> callback);

    private:

        struct Impl;
        std::unique_ptr<Impl> _impl;
    };

}
}
}

#endif // NAPA_CORE_H
