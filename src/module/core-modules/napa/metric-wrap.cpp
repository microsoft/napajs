// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "metric-wrap.h"

using namespace napa::module;
using namespace napa::v8_helpers;

NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(MetricWrap);

void MetricWrap::Init() {
    auto isolate = v8::Isolate::GetCurrent();

    // Prepare constructor template.
    auto functionTemplate = v8::FunctionTemplate::New(isolate, ConstructorCallback);
    functionTemplate->SetClassName(MakeV8String(isolate, _exportName));
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototypes.
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "set", Set);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "increment", Increment);
    NAPA_SET_PROTOTYPE_METHOD(functionTemplate, "decrement", Decrement);

    // Set persistent constructor into V8.
    NAPA_SET_PERSISTENT_CONSTRUCTOR(_exportName, functionTemplate->GetFunction());
}

MetricWrap::MetricWrap(napa::providers::Metric* metric, uint32_t dimensions) :
    _metric(metric), _dimensions(dimensions) {
}

void MetricWrap::ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    JS_ENSURE(isolate, args.IsConstructCall(), "class \"MetricWrap\" allows constructor call only.");

    CHECK_ARG(isolate, args.Length() == 4, "class \"MetricWrap\" accepts exactly 4 arguments (section, name, type, dimensions)");
    CHECK_ARG(isolate, args[0]->IsString(), "'section' must be a valid string");
    CHECK_ARG(isolate, args[1]->IsString(), "'name' must be a valid string");
    CHECK_ARG(isolate, args[2]->IsUint32(), "'type' must be a uint32 type that represents the native enum");
    CHECK_ARG(isolate, args[3]->IsArray(), "'dimensions' must be a valid array");

    auto section = V8ValueTo<Utf8String>(args[0]);
    auto name = V8ValueTo<Utf8String>(args[1]);
    auto type = static_cast<napa::providers::MetricType>(args[2]->Uint32Value());

    // Holds te dimensions strings on the stack for the GetMetric call.
    auto dimensionsStringsHolder = V8ArrayToVector<Utf8String>(isolate, v8::Local<v8::Array>::Cast(args[3]));

    std::vector<const char*> dimensions;
    dimensions.reserve(dimensionsStringsHolder.size());
    for (const auto& dimension : dimensionsStringsHolder) {
        dimensions.emplace_back(dimension.Data());
    }

    auto& metricProvider = napa::providers::GetMetricProvider();
    auto metric = metricProvider.GetMetric(section.Data(), name.Data(), type, dimensions.size(), dimensions.data());

    auto wrap = new MetricWrap(metric, static_cast<uint32_t>(dimensions.size()));

    wrap->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
}

void MetricWrap::Set(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate, args[0]->IsUint32(), "'value' argument must be a valid Uint32");
    auto value = args[0]->Uint32Value();

    InvokeWithDimensions(args, 1, [value](napa::providers::Metric* metric, std::vector<const char*>& dimensions) {
        metric->Set(value, dimensions.size(), dimensions.data());
    });
}

void MetricWrap::Increment(const v8::FunctionCallbackInfo<v8::Value>& args) {
    InvokeWithDimensions(args, 0, [](napa::providers::Metric* metric, std::vector<const char*>& dimensions) {
        metric->Increment(1, dimensions.size(), dimensions.data());
    });
}

void MetricWrap::Decrement(const v8::FunctionCallbackInfo<v8::Value>& args) {
    InvokeWithDimensions(args, 0, [](napa::providers::Metric* metric, std::vector<const char*>& dimensions) {
        metric->Decrement(1, dimensions.size(), dimensions.data());
    });
}

template <typename Func>
void MetricWrap::InvokeWithDimensions(const v8::FunctionCallbackInfo<v8::Value>& args, uint32_t index, Func&& func) {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    auto dimensionsArg = args[index];
    CHECK_ARG(isolate, dimensionsArg->IsArray() || dimensionsArg->IsUndefined(), "'dimensions' must be an array or undefined");

    auto wrap = NAPA_OBJECTWRAP::Unwrap<MetricWrap>(args.Holder());

    // Holds the dimensions strings on the stack for so it exists during the call to func.
    std::vector<napa::v8_helpers::Utf8String> dimensionsStringsHolder;

    std::vector<const char*> dimensions;
    if (dimensionsArg->IsArray() && wrap->_dimensions > 0) {
        auto arr = v8::Local<v8::Array>::Cast(dimensionsArg);
        JS_ENSURE(
            isolate,
            wrap->_dimensions == arr->Length(),
            "the dimensions count does not match. expected: %d, received: %d",
            wrap->_dimensions,
            arr->Length());

        dimensionsStringsHolder = napa::v8_helpers::V8ArrayToVector<napa::v8_helpers::Utf8String>(isolate, arr);

        dimensions.reserve(dimensionsStringsHolder.size());
        for (const auto& dimension : dimensionsStringsHolder) {
            dimensions.emplace_back(dimension.Data());
        }
    } else {
        JS_ENSURE(isolate, wrap->_dimensions == 0, "expected %s dimensions but received 0", wrap->_dimensions);
    }

    func(wrap->_metric, dimensions);
}