// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <vector>
#include <memory>

#include <iostream>

#include "timer-wrap.h"

#include <napa/zone.h>
#include <zone/worker.h>
#include <zone/scheduler.h>
#include <zone/worker-context.h>
#include <zone/async-context.h>
#include <zone/callback-task.h>

using namespace napa::module;

using napa::zone::WorkerId;
using napa::zone::WorkerContext;
using napa::zone::WorkerContextItem;
using napa::zone::NapaZone;

using v8::Array;
using v8::Boolean;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Global;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(TimerWrap);

void TimerWrap::Init() {
    auto isolate = Isolate::GetCurrent();
    auto constructorTemplate = FunctionTemplate::New(isolate, DefaultConstructorCallback<TimerWrap>);
    constructorTemplate->SetClassName(v8_helpers::MakeV8String(isolate, exportName));
    constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    NAPA_SET_PERSISTENT_CONSTRUCTOR(exportName, constructorTemplate->GetFunction());
}

Local<Object> TimerWrap::NewInstance(std::shared_ptr<napa::zone::Timer> timer) {
    auto object = napa::module::NewInstance<TimerWrap>().ToLocalChecked();
    auto wrap = NAPA_OBJECTWRAP::Unwrap<TimerWrap>(object);
    wrap->_timer = std::move(timer);
    return object;
}

napa::zone::Timer& TimerWrap::Get() {
    return *_timer;
}

std::shared_ptr<napa::zone::CallbackTask> buildTimeoutTask(
        std::shared_ptr<Global<Object>> sharedTimeout,
        std::shared_ptr<Global<Context>> sharedContext)
{
    return std::make_shared<napa::zone::CallbackTask>(
        [sharedTimeout, sharedContext]() {
            auto isolate = Isolate::GetCurrent();
            HandleScope handleScope(isolate);
            auto context = Local<Context>::New(isolate, *sharedContext);
            Context::Scope contextScope(context);

            auto timeout = Local<Object>::New(isolate, *sharedTimeout);

            Local<Boolean> active = Local<Boolean>::Cast(timeout->Get(String::NewFromUtf8(isolate, "_active")));
            if (active->Value()) {
                Local<Function> cb = Local<Function>::Cast(timeout->Get(String::NewFromUtf8(isolate, "_callback")));
                Local<Array> args = Local<Array>::Cast(timeout->Get(String::NewFromUtf8(isolate, "_args")));
                
                std::vector<Local<Value>> parameters;
                parameters.reserve(args->Length());
                for (int i = 0; i < args->Length(); ++i) {
                    Local<Value> v = args->Get(context, i).ToLocalChecked();
                    parameters.emplace_back(v);
                }
                cb->Call(context, context->Global(), parameters.size(), parameters.data());

                Local<Number> interval = Local<Number>::Cast(timeout->Get(String::NewFromUtf8(isolate, "_repeat")));
                bool isInterval = (interval->Value() >= 1);
                if (isInterval) {
                    auto jsTimer = NAPA_OBJECTWRAP::Unwrap<TimerWrap>(
                        Local<Object>::Cast(timeout->Get(String::NewFromUtf8(isolate, "_timer"))));
                    jsTimer->Get().Start(); //re-arm
                }
            }
        }
    );
}


void TimerWrap::SetImmediateCallback(const FunctionCallbackInfo<Value>& args) {
    auto isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument is required for calling 'SetImmediateCallback'.");
    CHECK_ARG(isolate, args[0]->IsObject(), "Argument \"timeout\" shall be 'Timeout' type.");

    auto zone = reinterpret_cast<NapaZone*>(WorkerContext::Get(WorkerContextItem::ZONE));
    if (zone == nullptr) {
        throw new std::runtime_error("Null zone encountered!");
    }
    auto scheduler = zone->GetScheduler().get();
    if (scheduler == nullptr) {
        throw new std::runtime_error("Null scheduler encountered!");
    }

    Local<Object> timeout = Local<Object>::Cast(args[0]);
    auto sharedTimeout = std::make_shared<Global<Object>>(isolate, timeout);

    auto context = isolate->GetCurrentContext();
    auto sharedContext = std::make_shared<Global<Context>>(isolate, context);

    auto immediateCallbackTask = buildTimeoutTask(sharedTimeout, sharedContext);

    auto workerId = static_cast<WorkerId>(
        reinterpret_cast<uintptr_t>(WorkerContext::Get(WorkerContextItem::WORKER_ID)));
    scheduler->ScheduleImmediateOnWorker(workerId, immediateCallbackTask);
}


void TimerWrap::SetTimeoutIntervalCallback(const FunctionCallbackInfo<Value>& args) {
    auto isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    CHECK_ARG(isolate, args.Length() == 1, "1 argument is required for calling 'SetTimeoutIntervalCallback'.");
    CHECK_ARG(isolate, args[0]->IsObject(), "Argument \"timeout\" shall be 'Timeout' type.");

    auto zone = reinterpret_cast<NapaZone*>(WorkerContext::Get(WorkerContextItem::ZONE));
    if (zone == nullptr) {
        throw new std::runtime_error("Null zone encountered!");
    }
    auto scheduler = zone->GetScheduler();
    if (scheduler.get() == nullptr) {
        throw new std::runtime_error("Null scheduler encountered!");
    }
    auto workerId = static_cast<WorkerId>(
        reinterpret_cast<uintptr_t>(WorkerContext::Get(WorkerContextItem::WORKER_ID)));

    Local<Object> timeout = Local<Object>::Cast(args[0]);
    auto sharedTimeout = std::make_shared<Global<Object>>(isolate, timeout);

    auto context = isolate->GetCurrentContext();
    auto sharedContext = std::make_shared<Global<Context>>(isolate, context);

    auto immediateCallbackTask = buildTimeoutTask(sharedTimeout, sharedContext);
    

    Local<Number> after = Local<Number>::Cast(timeout->Get(String::NewFromUtf8(isolate, "_after")));
    std::chrono::milliseconds msAfter{static_cast<int>(after->Value())};
    auto sharedTimer = std::make_shared<napa::zone::Timer>(
        [sharedTimeout, sharedContext, scheduler, workerId]() {
            auto timerCallbackTask = buildTimeoutTask(sharedTimeout, sharedContext);
            scheduler->ScheduleOnWorker(workerId, timerCallbackTask);
        }, msAfter);

    auto jsTimer = TimerWrap::NewInstance(sharedTimer);
    timeout->Set(String::NewFromUtf8(isolate, "_timer"), jsTimer);
    sharedTimer->Start();
}
