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

    v8::Local<v8::Function> function;
    std::vector<v8::Local<v8::Value>> args;

    if (_module.empty()) {
        // Lookup function in global scope
        auto funcVal = context->Global()->Get(MakeExternalV8String(isolate, _func));
        if (!funcVal->IsFunction()) {
            std::string errorMessage = "Function '" + _func + "' not defined";
            LOG_ERROR("Execute", errorMessage.c_str());
            _callback({ NAPA_RESPONSE_EXECUTE_FUNC_ERROR, std::move(errorMessage), "", nullptr });
            return;
        }

        function = v8::Local<v8::Function>::Cast(funcVal);

        // Create the V8 args for the function call.
        args.reserve(_args.size());
        for (const auto& arg : _args) {
            args.emplace_back(MakeExternalV8String(isolate, arg));
        }
    } else {
        // Use the global dispatcher function
        auto dispatcherValue = context->Global()->Get(MakeExternalV8String(isolate, "__napa_execute_dispatcher__"));
        NAPA_ASSERT(dispatcherValue->IsFunction(), "dispatcher function must exist in global scope");

        // The dispatcher function.
        function = v8::Local<v8::Function>::Cast(dispatcherValue);

        // Prepare args
        args.reserve(3); // (module, func, args)
        args.emplace_back(MakeExternalV8String(isolate, _module));
        args.emplace_back(MakeExternalV8String(isolate, _func));

        auto arr = v8::Array::New(isolate, static_cast<int>(_args.size()));
        for (size_t i = 0; i < _args.size(); ++i) {
            (void)arr->CreateDataProperty(context, static_cast<uint32_t>(i), MakeExternalV8String(isolate, _args[i]));
        }
        args.emplace_back(arr);
    }

    // Execute the function.
    v8::TryCatch tryCatch(isolate);
    auto res = function->Call(context->Global(), static_cast<int>(args.size()), args.data());

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
            _callback({ NAPA_RESPONSE_TIMEOUT, "Execute exceeded timeout", "", nullptr });
        } else {
            LOG_ERROR("Execute", "Task was terminated for unknown reason");
            _callback({ NAPA_RESPONSE_INTERNAL_ERROR, "Execute task terminated", "", nullptr });
        }

        return;
    }

    if (tryCatch.HasCaught()) {
        auto exception = tryCatch.Exception();
        v8::String::Utf8Value errorMessage(exception);

        LOG_ERROR("Execute", "Error occured while executing function '%s', error: %s", _func.c_str(), *errorMessage);
        _callback({ NAPA_RESPONSE_EXECUTE_FUNC_ERROR, *errorMessage, "", nullptr });
        return;
    }

    if (res->IsObject()) {
        res = v8_helpers::Stringify(isolate, res->ToObject());
    }

    // Note that the return value must be copied out if the callback wants to keep it.
    v8::String::Utf8Value returnValue(res);
    _callback({ NAPA_RESPONSE_SUCCESS, "", std::string(*returnValue, returnValue.length()), nullptr });
}
