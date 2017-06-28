#include "async-complete-task.h"

#include <v8.h>

using namespace napa::zone;

AsyncCompleteTask::AsyncCompleteTask(std::shared_ptr<AsyncContext> context) : _context(std::move(context)) {}

void AsyncCompleteTask::Execute() {
    // If asyncWork is empty, don't need to wait for async result. It just posts a completion only.
    if (_context->asyncWork != nullptr) {
        _context->future.wait();
    }

    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto jsCallback = v8::Local<v8::Function>::New(isolate, _context->jsCallback);
    _context->asyncCompleteCallback(jsCallback, _context->result);

    _context->jsCallback.Reset();
}