#include "load-task.h"
#include <napa-log.h>

#include <v8.h>

using namespace napa::scheduler;

LoadTask::LoadTask(std::string source, LoadTaskCallback callback) :
    _source(std::move(source)),
    _callback(std::move(callback)) {}

void LoadTask::Execute() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    // V8 garbage collection frees ExternalOneByteStringResourceImpl.
    auto externalString = new v8::ExternalOneByteStringResourceImpl(_source.data(), _source.length());

    // Create a V8 string from the exernal string resource.
    auto source = v8::String::NewExternal(isolate, externalString);

    // Compile the source code.
    auto compileResult = v8::Script::Compile(context, source);
    if (compileResult.IsEmpty()) {
        LOG_ERROR("LoadTask", "Failed compiling the provided source code");
        _callback(NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
        return;
    }
    auto script = compileResult.ToLocalChecked();

    // Run the source code.
    v8::TryCatch tryCatch(isolate);
    script->Run(context);
    if (tryCatch.HasCaught()) {
        auto exception = tryCatch.Exception();
        v8::String::Utf8Value exceptionStr(exception);

        LOG_ERROR("LoadTask", "JS exception thrown: %s", *exceptionStr);
        _callback(NAPA_RESPONSE_LOAD_SCRIPT_ERROR);
        return;
    }

    _callback(NAPA_RESPONSE_SUCCESS);
}
