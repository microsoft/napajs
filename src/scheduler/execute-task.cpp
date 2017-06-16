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
    _options = request.options;

    // Pass ownership of the transport context to the execute task.
    _transportContext = std::move(request.transportContext);
}

void ExecuteTask::Execute() {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();
    v8::HandleScope scope(isolate);

    // Get the module based main function from global scope.
    auto executeFunction = context->Global()->Get(MakeExternalV8String(isolate, "__zone_execute__"));
    NAPA_ASSERT(executeFunction->IsFunction(), "__zone_execute__ function must exist in global scope");

    // Prepare function args
    auto args = v8::Array::New(isolate, static_cast<int>(_args.size()));
    for (size_t i = 0; i < _args.size(); ++i) {
        (void)args->CreateDataProperty(context, static_cast<uint32_t>(i), MakeExternalV8String(isolate, _args[i]));
    }

    // Prepare execute options.
    // NOTE: export necessary fields from _options to options object here. Now it's empty.
    auto options = v8::ObjectTemplate::New(isolate)->NewInstance();

    v8::Local<v8::Value> argv[] = {
        MakeExternalV8String(isolate, _module),
        MakeExternalV8String(isolate, _func),
        args,
        PtrToV8Uint32Array(isolate, _transportContext.get()),
        options
    };

    // Execute the function.
    v8::TryCatch tryCatch(isolate);
    auto res = v8::Local<v8::Function>::Cast(executeFunction)->Call(context->Global(), 5, argv);

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
