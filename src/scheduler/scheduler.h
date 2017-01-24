#ifndef NAPA_SCHEDULER_H
#define NAPA_SCHEDULER_H

#include "core.h"
#include "settings.h"
#include "task.h"
#include "simple-thread-pool.h"

#include <list>
#include <memory>
#include <vector>
#include <queue>

namespace napa {
namespace runtime {
namespace internal {

    /// <summary> The scheduler is responsible for assigning tasks to cores. </summary>
    template <typename CoreType>
    class SchedulerImpl {
    public:

        /// <summary> Constructor. </summary>
        /// <param name="settings"> A settings object. </param>
        SchedulerImpl(const Settings& settings);

        /// <summary> Schedules the task on a single core. </summary>
        /// <param name="task"> Task to schedule. </param>
        void Schedule(std::unique_ptr<Task> task);

        /// <summary> Schedules the task on a specific core. </summary>
        /// <param name="coreId"> The id of the core. </param>
        /// <param name="task"> Task to schedule. </param>
        void ScheduleOnCore(CoreId coreId, std::unique_ptr<Task> task);

        /// <summary> Schedules the task on all cores. </summary>
        /// <param name="task"> Task to schedule. </param>
        void ScheduleOnAllCores(std::unique_ptr<Task> task);

    private:

        /// <summary> The cores that are used for running the tasks. </summary>
        std::vector<CoreType> _cores;

        /// <summary> New tasks that weren't assigned to a specific core. </summary>
        std::queue<std::unique_ptr<Task>> _nonScheduledTasks;

        /// <summary> List of idle cores, used when assigning non scheduled tasks. </summary>
        std::list<CoreId> _idleCores;

        /// <summary> Flags to indicate that a core is in the idle list. </summary>
        std::vector<std::list<CoreId>::iterator> _idleCoresFlags;

        /// <summary> Uses a single thread to synchronize task queuing and posting. </summary>
        SimpleThreadPool _synchronizer;
    };

    typedef SchedulerImpl<Core> Scheduler;

    template <typename CoreType>
    SchedulerImpl<CoreType>::SchedulerImpl(const Settings& settings) {
        // TODO @asib: add implementation
    }

    template <typename CoreType>
    void SchedulerImpl<CoreType>::Schedule(std::unique_ptr<Task> task) {
        // TODO @asib: add implementation
    }

    template <typename CoreType>
    void SchedulerImpl<CoreType>::ScheduleOnCore(CoreId coreId, std::unique_ptr<Task> task) {
        // TODO @asib: add implementation
    }

    template <typename CoreType>
    void SchedulerImpl<CoreType>::ScheduleOnAllCores(std::unique_ptr<Task> task) {
        // TODO @asib: add implementation
    }

}
}
}

#endif // NAPA_SCHEDULER_H
