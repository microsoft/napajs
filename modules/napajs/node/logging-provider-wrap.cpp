#include "logging-provider-wrap.h"

#include <napa-log.h>

using namespace napa::binding;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(LoggingProviderWrap);

void LoggingProviderWrap::Init(v8::Isolate* isolate) {
    // Prepare constructor template.
    auto functionTemplate = v8::FunctionTemplate::New(isolate, NewCallback);
    functionTemplate->SetClassName(v8_helpers::MakeV8String(isolate, _exportName));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // Set prototype method.
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "log", Log);

    // Set persistent constructor into V8.
    NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
}

void LoggingProviderWrap::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto constructor = NAPA_GET_PERSISTENT_CONSTRUCTOR(_exportName);
    auto context = isolate->GetCurrentContext();
    auto instance = constructor->NewInstance(context).ToLocalChecked();

    args.GetReturnValue().Set(instance);
}

void LoggingProviderWrap::NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    JS_ASSERT(isolate, args.IsConstructCall(), "Only constructor call is allowed");

    args.GetReturnValue().Set(args.This());
}

void LoggingProviderWrap::Log(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 4, "Log accepts exactly 4 arguments (level, section, traceId, message)");

    CHECK_ARG(isolate, args[0]->IsUint32(), "'level' must be a uint32 type that represents the native enum");
    CHECK_ARG(isolate, args[1]->IsString() || args[1]->IsNull(), "'section' must be a valid string or null");

    auto level = static_cast<napa::providers::LoggingProvider::Verboseness>(args[0]->Uint32Value());
    
    v8::String::Utf8Value sectionValue(args[1]->ToString());
    const char* section = sectionValue.length() > 0 ? *sectionValue : "";

    auto& logger = napa::providers::GetLoggingProvider();

    // If log is not enabled we can return early.
    if (!logger.IsLogEnabled(section, level)) {
        return;
    }

    CHECK_ARG(isolate, args[2]->IsString() || args[2]->IsNull(), "'traceId' must be a valid string or null");
    CHECK_ARG(isolate, args[3]->IsString(), "'message' must be a valid string");

    v8::String::Utf8Value traceIdValue(args[2]->ToString());
    const char* traceId = traceIdValue.length() > 0 ? *traceIdValue : "";

    v8::String::Utf8Value message(args[3]->ToString());

    // Get the top most frame of the current JS callstack.
    auto stackFrame = v8::StackTrace::CurrentStackTrace(isolate, 1)->GetFrame(0);

    v8::String::Utf8Value file(stackFrame->GetScriptName());
    int line = stackFrame->GetLineNumber();

    logger.LogMessage(section, level, traceId, *file, line, *message);
}
