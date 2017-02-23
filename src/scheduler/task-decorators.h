#pragma once

#include "task.h"
#include "terminable-task.h"
#include "timeout-service.h"

#include <v8.h>

#include <chrono>
#include <memory>
#include <utility>

namespace napa {
namespace scheduler {

    template <typename TaskType>
    class TaskDecorator : public Task {
    public:
        template <typename... Args>
        TaskDecorator(Args&&... args) : _innerTask(std::forward<Args>(args)...) {}

    protected:
        TaskType _innerTask;
    };

    template <typename TaskType>
    class TimeoutTaskDecorator : public TaskDecorator<TaskType> {
    public:
        static_assert(std::is_base_of<TerminableTask, TaskType>::value, "TaskType must inherit from TerminableTask");

        template <typename... Args>
        TimeoutTaskDecorator(std::chrono::milliseconds timeout, Args&&... args) :
            TaskDecorator<TaskType>(std::forward<Args>(args)...),
            _timeout(timeout) {}

        void Execute() override {
            auto isolate = v8::Isolate::GetCurrent();

            // RAII - token will cancel the callback upon destruction
            auto token = TimeoutService::Instance().Register(_timeout, [this, isolate]() {
                this->_innerTask.Terminate(TerminationReason::TIMEOUT, isolate);
            });

            this->_innerTask.Execute();
        }

    private:
        std::chrono::milliseconds _timeout;
    };
}
}