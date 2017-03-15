#include "run-task.h"

#include <napa-log.h>
#include <napa/v8-helpers.h>

#include <v8.h>

using namespace napa::scheduler;

RunTask::RunTask(std::string func, std::vector<std::string> args, RunTaskCallback callback) :
    _func(std::move(func)),
    _args(std::move(args)),
    _callback(std::move(callback)) {}

void RunTask::Execute() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    auto functionName = napa::v8_helpers::MakeExternalV8String(isolate, _func);

    // Find the function in the global context of this isolate.
    auto funcVal = context->Global()->Get(functionName);
    if (!funcVal->IsFunction()) {
        std::string errorMessage = "Function '" + _func + "' not defined";
        LOG_ERROR("Run", errorMessage.c_str());
        _callback(NAPA_RESPONSE_RUN_FUNC_ERROR, STD_STRING_TO_NAPA_STRING_REF(errorMessage), EMPTY_NAPA_STRING_REF);
        return;
    }

    // The actual function to run.
    auto function = v8::Local<v8::Function>::Cast(funcVal);

    // Create the V8 args for the function call.
    // TODO @asib: Check if this needs to be converted to wide strings.
    std::vector<v8::Local<v8::Value>> args;
    args.reserve(_args.size());
    for (const auto& arg : _args) {
        args.emplace_back(napa::v8_helpers::MakeExternalV8String(isolate, arg));
    }

    // Run the function.
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
            LOG_ERROR("Run", "Task was terminated due to timeout");
            _callback(NAPA_RESPONSE_TIMEOUT, NAPA_STRING_REF("Run exceeded timeout"), EMPTY_NAPA_STRING_REF);
        } else {
            LOG_ERROR("Run", "Task was terminated for unknown reason");
            _callback(NAPA_RESPONSE_INTERNAL_ERROR, NAPA_STRING_REF("Run task terminated"), EMPTY_NAPA_STRING_REF);
        }

        return;
    }

    if (tryCatch.HasCaught()) {
        auto exception = tryCatch.Exception();
        v8::String::Utf8Value errorMessage(exception);

        LOG_ERROR("Run", "Error occured while running function '%s', error: %s", _func.c_str(), *errorMessage);
        _callback(NAPA_RESPONSE_RUN_FUNC_ERROR, NAPA_STRING_REF(*errorMessage), EMPTY_NAPA_STRING_REF);
        return;
    }

    if (res->IsObject()) {
        res = v8_helpers::Stringify(isolate, res->ToObject());
    }

    // Note that the return value must be copied out if the callback wants to keep it.
    v8::String::Utf8Value returnValue(res);
    _callback(NAPA_RESPONSE_SUCCESS,
        EMPTY_NAPA_STRING_REF,
        NAPA_STRING_REF_WITH_SIZE(*returnValue, static_cast<size_t>(returnValue.length())));
}
