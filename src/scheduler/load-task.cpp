#include "load-task.h"
#include <napa-log.h>

#include <napa/v8-helpers.h>

#include <v8.h>

using namespace napa::scheduler;

LoadTask::LoadTask(std::string source, LoadTaskCallback callback) :
    _source(std::move(source)),
    _callback(std::move(callback)) {}

void LoadTask::Execute() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    // Create a V8 string from the string in this isolate.
    auto source = napa::v8_helpers::MakeV8String(isolate, _source);

    // Compile the source code.
    auto compileResult = v8::Script::Compile(context, source);
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

        LOG_ERROR("Load", "JS exception thrown: %s", *exceptionStr);
        _callback(NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
        return;
    }

    _callback(NAPA_RESPONSE_SUCCESS);
}
