#include "run-task.h"
#include <napa-log.h>

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

    // V8 garbage collection frees ExternalOneByteStringResourceImpl.
    auto externalFunctionName = new v8::ExternalOneByteStringResourceImpl(_func.data(), _func.length());
    auto functionName = v8::String::NewExternal(isolate, externalFunctionName);

    // Find the function in the global context of this isolate.
    auto funcVal = context->Global()->Get(functionName);
    if (!funcVal->IsFunction()) {
        std::string errorMessage = "Function not defined: " + _func;
        LOG_ERROR("RunTask", errorMessage.c_str());
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
        // V8 garbage collection frees ExternalOneByteStringResourceImpl.
        auto externalArgName = new v8::ExternalOneByteStringResourceImpl(arg.data(), arg.length());
        args.emplace_back(v8::String::NewExternal(isolate, externalArgName));
    }

    // Run the function.
    v8::TryCatch tryCatch(isolate);
    auto res = function->Call(context->Global(), static_cast<int>(args.size()), args.data());

    if (tryCatch.HasCaught()) {
        auto exception = tryCatch.Exception();
        v8::String::Utf8Value errorMessage(exception);

        LOG_ERROR("RunTask", "Error occured while running function '%s', error: %s", _func.c_str(), *errorMessage);
        _callback(NAPA_RESPONSE_RUN_FUNC_ERROR, 
                  CREATE_NAPA_STRING_REF_WITH_SIZE(*errorMessage, static_cast<size_t>(errorMessage.length())),
                  EMPTY_NAPA_STRING_REF);
        return;
    }

    // Note that the return value must be copied out if the callback wants to keep it.
    v8::String::Utf8Value returnValue(res);
    _callback(NAPA_RESPONSE_SUCCESS,
              EMPTY_NAPA_STRING_REF,
              CREATE_NAPA_STRING_REF_WITH_SIZE(*returnValue, static_cast<size_t>(returnValue.length())));
}
