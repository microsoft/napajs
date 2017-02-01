#include "task-decorators.h"

using namespace napa::scheduler;

TaskDecorator::TaskDecorator(std::unique_ptr<Task> innerTask) : _innerTask(std::move(innerTask)) {}


TimeoutTaskDecorator::TimeoutTaskDecorator(std::chrono::microseconds timeout, std::unique_ptr<Task> innerTask) :
    TaskDecorator(std::move(innerTask)),
    _timeout(timeout) {}

void TimeoutTaskDecorator::Execute() {
    // TODO @asib: Implement
}
