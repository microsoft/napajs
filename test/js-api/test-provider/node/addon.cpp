#include <napa-module.h>
#include <napa/v8-helpers.h>

#include "logging-provider.h"
#include "metric-provider.h"

#include <string>

using namespace napa::v8_helpers;

void LastLogGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    auto context = isolate->GetCurrentContext();
    
    auto& lastLog = TestLoggingProvider::GetInstance().GetLastLog();
    
    auto obj = v8::Object::New(isolate);
    
    obj->CreateDataProperty(context, MakeV8String(isolate, "section"), MakeV8String(isolate, lastLog.section));
    obj->CreateDataProperty(context, MakeV8String(isolate, "message"), MakeV8String(isolate, lastLog.message));
    obj->CreateDataProperty(context, MakeV8String(isolate, "traceId"), MakeV8String(isolate, lastLog.traceId));
    obj->CreateDataProperty(context, MakeV8String(isolate, "file"), MakeV8String(isolate, lastLog.file));
    obj->CreateDataProperty(context, MakeV8String(isolate, "line"), v8::Integer::New(isolate, lastLog.line));
    
    auto level = static_cast<uint32_t>(lastLog.level);
    obj->CreateDataProperty(context, MakeV8String(isolate, "level"),v8::Uint32::NewFromUnsigned(isolate, level));
    
    args.GetReturnValue().Set(obj);
}

static void LastMetricValueGetter(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto value = TestMetricProvider::GetInstance().GetLastMetric()->GetValue();

    args.GetReturnValue().Set(static_cast<int32_t>(value));
}

void InitAll(v8::Local<v8::Object> exports) {
    exports->SetAccessor(MakeV8String(v8::Isolate::GetCurrent(), "lastLog"), LastLogGetter);
    exports->SetAccessor(MakeV8String(v8::Isolate::GetCurrent(), "lastMetricValue"), LastMetricValueGetter);
}

NAPA_MODULE(addon, InitAll);
