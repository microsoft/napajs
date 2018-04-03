// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// See: https://groups.google.com/forum/#!topic/nodejs/onA0S01INtw
#ifdef BUILDING_NODE_EXTENSION
#include <node.h>
#endif

#include "eval-task.h"

#include <platform/filesystem.h>

#include <napa/log.h>
#include <napa/v8-helpers.h>

#include <v8.h>

using namespace napa;
using namespace napa::zone;

EvalTask::EvalTask(std::string source, std::string sourceOrigin, BroadcastCallback callback) :
    _source(std::move(source)),
    _sourceOrigin(std::move(sourceOrigin)),
    _callback(std::move(callback)) {}

void EvalTask::Execute() {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();

    NAPA_DEBUG("EvalTask", "Begin executing script:\"%s\"", _source.c_str());

    auto filename = v8_helpers::MakeV8String(isolate, _sourceOrigin);
    filesystem::Path originPath(_sourceOrigin);
    if (originPath.IsAbsolute()) {
        auto global = context->Global();

        auto dirname = v8_helpers::MakeV8String(isolate, originPath.Parent().Normalize().String());
        (void)global->Set(context, v8_helpers::MakeV8String(isolate, "__dirname"), dirname);
        (void)global->Set(context, v8_helpers::MakeV8String(isolate, "__filename"), filename);
    }

    auto source = napa::v8_helpers::MakeV8String(isolate, _source);
    auto sourceOrigin = v8::ScriptOrigin(filename);

    // Compile the source code.
    v8::MaybeLocal<v8::Script> compileResult;
    {
        v8::TryCatch tryCatch(isolate);
        compileResult = v8::Script::Compile(context, source, &sourceOrigin);
        if (tryCatch.HasCaught()) {
            auto exception = tryCatch.Exception();
            v8::String::Utf8Value exceptionStr(exception);

            std::stringstream ss;
            ss << "Compilation failed: " << *exceptionStr;
            std::string reason = ss.str();
            NAPA_DEBUG("EvalTask", reason.c_str());

            _callback({ NAPA_RESULT_BROADCAST_SCRIPT_ERROR, reason.c_str(), "", nullptr });
            return;
        }
    }
    
    NAPA_DEBUG("EvalTask", "Script compiled successfully");
    auto script = compileResult.ToLocalChecked();

    // Run the source code.
    {
        v8::TryCatch tryCatch(isolate);
        (void)script->Run(context);
        if (tryCatch.HasCaught()) {
            auto exception = tryCatch.Exception();
            v8::String::Utf8Value exceptionStr(exception);
            auto stackTrace = tryCatch.StackTrace();
            v8::String::Utf8Value stackTraceStr(stackTrace);

            std::stringstream ss;
            ss << "Eval failed: " << *exceptionStr << " - " << *stackTraceStr;
            std::string reason = ss.str();
            NAPA_DEBUG("EvalTask", reason.c_str());

            _callback({ NAPA_RESULT_BROADCAST_SCRIPT_ERROR, reason.c_str(), "", nullptr });
            return;
        }
    }

    NAPA_DEBUG("EvalTask", "Eval script completed with success");
    _callback({ NAPA_RESULT_SUCCESS, "", "", nullptr });
}
