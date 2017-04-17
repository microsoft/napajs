#include "load-task.h"
#include <napa-log.h>

#include <napa/v8-helpers.h>

#include <boost/filesystem.hpp>
#include <v8.h>

using namespace napa::scheduler;

LoadTask::LoadTask(std::string source, std::string sourceOrigin, LoadTaskCallback callback) :
    _source(std::move(source)),
    _sourceOrigin(std::move(sourceOrigin)),
    _callback(std::move(callback)) {}

void LoadTask::Execute() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    auto filename = v8_helpers::MakeV8String(isolate, _sourceOrigin);
    boost::filesystem::path originPath = boost::filesystem::path(_sourceOrigin);
    if (originPath.is_absolute()) {
        auto global = context->Global();

        auto dirname = v8_helpers::MakeV8String(isolate, originPath.parent_path().string());
        (void)global->Set(context, v8_helpers::MakeV8String(isolate, "__dirname"), dirname);
        (void)global->Set(context, v8_helpers::MakeV8String(isolate, "__filename"), filename);
    }

    auto source = napa::v8_helpers::MakeV8String(isolate, _source);
    auto sourceOrigin = v8::ScriptOrigin(filename);

    // Compile the source code.
    auto compileResult = v8::Script::Compile(context, source, &sourceOrigin);
    if (compileResult.IsEmpty()) {
        LOG_ERROR("Load", "Failed while compiling the provided source code");
        _callback(NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
        return;
    }
    auto script = compileResult.ToLocalChecked();

    // Run the source code.
    v8::TryCatch tryCatch(isolate);
    (void)script->Run(context);
    if (tryCatch.HasCaught()) {
        auto exception = tryCatch.Exception();
        v8::String::Utf8Value exceptionStr(exception);
        auto stackTrace = tryCatch.StackTrace();
        v8::String::Utf8Value stackTraceStr(stackTrace);

        LOG_ERROR("Load", "JS exception thrown: %s - %s", *exceptionStr, *stackTraceStr);
        _callback(NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
        return;
    }

    _callback(NAPA_RESPONSE_SUCCESS);
}
