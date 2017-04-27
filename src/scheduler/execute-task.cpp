#include "execute-task.h"

#include <napa-log.h>
#include <napa/v8-helpers.h>

#include <v8.h>

using namespace napa::scheduler;
using namespace napa::v8_helpers;

napa::scheduler::ExecuteTask::ExecuteTask(const ExecuteRequest& request, ExecuteCallback callback) :
    _module(NAPA_STRING_REF_TO_STD_STRING(request.module)),
    _func(NAPA_STRING_REF_TO_STD_STRING(request.function)),
    _callback(std::move(callback)) {

    _args.reserve(request.arguments.size());
    for (auto& arg : request.arguments) {
        _args.emplace_back(NAPA_STRING_REF_TO_STD_STRING(arg));
    }

    // Pass ownership of the transport context to the execute task.
    _transportContext = std::move(request.transportContext);
}

void ExecuteTask::Execute() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    v8::Local<v8::Function> dispatcher;
    std::vector<v8::Local<v8::Value>> args;

    if (_module.empty()) {
        auto dispatcherValue = context->Global()->Get(MakeExternalV8String(isolate, "__napa_function_dispatcher__"));
        NAPA_ASSERT(dispatcherValue->IsFunction(), "dispatcher function must exist in global scope");

        dispatcher = v8::Local<v8::Function>::Cast(dispatcherValue);

        auto funcValue = context->Global()->Get(MakeExternalV8String(isolate, _func));
        if (!funcValue->IsFunction()) {
            auto error = "Function '" + _func + "' does not exist in global scope";
            _callback({ NAPA_RESPONSE_EXECUTE_FUNC_ERROR, std::move(error), "", nullptr });
            return;
        }

        args.reserve(3); // (func, args, contextHandle)
        args.emplace_back(v8::Local<v8::Function>::Cast(funcValue));
    } else {
        // Get the dispatcher function from global scope.
        auto dispatcherValue = context->Global()->Get(MakeExternalV8String(isolate, "__napa_module_dispatcher__"));
        NAPA_ASSERT(dispatcherValue->IsFunction(), "dispatcher function must exist in global scope");

        dispatcher = v8::Local<v8::Function>::Cast(dispatcherValue);

        args.reserve(4); // (moduleName, functionName, args, contextHandle)
        args.emplace_back(MakeExternalV8String(isolate, _module));
        args.emplace_back(MakeExternalV8String(isolate, _func));
    }
    
    // Prepare function args
    auto arr = v8::Array::New(isolate, static_cast<int>(_args.size()));
    for (size_t i = 0; i < _args.size(); ++i) {
        (void)arr->CreateDataProperty(context, static_cast<uint32_t>(i), MakeExternalV8String(isolate, _args[i]));
    }
    args.emplace_back(arr);

    // Transport context handle
    args.emplace_back(PtrToV8Uint32Array(isolate, _transportContext.get()));

    // Execute the function.
    v8::TryCatch tryCatch(isolate);
    auto res = dispatcher->Call(context->Global(), static_cast<int>(args.size()), args.data());

    // Terminating an isolate may occur from a different thread, i.e. from timeout service.
    // If the function call already finished successfully when the isolate is terminated it may lead
    // to one the following:
    //      1. Terminate was called before tryCatch.HasTerminated(), the user gets an error code.
    //      2. Terminate was called after tryCatch.HasTerminated(), the user gets a success code.
    //
    //  In both cases the isolate is being restored since this happens before each task executes.
    if (tryCatch.HasTerminated()) {
        if (_terminationReason == TerminationReason::TIMEOUT) {
            LOG_ERROR("Execute", "Task was terminated due to timeout");
            _callback({ NAPA_RESPONSE_TIMEOUT, "Execute exceeded timeout", "", std::move(_transportContext) });
        } else {
            LOG_ERROR("Execute", "Task was terminated for unknown reason");
            _callback({ NAPA_RESPONSE_INTERNAL_ERROR, "Execute task terminated", "", std::move(_transportContext) });
        }

        return;
    }

    if (tryCatch.HasCaught()) {
        auto exception = tryCatch.Exception();
        v8::String::Utf8Value exceptionStr(exception);
        auto stackTrace = tryCatch.StackTrace();
        v8::String::Utf8Value stackTraceStr(stackTrace);

        LOG_ERROR("Execute", "JS exception thrown: %s - %s", *exceptionStr, *stackTraceStr);

        _callback({ NAPA_RESPONSE_EXECUTE_FUNC_ERROR, *exceptionStr, "", std::move(_transportContext) });
        return;
    }

    v8::String::Utf8Value val(res);
    _callback({ NAPA_RESPONSE_SUCCESS, "", std::string(*val, val.length()), std::move(_transportContext) });
}
