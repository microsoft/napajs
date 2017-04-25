#include "async-complete-task.h"

#include <v8.h>

using namespace napa;
using namespace napa::module;

AsyncCompleteTask::AsyncCompleteTask(std::shared_ptr<AsyncContext> context) : _context(std::move(context)) {}

void AsyncCompleteTask::Execute() {
    _context->future.wait();

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto jsCallback = v8::Local<v8::Function>::New(isolate, _context->jsCallback);
    _context->asyncCompleteCallback(jsCallback);

    _context->jsCallback.Reset();
}