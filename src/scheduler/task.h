#ifndef NAPA_TASK_H
#define NAPA_TASK_H


namespace napa {
namespace runtime {
namespace internal {

    /// <summary> Represents an execution logic that can be scheduled using the Napa scheduler. </summary>
    class Task {
    public:

        /// <summary> Executes the task. </summary>
        virtual void Execute() = 0;

        /// <summary> Virtual destructor. </summary>
        virtual ~Task() = default;
    };

}
}
}

#endif // NAPA_TASK_H
