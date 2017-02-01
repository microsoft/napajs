#pragma once

#include "task.h"

#include <chrono>
#include <memory>

namespace napa {
namespace scheduler {

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
