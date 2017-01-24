#ifndef NAPA_TASK_DECORATORS_H
#define NAPA_TASK_DECORATORS_H

#include "task.h"

#include <chrono>
#include <memory>

namespace napa {
namespace runtime {
namespace internal {

    /// <summary> Base class for task decorators. </summary>
    class TaskDecorator : public Task {
    public:
        TaskDecorator(std::unique_ptr<Task> innerTask);

    protected:
        std::unique_ptr<Task> _innerTask;
    };

    /// <summary> A task decorator that enables timeout. </summary>
    class TimeoutTaskDecorator : public TaskDecorator {
    public:
        TimeoutTaskDecorator(std::chrono::microseconds timeout, std::unique_ptr<Task> innerTask);

        virtual void Execute() override;

    private:
        std::chrono::microseconds _timeout;
    };
}
}
}

#endif // NAPA_TASK_DECORATORS_H
